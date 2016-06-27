
swiftpp
=======

A C++ to swift bridge.


Introduction
===========

swiftpp is a clang tool that automatically generate the glue code
to let a swift class inherit from a C++ class.  You can override C++
virtual methods from the swift derivative as well.

Things like this become possible:

```C++
// source.cpp
class swift MyCXXClass
{
	public:
		MyCXXClass();
		virtual ~MyCXXClass();

		virtual void doSomething( const std::string &i_text );
};
```

```
// source.swift
class MySwiftClass : MyCXXClass
{
	override func doSomething( text: String )
	{
		// ...
		super.doSomething( text )
	}
}
```

Building
========

You will need a clang/llvm install to be able to compile swiftpp
itself. Look in setup_llvm.txt for instructions (or run
setup_llvm.txt as a shell script). I worked with llvm trunk and try
to use the latest Xcode to track the latest swift version. When
things get more stable, I'll try to settle on an llvm version. As of
this writing I have llvm revision #272119 and Xcode 7.3.1.

If clang/llvm is installed at the same path as in setup_llvm.txt
(/opt/llvm), you can then compile swiftpp. CMake is the build system
used, just run:
```
cmake -G Xcode
```
to generate an Xcode project. Open and compile swiftpp.xcodeproj.
If llvm/clang development libraries are installed elsewhere, edit
CMakeList.txt accordingly.

That's it, you should now have a tool named swiftpp at the repository
root level.

Note that if you do an non-optimised (debug) build of clang/llvm, the
swiftpp pre-compiler might run very slow.


Usage
=====


Type mapping
============


User defined converters
=======================


todo:

Just like the Objective-C-to-swift bridge rely on a special header
(PROJECT_NAME-Bridging-Header.h) to find what Objective-C classes
you are exporting to swift, swiftpp rely on a header
(cxx-Bridging-Header.h in the example) to know what C++ classes
should be made visible to the Objective-C/swift side.

cxx-Bridging-Header.h will be parsed by swiftpp and every
class that are annotated with the "swiftpp" keyword will be exported.

example:

```C++
class __attribute__((annotate("swiftpp"))) MyCXXClass { ... };
```

 * You can use a #define swiftpp __attribute__((annotate("swiftpp"))) to
 simplify the syntax

Then, you will be able to instanciate or subclass MyCXXClass in swift.

swiftpp will also use every unary function defined in the namespace
swift_converter as data converter to/from Objective-C/swift compatible type.

example:

```C++
namespace swift_converter
{
NSBezierPath *to( const Path2D &s ); // Path2D is a C++ class that represent a Bezier path
Path2D from( NSBezierPath *s );
}
```

If your exported class (MyCXXClass) uses types that are not compatible
with the swift world, they will be auto converted using those converters.
Converter names are not important, type inference is used to select the
right one.  There are also some builtin conversions:
 - C types
 - C arrays
 - std::string
 - std::wstring
 - std::u16string
 - std::u32string
 - std::string_view
 - std::wstring_view
 - std::vector
 - std::array
 - std::any
 - std::tuple
 - std::variant
 - std::map
 - std::queue
 - std::stack
 - std::unordered_map
 - std::list
 - std::set
 - std::forward_list
 - std::chrono::duration
 - std::chrono::time_point
 - std::shared_ptr
 - std::unique_ptr


If swiftpp is already built you can try an extremely basic example in
examples/SimpleShapeDetector


Project setup
=============

This is the complicated part! You can always refer to
examples/SimpleShapeDetector to see how it's done...

1. swiftpp need to be run as a "Run Script" build phase before real
   compilation happen.
2. The "Run Script" should be:
	path/to/swiftpp "$PROJECT_DIR/path-to-your-cxx-Bridging-Header.h"
4. swiftpp will output 4 source files (cxx-objc-protocols.h,
   cxx-objc-proxies.h, cxx-objc-proxies.mm and cxx-subclasses.mm). Those
   need to be added back to your project.
5. cxx-objc-proxies.h need to be #imported in the Objective-C bridge
   header (PROJECT_NAME-Bridging-Header.h)

Those are one-time project setup, should work from now on, everytime you
compile.

Refer to examples/SimpleShapeDetector !

Notes on performance
====================


License
=======

Provided under the MIT license.


Comments are welcome.
