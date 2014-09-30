
SWIFTPP=../../swiftpp
SDKPATH=$(shell xcrun --show-sdk-path)
SWIFTCFLAGS=-target x86_64-apple-macosx10.9 -Onone -g -F $(SDKPATH)/System/Library/Frameworks -I$(SDKPATH)/usr/include
CPPFLAGS=-std=c++11 -mmacosx-version-min=10.9 -isysroot $(SDKPATH)
LDFLAGS=-L$(SDKPATH)/../../../../../Toolchains/XcodeDefault.xctoolchain/usr/lib/swift_static/macosx -fobjc-link-runtime -lc++ -isysroot $(SDKPATH) -mmacosx-version-min=10.9 -Xlinker -force_load_swift_libs -lswiftRuntime -lcurses -Xlinker -force_load -Xlinker $(SDKPATH)/../../../../../Toolchains/XcodeDefault.xctoolchain/usr/lib/arc/libarclite_macosx.a

all:
	$(SWIFTPP) classes.h
	xcrun swiftc $(SWIFTCFLAGS) -module-name test -c test.swift -import-objc-header bridging-header.h
	xcrun cc -x objective-c++ -c classes/cxx-objc-proxies.mm $(CPPFLAGS) -fobjc-arc
	xcrun cc -x objective-c++ -c classes/cxx-subclasses.mm -I. $(CPPFLAGS) -fobjc-arc
	xcrun cc -x c++ -c classes.cpp $(CPPFLAGS)
	xcrun c++ -o app test.o classes.o cxx-objc-proxies.o cxx-subclasses.o $(LDFLAGS) -fobjc-arc
	./app

clean:
	rm -rf app *.o classes

