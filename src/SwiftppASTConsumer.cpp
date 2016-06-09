//
//  SwiftppASTConsumer.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/08/27.
//  Copyright (c) 2014年. All rights reserved.
//

#include "SwiftppASTConsumer.h"
#include "SwiftppASTVisitor.h"
#include "SwiftppOutput.h"
#include <clang/AST/ASTContext.h>

SwiftppASTConsumer::SwiftppASTConsumer( clang::CompilerInstance &i_ci,
										const SwiftppOptions &i_options,
										const std::shared_ptr<SwiftppOutput> &i_output,
										llvm::StringRef i_inputFile )
	: _ci( i_ci ),
		_data( i_options ),
		_output( i_output ),
		_inputFile( i_inputFile )
{
}

void SwiftppASTConsumer::HandleTranslationUnit( clang::ASTContext &i_ctx )
{
	SwiftppASTVisitor visitor( _data );
	
	visitor.TraverseDecl( i_ctx.getTranslationUnitDecl() );
	
	if ( _ci.getDiagnostics().hasErrorOccurred() )
		return;

	// find all C++ #include needed for the converted C++ types
	auto data = &_data;
	auto collectInclude = [data]( clang::ASTContext &i_ctx, const clang::QualType &i_type )
		{
			auto decl = i_type->getAsCXXRecordDecl();
			if ( decl != nullptr )
			{
				auto loc = decl->clang::Decl::getLocStart();
			    clang::PresumedLoc ploc = i_ctx.getSourceManager().getPresumedLoc( loc );
			    if ( not ploc.isInvalid() )
				{
					data->addCXXTypeIncludePath( ploc.getFilename() );
				}
			}
		};
	
	for ( auto converter : _data.converters() )
	{
		if ( not converter.to().getCanonicalType()->isObjCObjectPointerType() )
			collectInclude( i_ctx, converter.to() );
		if ( not converter.from().getCanonicalType()->isObjCObjectPointerType() )
			collectInclude( i_ctx, converter.from() );
	}
	
	_data.addMissingConstructors();
	
	// find all enums used
	std::set<const clang::EnumDecl *> enumTypes;
	for ( const auto &c : _data.classes() )
	{
		for ( const auto m : c.methods() )
		{
			if ( m->returnType().getTypePtrOrNull() == nullptr )
				continue;

			auto td = m->returnType()->getAsTagDecl();
			if ( td == nullptr )
				continue;
			auto e = clang::dyn_cast<clang::EnumDecl>( td );
			if ( e != nullptr )
				enumTypes.insert( e->getDefinition() );
			for ( const auto &p : m->params() )
			{
				if ( p.type().getTypePtrOrNull() == nullptr )
					continue;

				auto td = p.type()->getAsTagDecl();
				if ( td == nullptr )
					continue;
				auto e = clang::dyn_cast<clang::EnumDecl>( td );
				if ( e != nullptr )
					enumTypes.insert( e->getDefinition() );
			}
		}
	}
	for ( const auto &t : enumTypes )
	{
		auto s = t->getIntegerType()->isSignedIntegerType();
		CXXEnum cxxEnum( t->getNameAsString(), s );
		for ( auto v = t->enumerator_begin(); v != t->enumerator_end(); ++v )
		{
			cxxEnum.addValue( v->getNameAsString(), v->getInitVal().getSExtValue() );
		}
		_data.addEnum( cxxEnum );
	}
	
	// synthesize missing converters
//	for ( const auto &c : _data.classes() )
//	{
//		for ( const auto &m : c.methods() )
//		{
//			addMissingConverters( m.returnType() );
//			for ( const auto &p : m.params() )
//				addMissingConverters( p.type() );
//		}
//	}
	
	// write bridge code!
	_output->write( _ci, _inputFile, _data );
}
