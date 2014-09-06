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
	

	required init(coder: NSCoder)
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

		NSColor.blackColor().set()
		for p in _coolPathList
		{
			p.stroke()
		}
		
		if let p = _currentPath
		{
			NSColor.redColor().set();
			p.stroke()
		}
    }
	
	override func mouseDown(theEvent: NSEvent!)
	{
		if let t = _timer
		{
			t.invalidate()
			_timer = nil
		}
		
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
	
	override func mouseDragged(theEvent: NSEvent!)
	{
		var	pt = self.convertPoint( theEvent.locationInWindow, fromView: nil )
		
		_currentPath!.lineToPoint( pt );
		self.needsDisplay = true
	}

	override func mouseUp(theEvent: NSEvent!)
	{
		var	pt = self.convertPoint( theEvent.locationInWindow, fromView: nil )

		_currentPath!.lineToPoint( pt );
		
		self.needsDisplay = true
		
		_timer = NSTimer.scheduledTimerWithTimeInterval( 1, target:self, selector:"onTimer", userInfo:nil, repeats:false )
	}
	
	class MyShapeDetector : ShapeDetector
	{
		let _view : ShapeView
		
		init( view: ShapeView )
		{
			_view = view
		}
		
		override func shapeDetected( name: String!, path: NSBezierPath! )
		{
			_view._coolPathList.append( path! )
			_view.report( name )
		}
	}
	
	func report( name: String )
	{
		println( name )
		_textView.textStorage.appendAttributedString( NSAttributedString( string: name + "\n" ) )
	}
	
	func onTimer()
	{
		let shapeDetector = MyShapeDetector( view: self )
		shapeDetector.detect( _currentPath )
		_currentPath = nil
		self.needsDisplay = true
	}
}
