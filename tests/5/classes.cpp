#include "classes.h"
#include <cassert>

Simple1::Simple1()
{
}

Weekday_t Simple1::method( int index )
{
	switch ( index )
	{
		case 1: return Weekday_t::kMonday;
		case 2: return Weekday_t::kTuesday;
		case 3: return Weekday_t::kWednesday;
		case 4: return Weekday_t::kThursday;
		case 5: return Weekday_t::kFriday;
		case 6: return Weekday_t::kSaturday;
	}
	return Weekday_t::kSunday;
}
