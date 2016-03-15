// generated cxx-objc-proxies.h
//  pure Objective-C, cannot contain any C++

#ifndef H_CXX_OBJC_PROXIES
#define H_CXX_OBJC_PROXIES

#include <AppKit/AppKit.h>

#ifdef __cplusplus
extern "C" {
#endif

const void *ShapeDetector_proxy_allocate();
void ShapeDetector_proxy_init( const void *i_cxxptr,
								const void *i_self,
								void (*i_shapeDetected_callback)(const void * ,const char *,NSBezierPath* ) );
void ShapeDetector_proxy_shapeDetected( const void *i_cxxptr, const char *i_name, NSBezierPath *i_path );
void ShapeDetector_proxy_detect( const void *i_cxxptr, NSBezierPath *i_path );
void ShapeDetector_proxy_destroy( const void *i_cxxptr );

#ifdef __cplusplus
}
#endif

#endif
