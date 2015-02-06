//
//  CodeTemplate.h
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/30.
//  Copyright (c) 2014年 dootaini. All rights reserved.
//

#ifndef H_CodeTemplate
#define H_CodeTemplate

#include "substringref.h"
#include <llvm/Support/raw_ostream.h>
#include <functional>
#include <deque>
#include <unordered_map>

struct CodeTemplateModel
{
	// for a name, just a callback that write to the stream
	std::unordered_map<std::string,std::function<void ( llvm::raw_ostream & )>> names;
	
	struct Section { size_t nb; std::function<void ( size_t, CodeTemplateModel & )> callback; };
	std::unordered_map<std::string,Section> sections;
};

class CodeTemplate
{
	public:
		CodeTemplate( const char *i_begin, const char *i_end );
	
		void render( const CodeTemplateModel &i_model, llvm::raw_ostream &ostr );
	
	private:
		substringref _tmpl;
		std::deque<CodeTemplateModel> _context;

		void render( const substringref &i_tmpl, llvm::raw_ostream &ostr );

		void resolveName( const std::string &i_name, llvm::raw_ostream &ostr );
		bool resolveSection( const std::string &i_name, size_t i_index, CodeTemplateModel &o_model );
};

#endif
