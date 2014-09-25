
SWIFTPP=../../swiftpp
SDKPATH=$(shell xcrun --show-sdk-path)
SWIFTCFLAGS=-target x86_64-apple-macosx10.9 -Onone -g -F $(SDKPATH)/System/Library/Frameworks -I$(SDKPATH)/usr/include
CPPFLAGS=-std=c++11
LDFLAGS=-L$(SDKPATH)/../../../../../Toolchains/XcodeDefault.xctoolchain/usr/lib/swift/macosx
LIBPATH=$(SDKPATH)/../../../../../Toolchains/XcodeDefault.xctoolchain/usr/lib/swift/macosx

all:
	$(SWIFTPP) classes.h
	xcrun swiftc $(SWIFTCFLAGS) -module-name test -c test.swift -import-objc-header bridging-header.h
	xcrun cc -x objective-c++ -c classes/cxx-objc-proxies.mm
	xcrun cc -x objective-c++ -c classes/cxx-subclasses.mm -I. $(CPPFLAGS)
	xcrun cc -x c++ -c classes.cpp $(CPPFLAGS)
	xcrun c++ -o app test.o classes.o cxx-objc-proxies.o cxx-subclasses.o $(LDFLAGS)
	export DYLD_LIBRARY_PATH=$(LIBPATH)
	./app

clean:
	rm -rf app *.o classes

