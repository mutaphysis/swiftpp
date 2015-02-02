#include "classes.h"

void Simple::method1()
{
	_test = "this ";
}

void Simple::method2()
{
	_test += "is ";
}

void Simple::method3()
{
	_test += "cor";
	printf( "%s", _test.c_str() );
}

void Simple::method4()
{
	printf( "%s\n", "rect" );
}
