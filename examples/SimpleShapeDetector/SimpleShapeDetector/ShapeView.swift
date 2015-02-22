//
//  ShapeView.swift
//  ShapeDetect
//
//  Created by Sandy Martel on 2014/07/07.
//  Copyright (c) 2014年 Sandy Martel. All rights reserved.
//

import Cocoa


class ShapeView : NSView
{
	var _timer : NSTimer?
	var _currentPath : NSBezierPath?
	var _coolPathList : [NSBezierPath] = []
	
	@IBOutlet var _textView: NSTextView!
	

	required init?(coder: NSCoder)
	{
		super.init(coder: coder)
	}

    override init(frame: NSRect)
	{
        super.init(frame: frame)
    }

    override func drawRect(dirtyRect: NSRect)
	{
        super.drawRect(dirtyRect)
		
		// draw some instructions
		NSAttributedString( string: "Draw some\nsimple shapes" ).drawAtPoint( CGPoint( x: 5, y: 5 ) );

		// draw each already detected paths
		NSColor.blackColor().set()
		for p in _coolPathList
		{
			p.stroke()
		}
		
		// draw the path that the user is currently building
		if let p = _currentPath
		{
			NSColor.redColor().set();
			p.stroke()
		}
    }
	
	override func mouseDown(theEvent: NSEvent)
	{
		// stop the timer
		if let t = _timer
		{
			t.invalidate()
			_timer = nil
		}
		
		// handle input
		var	pt = self.convertPoint( theEvent.locationInWindow, fromView: nil )

		if _currentPath == nil
		{
			_currentPath = NSBezierPath()
			_currentPath!.lineWidth = 10
			_currentPath!.lineCapStyle = .RoundLineCapStyle
			_currentPath!.lineJoinStyle = .RoundLineJoinStyle
		}
		_currentPath!.moveToPoint( pt )
		self.needsDisplay = true
	}
	
	override func mouseDragged(theEvent: NSEvent)
	{
		// just append to the current path and redraw
		var	pt = self.convertPoint( theEvent.locationInWindow, fromView: nil )
		
		_currentPath!.lineToPoint( pt );
		self.needsDisplay = true
	}

	override func mouseUp(theEvent: NSEvent)
	{
		// just append to the current path and redraw
		var	pt = self.convertPoint( theEvent.locationInWindow, fromView: nil )

		_currentPath!.lineToPoint( pt );
		
		self.needsDisplay = true
		
		// start a timer, after some time we'll try to guess what the user did.
		_timer = NSTimer.scheduledTimerWithTimeInterval( 1, target:self, selector:"onTimer", userInfo:nil, repeats:false )
	}
	
	// ShapeDetector does not have the best API
	// but it's there only to demonstrate subclassing C++ classes from swift.
	// note: MyShapeDetector is a swift subclass to C++ ShapeDetector class !
	class MyShapeDetector : ShapeDetector
	{
		let _view : ShapeView
		
		init( view: ShapeView )
		{
			_view = view
		}
		
		// this override will be called for each shape detected by ShapeDetector
		// note: it will be called from the C++ super class! name will be automatically
		// converted from std::string to NSString by swiftpp builtin converter
		// and path it will be converted from a C++ type to NSBezierPath
		// by a user defined converter, see cxx-converter.mm
		override func shapeDetected( name: String!, path: NSBezierPath! )
		{
			// we guessed one shape, record it!
			_view._coolPathList.append( path! )
			_view.report( name )
		}
	}
	
	func report( name: String )
	{
		println( name )
		_textView.textStorage?.appendAttributedString( NSAttributedString( string: name + "\n" ) )
	}
	
	func onTimer()
	{
		// waited long enough, try to guess what the user was drawing
		
		// 1- create a my sub class of shape detector
		let shapeDetector = MyShapeDetector( view: self )

		// 2- call detect: this will call the override for each shape detected
		// note: _currentPath is an NSBezierPath, it will be converted to the appropriate C++
		//  type by a user defined converter, see cxx-converter.mm
		shapeDetector.detect( _currentPath )
		
		// clear current path and redraw
		_currentPath = nil
		self.needsDisplay = true // redraw
	}
}
