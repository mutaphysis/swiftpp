/*
	test: non-virtual, virtual not overriden and static
*/

#ifndef H_CLASSES
#define H_CLASSES

#include <string>

#define swiftpp __attribute__((annotate("swiftpp")))

class swiftpp Simple
{
	public:
		virtual std::string method1( const std::string &i_s );
};


#endif
