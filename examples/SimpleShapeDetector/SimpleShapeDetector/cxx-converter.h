#ifndef H_SHAPEDETECT_CPP_CONVERTER
#define H_SHAPEDETECT_CPP_CONVERTER

#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include "Path2D.h"

namespace swift_converter
{

//  converters take one argument and return a value

NSBezierPath *to( const Path2D &s );
Path2D from( NSBezierPath *s );

}

#endif
