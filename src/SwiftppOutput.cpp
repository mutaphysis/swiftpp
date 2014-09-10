//
//  SwiftppOutput.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/10.
//  Copyright (c) 2014年 dootaini. All rights reserved.
//

#include "SwiftppOutput.h"

SwiftppOutput::~SwiftppOutput()
{
}

void SwiftppOutput::write( clang::CompilerInstance &i_ci, const std::string &i_inputFile, const SwiftppData &i_data )
{
	_ci = &i_ci;
	_inputFile = i_inputFile;
	_data = &i_data;
	try
	{
		write_impl();
	}
	catch ( ... )
	{
		_ci = nullptr;
		_inputFile.clear();
		_data = nullptr;
		throw;
	}
	_ci = nullptr;
	_inputFile.clear();
	_data = nullptr;
}
