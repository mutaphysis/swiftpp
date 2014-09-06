//
//  SwiftppASTConsumer.h
//  swiftpp
//
//  Created by Sandy Martel on 2014/08/27.
//  Copyright (c) 2014年. All rights reserved.
//

#ifndef H_SwiftppASTConsumer
#define H_SwiftppASTConsumer

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include "SwiftppASTVisitor.h"
#include "SwiftppData.h"

class SwiftppASTConsumer : public clang::ASTConsumer
{
	private:
		clang::CompilerInstance &_ci;
		SwiftppData _data;
		SwiftppASTVisitor _visitor;
		std::string _inputFile;
	
		virtual void HandleTranslationUnit( clang::ASTContext &i_ctx ) override;

	public:
		SwiftppASTConsumer( clang::CompilerInstance &i_ci, const SwiftppOptions &i_options, llvm::StringRef i_inputFile );
};

#endif