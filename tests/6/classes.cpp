#include "classes.h"

std::string Simple::method1( const std::string &i_s )
{
	if ( i_s == " " )
	{
		return i_s + method1( "is" );
	}
	else if ( i_s == " corr" )
	{
		return i_s + method1( "ect" );
	}
	else
	{
		return "";
	}
}
