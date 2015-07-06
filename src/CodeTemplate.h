//
//  CodeTemplate.h
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/30.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

#ifndef H_CodeTemplate
#define H_CodeTemplate

#include "substringref.h"
#include <llvm/Support/raw_ostream.h>
#include <functional>
#include <deque>
#include <unordered_map>

/*!
   @brief A user provided model to fill template's sections.

       The model provide callbacks to print names and callbacks
	   to fill sections.
*/
struct CodeTemplateModel
{
	// for a section, a number of items (0 mean don't show) and an indexed callback that
	// provide another model for each item in the section.
	
	struct ListSection { size_t nb; std::function<void ( size_t, CodeTemplateModel & )> callback; };

	//! convinience function for boolean section
	inline static ListSection BoolSection( bool on, std::function<void ( CodeTemplateModel & )> callback )
	{
		return ListSection{ size_t(on?1:0), [callback]( size_t, CodeTemplateModel &m ){ callback( m ); } };
	}

	std::unordered_map<std::string,ListSection> sections;

	// for a name, just a callback that write to the stream
	std::unordered_map<std::string,std::function<void ( llvm::raw_ostream & )>> names;
};

/*!
   @brief Code generator from template.

       render source code from the provided template.
*/
class CodeTemplate
{
	public:
		CodeTemplate( const substringref &i_tmpl );
	
		void render( const CodeTemplateModel &i_model, llvm::raw_ostream &ostr );
	
	private:
		substringref _tmpl;
		std::deque<CodeTemplateModel> _context;

		void render( const substringref &i_tmpl, llvm::raw_ostream &ostr );

		void resolveName( const substringref &i_name, llvm::raw_ostream &ostr );
		bool resolveSection( const substringref &i_name, size_t i_index, CodeTemplateModel &o_model );
};

#endif
