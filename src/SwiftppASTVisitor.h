//
//  SwiftppASTVisitor.h
//  swiftpp
//
//  Created by Sandy Martel on 2014/08/28.
//  Copyright (c) 2014年. All rights reserved.
//

#ifndef H_SwiftppASTVisitor
#define H_SwiftppASTVisitor

#include <clang/AST/RecursiveASTVisitor.h>

class SwiftppData;

class SwiftppASTVisitor : public clang::RecursiveASTVisitor<SwiftppASTVisitor>
{
  public:
	SwiftppASTVisitor( SwiftppData &io_data );

	bool VisitCXXRecordDecl( clang::CXXRecordDecl *i_decl );
	bool VisitFunctionDecl( clang::FunctionDecl *i_decl );

  private:
	SwiftppData &_data;

	void VisitCXXRecordDeclImpl( clang::CXXRecordDecl *i_decl );
};

#endif
