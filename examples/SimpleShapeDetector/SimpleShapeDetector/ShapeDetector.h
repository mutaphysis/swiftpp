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

#define swift __attribute__((annotate("swift")))

/*!
   @brief detect shapes from polylines.
	
       This class just runs a bunch of heuristics, trying to guess what the polyline
	   represent (triangle, rectangle, circle, etc).
	   It is not particularly good at what it is supposed to do :-(
	   
	   Usage:
	   	1- subclass and implement override shapeDetected()
		2- instantiate
		3- call detect with a path
		4- shapeDetected() override will be called with each shape detected
*/
class swift ShapeDetector
{
	public:
		ShapeDetector();
		virtual ~ShapeDetector();
	
		void detect( const Path2D &i_paths );

	protected:
		virtual void shapeDetected( const std::string &i_name, const Path2D &i_path );
	
	private:
		void report( const std::string &i_name, const std::vector<CGPoint> &i_path );
	
		void reportTriangle( const std::vector<CGPoint> &i_path );
		void reportQuad( const std::vector<CGPoint> &i_path );
};

#endif
