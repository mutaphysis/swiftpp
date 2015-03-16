//
//  substringref.cpp
//  swiftpp
//
//  Created by Sandy Martel on 30/01/2015.
//  Copyright (c) 2015 Sandy Martel. All rights reserved.
//

#include "substringref.h"

bool substringref::operator==( const substringref &i_other ) const
{
	if ( size() != i_other.size() )
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
