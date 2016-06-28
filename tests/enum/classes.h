/*
	test: constructors with params
*/

#ifndef H_CLASSES
#define H_CLASSES

#include <string>

#define swiftpp __attribute__((annotate("swiftpp")))

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

class swiftpp Simple1
{
	public:
		Simple1();

		Weekday_t method( int index );
};

#endif
