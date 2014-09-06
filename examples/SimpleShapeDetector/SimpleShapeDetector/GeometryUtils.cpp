//
//  GeometryUtils.cpp
//  ShapeDetect
//
//  Created by Sandy Martel on 2014/07/10.
//  Copyright (c) 2014年 dootaini. All rights reserved.
//

#include "GeometryUtils.h"

CGFloat angle( const CGPoint &a, const CGPoint &b, const CGPoint &c )
{
	auto v1 = a - b;
	auto v2 = b - c;
	CGFloat res = atan2( v2.y, v2.x ) - atan2( v1.y, v1.x );
	if ( res < 0 )
		res += 2 * M_PI;
	res = res * 180 / M_PI;
	return res;
}

CGFloat smallestAngle( const CGPoint &a, const CGPoint &b, const CGPoint &c )
{
	auto res = angle( a, b, c );
	if ( res > 180 )
		res -= 180;
	return res;
}

bool linesCross( const CGPoint &p1, const CGPoint &p2, const CGPoint &p3, const CGPoint &p4 )
{
	auto dx1 = p2.x - p1.x;
	auto dy1 = p2.y - p1.y;
	auto dx2 = p4.x - p3.x;
	auto dy2 = p4.y - p3.y;
	
	auto den = dx1 * dy2 - dy1 * dx2;
	if ( std::abs( den ) < 0.001 )
		return false;
	
	// this is the intersection point
	auto v1 = p2.x * p1.y - p2.y * p1.x;
	auto v2 = p4.x * p3.y - p4.y * p3.x;
	auto x = (v1 * dx2 - dx1 * v2) / den;
	auto y = (v1 * dy2 - dy1 * v2) / den;
	
	// compute r1
	CGFloat r1;
	if ( std::abs( dx1 ) > std::abs( dy1 ) or dy1 == 0 )
		r1 = (x - p1.x) / dx1;
	else
		r1 = (y - p1.y) / dy1;
	
	if ( r1 < 0.0 or r1 > 1.0 )
		return false;
	
	// compute r2
	CGFloat r2;
	if ( std::abs( dx2 ) > std::abs( dy2 ) or dy2 == 0 )
		r2 = (x - p3.x) / dx2;
	else
		r2 = (y - p3.y) / dy2;
	
	if ( r2 < 0.0 or r2 > 1.0 )
		return false;
	
	assert( not isnan( r1 ) );
	assert( not isnan( r2 ) );
	
	return true;
}
