//
//  SwiftppObjcOutput.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/10.
//  Copyright (c) 2014年 dootaini. All rights reserved.
//

#include "SwiftppObjcOutput.h"
#include <clang/Frontend/CompilerInstance.h>

namespace
{
	
	/*!
	 @brief Iterate over the parameters of a C++ method.
	 
	 Iterate over the parameters of a method, calling the f() for each
	 and building a comma separated string with all results.
	 @param i_method      the method
	 @param i_commaPrefix should it start with a comma
	 @param f             function to apply
	 */
	std::string write_cxx_params_decl( const CXXMethod &i_method, bool i_commaPrefix, const std::function<std::string (const CXXParam &)> f )
	{
		std::string result;
		result.reserve( i_method.params().size() * 6 );
		bool firstParam = true;
		for ( auto p : i_method.params() )
		{
			if ( firstParam )
			{
				if ( i_commaPrefix )
					result.append( ", ", 2 );
				else
					result.append( 1, ' ' );
				firstParam = false;
			}
			else
				result.append( ", ", 2 );
			result.append( f( p ) );
		}
		return result;
	}
	
}

void SwiftppObjcOutput::write_impl()
{
	// select an output folder
	auto outputFolder = _data->outputFolder();
	if ( outputFolder.empty() )
	{
		outputFolder = _inputFile;
		auto pos = outputFolder.rfind( '.' );
		if ( pos != std::string::npos )
		{
			outputFolder = outputFolder.substr( 0, pos );
			outputFolder += "/";
		}
		else
			outputFolder += "-cxx-bridge/";
	}

	auto ostr = _ci->createOutputFile( outputFolder + "cxx-objc-protocols.h", false, true, "", "", true, true );
	if ( ostr )
		write_cxx_objc_protocols_h( *ostr );
	
	ostr = _ci->createOutputFile( outputFolder + "cxx-objc-proxies.h", false, true, "", "", true, true );
	if ( ostr )
		write_cxx_objc_proxies_h( *ostr );
	
	ostr = _ci->createOutputFile( outputFolder + "cxx-objc-proxies.mm", false, true, "", "", true, true );
	if ( ostr )
		write_cxx_objc_proxies_mm( *ostr );
	
	ostr = _ci->createOutputFile( outputFolder + "cxx-subclasses.mm", false, true, "", "", true, true );
	if ( ostr )
		write_cxx_subclasses_mm( *ostr );
}

// write an Objective-C method declaration
std::string SwiftppObjcOutput::write_objc_method_decl( const CXXMethod &i_method ) const
{
	std::string result;
	result.reserve( i_method.params().size() * 6 );
	if ( i_method.isStatic() )
		result.append( "+ ", 2 );
	else
		result.append( "- ", 2 );
	
	if ( i_method.isConstructor() )
		result.append( "(instancetype)init" );
	else
	{
		result.append( 1, '(' );
		result.append( cxxType2ObjcTypeString( i_method.returnType() ) );
		result.append( 1, ')' );
		result.append( i_method.name() );
	}
	
	bool firstParam = true;
	for ( auto p : i_method.params() )
	{
		if ( firstParam )
		{
			if ( i_method.isConstructor() )
				result.append( "With" );
			firstParam = false;
		}
		else
		{
			result.append( 1, ' ' );
			if ( _data->options().usedNamedParams )
				result.append( p.cleanName() );
		}
		result.append( ":(", 2 );
		result.append( cxxType2ObjcTypeString( p.type() ) );
		result.append( 1, ')' );
		result.append( p.name() );
	}
	return result;
}

#if 0
#pragma mark-
#endif

void SwiftppObjcOutput::write_cxx_objc_protocols_h( llvm::raw_ostream &ostr ) const
{
	ostr << "// generated cxx-objc-protocols.h\n"
	"//  pure Objective-C, cannot contain any C++\n"
	"\n"
	"#ifndef H_CXX_OBJC_PROTOCOLS\n"
	"#define H_CXX_OBJC_PROTOCOLS\n"
	"\n"
	"#import <Foundation/Foundation.h>\n"
	"#import <AppKit/AppKit.h>\n"
	"\n"
	"// Objective-C proxy protocols for each classes\n"
	"\n";
	
	for ( auto oneClass : _data->classes() )
	{
		ostr << "@protocol " << oneClass.name() << "_protocol\n";
		
		for ( auto method : oneClass.methods() )
		{
			ostr << write_objc_method_decl( method );
			ostr << ";\n";
		}
	}
	
	ostr << "@end\n\n#endif\n";
}

void SwiftppObjcOutput::write_cxx_objc_proxies_h( llvm::raw_ostream &ostr ) const
{
	ostr << "// generated cxx-objc-proxies.h\n"
	"//  pure Objective-C, cannot contain any C++\n"
	"\n"
	"#ifndef H_CXX_OBJC_PROXIES\n"
	"#define H_CXX_OBJC_PROXIES\n"
	"\n"
	"#import \"cxx-objc-protocols.h\"\n"
	"\n"
	"// Objective-C proxies for each classes\n"
	"\n";
	
	for ( auto oneClass : _data->classes() )
	{
		ostr << "@interface " << oneClass.name() <<  " : NSObject<" << oneClass.name() << "_protocol>\n"
		"{ void *_ptr; }\n"
		"@end\n"
		"\n";
	}
	ostr << "#endif\n";
}

