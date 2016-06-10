/*
	test: multiple classes & inheritance
*/

#ifndef H_CLASSES
#define H_CLASSES

#include <string>

#define swift __attribute__((annotate("swift")))

class Simple1
{
	public:
		void method1();
		virtual std::string method2();

	protected:
		std::string _test;
};

class swift Simple2 : public Simple1
{
	public:
		void method3();
		virtual std::string method2();
};


#endif
