/*
 * This file is a part of TiledArray.
 * Copyright (C) 2013  Virginia Tech
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TILEDARRAY_ERROR_H__INCLUDED
#define TILEDARRAY_ERROR_H__INCLUDED

#include <TiledArray/config.h>

#ifdef TILEDARRAY_HAVE_STATIC_ASSERT

#define TA_STATIC_ASSERT( a ) static_assert( a )

#else

// This implementation of static assert is based on Boost.StaticAssert

//  (C) Copyright John Maddock 2000.
//  Use, modification and distribution are subject to the
//  Boost Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/static_assert for documentation.

//namespace TiledArray {
//  namespace error {
//    template <bool x> struct STATIC_ASSERTION_FAILURE;
//
//    template <> struct STATIC_ASSERTION_FAILURE<true> { enum { value = 1 }; };
//
//    template<int x> struct static_assert_test{};
//  }  // namespace error
//} // namespace TiledArray
#define TA_STATIC_ASSERT( A )
//       typedef ::TiledArray::error::static_assert_test<\
//          sizeof(::TiledArray::error::STATIC_ASSERTION_FAILURE< (( A ) == 0 ? false : true) >)>\
//             static_assert_##__FILE__##_##__LINE__
#endif // TILEDARRAY_HAVE_STATIC_ASSERT

// Check for default error checking method, which is determined by TA_DEFAULT
// error. It is defined in TiledArray/config.h.
#ifdef TA_DEFAULT_ERROR
# if !defined(TA_EXCEPTION_ERROR) && !defined(TA_ASSERT_ERROR) && !defined(TA_NO_ERROR)
#  if TA_DEFAULT_ERROR == 0
#   define TA_NO_ERROR
#  elif TA_DEFAULT_ERROR == 1
#   define TA_EXCEPTION_ERROR
#  elif TA_DEFAULT_ERROR == 2
#   define TA_ASSERT_ERROR
#  endif // TA_DEFAULT_ERROR == ?
# endif // !defined(TA_EXCEPTION_ERROR) && !defined(TA_EXCEPTION_ERROR) && !defined(TA_EXCEPTION_ERROR)
#endif // TA_DEFAULT_ERROR

#include <exception>
namespace TiledArray {

  class Exception : public std::exception {
  public:
    Exception(const char* m) : message_(m) { }

    virtual const char* what() const throw() { return message_; }

  private:
    const char* message_;
  }; // class Exception

  /// Place a break point on this function to stop before TiledArray exceptions are thrown.
  inline void exception_break() { }
} // namespace TiledArray

#ifdef TA_EXCEPTION_ERROR
// This section defines the behavior for TiledArray assertion error checking
// which will throw exceptions.
#ifdef TA_ASSERT_ERROR
#undef TA_ASSERT_ERROR
// WARNING: TA_EXCEPTION_ERROR supersedes TA_ASSERT_ERROR.
#endif

#define TA_STRINGIZE( s ) #s

#define TA_EXCEPTION_MESSAGE( file , line , mess ) \
  "TiledArray: exception at " file "(" TA_STRINGIZE( line ) "): " mess

#define TA_EXCEPTION( m ) \
    throw TiledArray::Exception ( TA_EXCEPTION_MESSAGE( __FILE__ , __LINE__ , m ) )

#define TA_ASSERT( a )  \
  if(! ( a ) ) \
    { \
      TiledArray::exception_break(); \
      TA_EXCEPTION( "assertion failure" ) ; \
    }

#define TA_TEST( a )  TA_ASSERT( a )

#elif defined(TA_ASSERT_ERROR)
// This sections defines behavior for TiledArray assertion error checking which
// uses assertions.
#include <cassert>
#define TA_ASSERT( a ) assert( a )
#define TA_EXCEPTION( m ) exit(1)
#define TA_TEST( a )  TA_ASSERT( a )
#else
// This section defines behavior for TiledArray assertion error checking which
// does no error checking.
// WARNING: TiledArray will perform no error checking.
#define TA_ASSERT( a ) { ; }
#define TA_EXCEPTION( m ) exit(1)
#define TA_TEST( a )  a

#endif //TA_EXCEPTION_ERROR

#define TA_CHECK( a )  \
  if(! ( a ) ) \
    { \
      TA_EXCEPTION( "check failure" ) ; \
    }

#endif // TILEDARRAY_ERROR_H__INCLUDED
