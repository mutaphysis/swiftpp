//
//  SwiftppObjcOutput.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/10.
//  Copyright (c) 2014年 dootaini. All rights reserved.
//

#include "SwiftppObjcOutput.h"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include "CodeTemplate.h"

#include <iostream>

namespace
{
	
	/*!
	 @brief Iterate over the parameters of a C++ method.
	 
	 Iterate over the parameters of a method, calling the i_formatFunc() for each
	 and building a comma separated string with all results.
	 @param i_method      the method
	 @param i_commaPrefix should it start with a comma
	 @param i_formatFunc  function to apply
	 */
	std::string write_cxx_params_decl( const CXXMethod &i_method, bool i_commaPrefix, const std::function<std::string (const CXXParam &)> i_formatFunc )
	{
		std::string result;
		result.reserve( i_method.params().size() * 6 );
		bool firstParam = true;
		for ( auto param : i_method.params() )
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
			result.append( i_formatFunc( param ) );
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
void SwiftppObjcOutput::write_objc_method_decl( llvm::raw_ostream &ostr, const CXXMethod &i_method ) const
{
	if ( i_method.isStatic() )
		ostr << "+ ";
	else
		ostr << "- ";
	
	if ( i_method.isConstructor() )
		ostr << "(instancetype)init";
	else
	{
		// (return type)methodName
		ostr << "(" << cxxType2ObjcTypeString( i_method.returnType() ) << ")" << i_method.name();
	}
	
	bool firstParam = true;
	for ( auto p : i_method.params() )
	{
		if ( firstParam )
		{
			if ( i_method.isConstructor() ) // constructor with a param will by name initWith:
				ostr << "With";
			firstParam = false;
		}
		else
		{
			ostr << " ";
			// if usedNamedParams, put a name for the param, like in [method:p1 nameOf2:p2 nameOf3:p3]
			if ( _data->options().usedNamedParams )
				ostr << p.cleanName();
		}
		// :(type)name
		ostr << ":(" << cxxType2ObjcTypeString( p.type() ) << ")" << p.name();
	}
}

#if 0
#pragma mark-
#endif

void SwiftppObjcOutput::write_cxx_objc_protocols_h( llvm::raw_ostream &ostr ) const
{
	const char tmpl[] = R"(
// generated cxx-objc-protocols.h
//  pure Objective-C, cannot contain any C++\n"

#ifndef H_CXX_OBJC_PROTOCOLS
#define H_CXX_OBJC_PROTOCOLS

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

// Objective-C proxy protocols for each classes

<{#classes}>
@protocol <{class_name}>_protocol
<{#methods}>
<{objc_method_decl}>;
<{/methods}>
@end
<{/classes}>
#endif
)";
	
	CodeTemplateModel model;
	auto data = _data;
	model.sections["classes"] = [=]( int i_index, CodeTemplateModel &o_model )
	{
		if ( i_index < data->classes().size() )
		{
			auto classPtr = &(data->classes()[i_index]);
			o_model.names["class_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << classPtr->name(); };
			o_model.sections["methods"] = [=]( int i_index, CodeTemplateModel &o_model )
			{
				if ( i_index < classPtr->methods().size() )
				{
					auto methodPtr = classPtr->methods().begin();
					std::advance( methodPtr, i_index );
					o_model.names["objc_method_decl"] = [=]( llvm::raw_ostream &ostr ){ this->write_objc_method_decl( ostr, *methodPtr ); };
					return true;
				}
				return false;
			};
			return true;
		}
		return false;
	};
	
	CodeTemplate renderer( std::begin(tmpl), std::end(tmpl) );
	renderer.render( model, ostr );
}

void SwiftppObjcOutput::write_cxx_objc_proxies_h( llvm::raw_ostream &ostr ) const
{
	const char tmpl[] = R"(
// generated cxx-objc-proxies.h
//  pure Objective-C, cannot contain any C++

#ifndef H_CXX_OBJC_PROXIES
#define H_CXX_OBJC_PROXIES

#import "cxx-objc-protocols.h"

// Objective-C proxies for each classes

<{#classes}>
@interface <{class_name}> : NSObject<<{class_name}>_protocol>
{ void *_ptr; }
@end
<{/classes}>
#endif
)";
	
	CodeTemplateModel model;
	auto data = _data;
	model.sections["classes"] = [=]( int i_index, CodeTemplateModel &o_model )
	{
		if ( i_index < data->classes().size() )
		{
			auto classPtr = &(data->classes()[i_index]);
			o_model.names["class_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << classPtr->name(); };
			return true;
		}
		return false;
	};

	CodeTemplate renderer( std::begin(tmpl), std::end(tmpl) );
	renderer.render( model, ostr );
}

void SwiftppObjcOutput::write_cxx_objc_proxies_mm( llvm::raw_ostream &ostr ) const
{
	const char tmpl[] = R"(
// generated cxx-objc-proxies.mm

#import "cxx-objc-proxies.h"
#include <string>
#include <vector>
#include <map>

<{#includes_for_cxx_types}>
#include "<{include_name}>"
<{/includes_for_cxx_types}>
namespace swift_converter
{
<{#converters}><{converter_decl}>;
<{/converters}>}

<{#classes}>
//********************************
// <{class_name}>
//********************************

// C-style

class <{class_name}>_subclass;
void <{class_name}>_subclass_delete( <{class_name}>_subclass *i_this );
<{#methods}>
<{c_proxy_method_decl}>;
<{/methods}>
// Objective-C proxy

#define _this ((<{class_name}>_subclass *)_ptr)

@implementation <{class_name}>

- (void)dealloc
{
  <{class_name}>_subclass_delete( _this );
#if !__has_feature(objc_arc)
  [super dealloc];
#endif
}
<{#methods}>
<{objc_method_impl}>

<{/methods}>
@end

#undef _this
<{/classes}>
)";
	
	CodeTemplateModel model;
	auto data = _data;
	model.sections["classes"] = [=]( int i_index, CodeTemplateModel &o_model )
	{
		if ( i_index < data->classes().size() )
		{
			auto classPtr = &(data->classes()[i_index]);
			o_model.names["class_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << classPtr->name(); };
			o_model.sections["methods"] = [=]( int i_index, CodeTemplateModel &o_model )
			{
				if ( i_index < classPtr->methods().size() )
				{
					auto methodPtr = classPtr->methods().begin();
					std::advance( methodPtr, i_index );
					o_model.names["c_proxy_method_decl"] = [=]( llvm::raw_ostream &ostr ){ this->write_c_proxy_method_decl( ostr, classPtr->name(), *methodPtr ); };
					o_model.names["objc_method_impl"] = [=]( llvm::raw_ostream &ostr ){ this->write_objc_method_impl( ostr, classPtr->name(), *methodPtr ); };
					return true;
				}
				return false;
			};
			return true;
			return true;
		}
		return false;
	};
	model.sections["includes_for_cxx_types"] = [=]( int i_index, CodeTemplateModel &o_model )
	{
		if ( i_index < data->includesForCXXTypes().size() )
		{
			auto includeName = data->formatIncludeName( data->includesForCXXTypes()[i_index] );
			o_model.names["include_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << includeName; };
			return true;
		}
		return false;
	};
	model.sections["converters"] = [=]( int i_index, CodeTemplateModel &o_model )
	{
		if ( i_index < data->converters().size() )
		{
			auto convPtr = &(data->converters()[i_index]);
			o_model.names["converter_decl"] = [=]( llvm::raw_ostream &ostr )
			{
				ostr << this->type2String(convPtr->to()) + " " + convPtr->name() + "( " + this->type2String(convPtr->from()) + " )";
			};
			return true;
		}
		return false;
	};

	CodeTemplate renderer( std::begin(tmpl), std::end(tmpl) );
	renderer.render( model, ostr );
}

void SwiftppObjcOutput::write_c_proxy_method_decl( llvm::raw_ostream &ostr, const std::string &i_className, const CXXMethod &i_method ) const
{
	if ( i_method.isConstructor() )
	{
		// a c proxy constructor has the form:
		//	className_subclass *className_subclass_new( id<className_protocol> i_link, args... );
		ostr << i_className << "_subclass *" << i_className << "_subclass_new( id<" << i_className << "_protocol> i_link";
	}
	else
	{
		// a c proxy method has the form:
		// class method (static):
		//	return_type className_subclass_method( args... );
		// memder method (normal)
		//	return_type className_subclass_method( className_subclass *i_this, args... );
		ostr << type2String( i_method.returnType() ) << " " << i_className << "_subclass_" << i_method.name() << "( ";
		if ( not i_method.isStatic() )
			ostr << i_className << "_subclass *i_this";
	}
	ostr << write_cxx_params_decl( i_method, not i_method.isStatic(), [&]( const CXXParam &p ){ return this->type2String( p.type() ) + " " + p.name(); } );
	ostr << " )";
}

void SwiftppObjcOutput::write_objc_method_impl( llvm::raw_ostream &ostr, const std::string &i_className, const CXXMethod &i_method ) const
{
	write_objc_method_decl( ostr, i_method );
	if ( i_method.isConstructor() )
	{
		ostr << "\n{\n  self = [super init];\n  if ( self )\n";
		ostr << "    _ptr = " << i_className << "_subclass_new( self";
		ostr << write_cxx_params_decl( i_method, true, [&]( const CXXParam &p ){ return this->converterForObjcType2CXXType( p.type(), p.name() ); } );
		ostr << " );\n  return self;\n}";
	}
	else
	{
		ostr << "\n{\n";
		std::string s;
		s.append( i_className );
		s.append( "_subclass_" );
		s.append( i_method.name() );
		s.append( "( " );
		if ( not i_method.isStatic() )
			s.append( "_this" );
		s.append( write_cxx_params_decl( i_method, not i_method.isStatic(), [&]( const CXXParam &p ){ return this->converterForObjcType2CXXType( p.type(), p.name() ); } ) );
		s.append( " )" );
		if ( not i_method.returnType()->isVoidType() )
		{
			ostr << "  return " << converterForCXXType2ObjcType( i_method.returnType(), s ) << ";\n";
		}
		else
		{
			ostr << "  " << s << ";\n";
		}
		ostr << "}";
	}
}

void SwiftppObjcOutput::write_cxx_subclasses_mm( llvm::raw_ostream &ostr ) const
{
	const char tmpl[] = R"(
// generated cxx-subclasses.mm

#import "cxx-objc-protocols.h"
#include "<{bridge_include}>"

template<typename T>
struct LinkSaver
{
  T saved, &link;
  LinkSaver( T &i_link ) : saved( i_link ), link( i_link ) { link = nil; }
  ~LinkSaver() { link = saved; }
};

// the wrapping sub-classes

<{#classes}>
class <{class_name}>_subclass : public <{class_name}>
{
  public:
    id<<{class_name}>_protocol> _link;

<{#methods}>
<{cpp_method_impl}>
<{/methods}>
};

<{/classes}>
// the c implementations

<{#classes}>
void <{class_name}>_subclass_delete( <{class_name}>_subclass *i_this )
{
  delete i_this;
}

<{#methods}>
<{c_proxy_method_impl}>

<{/methods}>
<{/classes}>
)";
	
	CodeTemplateModel model;
	auto data = _data;
	auto inputFile = _inputFile;
	model.names["bridge_include"] = [=]( llvm::raw_ostream &ostr ){ ostr << data->formatIncludeName( _inputFile ); };
	model.sections["classes"] = [=]( int i_index, CodeTemplateModel &o_model )
	{
		if ( i_index < data->classes().size() )
		{
			auto classPtr = &(data->classes()[i_index]);
			o_model.names["class_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << classPtr->name(); };
			o_model.sections["methods"] = [=]( int i_index, CodeTemplateModel &o_model )
			{
				if ( i_index < classPtr->methods().size() )
				{
					auto methodPtr = classPtr->methods().begin();
					std::advance( methodPtr, i_index );
					o_model.names["cpp_method_impl"] = [=]( llvm::raw_ostream &ostr ){ this->write_cpp_method_impl( ostr, classPtr->name(), *methodPtr ); };
					o_model.names["c_proxy_method_impl"] = [=]( llvm::raw_ostream &ostr ){ this->write_c_proxy_method_impl( ostr, classPtr->name(), *methodPtr ); };
					return true;
				}
				return false;
			};
			return true;
			return true;
		}
		return false;
	};

	CodeTemplate renderer( std::begin(tmpl), std::end(tmpl) );
	renderer.render( model, ostr );
}

void SwiftppObjcOutput::write_cpp_method_impl( llvm::raw_ostream &ostr, const std::string &i_className, const CXXMethod &i_method ) const
{
	if ( i_method.isConstructor() )
	{
		ostr << "    " << i_method.name() << "_subclass( id<" << i_className << "_protocol> i_link";
		ostr << write_cxx_params_decl( i_method, true, [&]( const CXXParam &p ){ return this->type2String( p.type() ) + " " + p.name(); } );
		ostr << " )\n      : " << i_className << "(";
		ostr << write_cxx_params_decl( i_method, false, [&]( const CXXParam &p ){ return p.name(); } );
		ostr << " ),\n      _link( i_link ){}";
	}
	else if ( i_method.isVirtual() )
	{
		ostr << "    virtual ";
		ostr << type2String( i_method.returnType() ) << " " << i_method.name() << "(";
		ostr << write_cxx_params_decl( i_method, false, [&]( const CXXParam &p ){ return this->type2String( p.type() ) + " " + p.name(); } );
		ostr << " )";
		if ( i_method.isConst() )
			ostr << " const";
		ostr << "\n    {\n";
		ostr << "      if ( _link == nil )\n";
		if ( i_method.isPureVirtual() )
		{
			ostr << "        abort(); // pure-virtual call!\n";
		}
		else
		{
			ostr << "        ";
			if ( not i_method.returnType()->isVoidType() )
				ostr << "return ";
			ostr << i_className << "::" << i_method.name() << "(";
			ostr << write_cxx_params_decl( i_method, false, [&]( const CXXParam &p ){ return p.name(); } );
			ostr << " );\n";
			ostr << "      else\n  ";
		}
		std::string s;
		s.append( "[_link " );
		s.append( i_method.name() );
		bool firstParam = true;
		for ( auto p : i_method.params() )
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
		if ( not i_method.returnType()->isVoidType() )
			ostr << "return " << converterForObjcType2CXXType( i_method.returnType(), s );
		else
			ostr << s;
		ostr << ";\n    }";
	}
}

void SwiftppObjcOutput::write_c_proxy_method_impl( llvm::raw_ostream &ostr, const std::string &i_className, const CXXMethod &i_method ) const
{
	if ( i_method.isConstructor() )
		ostr << i_className << "_subclass *" << i_className << "_subclass_new( id<" << i_className << "_protocol> i_link";
	else
	{
		ostr << type2String( i_method.returnType() ) << " " << i_className << "_subclass_" << i_method.name() << "( ";
		if ( not i_method.isStatic() )
			ostr << i_className << "_subclass *i_this";
	}
	ostr << write_cxx_params_decl( i_method, not i_method.isStatic(), [&]( const CXXParam &p ){ return this->type2String( p.type() ) + " " + p.name(); } );
	ostr << " )\n{\n";
	if ( i_method.isConstructor() )
	{
		ostr << "  return new " << i_className << "_subclass( i_link";
		ostr << write_cxx_params_decl( i_method, true, [&]( const CXXParam &p ){ return p.name(); } );
		ostr << " );\n";
	}
	else
	{
		if ( i_method.isVirtual() )
			ostr << "  LinkSaver<id<" << i_className << "_protocol>> s( i_this->_link );\n";
		ostr << "  ";
		if ( not i_method.returnType()->isVoidType() )
			ostr << "return ";
		if ( i_method.isStatic() )
			ostr << i_className << "::";
		else
			ostr << "i_this->";
		ostr << i_method.name() << "(";
		ostr << write_cxx_params_decl( i_method, false, [&]( const CXXParam &p ){ return p.name(); } );
		ostr << " );\n";
	}
	ostr << "}";
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

std::string SwiftppObjcOutput::typeNameForFunc( const clang::QualType &i_cxxtype ) const
{
	std::string result( type2UndecoratedTypeString( i_cxxtype ) );
	std::replace( std::begin(result), std::end(result), ' ', '_' );
	std::replace( std::begin(result), std::end(result), ':', '_' );
	std::replace( std::begin(result), std::end(result), '<', '_' );
	std::replace( std::begin(result), std::end(result), '>', '_' );
	return result;
}

std::string SwiftppObjcOutput::cxxType2ObjcTypeString( const clang::QualType &i_cxxtype ) const
{
	std::string cxxtype( type2UndecoratedTypeString( i_cxxtype ) );
	
	// is there a converter?
	for ( auto it : _data->converters() )
	{
		if ( cxxtype == type2UndecoratedTypeString( it.from() ) )
		{
			// converter found, use the converted type
			return type2UndecoratedTypeString( it.to() );
		}
	}
	
	// add a few default converters
	if ( cxxtype == "std::string" )
		return "NSString *";
	
	if ( isCXXVectorType( i_cxxtype ) )
		return "NSArray *";
	if ( isCXXMapType( i_cxxtype ) )
		return "NSDictionary *";
	if ( isCXXSetType( i_cxxtype ) )
		return "NSSet *";
	
	//! @todo: warn for unsupported types
	
	return cxxtype;
}

std::string SwiftppObjcOutput::converterForObjcType2CXXType( const clang::QualType &i_cxxtype, const std::string &i_code ) const
{
	std::string cxxtype( type2UndecoratedTypeString( i_cxxtype ) );
	
	// is there a converter?
	for ( auto converter : _data->converters() )
	{
		if ( cxxtype == type2UndecoratedTypeString( converter.to() ) )
		{
			// converter found, use the converted type
			return std::string("swift_converter::") + converter.name() + "( " + i_code + " )";
		}
	}
	
	// add a few default converters
	if ( cxxtype == "std::string" )
		return std::string("std::string( [") + i_code + " UTF8String] )";
	
	clang::QualType valueType;
	if ( isCXXVectorType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_to_vector_") + typeNameForFunc( valueType ) + "( " + i_code + " )";
	}
	if ( isCXXMapType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_to_map_") + typeNameForFunc( valueType ) + "( " + i_code + " )";
	}
	if ( isCXXSetType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_to_set_") + typeNameForFunc( valueType ) + "( " + i_code + " )";
	}
	
	return i_code;
}

/*!
 @brief find if a type is std::vector<T>.
 
 @param[in]  i_cxxtype   the type to check
 @param[out] o_valueType the type of T
 @return     true if i_cxxtype is std::vector<T>
 */
bool SwiftppObjcOutput::isCXXVectorType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType ) const
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
	
	// named 'vector' ?
	if ( templdecl->getNameAsString() != "vector" )
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
 @brief find if a type is std::map<std::string,T>.
 
 @param[in]  i_cxxtype   the type to check
 @param[out] o_valueType the type of T
 @return     true if i_cxxtype is std::map<std::string,T>
 */
bool SwiftppObjcOutput::isCXXMapType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType ) const
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
	
	// named 'map' ?
	if ( templdecl->getNameAsString() != "map" )
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
   @brief find if a type is std::set<T>.

   @param[in]  i_cxxtype   the type to check
   @param[out] o_valueType the type of T
   @return     true if i_cxxtype is std::set<T>
*/
bool SwiftppObjcOutput::isCXXSetType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType ) const
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
	
	// named 'set' ?
	if ( templdecl->getNameAsString() != "set" )
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

std::string SwiftppObjcOutput::converterForCXXType2ObjcType( const clang::QualType &i_cxxtype, const std::string &i_code ) const
{
	std::string cxxtype( type2UndecoratedTypeString( i_cxxtype ) );
	
	// is there a converter?
	for ( auto converter : _data->converters() )
	{
		if ( cxxtype == type2UndecoratedTypeString( converter.from() ) )
		{
			// converter found, use the converted type
			return std::string("swift_converter::") + converter.name() + "( " + i_code + " )";
		}
	}
	
	// add a few default converters
	if ( cxxtype == "std::string" )
		return std::string("[NSString stringWithUTF8String:") + i_code + ".c_str()]";
	
	clang::QualType valueType;
	if ( isCXXVectorType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_from_vector_") + typeNameForFunc( valueType ) + "( " + i_code + " )";
	}
	if ( isCXXMapType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_from_map_") + typeNameForFunc( valueType ) + "( " + i_code + " )";
	}
	if ( isCXXSetType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_from_set_") + typeNameForFunc( valueType ) + "( " + i_code + " )";
	}

	return i_code;
}
