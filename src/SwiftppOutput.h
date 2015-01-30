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
};

#endif
