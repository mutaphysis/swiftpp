
import Foundation

let s = Simple1()

for i : Int32 in 1...7
{
    switch ( s.method( index: i ) )
    {
		case kMonday:
			print( "Monday" );
		case kTuesday:
			print( "Tuesday" );
		case kWednesday:
			print( "Wednesday" );
		case kThursday:
			print( "Thursday" );
		case kFriday:
			print( "Friday" );
		case kSaturday:
			print( "Saturday" );
		case kSunday:
			print( "Sunday" );
		default:
			break
	}
}
