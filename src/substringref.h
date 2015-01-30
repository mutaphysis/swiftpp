//
//  substringref.h
//  swiftpp
//
//  Created by Sandy Martel on 30/01/2015.
//  Copyright (c) 2015 dootaini. All rights reserved.
//

#ifndef H_substringref
#define H_substringref

class substringref
{
	public:
		substringref() = default;
		substringref( const char *i_begin, const char *i_end )
			: _begin( i_begin ), _end( i_end ){}
	
		const char *begin() const;
		const char *end() const;
	
		bool operator==( const substringref &i_other ) const;
	
	private:
		const char *_begin = nullptr;
		const char *_end = nullptr;
};

inline const char *substringref::begin() const { return _begin; }
inline const char *substringref::end() const { return _end; }

#endif
