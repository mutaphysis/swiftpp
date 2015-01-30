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
	result.reserve( i_method.params().size() * 6 ); // make some room
	if ( i_method.isStatic() )
		result.append( "+ ", 2 );
	else
		result.append( "- ", 2 );
	
	if ( i_method.isConstructor() )
		result.append( "(instancetype)init" );
	else
	{
		// (return type)methodName
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
			if ( i_method.isConstructor() ) // constructor with a param will by name initWith:
				result.append( "With" );
			firstParam = false;
		}
		else
		{
			result.append( 1, ' ' );
			// if usedNamedParams, put a name for the param, like in [method:p1 nameOf2:p2 nameOf3:p3]
			if ( _data->options().usedNamedParams )
				result.append( p.cleanName() );
		}
		// :(type)name
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
<{#methods}><{objc_method_decl}>;
<{/methods}>@end
<{/classes}>
#endif
)";
	
	CodeTemplateModel model;
	auto data = _data;
	model.resolveSection = [=]( const std::string &i_name, int i_index, CodeTemplateModel &o_model )
	{
		if ( i_name == "classes" and i_index < data->classes().size() )
		{
			auto classPtr = &(data->classes()[i_index]);
			o_model.resolveName = [=]( const std::string &i_name, std::string &o_replc )
			{
				if ( i_name == "class_name" )
				{
					o_replc = classPtr->name();
					return true;
				}
				return false;
			};
			o_model.resolveSection = [=]( const std::string &i_name, int i_index, CodeTemplateModel &o_model )
			{
				if ( i_name == "methods" and i_index < classPtr->methods().size() )
				{
					auto methodPtr = classPtr->methods().begin();
					std::advance( methodPtr, i_index );
					o_model.resolveName = [=]( const std::string &i_name, std::string &o_replc )
					{
						if ( i_name == "objc_method_decl" )
						{
							o_replc = this->write_objc_method_decl( *methodPtr );
							return true;
						}
						return false;
					};
					return true;
				}
				return false;
			};
			return true;
		}
		return false;
	};
	
	CodeTemplate renderer( substringref( std::begin(tmpl) + 1, std::end(tmpl) -1 ) );
	renderer.render( model,
		[&]( const char *i_ptr, size_t i_len )
		{
			ostr.write( i_ptr, i_len );
		} );
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
	model.resolveSection = [=]( const std::string &i_name, int i_index, CodeTemplateModel &o_model )
	{
		if ( i_name == "classes" and i_index < data->classes().size() )
		{
			auto classPtr = &(data->classes()[i_index]);
			o_model.resolveName = [=]( const std::string &i_name, std::string &o_replc )
			{
				if ( i_name == "class_name" )
				{
					o_replc = classPtr->name();
					return true;
				}
				return false;
			};
			return true;
		}
		return false;
	};

	CodeTemplate renderer( substringref( std::begin(tmpl) + 1, std::end(tmpl) -1 ) );
	renderer.render( model,
					[&]( const char *i_ptr, size_t i_len )
					{
						ostr.write( i_ptr, i_len );
					} );
}

void SwiftppObjcOutput::write_cxx_objc_proxies_mm( llvm::raw_ostream &ostr ) const
{
	ostr << R"(// generated cxx-objc-proxies.mm

#import "cxx-objc-proxies.h"
#include <string>
#include <vector>
#include <map>

)";
	
	// type #includes, all c++ types used in converters
	for ( auto fi : _data->includesForCXXTypes() )
		ostr << "#include \"" << _data->formatIncludeFileName( fi ) << "\"\n";
	
	// wtite the converters prototype
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
			{
				ostr << type2String( method.returnType() ) << " " << oneClass.name() << "_subclass_" << method.name() << "( ";
				if ( not method.isStatic() )
					ostr << oneClass.name() << "_subclass *i_this";
			}
			ostr << write_cxx_params_decl( method, not method.isStatic(), [&]( const CXXParam &p ){ return this->type2String( p.type() ) + " " + p.name(); } );
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
				s.append( "( " );
				if ( not method.isStatic() )
					s.append( "_this" );
				s.append( write_cxx_params_decl( method, not method.isStatic(), [&]( const CXXParam &p ){ return this->converterForObjcType2CXXType( p.type(), p.name() ); } ) );
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
	ostr << R"(// generated cxx-subclasses.mm

#import "cxx-objc-protocols.h"
)";
	
	ostr << "#include \"" << _data->formatIncludeFileName( _inputFile ) << "\"\n";
	
	ostr << R"(
template<typename T>
struct LinkSaver
{
  T saved, &link;
  LinkSaver( T &i_link ) : saved( i_link ), link( i_link ) { link = nil; }
  ~LinkSaver() { link = saved; }
};

// the wrapping sub-classes

)";
	
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
					ostr << "        ";
					if ( not method.returnType()->isVoidType() )
						ostr << "return ";
					ostr << oneClass.name() << "::" << method.name() << "(";
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
			{
				ostr << type2String( method.returnType() ) << " " << oneClass.name() << "_subclass_" << method.name() << "( ";
				if ( not method.isStatic() )
					ostr << oneClass.name() << "_subclass *i_this";
			}
			ostr << write_cxx_params_decl( method, not method.isStatic(), [&]( const CXXParam &p ){ return this->type2String( p.type() ) + " " + p.name(); } );
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
				if ( method.isStatic() )
					ostr << oneClass.name() << "::";
				else
					ostr << "i_this->";
				ostr << method.name() << "(";
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
