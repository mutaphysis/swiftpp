//
//  SwiftppData.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/08/28.
//  Copyright (c) 2014年. All rights reserved.
//

#include "SwiftppData.h"
#include <iostream>

TypeConverter::TypeConverter( const std::string &i_name, const clang::QualType &i_to, const clang::QualType &i_from )
	: _name( i_name ),
		_to( i_to ),
		_from( i_from )
{
}

#if 0
#pragma mark-
#endif

CXXParam::CXXParam( const clang::QualType &i_type, const std::string &i_name )
	: _type( i_type ),
		_name( i_name )
{
}

std::string CXXParam::cleanName() const
{
	if ( _name.empty() )
		return "_";
	
	// filter out:
	//	i_ in_ o_ out_ io_ inOut_
	//	inX outX ioX inOutX
	if ( _name.compare( 0, 2, "i_" ) == 0 )
		return _name.substr( 2 );
	if ( _name.compare( 0, 3, "in_" ) == 0 )
		return _name.substr( 3 );
	if ( _name.compare( 0, 2, "o_" ) == 0 )
		return _name.substr( 2 );
	if ( _name.compare( 0, 4, "out_" ) == 0 )
		return _name.substr( 4 );
	if ( _name.compare( 0, 6, "inOut_" ) == 0 )
		return _name.substr( 6 );
	if ( _name.compare( 0, 2, "in" ) == 0 and isupper( _name[2] ) )
		return _name.substr( 2 );
	if ( _name.compare( 0, 3, "out" ) == 0 and isupper( _name[3] ) )
		return _name.substr( 3 );
	if ( _name.compare( 0, 2, "io" ) == 0 and isupper( _name[2] ) )
		return _name.substr( 2 );
	if ( _name.compare( 0, 5, "inOut" ) == 0 and isupper( _name[5] ) )
		return _name.substr( 5 );
	
	return _name;
}

bool CXXParam::operator<( const CXXParam &i_other ) const
{
	return std::make_tuple(_name,clang::QualType::getAsString(_type.split()))
			< std::make_tuple(i_other._name,clang::QualType::getAsString(i_other._type.split()));
}

#if 0
#pragma mark-
#endif

CXXMethod::CXXMethod( type_t i_type, access_t i_access, bool i_isConst,
				 const std::string &i_name,
				 const clang::QualType &i_returnType )
	: _isConst( i_isConst ),
		_type( i_type ),
 		_access( i_access ),
 		_name( i_name ),
		_returnType( i_returnType )
{
}

void CXXMethod::addParam( const CXXParam &i_param )
{
	_params.push_back( i_param );
}

bool CXXMethod::operator<( const CXXMethod &i_other ) const
{
	return std::tie(_isConst,_name,_params) < std::tie(i_other._isConst,i_other._name,i_other._params);
}

#if 0
#pragma mark-
#endif

CXXClass::CXXClass( const std::string &i_name )
	: _name( i_name )
{
}

void CXXClass::addMethod( const CXXMethod &i_method )
{
	if ( i_method.name()[0] == '~' )
		return; // ignore destructor, they're called automatically
	
	auto res = _methods.insert( i_method );
	
	// mark constructor
	if ( res.first->name() == name() )
		res.first->setIsConstructor();
}

#if 0
#pragma mark-
#endif

SwiftppData::SwiftppData( const SwiftppOptions &i_options )
	: _options( i_options )
{
}

void SwiftppData::addClass( const CXXClass &i_class )
{
	_classes.push_back( i_class );
}

void SwiftppData::addConverter( const TypeConverter &i_converter )
{
	_converters.push_back( i_converter );
}

void SwiftppData::addCXXTypeIncludePath( const std::string &i_fn )
{
	_includesForCXXTypes.push_back( i_fn );
}

std::string SwiftppData::formatIncludeFileName( const std::string &i_filepath ) const
{
	if ( not _options.usedFullPathForUserIncludes )
	{
		auto pos = i_filepath.rfind( '/' );
		if ( pos != std::string::npos )
			return i_filepath.substr( pos + 1 );
	}
	return i_filepath;
}

std::string SwiftppData::type2String( const clang::QualType &i_type ) const
{
	return clang::QualType::getAsString(i_type.split());
}

std::string SwiftppData::type2UndecoratedTypeString( const clang::QualType &i_type ) const
{
	clang::QualType type( i_type.getNonReferenceType() );
	type.removeLocalConst();
	return type2String( type );
}

std::string SwiftppData::cxxType2ObjcTypeString( const clang::QualType &i_type ) const
{
	std::string s( type2UndecoratedTypeString( i_type ) );
	
	// is there a converter?
	for ( auto it : _converters )
	{
		if ( s == type2UndecoratedTypeString( it.from() ) )
		{
			// converter found, use the converted type
			return type2UndecoratedTypeString( it.to() );
		}
	}
	
	// add a few default converters
	if ( s == "std::string" )
		return "NSString *";
	
	//! @todo: handle collections, vector<T>, map<string,T>, set<T>
	
	//! @todo: warn for unsupported types
	
	return s;
}

std::string SwiftppData::converterForObjcType2CXXType( const clang::QualType &i_type, const std::string &i_code ) const
{
	std::string s( type2UndecoratedTypeString( i_type ) );

	// is there a converter?
	for ( auto converter : _converters )
	{
		if ( s == type2UndecoratedTypeString( converter.to() ) )
		{
			// converter found, use the converted type
			return std::string("swift_converter::") + converter.name() + "( " + i_code + " )";
		}
	}
	
	// add a few default converters
	if ( s == "std::string" )
		return std::string("std::string( [") + i_code + " UTF8String] )";
	
	return i_code;
}

std::string SwiftppData::converterForCXXType2ObjcType( const clang::QualType &i_type, const std::string &i_code ) const
{
	std::string s( type2UndecoratedTypeString( i_type ) );
	
	// is there a converter?
	for ( auto converter : _converters )
	{
		if ( s == type2UndecoratedTypeString( converter.from() ) )
		{
			// converter found, use the converted type
			return std::string("swift_converter::") + converter.name() + "( " + i_code + " )";
		}
	}
	
	// add a few default converters
	if ( s == "std::string" )
		return std::string("[NSString stringWithUTF8String:") + i_code + ".c_str()]";
	
	return i_code;
}
