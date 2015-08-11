
//
//  ShapeDetector.cpp
//  ShapeDetect
//
//  Created by Sandy Martel on 2014/07/07.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

#include "ShapeDetector.h"
#include "DetectCorners.h"
#include "GeometryUtils.h"
#include <iostream>

namespace
{

const CGFloat kEpsilonForPoints = 10;
const CGFloat kEpsilonForStraightLines = 10;

bool aboutTheSame( CGFloat a, CGFloat b, CGFloat epsilon )
{
	return std::abs( a - b ) < epsilon;
}

bool aboutTheSame( const CGPoint &a, const CGPoint &b, CGFloat epsilon )
{
	return aboutTheSame( a.x, b.x, epsilon ) and aboutTheSame( a.y, b.y, epsilon );
}


}

ShapeDetector::ShapeDetector()
{
}

ShapeDetector::~ShapeDetector()
{
}

template<class T>
bool isStraightLine( T first, T last, CGFloat i_error2 )
{
	CGPoint	v = *first;
	CGPoint	w = *(last-1);
	
	CGPoint wMinusV = w - v;
	const CGFloat l2 = squareLength( wMinusV ); // i.e. |w-v|^2 -  avoid a sqrt
	if ( l2 == 0 )
	{
		// v == w
		assert( false );
	}
	else
	{
		for ( auto it = first + 1; it != (last-1); ++it )
		{
			const CGPoint &p( *it );
			CGFloat t = ((p - v) * wMinusV) / l2;
			if ( t < 0.0 )
			{
				// Beyond the 'v' end of the segment
				if ( squareLength( v - p ) > i_error2 )
					return false;
			}
			else if ( t > 1.0 )
			{
				// Beyond the 'w' end of the segment
				if ( squareLength( w - p ) > i_error2 )
					return false;
			}
			else
			{
				CGPoint projection = v + (t * wMinusV);  // Projection falls on the segment
				if ( squareLength( projection - p ) > i_error2 )
					return false;
			}
		}
	}
	return true;
}

void cleanUpPointsInStraightLines( const std::vector<CGPoint> &i_points, std::vector<CGPoint> &o_expath, CGFloat i_error2 )
{
	size_t nbOfPts = i_points.size();
	if ( nbOfPts < 3 )
	{
		o_expath.assign( i_points.begin(), i_points.end() );
		return;
	}
	
	auto current = i_points.begin();
	o_expath.push_back( *current );
	while ( current != i_points.end() )
	{
		size_t remaining = i_points.end() - current;
		if ( remaining < 3 )
		{
			o_expath.push_back( *current );
			++current;
		}
		else
		{
			auto segEnd = current + 3;
			while ( segEnd != i_points.end() and isStraightLine( current, segEnd, i_error2 ) )
			{
				++segEnd;
			}
			current = segEnd;
			--segEnd;	// back track
			o_expath.push_back( *segEnd );
		}
	}
}

void splitInSegments( const Path2D &i_paths, std::vector<std::vector<CGPoint>> &o_segments )
{
	size_t i = 0;
	CGPoint pt;
	Path2D::path_command_t cmd;
	while ( i_paths.point( i, pt, cmd ) )
	{
		switch ( cmd )
		{
			case Path2D::path_cmd_move_to:
				o_segments.push_back( std::vector<CGPoint>() );
				o_segments.back().push_back( pt );
				break;
			case Path2D::path_cmd_line_to:
				o_segments.back().push_back( pt );
				break;
			case Path2D::path_cmd_close:
				o_segments.back().push_back( pt );
				break;
			default:
				break;
		}
		++i;
	}
}

bool looksLikeACircle( const std::vector<CGPoint> &i_path, CGPoint &o_center, CGFloat &o_radius )
{
	CGPoint bottomLeft{ std::numeric_limits<CGFloat>::max(), std::numeric_limits<CGFloat>::max() };
	CGPoint topRight{ std::numeric_limits<CGFloat>::min(), std::numeric_limits<CGFloat>::min() };
	
	for ( auto it : i_path )
	{
		if ( it.x < bottomLeft.x )
			bottomLeft.x = it.x;
		if ( it.y < bottomLeft.y )
			bottomLeft.y = it.y;
		if ( it.x > topRight.x )
			topRight.x = it.x;
		if ( it.y > topRight.y )
			topRight.y = it.y;
	}
	
	o_center = CGPoint{ (bottomLeft.x+topRight.x)/2, (bottomLeft.y+topRight.y)/2 };
	
	CGFloat maxRadius = std::numeric_limits<CGFloat>::min();
	CGFloat minRadius = std::numeric_limits<CGFloat>::max();
	for ( auto it : i_path )
	{
		CGFloat l2 = length( it - o_center );
		if ( l2 > maxRadius )
			maxRadius = l2;
		if ( l2 < minRadius )
			minRadius = l2;
	}
	
	if ( aboutTheSame( minRadius, maxRadius, 25 ) )
	{
		CGFloat total = 0;
		for ( auto it : i_path )
			total += length( it - o_center );
		o_radius = total / i_path.size();
//		for ( auto it : i_path )
//		{
//			auto d = length( it - o_center );
//			if ( not aboutTheSame( d, o_radius, 10 ) )
//				return false;
//		}
		return true;
	}
	return false;
}

