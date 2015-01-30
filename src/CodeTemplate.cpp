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

CodeTemplate::CodeTemplate( const substringref &i_tmpl )
	: _tmpl( i_tmpl )
{
}

void CodeTemplate::render( const CodeTemplateModel &i_model, const std::function<void (const char *,size_t)> &i_writer )
{
	_context.push_front( i_model );
	render( _tmpl, i_writer );
	
	// cleanup context
	_context.clear();
}

void CodeTemplate::render( const substringref &i_tmpl, const std::function<void (const char *,size_t)> &i_writer )
{
	auto last = i_tmpl.begin();
	auto ptr = last;
	char lastChar = 0;
	const char *startTag = nullptr; // start of the tag currently being parsed
	const char *startOpenSectionTag = nullptr; // start of the open tag of the current section
	const char *endOpenSectionTag = nullptr; // end of the open tag of the current section
	while ( ptr != i_tmpl.end() )
	{
		if ( startTag != nullptr and *ptr == '>' and lastChar == '}' ) // }>
		{
			auto endTag = ptr + 1;
			if ( endOpenSectionTag != nullptr )
			{
				// looking for the end of a section
				if ( startTag[2] == '/' and substringref( startOpenSectionTag + 3, endOpenSectionTag - 2 ) == substringref( startTag + 3, endTag - 2 ) )
				{
					int i = 0;
					std::string sectionName( startOpenSectionTag + 3, endOpenSectionTag - 2 );

					// this allow nicer formatting for the template
					if ( *endOpenSectionTag == '\n' )
						++endOpenSectionTag;

					CodeTemplateModel m;
					while ( resolveSection( sectionName, i, m ) )
					{
						_context.push_front( m );
						render( substringref( endOpenSectionTag, startTag ), i_writer );
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
				auto replc = resolveName( std::string( startTag + 2, endTag - 2 ) );
				i_writer( replc.data(), replc.size() );
				last = ptr + 1;
			}
			startTag = nullptr;
		}
		else if ( *ptr == '{' and lastChar == '<' and startTag == nullptr ) // <{
		{
			startTag = ptr - 1;
			if ( endOpenSectionTag == nullptr )
				i_writer( last, startTag - last );
		}

		lastChar = *ptr;
		++ptr;
	}
	
	// write the remaining, if we're not in a tag
	if ( startTag == nullptr and endOpenSectionTag == nullptr )
		i_writer( last, ptr - last );
}

std::string CodeTemplate::resolveName( const std::string &i_name )
{
	for ( auto m : _context )
	{
		auto it = m.names.find( i_name );
		if ( it != m.names.end() )
			return it->second();
	}
	return std::string();
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
