//
//  ShapeDetector.h
//  ShapeDetect
//
//  Created by Sandy Martel on 2014/07/07.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

#ifndef H_SHAPEDETECTOR
#define H_SHAPEDETECTOR

#include "Path2D.h"
#include <string>
#include <vector>

class swift ShapeDetector
{
	public:
		ShapeDetector();
		virtual ~ShapeDetector();
	
		void detect( const Path2D &i_paths );

//		virtual void detect2( const std::vector<Path2D> &i_paths );
//		virtual void detect3( const std::vector<std::string> &i_paths );
	
	protected:
		virtual void shapeDetected( const std::string &i_name, const Path2D &i_path );
	
	private:
		void report( const std::string &i_name, const std::vector<CGPoint> &i_path );
	
		void reportTriangle( const std::vector<CGPoint> &i_path );
		void reportQuad( const std::vector<CGPoint> &i_path );
};

#endif