void SwiftppObjcOutput::write_cxx_objc_proxies_mm( llvm::raw_ostream &ostr ) const
{
	ostr << "// generated cxx-objc-proxies.mm\n"
	"\n"
	"#import \"cxx-objc-proxies.h\"\n"
	"\n";
	
	ostr << "#include <string>\n";
	ostr << "#include <vector>\n";
	ostr << "#include <map>\n";
	// type includes, all c++ types used in converters
	for ( auto fi : _data->includesForCXXTypes() )
		ostr << "#include \"" << _data->formatIncludeFileName( fi ) << "\"\n";
	
	ostr << "\nnamespace swift_converter\n"
	"{\n";
	
	for ( auto conv : _data->converters() )
	{
		ostr << type2String(conv.to()) << " " << conv.name() << "( " << type2String(conv.from()) << " );\n";
	}
	ostr << "}\n\n";
	
	for ( auto oneClass : _data->classes() )
	{
		ostr << "//********************************\n"
		"// " << oneClass.name() << "\n"
		"//********************************\n"
		"\n"
		"// C-style\n"
		"\n";
		
		ostr << "class " << oneClass.name() << "_subclass;\n";
		ostr << "void " << oneClass.name() << "_subclass_delete( " << oneClass.name() << "_subclass *i_this );\n";
		
		for ( auto method : oneClass.methods() )
		{
			if ( method.isConstructor() )
				ostr << oneClass.name() << "_subclass *" << oneClass.name() << "_subclass_new( id<" << oneClass.name() << "_protocol> i_link";
			else
				ostr << type2String( method.returnType() ) << " " << oneClass.name() << "_subclass_" << method.name() << "( "
				<< oneClass.name() << "_subclass *i_this";
			ostr << write_cxx_params_decl( method, true, [&]( const CXXParam &p ){ return this->type2String( p.type() ) + " " + p.name(); } );
			ostr << " );\n";
		}
		ostr << "\n"
		"// Objective-C proxy\n"
		"\n";
		
		ostr << "#define _this ((" << oneClass.name() << "_subclass *)_ptr)\n\n";
		
		ostr << "@implementation " << oneClass.name() << "\n\n";
		
		ostr << "- (void)dealloc\n{\n";
		ostr << "  " << oneClass.name() << "_subclass_delete( _this );\n";
		ostr << "#if !__has_feature(objc_arc)\n"
		"  [super dealloc];\n"
		"#endif\n}\n";
		
		for ( auto method : oneClass.methods() )
		{
			ostr << write_objc_method_decl( method );
			if ( method.isConstructor() )
			{
				ostr << "\n{\n  self = [super init];\n  if ( self )\n";
				ostr << "    _ptr = " << oneClass.name() << "_subclass_new( self";
				ostr << write_cxx_params_decl( method, true, [&]( const CXXParam &p ){ return this->converterForObjcType2CXXType( p.type(), p.name() ); } );
				ostr << " );\n  return self;\n}\n\n";
			}
			else
			{
				ostr << "\n{\n";
				std::string s;
				s.append( oneClass.name() );
				s.append( "_subclass_" );
				s.append( method.name() );
				s.append( "( _this" );
				s.append( write_cxx_params_decl( method, true, [&]( const CXXParam &p ){ return this->converterForObjcType2CXXType( p.type(), p.name() ); } ) );
				s.append( " )" );
				if ( not method.returnType()->isVoidType() )
				{
					ostr << "  return " << converterForCXXType2ObjcType( method.returnType(), s ) << ";\n";
				}
				else
				{
					ostr << "  " << s << ";\n";
				}
				ostr << "}\n\n";
			}
		}
		
		ostr << "@end\n\n#undef _this\n\n";
	}
}

