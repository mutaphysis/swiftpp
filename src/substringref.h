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

	inline size_t size() const { return end() - begin(); }
	inline bool empty() const { return begin() == end(); }

	inline const char *begin() const { return _begin; }
	inline const char *end() const { return _end; }

	inline void pop_front() { assert( not empty() ); ++_begin; }
	inline void pop_back() { assert( not empty() ); --_end; }

	inline char back() const { assert( not empty() ); return *(end()-1); }

	bool operator==( const substringref &i_other ) const;

  private:
	const char *_begin = nullptr;
	const char *_end = nullptr;
};

#endif
