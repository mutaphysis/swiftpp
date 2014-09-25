#include "classes.h"

void Simple::method()
{
	_test = "this ";
	_test += "is ";
	_test += "correct";
	printf( "%s\n", _test.c_str() );
}
