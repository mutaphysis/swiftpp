//
//  substringref.h
//  swiftpp
//
//  Created by Sandy Martel on 30/01/2015.
//  Copyright (c) 2015 Sandy Martel. All rights reserved.
//

#ifndef H_substringref
#define H_substringref

#include <cstddef>
#include <cassert>
#include <iterator>

/*!
   @brief simple wrapper for a substring.
*/
class substringref
{
	public:
		substringref() = default;
	
		template<size_t N>
		substringref( const char (&i_array)[N] )
			: _begin( std::begin( i_array ) ), _end( std::end( i_array ) ){}
	
		substringref( const char *i_begin, const char *i_end )
			: _begin( i_begin ), _end( i_end ){}
	
		size_t size() const;
		bool empty() const;
	
		const char *begin() const;
		const char *end() const;
	
		void pop_front();
		void pop_back();
	
		char back() const;
	
		bool operator==( const substringref &i_other ) const;
	
	private:
		const char *_begin = nullptr;
		const char *_end = nullptr;
};

inline size_t substringref::size() const { return end() - begin(); }
inline bool substringref::empty() const { return begin() == end(); }
inline const char *substringref::begin() const { return _begin; }
inline const char *substringref::end() const { return _end; }
inline void substringref::pop_front() { assert( not empty() ); ++_begin; }
inline void substringref::pop_back() { assert( not empty() ); --_end; }
inline char substringref::back() const { assert( not empty() ); return *(end()-1); }
#endif
