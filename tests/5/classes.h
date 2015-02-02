/*
	test: constructors with params
*/

#ifndef H_CLASSES
#define H_CLASSES

#include <string>

#define swift __attribute__((annotate("swift")))

enum class Weekday_t
{
	kMonday = 0,
	kTuesday,
	kWednesday,
	kThursday,
	kFriday,
	kSaturday,
	kSunday
};

class swift Simple1
{
	public:
		Simple1();
		
		Weekday_t method( int index );
};

#endif
