//
//  SwiftppOutput.h
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/10.
//  Copyright (c) 2014年 dootaini. All rights reserved.
//

#ifndef H_SwiftppOutput
#define H_SwiftppOutput

#include "SwiftppData.h"

namespace clang
{
class CompilerInstance;
}

class SwiftppOutput
{
	public:
		SwiftppOutput() = default;
		virtual ~SwiftppOutput();
	
		void write( clang::CompilerInstance &i_ci, const std::string &i_inputFile, const SwiftppData &i_data );
	
	protected:
		clang::CompilerInstance *_ci = nullptr;
		std::string _inputFile;
		const SwiftppData *_data = nullptr;
	
		virtual void write_impl() = 0;

		// utils

		//! transform a QualType to a string
		std::string type2String( const clang::QualType &i_type ) const;
		
		//! transform a QualType to a string, removing constness and referenced
		std::string type2UndecoratedTypeString( const clang::QualType &i_type ) const;

		std::string typeNameForFunc( const clang::QualType &i_cxxtype ) const;
		
		bool isCXXVectorType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType = nullptr ) const;
		bool isCXXListType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType = nullptr ) const;
		bool isCXXMapType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType = nullptr ) const;
		bool isCXXUnorderedMapType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType = nullptr ) const;
		bool isCXXSetType( const clang::QualType &i_cxxtype, clang::QualType *o_valueType = nullptr ) const;
};

#endif
