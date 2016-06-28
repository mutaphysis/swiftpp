#!/usr/bin/perl

use File::Basename;
use Term::ANSIColor;

$root = dirname( $0 );

my @tests = grep { -d } glob "$root/*";

$SWIFTPP = "$root/../swiftpp";
$SDKPATH = `xcrun --show-sdk-path`;
chomp $SDKPATH;
$SWIFTCFLAGS = "-target x86_64-apple-macosx10.11 -Onone -g -F $SDKPATH/System/Library/Frameworks -I$SDKPATH/usr/include";
$CPPFLAGS = "-std=c++11 -mmacosx-version-min=10.11 -isysroot $SDKPATH";
$LDFLAGS = "-L$SDKPATH/../../../../../Toolchains/XcodeDefault.xctoolchain/usr/lib/swift_static/macosx -fobjc-link-runtime -lc++ -isysroot $SDKPATH -mmacosx-version-min=10.11 -Xlinker -force_load_swift_libs -lcurses -Xlinker -force_load -Xlinker $SDKPATH/../../../../../Toolchains/XcodeDefault.xctoolchain/usr/lib/arc/libarclite_macosx.a";

foreach( @tests )
{
	$test = $_;
	`rm -rf $test/app $test/*.o $test/classes`;

	@data = grep { chomp; } `cat $test/readme.txt`;
	print( color('blue') );
	print( "$data[0]\n" );
	print( color('reset') );

	print( "  --> compiling C++\n" );
	`xcrun cc -x c++ -c $test/classes.cpp $CPPFLAGS -o $test/classes.o`;
	print( "  --> running swiftpp\n" );
	`$SWIFTPP $test/classes.h`;
	print( "  --> compiling c_impl\n" );
	`xcrun cc -x c++ -c $test/classes/c_impl.cpp -I. -I$test $CPPFLAGS -o $test/c_impl.o`;
	print( "  --> compiling swift\n" );
	`cd $test ; xcrun swiftc $SWIFTCFLAGS -module-name test -c main.swift -c classes/bridge.swift -import-objc-header bridging-header.h`;
	print( "  --> linking\n" );
	`xcrun c++ $test/classes.o $test/c_impl.o $test/main.o $test/bridge.o -o $test/app $LDFLAGS -fobjc-arc`;

	my $result = `./$test/app`;
	chomp $result;
	if ( $result eq "this is correct" )
	{
		print( color('green') );
		print( "$result\n" );
		print( color('reset') );
	}
	else
	{
		print( color('red') );
		print( "$result\n" );
		print( color('reset') );
	}
	print( "\n" );
}
