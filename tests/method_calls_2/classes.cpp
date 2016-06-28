#include "classes.h"

void Simple::method1()
{
	_test = "this ";
	_test += method2();
}

std::string Simple::method2()
{
	return "correct";
}

void Simple::method3( const std::string &s )
{
	printf( "%s\n", s.c_str() );
}
