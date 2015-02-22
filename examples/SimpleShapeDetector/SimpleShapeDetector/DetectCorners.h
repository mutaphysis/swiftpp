/*
 *  DetectCorners.h
 *  sgeom
 *
 *  Created by Sandy Martel on 11/01/15.
 *  Copyright 2011 by Sandy Martel. All rights reserved.
 *
 */

#ifndef H_DETECTCORNERS
#define H_DETECTCORNERS

#include <CoreGraphics/CoreGraphics.h>
#include <deque>
#include <vector>

// simple C++ code that detect sharp corners in a polyline

void detectCorners( const std::deque<CGPoint> &i_path, bool i_closedPath, std::vector<size_t> &o_corners, int W );

std::deque<CGPoint> uniformResample( const std::vector<CGPoint> &v );

#endif
