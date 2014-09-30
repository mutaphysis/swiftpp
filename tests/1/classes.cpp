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
	_test += "correct";
	printf( "%s\n", _test.c_str() );
}
