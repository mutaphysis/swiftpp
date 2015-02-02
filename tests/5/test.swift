
import Foundation

let s = Simple1()

for i : Int32 in 1...7
{
    switch ( s.method( i ) )
    {
		case .kMonday:
			println( "Monday" );
		case .kTuesday:
			println( "Tuesday" );
		case .kWednesday:
			println( "Wednesday" );
		case .kThursday:
			println( "Thursday" );
		case .kFriday:
			println( "Friday" );
		case .kSaturday:
			println( "Saturday" );
		case .kSunday:
			println( "Sunday" );
	}
}
