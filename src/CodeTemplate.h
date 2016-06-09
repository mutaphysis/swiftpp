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

using ostream = llvm::raw_ostream;

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
	inline static ListSection BoolSection( bool on )
	{
		return ListSection{ size_t(on?1:0), nullptr };
	}

	std::unordered_map<std::string,ListSection> sections;

	// for a name, just a callback that write to the stream
	struct CallbackOrString
	{
		CallbackOrString(){}
		CallbackOrString( const std::string &i_text ) : text( i_text ){}
		CallbackOrString( const std::function<void ( ostream & )> &i_callback ) : callback( i_callback ){}
		
		CallbackOrString &operator=( const std::string &i_text )
		{
			text = i_text;
			callback = nullptr;
			return *this;
		}
		CallbackOrString &operator=( const std::function<void ( ostream & )> &i_callback )
		{
			callback = i_callback;
			return *this;
		}
		
		std::function<void ( ostream & )> callback;
		std::string text;
	};
	std::unordered_map<std::string,CallbackOrString> names;
};

/*!
   @brief Code generator from template.

       render source code from the provided template.
*/
class CodeTemplate
{
	public:
		CodeTemplate( const substringref &i_tmpl );
	
		void render( const CodeTemplateModel &i_model, ostream &ostr );
	
	private:
		substringref _tmpl;
		std::deque<CodeTemplateModel> _context;

		void render( const substringref &i_tmpl, ostream &ostr );

		void resolveName( const std::string &i_prefix, const substringref &i_name, ostream &ostr );
		bool resolveSection( const substringref &i_name, size_t i_index, CodeTemplateModel &o_model );
};

#endif
