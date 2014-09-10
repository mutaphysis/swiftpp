//
//  SwiftppObjcOutput.h
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/10.
//  Copyright (c) 2014年 dootaini. All rights reserved.
//

#ifndef H_SwiftppObjcOutput
#define H_SwiftppObjcOutput

#include "SwiftppOutput.h"

class SwiftppObjcOutput : public SwiftppOutput
{
	private:
		virtual void write_impl();
	
		void write_cxx_objc_protocols_h( llvm::raw_ostream &ostr ) const;
		void write_cxx_objc_proxies_h( llvm::raw_ostream &ostr ) const;
		void write_cxx_objc_proxies_mm( llvm::raw_ostream &ostr ) const;
		void write_cxx_subclasses_mm( llvm::raw_ostream &ostr ) const;

		//! transform a QualType to a string
		std::string type2String( const clang::QualType &i_type ) const;
		
		//! transform a QualType to a string, removing constness and referenced
		std::string type2UndecoratedTypeString( const clang::QualType &i_type ) const;
		
		//! return the Objective-C type corresponding to the C++ type, according to the available converters
		std::string cxxType2ObjcTypeString( const clang::QualType &i_type ) const;
		
		/*!
		 @brief Write a function call that convert i_code to a C++ type.
		 
		 i_code is an expression of type i_type. This will return a string
		 that is a function call converting i_code to a C++ type.
		 example:
		 converterForObjcType2CXXType( {NSString}, "variable" )
		 return: "std::string( [variable UTF8String] )"
		 
		 @param[in] i_type type of i_code
		 @param[in] i_code expression of type i_type
		 @return     a function call converting i_code
		*/
		std::string converterForObjcType2CXXType( const clang::QualType &i_type, const std::string &i_code ) const;
		
		/*!
		 @brief Write a function call that convert i_code to an Objective-C type.
		 
		 i_code is an expression of type i_type. This will return a string
		 that is a function call converting i_code to an Objective-C type.
		 example:
		 converterForObjcType2CXXType( {std::string}, "variable" )
		 return: "[NSString stringWithUTF8String:variable.c_str()]"
		 
		 @param[in] i_type type of i_code
		 @param[in] i_code expression of type i_type
		 @return     a function call converting i_code
		*/
		std::string converterForCXXType2ObjcType( const clang::QualType &i_type, const std::string &i_code ) const;

		/*!
		 @brief Write an Objectvie-C method.
		 
		 @param[in] i_method the methos
		*/
		std::string write_objc_method_decl( const CXXMethod &i_method ) const;
};

#endif
