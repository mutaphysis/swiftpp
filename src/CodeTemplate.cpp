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
	const char *startTag = nullptr;
	const char *startSectionTag = nullptr;
	const char *endSectionTag = nullptr;
	while ( ptr != i_tmpl.end() )
	{
		if ( startTag != nullptr and *ptr == '>' and lastChar == '}' ) // }>
		{
			auto endTag = ptr + 1;
			if ( endSectionTag != nullptr )
			{
				// looking for the end of a section
				if ( startTag[2] == '/' and substringref( startSectionTag + 3, endSectionTag - 2 ) == substringref( startTag + 3, endTag - 2 ) )
				{
					int i = 0;
					std::string sectionName( startSectionTag + 3, endSectionTag - 2 );
					CodeTemplateModel m;
					while ( resolveSection( sectionName, i, m ) )
					{
						_context.push_front( m );
						render( substringref( endSectionTag, startTag ), i_writer );
						_context.pop_front();
						++i;
					}
					startSectionTag = endSectionTag = nullptr;
					last = endTag;
				}
			}
			else if ( startTag[2] == '#' )
			{
				// begin section
				startSectionTag = startTag;
				endSectionTag = endTag;
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
			if ( endSectionTag == nullptr )
				i_writer( last, startTag - last );
		}

		lastChar = *ptr;
		++ptr;
	}
	
	// write the remaining, if we're not in a tag
	if ( startTag == nullptr and endSectionTag == nullptr )
		i_writer( last, ptr - last );
}

std::string CodeTemplate::resolveName( const std::string &i_name )
{
	std::string result;
	for ( auto m : _context )
	{
		if ( m.resolveName and m.resolveName( i_name, result ) )
			break;
	}
	return result;
}

bool CodeTemplate::resolveSection( const std::string &i_name, int i_index, CodeTemplateModel &o_model )
{
	for ( auto m : _context )
	{
		if ( m.resolveSection and m.resolveSection( i_name, i_index, o_model ) )
			return true;;
	}
	return false;
}
