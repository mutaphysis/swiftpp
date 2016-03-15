// generated cxx-objc-proxies.mm

#import "cxx-objc-proxies.h"
#include "ShapeDetector.h"
#include "cxx-converter.h"

class ShapeDetector_subclass : public ShapeDetector
{
public:
	const void *_self = nullptr;
	void (*_shapeDetected_callback)(const void * ,const char *,NSBezierPath* );

  ShapeDetector_subclass()
   : ShapeDetector( ){}

  virtual void shapeDetected( const std::string & i_name, const class Path2D & i_path )
  {
	(*_shapeDetected_callback)( _self , i_name.c_str(), swift_converter::to( i_path ) );
  }
	
  void shapeDetected_forward( const std::string & i_name, const class Path2D & i_path )
  {
  	ShapeDetector::shapeDetected( i_name, i_path );
  }
};


const void *ShapeDetector_proxy_allocate()
{
	return new ShapeDetector_subclass();
}
void ShapeDetector_proxy_init( const void *i_this,
								const void *i_self,
								void (*i_shapeDetected_callback)(const void * ,const char *,NSBezierPath*) )
{
	((ShapeDetector_subclass*)i_this)->_self = i_self;
	((ShapeDetector_subclass*)i_this)->_shapeDetected_callback = i_shapeDetected_callback;
}

void ShapeDetector_proxy_destroy( const void *i_this )
{
	delete ((ShapeDetector_subclass*)i_this);
}

void ShapeDetector_proxy_shapeDetected( const void *i_this, const char *i_name, NSBezierPath *i_path )
{
	((ShapeDetector_subclass*)i_this)->shapeDetected_forward( i_name, swift_converter::from( i_path ) );
}

void ShapeDetector_proxy_detect( const void *i_this, NSBezierPath *i_path )
{
	((ShapeDetector_subclass*)i_this)->ShapeDetector::detect( swift_converter::from( i_path ) );
}

