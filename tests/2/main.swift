
import Foundation

class MySimple : Simple
{
	override func method2() -> String
	{
		return "is " + super.method2()
	}
}

print( "\u{1B}[32m" )
print( "--> 2. overriden virtual with call to super, swift -> C++ -> swift -> C++" )

let s1 = MySimple()
s1.method1()
MySimple.method3( s: s1.text() )

print( "\u{1B}[0m" )
