/*
	test: non-virtual, virtual, static method
*/

#ifndef H_CLASSES
#define H_CLASSES

#include <string>

#define swift __attribute__((annotate("swift")))

class swift Simple
{
	public:
		void method();
	
	private:
		std::string _test;
};


#endif
