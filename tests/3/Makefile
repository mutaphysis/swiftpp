
SWIFTPP=../../swiftpp
SDKPATH=$(shell xcrun --show-sdk-path)
SWIFTCFLAGS=-target x86_64-apple-macosx10.11 -Onone -g -F $(SDKPATH)/System/Library/Frameworks -I$(SDKPATH)/usr/include
CPPFLAGS=-std=c++11 -mmacosx-version-min=10.11 -isysroot $(SDKPATH)
LDFLAGS=-L$(SDKPATH)/../../../../../Toolchains/XcodeDefault.xctoolchain/usr/lib/swift_static/macosx -fobjc-link-runtime -lc++ -isysroot $(SDKPATH) -mmacosx-version-min=10.11 -Xlinker -force_load_swift_libs -lswiftRuntime -lcurses -Xlinker -force_load -Xlinker $(SDKPATH)/../../../../../Toolchains/XcodeDefault.xctoolchain/usr/lib/arc/libarclite_macosx.a

all: app
	./app

classes/cxx-objc-proxies.h: classes.h
	$(SWIFTPP) classes.h

test.o: test.swift classes/cxx-objc-proxies.h
	xcrun swiftc $(SWIFTCFLAGS) -module-name test -c test.swift -import-objc-header bridging-header.h

classes.o: classes.h classes.cpp
	xcrun cc -x c++ -c classes.cpp $(CPPFLAGS)

cxx-objc-proxies.o: classes/cxx-objc-proxies.h
	xcrun cc -x objective-c++ -c classes/cxx-objc-proxies.mm $(CPPFLAGS) -fobjc-arc

cxx-subclasses.o: classes/cxx-objc-proxies.h
	xcrun cc -x objective-c++ -c classes/cxx-subclasses.mm -I. $(CPPFLAGS) -fobjc-arc

app: test.o classes.o cxx-objc-proxies.o cxx-subclasses.o
	xcrun c++ -o app test.o classes.o cxx-objc-proxies.o cxx-subclasses.o $(LDFLAGS) -fobjc-arc

clean:
	rm -rf app *.o classes