void ShapeDetector::detect( const Path2D &i_paths )
{
	// split in segments
	std::vector<std::vector<CGPoint>> segments;
	splitInSegments( i_paths, segments );

	// all different segements, the bool tells if it's a closed path or not
	std::vector<std::pair<std::vector<CGPoint>,bool>> allSegments;
	for ( auto segment : segments )
	{
		if ( segment.size() < 3 )
			continue;
		
		if ( aboutTheSame( segment.front(), segment.back(), kEpsilonForPoints ) )
		{
			// first and last are about the same, a closed segment
			segment.back() = segment.front();
			allSegments.push_back( std::make_pair( segment, true ) );
		}
		else
		{
			// try to merge it with a previous segments
			auto it = allSegments.begin();
			for ( ; it != allSegments.end(); ++it )
			{
				if ( it->second )
					continue; // skip closed ones
				
				if ( aboutTheSame( segment.front(), it->first.front(), kEpsilonForPoints ) )
				{
					it->first.insert( it->first.begin(), segment.rbegin(), segment.rend() );
					break;
				}
				else if ( aboutTheSame( segment.front(), it->first.back(), kEpsilonForPoints ) )
				{
					it->first.insert( it->first.end(), segment.begin(), segment.end() );
					break;
				}
				else if ( aboutTheSame( segment.back(), it->first.front(), kEpsilonForPoints ) )
				{
					it->first.insert( it->first.begin(), segment.begin(), segment.end() );
					break;
				}
				else if ( aboutTheSame( segment.back(), it->first.back(), kEpsilonForPoints ) )
				{
					it->first.insert( it->first.end(), segment.rbegin(), segment.rend() );
					break;
				}
			}
			if ( it == allSegments.end() )
			{
				// new independant open segment
				allSegments.push_back( std::make_pair( segment, false ) );
			}
			else
			{
				// check if the newly created combined segment is closed
				if ( aboutTheSame( it->first.front(), it->first.back(), kEpsilonForPoints ) )
				{
					it->first.back() = it->first.front();
					it->second = true;
				}
			}
		}
	}
	
	for ( auto it : allSegments )
	{
		std::vector<size_t> corners;

		std::vector<CGPoint> polyline;
		cleanUpPointsInStraightLines( it.first, polyline, kEpsilonForStraightLines * kEpsilonForStraightLines );
		if ( it.second )
			polyline.push_back( polyline.front() );
		
		CGPoint center;
		CGFloat radius;
		if ( it.second and looksLikeACircle( it.first, center, radius ) )
		{
			shapeDetected( "Circle", Path2D::circle( radius, center ) );
		}
		else
		{
			std::deque<CGPoint> resampled = uniformResample( polyline );
			detectCorners( resampled, it.second, corners, 16 );
			
			if ( corners.size() == 0 and not it.second and polyline.size() == 2 )
			{
				report( "line", polyline );
				continue;
			}
			else if ( corners.size() == 3 and it.second and polyline.size() < 9 )
			{
				polyline.clear();
				polyline.push_back( resampled[corners[0]] );
				polyline.push_back( resampled[corners[1]] );
				polyline.push_back( resampled[corners[2]] );
				polyline.push_back( resampled[corners[0]] );
				reportTriangle( polyline );
				continue;
			}
			else if ( corners.size() == 3 and not it.second and polyline.size() < 12 )
			{
				// potential arrow
				//! @todo: improve
				CGFloat a1 = smallestAngle( resampled[0], resampled[corners[0]], resampled[corners[1]] );
				CGFloat a2 = smallestAngle( resampled[corners[0]], resampled[corners[1]], resampled[corners[2]] );
				CGFloat a3 = angle( resampled[corners[1]], resampled[corners[2]], resampled.back() );
				if ( a1 < 80 and a2 < 10 and a3 < 100 )
				{
					//! @todo: cleanup path
					report( "Arrow", it.first );
				}
			}
			else if ( corners.size() == 4 and it.second and polyline.size() < 12 )
			{
				if ( not linesCross( resampled[corners[0]], resampled[corners[1]], resampled[corners[2]], resampled[corners[3]] ) and
							not linesCross( resampled[corners[1]], resampled[corners[2]], resampled[corners[3]], resampled[corners[0]] ) )
				{
					polyline.clear();
					polyline.push_back( resampled[corners[0]] );
					polyline.push_back( resampled[corners[1]] );
					polyline.push_back( resampled[corners[2]] );
					polyline.push_back( resampled[corners[3]] );
					polyline.push_back( resampled[corners[0]] );
					reportQuad( polyline );
					continue;
				}
			}
			report( "unkown", it.first );
		}
	}
}

