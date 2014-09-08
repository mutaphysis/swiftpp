swiftpp
=======

An attempt at a C++ to swift bridge.


Intoduction
===========

swiftpp is a clang tool that automatically generate the Objective-C++ glue code to let a swift class inherit from a C++ class.  You can override C++ virtual methods from the swift derivative as well.


Building
========

You will need a clang/llvm install to be able to compile swiftpp itself.  Look in setup_llvm.txt for  instructions. I worked with trunk revision #217018, should compile with clang/llvm release 3.5.

If clang/llvm is installed at the same path as in setup_llvm.txt (/opt/llvm), you can then open and compile swiftpp.xcodeproj.

That's it, you should now have a tool named swiftpp at the repository root level.


Usage
=====

Just like the Objective-C-to-swift bridge rely on a special header (PROJECT_NAME-Bridging-Header.h) to find what Objective-C classes you are exporting to swift, swiftpp rely on a header (cxx-Bridging-Header.h) to now what C++ classes should be made visible to the Objective-C/swift side.

cxx-Bridging-Header.h will be parsed by swiftpp and every class that are annotated with the "swift" keyword will be exported.

example:

class __attribute__((annotate("swift"))) MyCXXClass { ... };

 * use a #define swift __attribute__((annotate("swift"))) to simplify the syntax

Then, you will be able to instanciate or subclass MyCXXClass.

swiftpp will also use every unary function defined in the namespace swift_converter as data converter to/from Objective-C/swift compatible type.

example:

namespace swift_converter
{
NSBezierPath *to( const Path2D &s ); // Path2D is a C++ class that represent a Bezier path
Path2D from( NSBezierPath *s );
}

If your exported class (MyCXXClass) uses types that are not compatible with the swift world, they will be auto converted using those converters.  Converter names are not important, type inference is used to select the right one.  std::string converters are built-in.

If swiftpp is already build you can try a simple example in examples/SimpleShapeDetector


Project setup
=============

This is the complicated part! You can always refer to examples/SimpleShapeDetector to see how it's done...

1- swiftpp need to be run as a "Run Script" build phase before real compilation happen.
2- simply adding a "Run Script" before "Compile Sources" doest not work in Xcode6-beta7, thanks to some bug. Add a dummy "Copy File" between "Run Script" and "Compile Sources". See examples/SimpleShapeDetector
3- The "Run Script" should be:
	path/to/swiftpp '-Dswift=__attribute__((annotate("swift")))' "$SCRIPT_INPUT_FILE_0"
	and you set cxx-Bridging-Header.h as the input file.
4- swiftpp will output 4 source files (cxx-objc-protocols.h, cxx-objc-proxies.h, cxx-objc-proxies.mm and cxx-subclasses.mm). Those need to be added back to your project.
5- cxx-bridge/cxx-objc-proxies.h need to be #imported in the Objective-C bridge header (PROJECT_NAME-Bridging-Header.h)

Those are one-time project setup, should work from now on.


Note
====

