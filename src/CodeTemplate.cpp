//
//  CodeTemplate.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/30.
//  Copyright (c) 2014年 dootaini. All rights reserved.
//

#include "CodeTemplate.h"
#include <string>
#include <cassert>

namespace
{
	// tags are delimited with <{ and }>
	inline bool isOpenTag( const char *i_ptr, const char *i_end )
	{
		return (i_end - i_ptr) >= 2 and i_ptr[0] == '<' and i_ptr[1] == '{';
	}
	inline void skipOpenTag( const char *&ptr ) { ptr += 2; } // <{
	inline bool isCloseTag( const char *i_ptr, const char *i_end )
	{
		return (i_end - i_ptr) >= 2 and i_ptr[0] == '}' and i_ptr[1] == '>';
	}
	inline void skipCloseTag( const char *&ptr ) { ptr += 2; } // }>
}

CodeTemplate::CodeTemplate( const char *i_begin, const char *i_end )
	: _tmpl( i_begin, i_end )
{
	_tmpl.trim();
}

void CodeTemplate::render( const CodeTemplateModel &i_model, llvm::raw_ostream &ostr )
{
	_context.push_front( i_model );
	render( _tmpl, ostr );
	
	// cleanup context
	_context.clear();
}

void CodeTemplate::render( const substringref &i_tmpl, llvm::raw_ostream &ostr )
{
	auto last = i_tmpl.begin();
	auto ptr = last;
	const char *startTag = nullptr; // start of the tag currently being parsed
	const char *startOpenSectionTag = nullptr; // start of the open tag of the current section
	const char *endOpenSectionTag = nullptr; // end of the open tag of the current section
	while ( ptr != i_tmpl.end() )
	{
		if ( startTag != nullptr and isCloseTag( ptr, i_tmpl.end() ) )
		{
			auto endTag = ptr + 2;
			if ( endOpenSectionTag != nullptr )
			{
				// looking for the end of a section
				if ( startTag[2] == '/' and substringref( startOpenSectionTag + 3, endOpenSectionTag - 2 ) == substringref( startTag + 3, endTag - 2 ) )
				{
					std::string sectionName( startOpenSectionTag + 3, endOpenSectionTag - 2 );

					// this allow nicer formatting in the template
					if ( *endOpenSectionTag == '\n' )
						++endOpenSectionTag;

					CodeTemplateModel m;
					int i = 0;
					while ( resolveSection( sectionName, i, m ) )
					{
						_context.push_front( m );
						render( substringref( endOpenSectionTag, startTag ), ostr );
						_context.pop_front();
						++i;
					}
					startOpenSectionTag = endOpenSectionTag = nullptr;
					last = endTag;
				}
			}
			else if ( startTag[2] == '#' )
			{
				// begin section
				startOpenSectionTag = startTag;
				endOpenSectionTag = endTag;
			}
			else
			{
				// name
				resolveName( std::string( startTag + 2, endTag - 2 ), ostr );
				last = ptr + 2;
			}
			startTag = nullptr;
			skipCloseTag( ptr );
		}
		else if ( startTag == nullptr and isOpenTag( ptr, i_tmpl.end() ) )
		{
			startTag = ptr;
			if ( endOpenSectionTag == nullptr )
				ostr.write( last, startTag - last );
			skipOpenTag( ptr );
		}
		else
			++ptr;
	}
	
	// write the remaining, if we're not in a tag
	if ( startTag == nullptr and endOpenSectionTag == nullptr )
		ostr.write( last, ptr - last );
}

void CodeTemplate::resolveName( const std::string &i_name, llvm::raw_ostream &ostr )
{
	for ( auto m : _context )
	{
		auto it = m.names.find( i_name );
		if ( it != m.names.end() )
			it->second( ostr );
	}
}

bool CodeTemplate::resolveSection( const std::string &i_name, int i_index, CodeTemplateModel &o_model )
{
	for ( auto m : _context )
	{
		auto it = m.sections.find( i_name );
		if ( it != m.sections.end() )
			return it->second( i_index, o_model );
	}
	return false;
}