void ShapeDetector::reportTriangle( const std::vector<CGPoint> &i_path )
{
	assert( i_path.size() >= 3 );
	
	CGFloat a1 = smallestAngle( i_path[0], i_path[1], i_path[2] );
	CGFloat a2 = smallestAngle( i_path[1], i_path[2], i_path[0] );
	CGFloat a3 = smallestAngle( i_path[2], i_path[0], i_path[1] );
	
	// epsilon values determined by tryout
	if ( aboutTheSame( a1, 90, 3 ) )
	{
		//! @todo: cleanup i_path
		report( "Right triangle", i_path );
	}
	else if ( aboutTheSame( a2, 90, 3 ) )
	{
		//! @todo: cleanup i_path
		report( "Right triangle", i_path );
	}
	else if ( aboutTheSame( a3, 90, 3 ) )
	{
		//! @todo: cleanup i_path
		report( "Right triangle", i_path );
	}
	else if ( aboutTheSame( a1, 60, 10 ) and aboutTheSame( a2, 60, 10 ) and aboutTheSame( a3, 60, 10 ) )
	{
		//! @todo: cleanup i_path
		report( "Equilateral triangle", i_path );
	}
	else if ( aboutTheSame( a1, a2, 15 ) )
	{
		//! @todo: cleanup i_path
		report( "Isosceles triangle", i_path );
	}
	else if ( aboutTheSame( a1, a3, 15 ) )
	{
		//! @todo: cleanup i_path
		report( "Isosceles triangle", i_path );
	}
	else if ( aboutTheSame( a2, a3, 15 ) )
	{
		//! @todo: cleanup path
		report( "Isosceles triangle", i_path );
	}
	else
	{
		report( "Triangle", i_path );
	}
}

void ShapeDetector::reportQuad( const std::vector<CGPoint> &i_path )
{
	assert( i_path.size() >= 4 );

	CGFloat a1 = smallestAngle( i_path[0], i_path[1], i_path[2] );
	CGFloat a2 = smallestAngle( i_path[1], i_path[2], i_path[3] );
	CGFloat a3 = smallestAngle( i_path[2], i_path[3], i_path[0] );
	CGFloat a4 = smallestAngle( i_path[3], i_path[0], i_path[1] );
	
	// epsilon values determined by tryout
	if ( aboutTheSame( a1, 90, 15 ) and aboutTheSame( a2, 90, 15 ) and aboutTheSame( a3, 90, 15 ) and aboutTheSame( a4, 90, 15 ) )
	{
		CGFloat l1 = length( i_path[0] - i_path[1] );
		CGFloat l2 = length( i_path[1] - i_path[2] );
		CGFloat l3 = length( i_path[2] - i_path[3] );
		CGFloat l4 = length( i_path[3] - i_path[0] );
		
		if ( aboutTheSame( l1, l2, 15 ) and aboutTheSame( l2, l3, 15 ) and aboutTheSame( l3, l4, 15 ) and aboutTheSame( l4, l1, 15 ) )
		{
			//! @todo: cleanup i_path
			report( "Square", i_path );
		}
		else
		{
			//! @todo: cleanup i_path
			report( "Rectangle", i_path );
		}
	}
	else
	{
		report( "Quadrilateral", i_path );
	}
}

void ShapeDetector::report( const std::string &i_name, const std::vector<CGPoint> &i_path )
{
	Path2D p;
	for ( auto pt : i_path )
	{
		if ( p.size() == 0 )
			p.move_to( pt );
		else
			p.line_to( pt );
	}
	shapeDetected( i_name, p );
}


void ShapeDetector::shapeDetected( const std::string &i_name, const Path2D & /*i_path*/ )
{
	std::cout << i_name << std::endl;
}
