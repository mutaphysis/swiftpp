//
//  ShapeDetect-cpp-converter.mm
//  ShapeDetect
//
//  Created by Sandy Martel on 2014/07/07.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

#import "cxx-converter.h"

namespace swift_converter
{

NSBezierPath *to( const Path2D &s )
{
	NSBezierPath *b = [NSBezierPath bezierPath];
	size_t i = 0;
	CGPoint pt;
	Path2D::path_command_t cmd;
	while ( s.point( i, pt, cmd ) )
	{
		switch ( cmd )
		{
			case Path2D::path_cmd_move_to:
				[b moveToPoint:pt];
				break;
			case Path2D::path_cmd_line_to:
				[b lineToPoint:pt];
				break;
			case Path2D::path_cmd_curve:
			{
				CGPoint ctrl1 = pt, ctrl2;
				s.point( ++i, ctrl2, cmd );
				s.point( ++i, pt, cmd );
				[b curveToPoint:pt
								controlPoint1:ctrl1
								controlPoint2:ctrl2];
				break;
			}
			case Path2D::path_cmd_close:
				[b closePath];
				break;
			default:
				break;
		}
		++i;
	}
	return b;
}
Path2D from( NSBezierPath *s )
{
	Path2D p;
	
	for ( NSInteger i = 0; i < [s elementCount]; ++i )
	{
		NSPoint pts[3];
		auto cmd = [s elementAtIndex:i associatedPoints:pts];
		switch ( cmd )
		{
			case NSMoveToBezierPathElement:
				p.move_to( pts[0] );
				break;
			case NSLineToBezierPathElement:
				p.line_to( pts[0] );
				break;
			case NSCurveToBezierPathElement:
				p.curve_to( pts[0], pts[1], pts[2] );
				break;
			case NSClosePathBezierPathElement:
				p.close();
				break;
			default:
				break;
		}
	}
	
	return p;
}

}
