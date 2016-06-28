
import Foundation

class MySimple : Simple
{
	override func method2() -> String
	{
		return "is " + super.method2()
	}
}

let s1 = MySimple()
s1.method1()
MySimple.method3( s: s1.text() )
