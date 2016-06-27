/*
	test: constructors with params
*/

#ifndef H_CLASSES
#define H_CLASSES

#include <string>

#define swiftpp __attribute__((annotate("swiftpp")))

class swiftpp OneArgs
{
	public:
		OneArgs( int a );

		void method();

	protected:
		int _value;
};

class swiftpp TwoArgs
{
	public:
		TwoArgs( const std::string &a, int b = 0 );

		void method();

	protected:
		std::string _v1;
		int _v2;
};

class swiftpp ThreeArgs
{
	public:
		ThreeArgs( bool a, const std::string &s = "default", bool b = true );

		void method();

	protected:
		bool _v1, _v3;
		std::string _v2;
};

#endif
