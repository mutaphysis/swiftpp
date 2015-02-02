//
//  substringref.cpp
//  swiftpp
//
//  Created by Sandy Martel on 30/01/2015.
//  Copyright (c) 2015 dootaini. All rights reserved.
//

#include "substringref.h"

bool substringref::operator==( const substringref &i_other ) const
{
	if ( (end() - begin()) != (i_other.end() - i_other.begin()) )
		return false; // length is different
	
	auto ptr = begin();
	for ( auto c : i_other )
	{
		if ( c != *ptr )
			return false;
		++ptr;
	}
	return true;
}

void substringref::trim()
{
	while ( _begin != _end and *_begin == '\n' )
		++_begin;
	while ( _end > _begin and *(_end-1) == 0 )
		--_end;
}
