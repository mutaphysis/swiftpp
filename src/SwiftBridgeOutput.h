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

class SwiftppObjcOutput : public SwiftppOutput
{
	private:
		virtual void write_impl();
	
		void write_cxx_bridge_h( llvm::raw_ostream &ostr ) const;
		void write_cxx_bridge_cpp( llvm::raw_ostream &ostr ) const;
		void write_cxx_bridge_swift( llvm::raw_ostream &ostr ) const;
	
		//! return the C type corresponding to the C++ type, according to the available converters
		std::string cxxType2CTypeString( const clang::QualType &i_cxxtype ) const;
		std::string cxxType2SwiftTypeString( const clang::QualType &i_cxxtype ) const;
		std::string cxxType2SwiftCTypeString( const clang::QualType &i_cxxtype ) const;
	
		/*!
		 @brief Write a function call that convert i_code to a C++ type.
		 
		 i_code is an expression of a C type convertible to i_cxxtype. This will return a string
		 that is a function call converting i_code to the C++ type.
		 example:
		 converterForCType2CXXType( const char *, "variable" )
		 return: "std::string(  )"
		 
		 @param[in] i_cxxtype type of i_code
		 @param[in] i_code expression of type i_cxxtype
		 @return     a function call converting i_code
		*/
		std::string converterForCType2CXXType( const clang::QualType &i_cxxtype, const std::string &i_code ) const;

		/*!
		 @brief Write a function call that convert i_code to a C type.
		 
		 i_code is an expression of type i_cxxtype. This will return a string
		 that is a function call converting i_code to a C type.
		 example:
		 converterForCXXType2CType( {std::string}, "variable" )
		 return: "variable.c_str()"
		 
		 @param[in] i_cxxtype type of i_code
		 @param[in] i_code expression that can be converted to type i_cxxtype
		 @return     a function call converting i_code
		*/
		std::string converterForCXXType2CType( const clang::QualType &i_cxxtype, const std::string &i_code ) const;

		std::string converterForCType2SwiftType( const clang::QualType &i_cxxtype, const std::string &i_code ) const;
};

#endif
