/*
	test: virtual & static
*/

#ifndef H_CLASSES
#define H_CLASSES

#include <string>

#define swift __attribute__((annotate("swift")))

class swift Simple
{
	public:
		void method1();
		virtual std::string method2();
		static void method3( const std::string &s );
		
		inline std::string text() const { return _test; }
		
	private:
		std::string _test;
};


#endif
