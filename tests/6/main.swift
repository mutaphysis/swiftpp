

class MySimple : Simple
{
    override func method1( s : String ) -> String
    {
	    if ( s == "this" )
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

let s1 = MySimple()
print( s1.method1( s: "this" ) )
