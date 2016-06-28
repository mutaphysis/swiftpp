
import Foundation

let oneArgs = OneArgs( a: 1 )
let twoArgs = TwoArgs( a: "cor", b: 2 )
let threeArgs1 = ThreeArgs( a: true )
let threeArgs2 = ThreeArgs( a: false, s: "ct" )
let threeArgs3 = ThreeArgs( a: true, s: "other", b: true )

oneArgs.method();
twoArgs.method();
threeArgs2.method();
