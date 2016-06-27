

class MySimple : Simple
{
    override func method1( s : String ) -> String
    {
	    if ( s == "This" )
	    {
		    return s + super.method1( s: " " )
		}
	    else if ( s == "is" )
		{
			return s + super.method1( s: " corr" )
		}
		else
		{
			return s
		}
    }
}

print( "\u{1B}[32m" )
print( "--> 6. re-entrency" )

let s1 = MySimple()
print( s1.method1( s: "This" ) )

print( "\u{1B}[0m" )
