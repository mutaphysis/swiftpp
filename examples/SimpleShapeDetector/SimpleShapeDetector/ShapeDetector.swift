//
//  ShapeDetector.swift
//  SimpleShapeDetector
//
//  Created by Sandy Martel on 13/03/2016.
//  Copyright © 2016 sm. All rights reserved.
//

import Foundation

class ShapeDetector
{
	let _super : UnsafePointer<Void>
	
	init()
	{
		self._super = ShapeDetector_proxy_allocate()
		ShapeDetector_proxy_init( self._super,
						UnsafePointer(Unmanaged.passUnretained(self).toOpaque()),
						{ ( i_self: UnsafePointer<Void>, name: UnsafePointer<CChar>, path: NSBezierPath! ) in
							// get self
							let _self  = Unmanaged<ShapeDetector>.fromOpaque( COpaquePointer(i_self) ).takeUnretainedValue()
							_self.shapeDetected( String.fromCString( name )!, path: path )
						} )
	}
	
	deinit
	{
		ShapeDetector_proxy_destroy( self._super )
	}
	
	func shapeDetected( name: String, path: NSBezierPath! )
	{
		ShapeDetector_proxy_shapeDetected( self._super, name, path )
	}
	
	func detect( p : NSBezierPath )
	{
		ShapeDetector_proxy_detect( self._super, p )
	}
};
