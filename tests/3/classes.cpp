#include "classes.h"

void Simple1::method1()
{
	_test = "t";
	printf( "%s", _test.c_str() );
}

std::string Simple1::method2()
{
	return " ";
}

void Simple2::method3()
{
	printf( "is" );
}

std::string Simple2::method2()
{
	return Simple1::method2() + "correc";
}
