//
//  main.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/08/27.
//  Copyright (c) 2014年. All rights reserved.
//

#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Basic/Version.h>
#include <iostream>

#include "SwiftppASTConsumer.h"
#include "SwiftBridgeOutput.h"
#include "Version.h"

namespace
{

SwiftppOptions g_options;

class SwiftppAction : public clang::ASTFrontendAction
{
	protected:
		virtual std::unique_ptr<clang::ASTConsumer> CreateASTConsumer( clang::CompilerInstance &ci, llvm::StringRef i_inputFile ) override;
};

std::unique_ptr<clang::ASTConsumer> SwiftppAction::CreateASTConsumer( clang::CompilerInstance &ci, llvm::StringRef i_inputFile )
{
	// enable some extensions
	ci.getFrontendOpts().SkipFunctionBodies = true;
//	ci.getPreprocessor().enableIncrementalProcessing( true );
//	ci.getPreprocessor().SetSuppressIncludeNotFoundError( true );
	ci.getLangOpts().DelayedTemplateParsing = true;
	ci.getLangOpts().CPlusPlus11 = true;
	ci.getLangOpts().CPlusPlus14 = true;
	ci.getLangOpts().GNUMode = true;
	
	std::shared_ptr<SwiftppOutput> output( new SwiftppObjcOutput );
	
	return std::unique_ptr<clang::ASTConsumer>( new SwiftppASTConsumer( ci,
																	g_options,
																	output,
																	i_inputFile ) );
}

void showVersion( int i_exitCode )
{
	std::cerr << "swiftpp version " SWIFTPP_VERSION << std::endl;
	exit( i_exitCode );
}

void showHelp( int i_exitCode )
{
	std::cerr << "Usage swiftpp: [options] header_file\n"
	 	 	"  -o <path>                Write output files to this directory\n"
			"  -swift                   Output swift bridge\n"
			"  -named-params            Parameters of C++ methods exported to swift have\n"
			"                           names (default)\n"
			"  -no-named-params         Parameters of C++ methods exported to swift have\n"
			"                           no name\n"
			"  -full-path-for-includes  User includes use full paths (default)\n"
			"  -name-only-for-includes  User includes use file name only\n"
	 	 	"  -I <dir>                 Add dir to the include path for header files\n"
	 	 	"  -D <macro>[=<def>]       Define macro, with optional definition\n"
	 	 	"  -v, -version             Display version of swiftpp\n"
	 	 	<< std::endl;

	showVersion( i_exitCode );
}

}

int main( int argc, const char **argv )
{
	std::vector<std::string> args;
	args.emplace_back( argv[0] );
	args.emplace_back( "-x" );
	args.emplace_back( "objective-c++" );
	args.emplace_back( "-fPIE" );
	args.emplace_back( "-std=c++11" );
// I'd like to use C++11 standard annotations, but the AST does not retain them :-(
//	args.emplace_back( "-Wno-attributes" );
	args.emplace_back( "-Wno-unknown-attributes" ); // use verbose gnu style attributes....

	bool hasInputFile = false;

	for ( int i = 1 ; i < argc; ++i )
	{
		if ( strcmp( argv[i], "-h" ) == 0 or strcmp( argv[i], "-?" ) == 0 or strcmp( argv[i], "-help" ) == 0 )
		{
			showHelp( EXIT_SUCCESS );
		}
		else if ( strcmp( argv[i], "-v" ) == 0 or strcmp( argv[i], "-version" ) == 0 )
		{
			showVersion( EXIT_SUCCESS );
		}
		else if ( strcmp( argv[i], "-include" ) == 0 and (i+1) < argc )
		{
			args.emplace_back( argv[i] );
			++i;
			args.emplace_back( argv[i] );
		}
		else if ( argv[i][0] == '-' and (argv[i][1] == 'I' or argv[i][1] == 'U' or argv[i][1] == 'D') )
		{
			args.emplace_back( argv[i] );
			if ( argv[i][2] == 0 and (i+1) < argc )
			{
				++i;
				args.emplace_back( argv[i] );
			}
		}
		else if ( strcmp( argv[i], "-X" ) == 0 and (i+1) < argc )
		{
			args.emplace_back( argv[i] );
			++i;
			args.emplace_back( argv[i] );
			hasInputFile = true;
		}
		else if ( argv[i][0] == '-' and argv[i][1] == 'o' )
		{
			if ( argv[i][2] == 0 and (i+1) < argc )
			{
				++i;
				g_options.output = argv[i];
			}
			else
				g_options.output = (argv[i] + 2);
		}
		else if ( strcmp( argv[i], "-named-params" ) == 0 )
		{
			g_options.usedNamedParams = true;
		}
		else if ( strcmp( argv[i], "-no-named-params" ) == 0 )
		{
			g_options.usedNamedParams = false;
		}
		else if ( strcmp( argv[i], "-full-path-for-includes" ) == 0 )
		{
			g_options.usedFullPathForUserIncludes = true;
		}
		else if ( strcmp( argv[i], "-name-only-for-includes" ) == 0 )
		{
			g_options.usedFullPathForUserIncludes = false;
		}
		else if ( argv[i][1] == '-' )
		{
			std::cerr << "swiftpp: Invalid argument '" << argv[i] << "'" << std::endl;
			showHelp( EXIT_FAILURE );
		}
		else if ( hasInputFile )
		{
			std::cerr << "error: Too many input files specified" << std::endl;
			return EXIT_FAILURE;
		}
		else
		{
			args.emplace_back( argv[i] );
			hasInputFile = true;
		}
	}

	//! @todo: should pick up the Xcode installed clang?
	// clang++ -E -x c++ - -v < /dev/null
	args.emplace_back( "-I" LLVM_INCLUDEDIR "/c++/v1" );
	args.emplace_back( "-I" LLVM_BINDIR "/../lib/clang/" CLANG_VERSION_STRING "/include" );
	args.emplace_back( "-fsyntax-only" );

	clang::FileManager fm( {"."} );
	fm.Retain();

	clang::tooling::ToolInvocation tool( args, new SwiftppAction, &fm );
	return not tool.run();
}
