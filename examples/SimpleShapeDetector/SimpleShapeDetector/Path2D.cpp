//
//  Path2D.cpp
//  ShapeDetect
//
//  Created by Sandy Martel on 2014/07/07.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

#include "Path2D.h"
#include "GeometryUtils.h"

void Path2D::move_to( const CGPoint &pt )
{
	_pts.push_back( pt );
	_cmds.push_back( path_cmd_move_to );
}

void Path2D::line_to( const CGPoint &pt )
{
	assert( not _pts.empty() );
	_pts.push_back( pt );
	_cmds.push_back( path_cmd_line_to );
}

void Path2D::curve_to( const CGPoint &pt1, const CGPoint &pt2, const CGPoint &pt )
{
	assert( not _pts.empty() );
	_pts.push_back( pt1 );
	_pts.push_back( pt2 );
	_pts.push_back( pt );
	_cmds.push_back( path_cmd_curve );
	_cmds.push_back( path_cmd_curve );
	_cmds.push_back( path_cmd_curve );
}

void Path2D::close()
{
	if ( _pts.empty() )
		return;
	
	// find the previous move_to
	auto cmdIt = _cmds.end() - 1;
	while ( cmdIt != _cmds.begin() and *cmdIt != path_cmd_move_to )
		--cmdIt;
	auto prevMoveTo = _pts[std::distance( _cmds.begin(), cmdIt )];
	_pts.push_back( prevMoveTo );
	_cmds.push_back( path_cmd_close );
}

bool Path2D::point( size_t i, CGPoint &o_pt, path_command_t &o_cmd ) const
{
	if ( i < _pts.size() )
	{
		o_pt = _pts[i];
		o_cmd = static_cast<path_command_t>( _cmds[i] );
		return true;
	}
	else
		return false;
}

Path2D Path2D::circle( CGFloat r, const CGPoint &i_center )
{
	Path2D p;
	r = std::abs( r );
	CGFloat c = r * 0.551915024494;
	p.move_to( i_center + CGPoint{ r, 0 } );
	p.curve_to( i_center + CGPoint{ r, c }, i_center + CGPoint{ c, r }, i_center + CGPoint{ 0, r } );
	p.curve_to( i_center + CGPoint{ -c, r }, i_center + CGPoint{ -r, c }, i_center + CGPoint{ -r, 0 } );
	p.curve_to( i_center + CGPoint{ -r, -c }, i_center + CGPoint{ -c, -r }, i_center + CGPoint{ 0, -r } );
	p.curve_to( i_center + CGPoint{ c, -r }, i_center + CGPoint{ r, -c }, i_center + CGPoint{ r, 0 } );
	p.close();
	return p;
}
