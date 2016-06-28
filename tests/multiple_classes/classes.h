/*
	test: multiple classes & inheritance
*/

#ifndef H_CLASSES
#define H_CLASSES

#include <string>

#define swiftpp __attribute__((annotate("swiftpp")))

class Simple1
{
	public:
		void method1();
		virtual std::string method2();

	protected:
		std::string _test;
};

class swiftpp Simple2 : public Simple1
{
	public:
		void method3();
		virtual std::string method2();
};


#endif
