//
//  SwiftppASTVisitor.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/08/28.
//  Copyright (c) 2014年. All rights reserved.
//

#include "SwiftppASTVisitor.h"
#include "SwiftppData.h"
#include <iostream>

namespace
{

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
	if ( i_decl->getAccess() != clang::AS_public and i_decl->getAccess() != clang::AS_protected )
		return true;
	
	CXXMethod::type_t type = CXXMethod::type_t::kNormal;
	if ( i_decl->isStatic() )
		type = CXXMethod::type_t::kStatic;
	else if ( i_decl->isVirtual() )
		type = i_decl->isPure() ? CXXMethod::type_t::kPureVirtual : CXXMethod::type_t::kVirtual;
	CXXMethod::access_t access = CXXMethod::access_t::kPublic;
	if ( i_decl->getAccess() != clang::AS_public )
		access = CXXMethod::access_t::kProtected;
	bool isConst = i_decl->isConst();
	
	auto returnType = i_decl->getReturnType();
	
	std::string qualName( i_decl->getQualifiedNameAsString() );
	qualName = qualName.substr( _class.name().length() + 2 );
	
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
	
	//! @todo: collect methods on super classes as well
	
	return true;
}

}

SwiftppASTVisitor::SwiftppASTVisitor( SwiftppData &io_data )
	: _data( io_data )
{
}

bool SwiftppASTVisitor::VisitCXXRecordDecl( clang::CXXRecordDecl *i_decl )
{
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
	classVisitor.TraverseDecl( i_decl );
	
	_data.addClass( c );
	
	return true;
}

bool SwiftppASTVisitor::VisitFunctionDecl( clang::FunctionDecl *i_decl )
{
	if ( i_decl->getNumParams() != 1 )
		return true;
	
	auto returnType = i_decl->getReturnType();
	if ( returnType->isVoidType() )
		return true;
	
	std::string qualName( i_decl->getQualifiedNameAsString() );
	if ( qualName.compare( 0, 17, "swift_converter::" ) != 0 )
		return true;
	qualName = qualName.substr( 17 );
	
	auto param = i_decl->getParamDecl( 0 );
	
	TypeConverter conv( qualName, returnType, param->getType() );
	_data.addConverter( conv );
	
	return true;
}