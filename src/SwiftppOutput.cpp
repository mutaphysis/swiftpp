//
//  SwiftppOutput.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/10.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

#include "SwiftppOutput.h"
#include <clang/AST/DeclTemplate.h>

SwiftppOutput::~SwiftppOutput()
{
}

void SwiftppOutput::write( clang::CompilerInstance &i_ci, const std::string &i_inputFile, const SwiftppData &i_data )
{
	_ci = &i_ci;
	_inputFile = i_inputFile;
	_data = &i_data;
	try
	{
		write_impl();
	}
	catch ( ... )
	{
		_ci = nullptr;
		_inputFile.clear();
		_data = nullptr;
		throw;
	}
	_ci = nullptr;
	_inputFile.clear();
	_data = nullptr;
}

std::string SwiftppOutput::type2String( const clang::QualType &i_type ) const
{
	auto v = clang::QualType::getAsString(i_type.split());
	if ( v == "_Bool" )
		return "bool";
	return v;
}

std::string SwiftppOutput::type2UndecoratedTypeString( const clang::QualType &i_type ) const
{
	clang::QualType type( i_type.getNonReferenceType() );
	type.removeLocalConst();
	return type2String( type );
}

std::string SwiftppOutput::typeNameForFunc( const clang::QualType &i_cxxtype ) const
{
	std::string result( type2UndecoratedTypeString( i_cxxtype ) );
	std::replace( std::begin(result), std::end(result), ' ', '_' );
	std::replace( std::begin(result), std::end(result), ':', '_' );
	std::replace( std::begin(result), std::end(result), '<', '_' );
	std::replace( std::begin(result), std::end(result), '>', '_' );
	return result;
}

bool SwiftppOutput::isCXXContainerType( const clang::QualType &i_cxxtype, const std::string &i_typeName, clang::QualType *o_valueType ) const
{
	auto type = i_cxxtype.getCanonicalType().getNonReferenceType().getTypePtrOrNull();
	if ( type == nullptr )
		return false;
	
	// is it a C++ class in 'std' namespace?
	auto cxxdecl = type->getAsCXXRecordDecl();
	if ( cxxdecl == nullptr or not cxxdecl->getDeclContext()->isStdNamespace() )
		return false;
	
	// is it a template specialisation?
	auto templdecl = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>( cxxdecl );
	if ( templdecl == nullptr )
		return false;
	
	// named i_typeName ?
	if ( templdecl->getNameAsString() != i_typeName )
		return false;
	
	// with at least one template argument...
	const auto &targs = templdecl->getTemplateArgs();
	if ( targs.size() < 1 )
		return false;
	
	// ... that is a type
	const auto &arg = targs[0];
	if ( arg.getKind() != clang::TemplateArgument::Type )
		return false;
	
	if ( o_valueType )
		*o_valueType = arg.getAsType();
	
	return true;
}

/*!
 @brief find if a type is std::vector<T>.
 
 @param[in]  i_cxxtype   the type to check
 @param[out] o_valueType the type of T
 @return     true if i_cxxtype is std::vector<T>
 */
bool SwiftppOutput::isCXXVectorType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType ) const
{
	return isCXXContainerType( i_cxxtype, "vector", o_valueType );
}

/*!
 @brief find if a type is std::list<T>.
 
 @param[in]  i_cxxtype   the type to check
 @param[out] o_valueType the type of T
 @return     true if i_cxxtype is std::list<T>
 */
bool SwiftppOutput::isCXXListType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType ) const
{
	return isCXXContainerType( i_cxxtype, "list", o_valueType );
}

bool SwiftppOutput::isCXXAssociativeContainerType( const clang::QualType &i_cxxtype, const std::string &i_typeName, clang::QualType *o_valueType ) const
{
	auto type = i_cxxtype.getCanonicalType().getNonReferenceType().getTypePtrOrNull();
	if ( type == nullptr )
		return false;
	
	// is it a C++ class in 'std' namespace?
	auto cxxdecl = type->getAsCXXRecordDecl();
	if ( cxxdecl == nullptr or not cxxdecl->getDeclContext()->isStdNamespace() )
		return false;
	
	// is it a template specialisation?
	auto templdecl = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>( cxxdecl );
	if ( templdecl == nullptr )
		return false;
	
	// named i_typeName ?
	if ( templdecl->getNameAsString() != i_typeName )
		return false;
	
	// with at least 2 template arguments...
	const auto &targs = templdecl->getTemplateArgs();
	if ( targs.size() < 2 )
		return false;
	
	// ... that are types
	const auto &arg1 = targs[0];
	const auto &arg2 = targs[1];
	if ( arg1.getKind() != clang::TemplateArgument::Type or arg2.getKind() != clang::TemplateArgument::Type )
		return false;
	
	// the first one should be 'std::string'
	if ( type2UndecoratedTypeString( arg1.getAsType() ) != "std::string" )
		return false;
	
	if ( o_valueType )
		*o_valueType = arg2.getAsType();
	
	return true;
}

/*!
 @brief find if a type is std::map<std::string,T>.
 
 @param[in]  i_cxxtype   the type to check
 @param[out] o_valueType the type of T
 @return     true if i_cxxtype is std::map<std::string,T>
 */
bool SwiftppOutput::isCXXMapType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType ) const
{
	return isCXXAssociativeContainerType( i_cxxtype, "map", o_valueType );
}

/*!
 @brief find if a type is std::unordered_map<std::string,T>.
 
 @param[in]  i_cxxtype   the type to check
 @param[out] o_valueType the type of T
 @return     true if i_cxxtype is std::unordered_map<std::string,T>
 */
bool SwiftppOutput::isCXXUnorderedMapType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType ) const
{
	return isCXXAssociativeContainerType( i_cxxtype, "unordered_map", o_valueType );
}

/*!
 @brief find if a type is std::set<T>.
 
 @param[in]  i_cxxtype   the type to check
 @param[out] o_valueType the type of T
 @return     true if i_cxxtype is std::set<T>
 */
bool SwiftppOutput::isCXXSetType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType ) const
{
	return isCXXContainerType( i_cxxtype, "set", o_valueType );
}

