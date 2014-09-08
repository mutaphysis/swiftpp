//
//  SwiftppASTVisitor.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/08/28.
//  Copyright (c) 2014年. All rights reserved.
//

#include "SwiftppASTVisitor.h"
#include "SwiftppData.h"
#include <deque>

namespace
{

//! Handle individual classes
class SwiftppClassVisitor : public clang::RecursiveASTVisitor<SwiftppClassVisitor>
{
	public:
		SwiftppClassVisitor( CXXClass &io_class );
		bool VisitCXXMethodDecl( clang::CXXMethodDecl *i_decl );
	
	private:
		CXXClass &_class;
};

SwiftppClassVisitor::SwiftppClassVisitor( CXXClass &io_class )
	: _class( io_class )
{
}

bool SwiftppClassVisitor::VisitCXXMethodDecl( clang::CXXMethodDecl *i_decl )
{
	// just interrested in protected and public methods
	if ( i_decl->getAccess() != clang::AS_public and i_decl->getAccess() != clang::AS_protected )
		return true;
	
	// type of method
	CXXMethod::type_t type = CXXMethod::type_t::kNormal;
	if ( i_decl->isStatic() )
		type = CXXMethod::type_t::kStatic;
	else if ( i_decl->isVirtual() )
		type = i_decl->isPure() ? CXXMethod::type_t::kPureVirtual : CXXMethod::type_t::kVirtual;
	
	// access
	CXXMethod::access_t access = CXXMethod::access_t::kPublic;
	if ( i_decl->getAccess() != clang::AS_public )
		access = CXXMethod::access_t::kProtected;
	
	bool isConst = i_decl->isConst();
	
	auto returnType = i_decl->getReturnType();
	
	// the name has the form classname::methodname, remove 'classname::'
	std::string qualName( i_decl->getQualifiedNameAsString() );
	qualName = qualName.substr( _class.name().length() + 2 );
	
	// the clones trick will handle methods with default parameters as being multiple
	// different methods.
	unsigned numParams = i_decl->getNumParams();
	int clones = numParams - i_decl->getMinRequiredArguments();
	for ( int i = 0; i <= clones; ++i )
	{
		CXXMethod m( type,
						   access,
						   isConst,
						   qualName,
						   returnType );
	
		for ( unsigned p = 0; p < numParams; ++p )
		{
			auto param = i_decl->getParamDecl( p );
			m.addParam( CXXParam( param->getType(), param->getNameAsString() ) );
		}
		--numParams;
	
		_class.addMethod( m );
	}
	
	return true;
}

}

SwiftppASTVisitor::SwiftppASTVisitor( SwiftppData &io_data )
	: _data( io_data )
{
}

bool SwiftppASTVisitor::VisitCXXRecordDecl( clang::CXXRecordDecl *i_decl )
{
	// look if this class has the 'swift' annotation
	bool swiftAttrFound = false;
	for ( auto attr = i_decl->specific_attr_begin<clang::AnnotateAttr>(); attr != i_decl->specific_attr_end<clang::AnnotateAttr>(); ++attr )
	{
		if ( (*attr)->getAnnotation() == "swift" )
		{
			swiftAttrFound = true;
			break;
		}
	}
	if ( not swiftAttrFound )
		return true;
	
	std::string qualName( i_decl->getQualifiedNameAsString() );
	
	CXXClass c( qualName );
	SwiftppClassVisitor classVisitor( c );
	
	// visit class and recursively visit all the base classes
	std::set<clang::CXXRecordDecl *> alreadyVisited;
	std::deque<clang::CXXRecordDecl *> todo;
	todo.push_back( i_decl );
	while ( not todo.empty() )
	{
		auto decl = todo.back();
		todo.pop_back();
		if ( alreadyVisited.find( decl ) != alreadyVisited.end() )
			continue;
		
		classVisitor.TraverseDecl( decl );
		
		alreadyVisited.insert( decl );

		for ( auto base = decl->bases_begin(); base != decl->bases_end(); ++base )
		{
			// skip private bases
			if ( base->getAccessSpecifier() != clang::AS_private and base->getAccessSpecifier() != clang::AS_none )
			{
				auto t = base->getType().getTypePtrOrNull();
				if ( t != nullptr )
				{
					auto super = t->getAsCXXRecordDecl();
					if ( super != nullptr )
						todo.push_back( super );
				}
			}
		}
	}
	
	_data.addClass( c );
	
	return true;
}

bool SwiftppASTVisitor::VisitFunctionDecl( clang::FunctionDecl *i_decl )
{
	// catch swift_converters
	
	if ( i_decl->getNumParams() != 1 ) // must take exactly 1 parameter
		return true;
	
	auto returnType = i_decl->getReturnType();
	if ( returnType->isVoidType() ) // must return something
		return true;
	
	// must be in swift_converter namespace
	std::string qualName( i_decl->getQualifiedNameAsString() );
	if ( qualName.compare( 0, 17, "swift_converter::" ) != 0 )
		return true;
	qualName = qualName.substr( 17 );
	
	auto param = i_decl->getParamDecl( 0 );
	
	TypeConverter conv( qualName, returnType, param->getType() );
	_data.addConverter( conv );
	
	return true;
}