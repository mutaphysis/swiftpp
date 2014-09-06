//
//  SwiftppASTConsumer.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/08/27.
//  Copyright (c) 2014年. All rights reserved.
//

#include "SwiftppASTConsumer.h"
#include <clang/AST/ASTContext.h>
#include <iostream>

SwiftppASTConsumer::SwiftppASTConsumer( clang::CompilerInstance &i_ci, const SwiftppOptions &i_options, llvm::StringRef i_inputFile )
	: _ci( i_ci ),
		_data( i_options ),
		_visitor( _data ),
		_inputFile( i_inputFile )
{
}

void SwiftppASTConsumer::HandleTranslationUnit( clang::ASTContext &i_ctx )
{
	_visitor.TraverseDecl( i_ctx.getTranslationUnitDecl() );
	
	if ( _ci.getDiagnostics().hasErrorOccurred() )
		return;

	auto collectInclude = [&]( clang::ASTContext &i_ctx, const clang::QualType &i_type )
		{
			auto decl = i_type->getAsCXXRecordDecl();
			if ( decl != nullptr )
			{
				auto loc = decl->clang::Decl::getLocStart();
			    clang::PresumedLoc ploc = i_ctx.getSourceManager().getPresumedLoc( loc );
			    if ( not ploc.isInvalid() )
				{
					this->_data.addCXXTypeIncludePath( ploc.getFilename() );
				}
			}
		};
	
	for ( auto converter : _data.converters() )
	{
		collectInclude( i_ctx, converter.to() );
		collectInclude( i_ctx, converter.from() );
	}
	
	auto outputFolder = _data.outputFolder();
	if ( outputFolder.empty() )
	{
		auto pos = _inputFile.rfind( '/' );
		if ( pos != std::string::npos )
			outputFolder = _inputFile.substr( 0, pos + 1 );
		
		outputFolder += "cxx-bridge/";
	}
	
	auto ostr = _ci.createOutputFile( outputFolder + "cxx-objc-protocols.h", false, true, "", "", true, true );
	if ( ostr )
		_data.write_cxx_objc_protocols_h( *ostr );
	
	ostr = _ci.createOutputFile( outputFolder + "cxx-objc-proxies.h", false, true, "", "", true, true );
	if ( ostr )
		_data.write_cxx_objc_proxies_h( *ostr );

	ostr = _ci.createOutputFile( outputFolder + "cxx-objc-proxies.mm", false, true, "", "", true, true );
	if ( ostr )
		_data.write_cxx_objc_proxies_mm( *ostr );

	ostr = _ci.createOutputFile( outputFolder + "cxx-subclasses.mm", false, true, "", "", true, true );
	if ( ostr )
		_data.write_cxx_subclasses_mm( *ostr, _inputFile );
}
