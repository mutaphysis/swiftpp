//
//  SwiftppObjcOutput.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/10.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

#include "SwiftBridgeOutput.h"
#include <clang/Frontend/CompilerInstance.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include "CodeTemplate.h"
#include "SwiftBridgeTmpl.h"

#include <iostream>

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

	auto ostr = _ci->createOutputFile( outputFolder + "c_impl.h", false, true, "", "", true, true );
	if ( ostr )
		write_cxx_bridge_h( *ostr );
	
	auto cpp_ext = "cpp";
	if ( _data->anyObjcTypes() )
		cpp_ext = "mm";
	ostr = _ci->createOutputFile( outputFolder + "c_impl." + cpp_ext, false, true, "", "", true, true );
	if ( ostr )
		write_cxx_bridge_cpp( *ostr );
	
	ostr = _ci->createOutputFile( outputFolder + "bridge.swift", false, true, "", "", true, true );
	if ( ostr )
		write_cxx_bridge_swift( *ostr );
}

std::string SwiftppObjcOutput::type2SwiftTypeString( const clang::QualType &i_cxxtype ) const
{
	std::string cxxtype( type2UndecoratedCXXTypeString( i_cxxtype ) );
	
	// is there a converter?
	for ( auto it : _data->converters() )
	{
		if ( cxxtype == type2UndecoratedCXXTypeString( it.from() ) )
		{
			// converter found, use the converted type
			if ( clang::isa<clang::ObjCObjectPointerType>( it.to() ) )
				return clang::QualType::getAsString(it.to().getTypePtr()->getPointeeType().split());

			return type2UndecoratedCXXTypeString( it.to() );
		}
	}
	
	// add a few default converters
	if ( cxxtype == "std::string" )
		return "String";
	
	if ( isCXXVectorType( i_cxxtype ) or isCXXListType( i_cxxtype ) )
	{
		assert( false );
	}
	if ( isCXXMapType( i_cxxtype ) or isCXXUnorderedMapType( i_cxxtype ) )
	{
		assert( false );
	}
	if ( isCXXSetType( i_cxxtype ) )
	{
		assert( false );
	}
	if ( cxxtype == "int" )
		return "CInt";
	
	//! @todo: warn for unsupported types
	assert( false );
	
	return cxxtype;
}

std::string SwiftppObjcOutput::type2SwiftCompatibleCTypeString( const clang::QualType &i_cxxtype ) const
{
	std::string cxxtype( type2UndecoratedCXXTypeString( i_cxxtype ) );
	
	// is there a converter?
	for ( auto it : _data->converters() )
	{
		if ( cxxtype == type2UndecoratedCXXTypeString( it.from() ) )
		{
			// converter found, use the converted type
			if ( clang::isa<clang::ObjCObjectPointerType>( it.to() ) )
				return clang::QualType::getAsString(it.to().getTypePtr()->getPointeeType().split()) + "!";

			return type2UndecoratedCXXTypeString( it.to() );
		}
	}
	
	// add a few default converters
	if ( cxxtype == "std::string" )
		return "UnsafePointer<CChar>";
	
	if ( isCXXVectorType( i_cxxtype ) or isCXXListType( i_cxxtype ) )
	{
		assert( false );
	}
	if ( isCXXMapType( i_cxxtype ) or isCXXUnorderedMapType( i_cxxtype ) )
	{
		assert( false );
	}
	if ( isCXXSetType( i_cxxtype ) )
	{
		assert( false );
	}
	if ( cxxtype == "int" )
		return "CInt";
	
	//! @todo: warn for unsupported types
	assert( false );
	
	return cxxtype;
}


