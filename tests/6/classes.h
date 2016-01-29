/*
	test: non-virtual, virtual not overriden and static
*/

#ifndef H_CLASSES
#define H_CLASSES

#include <string>

#define swift __attribute__((annotate("swift")))

class swift Simple
{
	public:
		virtual std::string method1( const std::string &i_s );
};


#endif
