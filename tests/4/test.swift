
import Foundation

print( "\u{1B}[32m" )
print( "--> 4. constructors with params & default arguments" )

let oneArgs = OneArgs( with: 1 )
let twoArgs = TwoArgs( with: "cor", b: 2 )
let threeArgs1 = ThreeArgs( with: true )
let threeArgs2 = ThreeArgs( with: false, s: "ct" )
let threeArgs3 = ThreeArgs( with: true, s: "other", b: true )

oneArgs.method();
twoArgs.method();
threeArgs2.method();

print( "\u{1B}[0m" )
