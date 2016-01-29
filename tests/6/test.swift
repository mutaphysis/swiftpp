

class MySimple : Simple
{
    override func method1( s : String ) -> String
    {
	    if ( s == "This" )
	    {
		    return s + super.method1( " " )
		}
	    else if ( s == "is" )
		{
			return s + super.method1( " corr" )
		}
		
		return ""
    }
}

let s1 = MySimple()
print( s1.method1( "This" ) )
