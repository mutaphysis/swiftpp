//
//  Path2D.h
//  ShapeDetect
//
//  Created by Sandy Martel on 2014/07/07.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

#ifndef H_PATH2D
#define H_PATH2D

#include <CoreGraphics/CoreGraphics.h>
#include <vector>

class Path2D
{
	public:
	
		void move_to( const CGPoint &pt );
		void line_to( const CGPoint &pt );
		void curve_to( const CGPoint &pt1, const CGPoint &pt2, const CGPoint &pt );
		void close();
	
		static Path2D circle( CGFloat r, const CGPoint &i_center );

		enum path_command_t
		{
			path_cmd_undefined = 0,
			path_cmd_move_to,
			path_cmd_line_to,
			path_cmd_curve,
			path_cmd_close
		};
	
		inline size_t size() const { return _pts.size(); }
		bool point( size_t i, CGPoint &o_pt, path_command_t &o_cmd ) const;

	private:
		std::vector<CGPoint> _pts;
		std::vector<uint8_t> _cmds;
};

#endif
