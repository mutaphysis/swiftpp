
import Foundation

let s = Simple1()

for i : Int32 in 1...7
{
    switch ( s.method( index: i ) )
    {
		case kMonday:
			print( "this", terminator: "" )
		case kTuesday:
			print( " ", terminator: "" )
		case kWednesday:
			print( "is", terminator: "" )
		case kThursday:
			print( " ", terminator: "" )
		case kFriday:
			print( "cor", terminator: "" )
		case kSaturday:
			print( "re", terminator: "" )
		case kSunday:
			print( "ct" )
		default:
			break
	}
}
