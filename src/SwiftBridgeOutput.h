//
//  SwiftppObjcOutput.h
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/10.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

#ifndef H_SwiftppObjcOutput
#define H_SwiftppObjcOutput

#include "SwiftppOutput.h"

struct CodeTemplateModel;

class SwiftppObjcOutput : public SwiftppOutput
{
	private:
		virtual void write_impl();
	
		void write_cxx_bridge_h( CodeTemplateModel &i_model, llvm::raw_ostream &ostr ) const;
		void write_cxx_bridge_cpp( CodeTemplateModel &i_model, llvm::raw_ostream &ostr ) const;
		void write_cxx_bridge_swift( CodeTemplateModel &i_model, llvm::raw_ostream &ostr ) const;
	
		void buildCodeModel( CodeTemplateModel &model );
	
		std::string return_swift_c_type( const clang::QualType &i_cxxtype ) const;
		std::string return_converter_swift_to_c( const clang::QualType &i_cxxtype ) const;
	
		//! return the C type corresponding to the C++ type, according to the available converters
		std::string type2SwiftTypeString( const clang::QualType &i_cxxtype ) const;
		std::string param_swift_c_type( const clang::QualType &i_cxxtype ) const;
	
		/*!
		 @brief Write a function call that convert i_code to a C++ type.
		 
		 i_code is an expression of a C type convertible to i_cxxtype. This will return a string
		 that is a function call converting i_code to the C++ type.
		 example:
		 param_as_cxx_type( p )
		 return: "std::string( name )"
		 
		 @param[in] i_cxxtype type of i_code
		 @param[in] i_code expression of type i_cxxtype
		 @return     a function call converting i_code
		*/
		std::string param_as_cxx_type( const CXXParam &i_param ) const;
	
		std::string returnConverterForCType2CXXType( const clang::QualType &i_cxxtype ) const;
		std::string returnConverterForCXXType2CType( const clang::QualType &i_cxxtype ) const;
		std::string returnConverterForCType2SwiftType( const clang::QualType &i_cxxtype ) const;

		/*!
		 @brief Write a function call that convert i_code to a C type.
		 
		 i_code is an expression of type i_cxxtype. This will return a string
		 that is a function call converting i_code to a C type.
		 example:
		 param_as_c_type( {std::string}, "variable" )
		 return: "variable.c_str()"
		 
		 @param[in] i_cxxtype type of i_code
		 @param[in] i_code expression that can be converted to type i_cxxtype
		 @return     a function call converting i_code
		*/
		std::string param_as_c_type( const CXXParam &i_param ) const;

		std::string converterForCType2SwiftType( const clang::QualType &i_cxxtype, const std::string &i_code ) const;
};

#endif