void SwiftppObjcOutput::write_cxx_bridge_h( llvm::raw_ostream &ostr ) const
{
	CodeTemplateModel model;
	auto data = _data;
	
	model.names["objc_forward_decl"] = [data]( llvm::raw_ostream &ostr )
		{
			auto l = data->allObjcTypes();
			for ( auto &it : l )
			{
				ostr << "@class " << it << ";\n";
			}
		};
	
	auto classes = [this, data]( size_t i, CodeTemplateModel &o_model )
		{
			auto classPtr = &(data->classes()[i]);

			o_model.names["class_name"] = CodeTemplateModel::Name( classPtr->name() );

			// constructors
			o_model.sections["constructors"] = CodeTemplateModel::ListSection{ classPtr->constructors().size(),
					[this,classPtr]( size_t i, CodeTemplateModel &o_model )
					{
						auto it = classPtr->constructors()[i];
						// params
						o_model.sections["params"] = CodeTemplateModel::ListSection{ it->params().size(),
							[this,it]( size_t i, CodeTemplateModel &o_model )
							{
								auto param = &(it->params()[i]);
								o_model.names["param_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( param->type() ) );
								o_model.names["param_name"] = CodeTemplateModel::Name( param->name() );
							}
						};
					}
				};
			// callbacks
			o_model.sections["virtual_methods"] = CodeTemplateModel::ListSection{ classPtr->virtualMethods().size(),
				[this,classPtr]( size_t i, CodeTemplateModel &o_model )
				{
					auto vm = classPtr->virtualMethods()[i];
						o_model.names["return_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( vm->returnType() ) );
						o_model.names["name"] = CodeTemplateModel::Name( vm->name() );
						o_model.sections["params"] = CodeTemplateModel::ListSection{ vm->params().size(),
							[this,vm]( size_t i, CodeTemplateModel &o_model )
							{
								auto p = &(vm->params()[i]);
								o_model.names["param_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( p->type() ) );
							}
						};
				}
			};
			
			o_model.sections["methods"] = CodeTemplateModel::ListSection{ classPtr->nonStaticMethods().size(),
					[this,classPtr]( size_t i, CodeTemplateModel &o_model )
					{
						auto m = classPtr->nonStaticMethods()[i];
						o_model.names["return_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( m->returnType() ) );
						o_model.sections["params"] = CodeTemplateModel::ListSection{ m->params().size(),
							[this,m]( size_t i, CodeTemplateModel &o_model )
							{
								auto p = &(m->params()[i]);
								o_model.names["param_name"] = CodeTemplateModel::Name( p->name() );
								o_model.names["param_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( p->type() ) );
							}
						};
						o_model.names["name"] = CodeTemplateModel::Name( m->name() );
					}
				};
			o_model.sections["static_methods"] = CodeTemplateModel::ListSection{ classPtr->staticMethods().size(),
					[this,classPtr]( size_t i, CodeTemplateModel &o_model )
					{
						auto m = classPtr->staticMethods()[i];
						o_model.names["return_cxx_type"] = CodeTemplateModel::Name( this->type2CXXTypeString( m->returnType() ) );
						o_model.names["return_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( m->returnType() ) );
						o_model.sections["params"] = CodeTemplateModel::ListSection{ m->params().size(),
							[this,m]( size_t i, CodeTemplateModel &o_model )
							{
								auto p = &(m->params()[i]);
								o_model.names["param_name"] = CodeTemplateModel::Name( p->name() );
								o_model.names["param_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( p->type() ) );
								o_model.names["param_cxx_type"] = CodeTemplateModel::Name( this->type2CXXTypeString( p->type() ) );
							}
						};
						o_model.names["name"] = CodeTemplateModel::Name( m->name() );
					}
				};
		};
	
	model.sections["classes"] = CodeTemplateModel::ListSection{ data->classes().size(), classes };

	CodeTemplate renderer( kCXX_BRIDGE_H_TEMPLATE );
	renderer.render( model, ostr );
}

void SwiftppObjcOutput::write_cxx_bridge_cpp( llvm::raw_ostream &ostr ) const
{
	CodeTemplateModel model;
	auto data = _data;

	model.names["bridge_include"] = CodeTemplateModel::Name( data->formatIncludeName( _inputFile ) );

	auto classes = [this, data]( size_t i, CodeTemplateModel &o_model )
		{
			auto classPtr = &(data->classes()[i]);

			o_model.names["class_name"] = CodeTemplateModel::Name( classPtr->name() );
			
			o_model.sections["virtual_methods"] = CodeTemplateModel::ListSection{ classPtr->virtualMethods().size(),
					[this,classPtr]( size_t i, CodeTemplateModel &o_model )
					{
						auto m = classPtr->virtualMethods()[i];
						o_model.names["return_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( m->returnType() ) );
						o_model.names["return_cxx_type"] = CodeTemplateModel::Name( this->type2CXXTypeString( m->returnType() ) );
						o_model.sections["params"] = CodeTemplateModel::ListSection{ m->params().size(),
							[this,m]( size_t i, CodeTemplateModel &o_model )
							{
								auto p = &(m->params()[i]);
								o_model.names["param_name"] = CodeTemplateModel::Name( p->name() );
								o_model.names["param_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( p->type() ) );
								o_model.names["param_cxx_type"] = CodeTemplateModel::Name( this->type2CXXTypeString( p->type() ) );
								o_model.names["param_as_c_type"] = CodeTemplateModel::Name( this->converterForCXXType2CType( p->type(), p->name() ) );
							}
						};
						o_model.names["name"] = CodeTemplateModel::Name( m->name() );
					}
				};
			o_model.sections["constructors"] = CodeTemplateModel::ListSection{ classPtr->constructors().size(),
					[this,classPtr]( size_t i, CodeTemplateModel &o_model )
					{
						auto m = classPtr->constructors()[i];
						// params
						o_model.sections["params"] = CodeTemplateModel::ListSection{ m->params().size(),
							[this,m]( size_t i, CodeTemplateModel &o_model )
							{
								auto param = &(m->params()[i]);
								o_model.names["param_cxx_type"] = CodeTemplateModel::Name( this->type2CXXTypeString( param->type() ) );
								o_model.names["param_name"] = CodeTemplateModel::Name( param->name() );
								o_model.names["param_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( param->type() ) );
							}
						};
					}
				};
			o_model.sections["methods"] = CodeTemplateModel::ListSection{ classPtr->nonStaticMethods().size(),
					[this,classPtr]( size_t i, CodeTemplateModel &o_model )
					{
						auto m = classPtr->nonStaticMethods()[i];
						o_model.names["return_c_type"] = CodeTemplateModel::Name( this->type2CXXTypeString( m->returnType() ) );
						o_model.names["return_cxx_type"] = CodeTemplateModel::Name( this->type2CXXTypeString( m->returnType() ) );
						o_model.sections["params"] = CodeTemplateModel::ListSection{ m->params().size(),
							[this,m]( size_t i, CodeTemplateModel &o_model )
							{
								auto p = &(m->params()[i]);
								o_model.names["param_name"] = CodeTemplateModel::Name( p->name() );
								o_model.names["param_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( p->type() ) );
								o_model.names["param_cxx_type"] = CodeTemplateModel::Name( this->type2CXXTypeString( p->type() ) );
								o_model.names["param_as_cxx_type"] = CodeTemplateModel::Name( this->converterForCType2CXXType( p->type(), p->name() ) );
							}
						};
						o_model.names["name"] = CodeTemplateModel::Name( m->name() );
					}
				};
			o_model.sections["static_methods"] = CodeTemplateModel::ListSection{ classPtr->staticMethods().size(),
					[this,classPtr]( size_t i, CodeTemplateModel &o_model )
					{
						auto m = classPtr->staticMethods()[i];
						o_model.names["return_cxx_type"] = CodeTemplateModel::Name( this->type2CXXTypeString( m->returnType() ) );
						o_model.names["return_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( m->returnType() ) );
						o_model.sections["params"] = CodeTemplateModel::ListSection{ m->params().size(),
							[this,m]( size_t i, CodeTemplateModel &o_model )
							{
								auto p = &(m->params()[i]);
								o_model.names["param_name"] = CodeTemplateModel::Name( p->name() );
								o_model.names["param_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( p->type() ) );
							}
						};
						o_model.names["name"] = CodeTemplateModel::Name( m->name() );
					}
				};
		};
	
	model.sections["classes"] = CodeTemplateModel::ListSection{ data->classes().size(), classes };

	CodeTemplate renderer( kCXX_BRIDGE_CPP_TEMPLATE );
	renderer.render( model, ostr );
}

void SwiftppObjcOutput::write_cxx_bridge_swift( llvm::raw_ostream &ostr ) const
{
	CodeTemplateModel model;
	auto data = _data;

	auto classes = [this, data]( size_t i, CodeTemplateModel &o_model )
		{
			auto classPtr = &(data->classes()[i]);

			o_model.names["class_name"] = CodeTemplateModel::Name( classPtr->name() );

			o_model.sections["constructors"] = CodeTemplateModel::ListSection{ classPtr->constructors().size(),
					[this,classPtr]( size_t i, CodeTemplateModel &o_model )
					{
						auto it = classPtr->constructors()[i];
						// params
						o_model.sections["params"] = CodeTemplateModel::ListSection{ it->params().size(),
							[this,it]( size_t i, CodeTemplateModel &o_model )
							{
								auto param = &(it->params()[i]);
								o_model.names["params"] = [this,param]( llvm::raw_ostream &ostr )
								{
									ostr << this->type2CXXTypeString( param->type() ) << " " << param->name();
								};
								o_model.names["param_name"] = CodeTemplateModel::Name( param->name() );
								o_model.names["param_clean_name"] = CodeTemplateModel::Name( param->cleanName() );
								o_model.names["param_swift_type"] = CodeTemplateModel::Name( this->type2SwiftTypeString( param->type() ) );
							}
						};
					}
				};

			o_model.sections["virtual_methods"] = CodeTemplateModel::ListSection{ classPtr->virtualMethods().size(),
					[this,classPtr]( size_t i, CodeTemplateModel &o_model )
					{
						auto m = classPtr->virtualMethods()[i];
						o_model.names["return_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( m->returnType() ) );
						o_model.names["return_cxx_type"] = CodeTemplateModel::Name( this->type2CXXTypeString( m->returnType() ) );
						o_model.sections["params"] = CodeTemplateModel::ListSection{ m->params().size(),
							[this,m]( size_t i, CodeTemplateModel &o_model )
							{
								auto p = &(m->params()[i]);
								o_model.names["param_name"] = CodeTemplateModel::Name( p->name() );
								o_model.names["param_c_type"] = CodeTemplateModel::Name( this->type2CTypeString( p->type() ) );
								o_model.names["param_cxx_type"] = CodeTemplateModel::Name( this->type2CXXTypeString( p->type() ) );
								o_model.names["param_as_c_type"] = CodeTemplateModel::Name( this->converterForCXXType2CType( p->type(), p->name() ) );
								o_model.names["param_swift_c_type"] = CodeTemplateModel::Name( this->type2SwiftCompatibleCTypeString( p->type() ) );
								o_model.names["param_clean_name"] = CodeTemplateModel::Name( p->cleanName() );
								o_model.names["param_as_swift_type"] = [this,i,p]( llvm::raw_ostream &ostr )
									{
										if ( i != 0 )
											ostr << p->cleanName() << ": ";
										ostr << this->converterForCType2SwiftType( p->type(), p->cleanName() );
									};
							}
						};
						o_model.names["name"] = CodeTemplateModel::Name( m->name() );
					}
				};

			o_model.sections["methods"] = CodeTemplateModel::ListSection{ classPtr->nonStaticMethods().size(),
					[this,classPtr]( size_t i, CodeTemplateModel &o_model )
					{
						auto m = classPtr->nonStaticMethods()[i];
						o_model.names["name"] = CodeTemplateModel::Name( m->name() );
						o_model.sections["params"] = CodeTemplateModel::ListSection{ m->params().size(),
							[this,m]( size_t i, CodeTemplateModel &o_model )
							{
								auto param = &(m->params()[i]);
								o_model.names["params"] = [this,param]( llvm::raw_ostream &ostr )
								{
									ostr << this->type2CXXTypeString( param->type() ) << " " << param->name();
								};
								o_model.names["param_name"] = CodeTemplateModel::Name( param->name() );
								o_model.names["param_clean_name"] = CodeTemplateModel::Name( param->cleanName() );
								o_model.names["param_swift_type"] = CodeTemplateModel::Name( this->type2SwiftTypeString( param->type() ) );
							}
						};
					}
				};
			o_model.sections["static_methods"] = CodeTemplateModel::ListSection{ classPtr->staticMethods().size(),
					[this,classPtr]( size_t i, CodeTemplateModel &o_model )
					{
						auto m = classPtr->staticMethods()[i];
						o_model.names["name"] = CodeTemplateModel::Name( m->name() );
						o_model.sections["params"] = CodeTemplateModel::ListSection{ m->params().size(),
							[this,m]( size_t i, CodeTemplateModel &o_model )
							{
								auto param = &(m->params()[i]);
								o_model.names["params"] = [this,param]( llvm::raw_ostream &ostr )
								{
									ostr << this->type2CXXTypeString( param->type() ) << " " << param->name();
								};
								o_model.names["param_name"] = CodeTemplateModel::Name( param->name() );
								o_model.names["param_clean_name"] = CodeTemplateModel::Name( param->cleanName() );
								o_model.names["param_swift_type"] = CodeTemplateModel::Name( this->type2SwiftTypeString( param->type() ) );
							}
						};
					}
				};
		};
	
	model.sections["classes"] = CodeTemplateModel::ListSection{ data->classes().size(), classes };

	CodeTemplate renderer( kCXX_BRIDGE_SWIFT_TEMPLATE );
	renderer.render( model, ostr );
}

std::string SwiftppObjcOutput::converterForCType2CXXType( const clang::QualType &i_cxxtype, const std::string &i_code ) const
{
	std::string cxxtype( type2UndecoratedCXXTypeString( i_cxxtype ) );
	
	// is there a converter?
	for ( auto converter : _data->converters() )
	{
		if ( cxxtype == type2UndecoratedCXXTypeString( converter.to() ) )
		{
			// converter found, use the converted type
			return std::string("swift_converter::") + converter.name() + "(" + i_code + ")";
		}
	}
	
	// add a few default converters
	if ( cxxtype == "std::string" )
		return std::string("std::string(") + i_code + ")";
	
	clang::QualType valueType;
	if ( isCXXVectorType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXListType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXMapType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXUnorderedMapType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXSetType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	
	return i_code;
}

std::string SwiftppObjcOutput::converterForCXXType2CType( const clang::QualType &i_cxxtype, const std::string &i_code ) const
{
	std::string cxxtype( type2UndecoratedCXXTypeString( i_cxxtype ) );
	
	// is there a converter?
	for ( auto converter : _data->converters() )
	{
		if ( cxxtype == type2UndecoratedCXXTypeString( converter.from() ) )
		{
			// converter found, use the converted type
			return std::string("swift_converter::") + converter.name() + "(" + i_code + ")";
		}
	}
	
	// add a few default converters
	if ( cxxtype == "std::string" )
		return i_code + ".c_str()";
	
	clang::QualType valueType;
	if ( isCXXVectorType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXListType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXMapType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXUnorderedMapType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXSetType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}

	return i_code;
}

std::string SwiftppObjcOutput::converterForCType2SwiftType( const clang::QualType &i_cxxtype, const std::string &i_code ) const
{
	std::string cxxtype( type2UndecoratedCXXTypeString( i_cxxtype ) );
	
	// add a few default converters
	if ( cxxtype == "std::string" )
		return "String.fromCString(" + i_code + ")!";
	
	clang::QualType valueType;
	if ( isCXXVectorType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXListType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXMapType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXUnorderedMapType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}
	else if ( isCXXSetType( i_cxxtype, &valueType ) )
	{
		assert( false );
	}

	return i_code;
}
