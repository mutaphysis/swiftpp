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
#include <iostream>

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
	
	bool matchingCloseTag( const substringref &a, const substringref &b )
	{
		// a is an open tag, strip all attribures
		auto col = std::find( a.begin(), a.end(), ':' );
		if ( col == a.end() )
			return a == b;
		else
			return substringref( a.begin(), col ) == b;
	}
	void parseTag( const substringref &i_tag, substringref &o_name, std::unordered_map<std::string,std::string> &o_attributes )
	{
		o_attributes.clear();
		auto col = std::find( i_tag.begin(), i_tag.end(), ':' );
		if ( col == i_tag.end() )
		{
			o_name = i_tag;
			return;
		}
		o_name = substringref( i_tag.begin(), col );
		
		auto attrNameStart = col + 1;
		auto attrStart = attrNameStart;
		bool inBracket = false;
		std::string current;
		for ( auto ptr = attrNameStart; ptr != i_tag.end(); ++ptr )
		{
			switch ( *ptr )
			{
				case '(':
					if ( not inBracket )
					{
						if ( ptr == attrNameStart )
							return; // malformed, empty name
						attrStart = ptr;
						inBracket = true;
					}
					break;
				case ')':
					o_attributes[std::string(attrNameStart,attrStart)] = current;
					attrNameStart = ptr + 1;
					inBracket = false;
					current.clear();
					break;
				case '\\':
					if ( inBracket and (ptr+1) != i_tag.end() )
					{
						++ptr;
						switch ( *ptr )
						{
							case ')': current.push_back( ')' ); break;
							case '\\': current.push_back( '\\' ); break;
							case 'r': current.push_back( '\r' ); break;
							case 'n': current.push_back( '\n' ); break;
							case 't': current.push_back( '\t' ); break;
							default: break;
						}
					}
					break;
				default:
					if ( inBracket )
						current.push_back( *ptr );
					break;
			}
		}
	}
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

void CodeTemplate::render( const CodeTemplateModel &i_model, ostream &ostr )
{
	_context.push_front( i_model );
	//dumpContext();
	render( _tmpl, ostr );
	
	// cleanup context
	_context.clear();
}

void CodeTemplate::render( const substringref &i_tmpl, ostream &ostr )
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
				substringref openTag( startOpenSectionTag + 3, endOpenSectionTag - 2 );
				if ( startTag[2] == '/' and matchingCloseTag( openTag, substringref( startTag + 3, endTag - 2 ) ) )
				{
					// this allow nicer formatting in the template
					if ( *(startOpenSectionTag-1) == '\n' and *endOpenSectionTag == '\n' )
						++endOpenSectionTag;

					std::unordered_map<std::string,std::string> attributes;
					substringref sectionName;
					parseTag( openTag, sectionName, attributes );
					
					std::string sep, prefix;
					auto it = attributes.find( "prefix" );
					if ( it != attributes.end() )
						prefix = it->second;
					it = attributes.find( "separator" );
					if ( it != attributes.end() )
						sep = it->second;
					
					CodeTemplateModel m;
					size_t i = 0;
					while ( resolveSection( sectionName, i, m ) )
					{
						if ( i == 0 )
							ostr << prefix;
						else
							ostr << sep;
						_context.push_front( m );
						//dumpContext();
						render( substringref( endOpenSectionTag, startTag ), ostr );
						_context.pop_front();
						//dumpContext();
						++i;
						m.clear();
					}
					startOpenSectionTag = endOpenSectionTag = nullptr;
					last = endTag;
					if ( *(startTag-1) == '\n' and *endTag == '\n' )
						++last;
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
				std::unordered_map<std::string,std::string> attributes;
				substringref name;
				parseTag( substringref( startTag + 2, endTag - 2 ), name, attributes );
				
				std::string prefix;
				auto it = attributes.find( "prefix" );
				if ( it != attributes.end() )
					prefix = it->second;
				resolveName( prefix, name, ostr );
				last = ptr + 2;
			}
			startTag = nullptr;
			skipCloseTag( ptr );
		}
		else if ( startTag == nullptr and isOpenTag( ptr, i_tmpl.end() ) )
		{
			startTag = ptr;
			if ( endOpenSectionTag == nullptr )
			{
				ostr.write( last, startTag - last );
				//ostr.flush();
			}
			skipOpenTag( ptr );
		}
		else
			++ptr;
	}
	
	// write the remaining, if we're not in a tag
	if ( startTag == nullptr and endOpenSectionTag == nullptr )
		ostr.write( last, ptr - last );
}

void CodeTemplate::resolveName( const std::string &i_prefix, const substringref &i_name, ostream &ostr )
{
	std::string name( i_name.begin(), i_name.end() );
	for ( auto m : _context )
	{
		auto it = m.names.find( name );
		if ( it != m.names.end() )
		{
			ostr << i_prefix;
			if ( it->second.callback )
				it->second.callback( ostr );
			else
				ostr << it->second.text;
			return;
		}
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
			if ( it->second.callback )
				it->second.callback( i_index, o_model );
			return true;
		}
	}
	return false;
}

void CodeTemplate::dumpContext() const
{
	std::cout << "------------" << std::endl;
	for ( auto it = _context.rbegin(); it != _context.rend(); ++it )
	{
		std::cout << "------------" << std::endl;
		it->dump();
	}
}

void CodeTemplateModel::dump( int i_indent ) const
{
	for ( auto &it : sections )
	{
		std::cout << std::string( i_indent, ' ' ) << it.first << " : " << it.second.nb << std::endl;
		for ( size_t i = 0; i < it.second.nb; ++i )
		{
			std::cout << std::string( i_indent + 2, ' ' ) << i << "-" << std::endl;
			if ( it.second.callback )
			{
				CodeTemplateModel m;
				it.second.callback( i, m );
				m.dump( i_indent + 4 );
			}
			else
			{
				std::cout << std::string( i_indent + 4, ' ' ) << "empty" << std::endl;
			}
		}
	}
	for ( auto &it : names )
	{
		std::cout << std::string( i_indent, ' ' ) << it.first << " : ";
		if ( it.second.callback )
		{
			std::string data;
			llvm::raw_string_ostream os( data );
			it.second.callback( os );
			os.flush();
			std::cout << "\"" << data << "\"" << std::endl;
		}
		else
			std::cout << "\"" << it.second.text << "\"" << std::endl;
	}
}
