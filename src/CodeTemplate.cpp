//
//  CodeTemplate.cpp
//  swiftpp
//
//  Created by Sandy Martel on 2014/09/30.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
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

CodeTemplate::CodeTemplate( const substringref &i_tmpl )
	: _tmpl( i_tmpl )
{
	// trim new line at the start
	while ( not _tmpl.empty() and *_tmpl.begin() == '\n' )
		_tmpl.pop_front();
	
	// trim 0 at end
	while ( not _tmpl.empty() and _tmpl.back() == 0 )
		_tmpl.pop_back();
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
					substringref sectionName( startOpenSectionTag + 3, endOpenSectionTag - 2 );

					// this allow nicer formatting in the template
					if ( *endOpenSectionTag == '\n' )
						++endOpenSectionTag;

					CodeTemplateModel m;
					size_t i = 0;
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
				resolveName( substringref( startTag + 2, endTag - 2 ), ostr );
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

void CodeTemplate::resolveName( const substringref &i_name, llvm::raw_ostream &ostr )
{
	std::string name( i_name.begin(), i_name.end() );
	for ( auto m : _context )
	{
		auto it = m.names.find( name );
		if ( it != m.names.end() )
			it->second( ostr );
	}
}

bool CodeTemplate::resolveSection( const substringref &i_name, size_t i_index, CodeTemplateModel &o_model )
{
	std::string name( i_name.begin(), i_name.end() );
	for ( auto m : _context )
	{
		auto it = m.sections.find( name );
		if ( it != m.sections.end() )
		{
			if ( i_index >= it->second.nb )
				return false;
			it->second.callback( i_index, o_model );
			return true;
		}
	}
	return false;
}
