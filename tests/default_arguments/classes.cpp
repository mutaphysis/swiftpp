#include "classes.h"

OneArgs::OneArgs( int a )
	: _value( a )
{
}

void OneArgs::method()
{
	if ( _value == 1 )
		printf( "this is" );
	else
		printf( "this is not" );
}

TwoArgs::TwoArgs( const std::string &a, int b )
	: _v1( a ), _v2( b )
{
}

void TwoArgs::method()
{
	printf( " %s", _v1.c_str() );
	if ( _v2 == 2 )
		printf( "re" );
	else
		printf( "rupt" );
}

ThreeArgs::ThreeArgs( bool a, const std::string &s, bool b )
	: _v1( a ), _v2( s ), _v3( b )
{
}

void ThreeArgs::method()
{
	printf( "%s\n", _v2.c_str() );
}
