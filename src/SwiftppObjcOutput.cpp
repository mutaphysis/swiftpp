//
//  SwiftppObjcOutput.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/10.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

#include "SwiftppObjcOutput.h"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include "CodeTemplate.h"
#include "SwiftppObjcTemplates.h"

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

//! write an Objective-C method declaration
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
	CodeTemplateModel model;
	auto data = _data;
	model.sections["classes"] = CodeTemplateModel::Section{ data->classes().size(),
						[=]( size_t i_index, CodeTemplateModel &o_model )
						{
							auto classPtr = &(data->classes()[i_index]);
							o_model.names["class_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << classPtr->name(); };
							o_model.sections["methods"] = CodeTemplateModel::Section{ classPtr->methods().size(),
								[=]( size_t i_index, CodeTemplateModel &o_model )
								{
									auto methodPtr = classPtr->methods().begin();
									std::advance( methodPtr, i_index );
									o_model.names["objc_method_decl"] = [=]( llvm::raw_ostream &ostr ){ this->write_objc_method_decl( ostr, *methodPtr ); };
								} };
						} };
	model.sections["has_enums"] = CodeTemplateModel::BoolSection( not data->enums().empty(),
						[=]( CodeTemplateModel &o_model )
						{
							o_model.sections["enums"] = CodeTemplateModel::Section{ data->enums().size(),
									[=]( size_t i_index, CodeTemplateModel &o_model )
									{
										auto enumPtr = &(data->enums()[i_index]);
										o_model.names["enum_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << enumPtr->name(); };
										o_model.names["enum_type"] = [=]( llvm::raw_ostream &ostr ){ ostr << (enumPtr->isSigned() ? "NSInteger" : "NSUInteger"); };
										o_model.sections["enum_values"] = CodeTemplateModel::Section{ enumPtr->values().size(),
											[=]( size_t i_index, CodeTemplateModel &o_model )
											{
												auto valPtr = &(enumPtr->values()[i_index]);
												o_model.names["enum_value_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << valPtr->first << " = " << valPtr->second; };
											} };
									} };
						} );
	
	CodeTemplate renderer( kCXX_OBJC_PROTOCOLS_H_TEMPLATE );
	renderer.render( model, ostr );
}

void SwiftppObjcOutput::write_cxx_objc_proxies_h( llvm::raw_ostream &ostr ) const
{
	CodeTemplateModel model;
	auto data = _data;
	model.sections["classes"] = CodeTemplateModel::Section{ data->classes().size(),
					[=]( size_t i_index, CodeTemplateModel &o_model )
					{
						auto classPtr = &(data->classes()[i_index]);
						o_model.names["class_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << classPtr->name(); };
					} };

	CodeTemplate renderer( kCXX_OBJC_PROXIES_H_TEMPLATE );
	renderer.render( model, ostr );
}

void SwiftppObjcOutput::write_cxx_objc_proxies_mm( llvm::raw_ostream &ostr ) const
{
	CodeTemplateModel model;
	auto data = _data;
	model.sections["classes"] = CodeTemplateModel::Section{ data->classes().size(),
					[=]( size_t i_index, CodeTemplateModel &o_model )
					{
						auto classPtr = &(data->classes()[i_index]);
						o_model.names["class_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << classPtr->name(); };
						o_model.sections["methods"] = CodeTemplateModel::Section{ classPtr->methods().size(),
								[=]( size_t i_index, CodeTemplateModel &o_model )
								{
									auto methodPtr = classPtr->methods().begin();
									std::advance( methodPtr, i_index );
									o_model.names["c_proxy_method_decl"] = [=]( llvm::raw_ostream &ostr ){ this->write_c_proxy_method_decl( ostr, classPtr->name(), *methodPtr ); };
									o_model.names["objc_method_impl"] = [=]( llvm::raw_ostream &ostr ){ this->write_objc_method_impl( ostr, classPtr->name(), *methodPtr ); };
								} };
					} };
	model.sections["includes_for_cxx_types"] = CodeTemplateModel::Section{ data->includesForCXXTypes().size(),
					[=]( size_t i_index, CodeTemplateModel &o_model )
					{
						auto includeName = data->formatIncludeName( data->includesForCXXTypes()[i_index] );
						o_model.names["include_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << includeName; };
					} };
	model.sections["converters"] = CodeTemplateModel::Section{ data->converters().size(),
					[=]( size_t i_index, CodeTemplateModel &o_model )
					{
						auto convPtr = &(data->converters()[i_index]);
						o_model.names["converter_decl"] = [=]( llvm::raw_ostream &ostr )
						{
							ostr << this->type2String(convPtr->to()) + " " + convPtr->name() + "( " + this->type2String(convPtr->from()) + " )";
						};
					} };

	CodeTemplate renderer( kCXX_OBJC_PROXIES_MM_TEMPLATE );
	renderer.render( model, ostr );
}

void SwiftppObjcOutput::write_cxx_subclasses_mm( llvm::raw_ostream &ostr ) const
{
	CodeTemplateModel model;
	auto data = _data;
	auto inputFile = _inputFile;
	model.names["bridge_include"] = [=]( llvm::raw_ostream &ostr ){ ostr << data->formatIncludeName( _inputFile ); };
	model.sections["classes"] = CodeTemplateModel::Section{ data->classes().size(),
					[=]( size_t i_index, CodeTemplateModel &o_model )
					{
						auto classPtr = &(data->classes()[i_index]);
						o_model.names["class_name"] = [=]( llvm::raw_ostream &ostr ){ ostr << classPtr->name(); };
						o_model.sections["methods"] = CodeTemplateModel::Section{ classPtr->methods().size(),
								[=]( size_t i_index, CodeTemplateModel &o_model )
								{
									auto methodPtr = classPtr->methods().begin();
									std::advance( methodPtr, i_index );
									o_model.names["cpp_method_impl"] = [=]( llvm::raw_ostream &ostr ){ this->write_cpp_method_impl( ostr, classPtr->name(), *methodPtr ); };
									o_model.names["c_proxy_method_impl"] = [=]( llvm::raw_ostream &ostr ){ this->write_c_proxy_method_impl( ostr, classPtr->name(), *methodPtr ); };
								} };
					} };
	
	CodeTemplate renderer( kCXX_SUBCLASSES_MM_TEMPLATE );
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
		if ( not i_method.isStatic() )
			ostr << "  assert( _this != nullptr );\n";
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
	if ( cxxtype == "bool" )
		return "BOOL";
	
	if ( isCXXVectorType( i_cxxtype ) or isCXXListType( i_cxxtype ) )
		return "NSArray *";
	if ( isCXXMapType( i_cxxtype ) or isCXXUnorderedMapType( i_cxxtype ) )
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
	else if ( isCXXListType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_to_list_") + typeNameForFunc( valueType ) + "( " + i_code + " )";
	}
	else if ( isCXXMapType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_to_map_") + typeNameForFunc( valueType ) + "( " + i_code + " )";
	}
	else if ( isCXXUnorderedMapType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_to_map_") + typeNameForFunc( valueType ) + "( " + i_code + " )";
	}
	else if ( isCXXSetType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_to_set_") + typeNameForFunc( valueType ) + "( " + i_code + " )";
	}
	
	return i_code;
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
		return std::string("swift_converter::generated_from_vector( ") + i_code + " )";
	}
	else if ( isCXXListType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_from_list( ") + i_code + " )";
	}
	else if ( isCXXMapType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_from_map_( ") + i_code + " )";
	}
	else if ( isCXXUnorderedMapType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_from_map_( ") + i_code + " )";
	}
	else if ( isCXXSetType( i_cxxtype, &valueType ) )
	{
		return std::string("swift_converter::generated_from_set_( ") + i_code + " )";
	}

	return i_code;
}
