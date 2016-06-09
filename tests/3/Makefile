
SWIFTPP=../../swiftpp
SDKPATH=$(shell xcrun --show-sdk-path)
SWIFTCFLAGS=-target x86_64-apple-macosx10.11 -Onone -g -F $(SDKPATH)/System/Library/Frameworks -I$(SDKPATH)/usr/include
CPPFLAGS=-std=c++11 -mmacosx-version-min=10.11 -isysroot $(SDKPATH)
LDFLAGS=-L$(SDKPATH)/../../../../../Toolchains/XcodeDefault.xctoolchain/usr/lib/swift_static/macosx -fobjc-link-runtime -lc++ -isysroot $(SDKPATH) -mmacosx-version-min=10.11 -Xlinker -force_load_swift_libs -lswiftRuntime -lcurses -Xlinker -force_load -Xlinker $(SDKPATH)/../../../../../Toolchains/XcodeDefault.xctoolchain/usr/lib/arc/libarclite_macosx.a

all: app
	./app

classes/c_impl.h: classes.h
	$(SWIFTPP) classes.h

main.o: main.swift classes/c_impl.h
	xcrun swiftc $(SWIFTCFLAGS) -module-name test -c main.swift -c classes/bridge.swift -import-objc-header bridging-header.h

classes.o: classes.h classes.cpp
	xcrun cc -x c++ -c classes.cpp $(CPPFLAGS)

c_impl.o: classes/c_impl.h
	xcrun cc -x c++ -c classes/c_impl.cpp -I. $(CPPFLAGS)

app: main.o classes.o c_impl.o bridge.o
	xcrun c++ -o app c_impl.o bridge.o main.o classes.o $(LDFLAGS) -fobjc-arc

clean:
	rm -rf app *.o classes

