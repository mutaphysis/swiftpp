
import Foundation

let s1 = Simple1()
let s2 = Simple2()

s1.method1()
print( "his" + s1.method2(), terminator: "" )
s2.method3()
print( s2.method2(), terminator: "" )
s2.method1()
print( "" )
