
#pragma once
#ifndef EXTRA_H
#define EXTRA_H


// Include the correct headers for the language
#ifdef __cplusplus
#include <cassert>
#include <cstddef>
#else
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#endif


// Define empty xtr namespace in C++
#ifdef __cplusplus
namespace xtr {}
#endif


// Namespace qualifier for C++
#ifdef __cplusplus
#define XTR_NAMESPACE_STD  ::std::
#else
#define XTR_NAMESPACE_STD
#endif


// Macro to combine assert and MSVC compiler specific keyword __assume behavior
#ifndef assert_assume

#ifdef _MSC_VER
#define assert_assume(expr)  do { assert(expr); __assume(expr); } while(0)
#else
#define assert_assume(expr)  assert(expr)
#endif

#endif // assert_assume


// Macro to conditionally portably enable MSVC compiler specific keyword __restrict in C++
#if !defined(cpp_restrict) && defined(__cplusplus)

#ifdef _MSC_VER
#define cpp_restrict  __restrict
#else
#define cpp_restrict
#endif

#endif // cpp_restrict


// Macros for meta string literal conversion
#if !defined(concat_string) && !defined(literal_string) && !defined(macro_string)

#define concat_string(expr1, expr2)  expr1 ## expr2
#define literal_string(expr)  #expr
#define macro_string(expr)  literal_string(expr)

#endif // macro_string, literal_string, concat_string


// Debug logging macro in the style of assert()
#if defined(XTR_LOGGING) || defined(XTR_ALL)

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

#if !defined(debug_log)

#ifndef NDEBUG
#define debug_log(msg)  XTR_NAMESPACE_STD fputs("Debug log: " msg ", file " __FILE__ ", line " macro_string(__LINE__) "\n", stderr)
#else
#define debug_log(msg)
#endif

#endif // debug_log

#endif // XTR_LOGGING


// Multidimensional array alias for std::array in C++
#if (defined(XTR_MULTIARRAY) || defined(XTR_ALL)) && defined(__cplusplus)

#include <array>

namespace xtr {

	namespace impl {

		template <typename T, std::size_t I, std::size_t... J>
		struct multiarray_impl {
			using nested = typename multiarray_impl<T, J...>::type;
			using type = std::array<nested, I>;
		};

		template <typename T, std::size_t I>
		struct multiarray_impl<T, I> {
			using type = std::array<T, I>;
		};
	}

	template <typename T, std::size_t I, std::size_t... J>
	using multiarray = typename impl::multiarray_impl<T, I, J...>::type;
}

#endif // XTR_MULTIARRAY


#endif // EXTRA_H