void SwiftppObjcOutput::write_cxx_subclasses_mm( llvm::raw_ostream &ostr ) const
{
	ostr << "// generated cxx-subclasses.mm\n"
	"\n"
	"#import \"cxx-objc-protocols.h\"\n";
	
	ostr << "#include \"" << _data->formatIncludeFileName( _inputFile ) << "\"\n";
	
	ostr << "\ntemplate<typename T>\n"
	"struct LinkSaver\n"
	"{\n"
	"  T saved, &link;\n"
	"  LinkSaver( T &i_link ) : saved( i_link ), link( i_link ) { link = nil; }\n"
	"  ~LinkSaver() { link = saved; }\n"
	"};\n\n"
	"// the wrapping sub-classes\n\n";
	
	for ( auto oneClass : _data->classes() )
	{
		ostr << "class " << oneClass.name() << "_subclass : public " << oneClass.name() << "\n";
		ostr << "{\n"
		"  public:\n";
		ostr << "    id<" << oneClass.name() << "_protocol> _link;\n";
		
		for ( auto method : oneClass.methods() )
		{
			if ( method.isConstructor() )
			{
				ostr << "    " << method.name() << "_subclass( id<" << oneClass.name() << "_protocol> i_link";
				ostr << write_cxx_params_decl( method, true, [&]( const CXXParam &p ){ return this->type2String( p.type() ) + " " + p.name(); } );
				ostr << " )\n      : " << oneClass.name() << "(";
				ostr << write_cxx_params_decl( method, false, [&]( const CXXParam &p ){ return p.name(); } );
				ostr << " ),\n      _link( i_link ){}\n";
			}
			else if ( method.isVirtual() )
			{
				ostr << "    virtual ";
				ostr << type2String( method.returnType() ) << " " << method.name() << "(";
				ostr << write_cxx_params_decl( method, false, [&]( const CXXParam &p ){ return this->type2String( p.type() ) + " " + p.name(); } );
				ostr << " )";
				if ( method.isConst() )
					ostr << " const";
				ostr << "\n    {\n";
				ostr << "      if ( _link == nil )\n";
				if ( method.isPureVirtual() )
				{
					ostr << "        abort(); // pure-virtual call!\n";
				}
				else
				{
					ostr << "        " << oneClass.name() << "::" << method.name() << "(";
					ostr << write_cxx_params_decl( method, false, [&]( const CXXParam &p ){ return p.name(); } );
					ostr << " );\n";
					ostr << "      else\n  ";
				}
				std::string s;
				s.append( "[_link " );
				s.append( method.name() );
				bool firstParam = true;
				for ( auto p : method.params() )
				{
					if ( firstParam )
						firstParam = false;
					else
					{
						s.append( 1, ' ' );
						if ( _data->options().usedNamedParams )
							s.append( p.cleanName() );
					}
					s.append( 1, ':' );
					s.append( converterForCXXType2ObjcType( p.type(), p.name() ) );
				}
				s.append( 1, ']' );
				
				ostr << "      ";
				if ( not method.returnType()->isVoidType() )
					ostr << "return " << converterForObjcType2CXXType( method.returnType(), s );
				else
					ostr << s;
				ostr << ";\n    }\n";
			}
		}
		ostr << "};\n";
	}
	
	ostr << "\n"
	"// the c implementations\n"
	"\n";
	
	for ( auto oneClass : _data->classes() )
	{
		ostr << "void " << oneClass.name() << "_subclass_delete( " << oneClass.name() << "_subclass *i_this )\n{\n";
		ostr << "  delete i_this;\n}\n\n";
		
		for ( auto method : oneClass.methods() )
		{
			if ( method.isConstructor() )
				ostr << oneClass.name() << "_subclass *" << oneClass.name() << "_subclass_new( id<" << oneClass.name() << "_protocol> i_link";
			else
				ostr << type2String( method.returnType() ) << " " << oneClass.name() << "_subclass_" << method.name() << "( "
				<< oneClass.name() << "_subclass *i_this";
			ostr << write_cxx_params_decl( method, true, [&]( const CXXParam &p ){ return this->type2String( p.type() ) + " " + p.name(); } );
			ostr << " )\n{\n";
			if ( method.isConstructor() )
			{
				ostr << "  return new " << oneClass.name() << "_subclass( i_link";
				ostr << write_cxx_params_decl( method, true, [&]( const CXXParam &p ){ return p.name(); } );
				ostr << " );\n";
			}
			else
			{
				if ( method.isVirtual() )
					ostr << "  LinkSaver<id<" << oneClass.name() << "_protocol>> s( i_this->_link );\n";
				ostr << "  ";
				if ( not method.returnType()->isVoidType() )
					ostr << "return ";
				ostr << "i_this->" << method.name() << "(";
				ostr << write_cxx_params_decl( method, false, [&]( const CXXParam &p ){ return p.name(); } );
				ostr << " );\n";
			}
			ostr << "}\n\n";
		}
		ostr << "\n";
	}
}

std::string SwiftppObjcOutput::type2String( const clang::QualType &i_type ) const
{
	return clang::QualType::getAsString(i_type.split());
}

std::string SwiftppObjcOutput::type2UndecoratedTypeString( const clang::QualType &i_type ) const
{
	clang::QualType type( i_type.getNonReferenceType() );
	type.removeLocalConst();
	return type2String( type );
}

std::string SwiftppObjcOutput::cxxType2ObjcTypeString( const clang::QualType &i_type ) const
{
	std::string s( type2UndecoratedTypeString( i_type ) );
	
	// is there a converter?
	for ( auto it : _data->converters() )
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

std::string SwiftppObjcOutput::converterForObjcType2CXXType( const clang::QualType &i_type, const std::string &i_code ) const
{
	std::string s( type2UndecoratedTypeString( i_type ) );
	
	// is there a converter?
	for ( auto converter : _data->converters() )
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

std::string SwiftppObjcOutput::converterForCXXType2ObjcType( const clang::QualType &i_type, const std::string &i_code ) const
{
	std::string s( type2UndecoratedTypeString( i_type ) );
	
	// is there a converter?
	for ( auto converter : _data->converters() )
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