/*
 *  DetectCorners.cpp
 *  sgeom
 *
 *  Created by Sandy Martel on 11/01/15.
 *  Copyright 2011 by Sandy Martel. All rights reserved.
 *
 */

#include "DetectCorners.h"
#include "GeometryUtils.h"
#include <numeric>

class Straws
{
	public:
		typedef std::vector<CGFloat>::const_iterator const_iterator;
		
		Straws( const std::deque<CGPoint> &i_path, int W );
		
		inline const_iterator begin() const { return _straws.begin(); }
		inline const_iterator end() const { return _straws.end(); }
		
		CGFloat median() const;
		
	private:
		std::vector<CGFloat> _straws;
};

Straws::Straws( const std::deque<CGPoint> &i_path, int W )
{
	_straws.resize( i_path.size() );
	auto straw = _straws.begin();
	auto p1 = i_path.end() - W;
	auto p2 = i_path.begin() + W;
	for ( int i = 0; i < W; ++i )
	{
		*straw = squareLength( *p1 - *p2 );
		++straw;
		++p1;
		++p2;
	}
	p1 = i_path.begin();
	for ( ; p2 != i_path.end(); ++p1, ++p2 )
	{
		*straw = squareLength( *p1 - *p2 );
		++straw;
	}
	p2 = i_path.begin();
	for ( int i = 0; i < W; ++i )
	{
		*straw = squareLength( *p1 - *p2 );
		++straw;
		++p1;
		++p2;
	}
}

CGFloat Straws::median() const
{
	std::vector<CGFloat> vec( _straws );
	std::sort( vec.begin(), vec.end() );
	return vec[vec.size()/2];
}

void DetectCorners( const std::deque<CGPoint> &i_path, bool i_closedPath, std::vector<size_t> &o_corners, int W )
{
	// give us some space, if the path is too small compared to the window, just return
	if ( i_path.size() < (W * 4) )
		return;
	
	Straws straws( i_path, W );
	
	// median
	auto t = straws.median() * 0.8;
	
	Straws::const_iterator b, e;
	if ( i_closedPath )
	{
		b = straws.begin();
		e = straws.end();
	}
	else
	{
		b = straws.begin() + 1;
		e = straws.end() - W - 1;
	}
	// cannot start in a low region
	auto it = b;
	while ( it != e and *it < t )
		++it;
	while ( it != e )
	{
		if ( *it < t )
		{
			auto localMin = std::numeric_limits<CGFloat>::infinity();
			auto localMinIndex = it;
			while ( it != e and *it < t )
			{
				if ( *it < localMin )
				{
					localMin = *it;
					localMinIndex = it;
				}
				++it;
			}
			if ( it == e )
			{
				if ( i_closedPath )
				{
					// need to wrap to begining
					it = b;
					while ( it != e and *it < t )
					{
						if ( *it < localMin )
						{
							localMin = *it;
							localMinIndex = it;
						}
						++it;
					}
					o_corners.push_back( std::distance( straws.begin(), localMinIndex ) );
					it = e;	// to get out of the loop
				}
				// ignore for non-closed line
			}
			else
				o_corners.push_back( std::distance( straws.begin(), localMinIndex ) );
		}
		else
			++it;
	}
}

std::deque<CGPoint> uniformResample( const std::vector<CGPoint> &v )
{
	std::deque<CGPoint> r;
	
	for ( auto pt : v )
	{
		if ( r.empty() )
			r.push_back( pt );
		else
		{
			auto from = r.back();
			auto l = length( pt - from );
			auto nb = l / 2;
			
			for ( int i = 1; i < nb; ++i )
			{
				auto f = i / nb;
				if ( f < 1 )
					r.push_back( from + (f * (pt - from)) );
			}
			r.push_back( pt );
		}
	}
	
	return r;
}

