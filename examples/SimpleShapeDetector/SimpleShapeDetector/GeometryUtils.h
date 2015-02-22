//
//  CGPointUtils.h
//  ShapeDetect
//
//  Created by Sandy Martel on 2014/07/07.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

#ifndef H_CGPOINTUTILS
#define H_CGPOINTUTILS

#include <CoreGraphics/CoreGraphics.h>
#include <cmath>

// various geometry utilities

inline CGPoint operator-( const CGPoint &a, const CGPoint &b )
{
	return CGPoint{ a.x - b.x, a.y - b.y };
}

inline CGPoint operator+( const CGPoint &a, const CGPoint &b )
{
	return CGPoint{ a.x + b.x, a.y + b.y };
}

inline CGFloat squareLength( const CGPoint &a )
{
	return a.x*a.x + a.y*a.y;
}

inline CGFloat length( const CGPoint &a )
{
	return std::sqrt( squareLength( a ) );
}

inline CGFloat operator*( const CGPoint &a, const CGPoint &b )
{
	return a.x * b.x + a.y * b.y;
}

inline CGPoint operator*( CGFloat s, const CGPoint &a )
{
	return CGPoint{ a.x * s, a.y * s };
}

CGFloat angle( const CGPoint &a, const CGPoint &b, const CGPoint &c );
CGFloat smallestAngle( const CGPoint &a, const CGPoint &b, const CGPoint &c );

bool linesCross( const CGPoint &p1, const CGPoint &p2, const CGPoint &p3, const CGPoint &p4 );

#endif
