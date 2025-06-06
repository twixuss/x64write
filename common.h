#pragma once

template <class ...Args>
void assertion_failure(char const *cause, char const *expr, Args...args);

#define ASSERTION_FAILURE(cause_string, expression_string, ...) (assertion_failure(cause_string, expression_string __VA_OPT__(,) __VA_ARGS__), debug_break())
#include <tl/string.h>
#include <tl/variant.h>
#include <tl/array.h>
#include <tl/console.h>
#include <tl/process.h>
#include <tl/default_logger.h>

#include <Windows.h>

using namespace tl;

using String = Span<utf8>;

inline bool operator==(String a, char const *b) { return a == as_utf8(as_span(b)); }
inline bool operator==(char const *a, String b) { return as_utf8(as_span(a)) == b; }
inline bool operator==(Span<char> a, char const *b) { return a == as_span(b); }
inline bool operator==(char const *a, Span<char> b) { return as_span(a) == b; }

inline String root_directory;
inline String exectable_directory;

template <class ...Args>
void assertion_failure(char const *cause_string, char const *expression_string, Args...args) {
	println("Assertion failed");
	println("Cause: {}", cause_string);
	println("Expression: {}", expression_string);
	print("Message: ");
	println(args...);
}

inline DefaultLogger logger = {.module = u8"app"s};

inline void init_common(Span<String> args) {
	current_logger = logger;

	exectable_directory = parse_path(args[0]).directory;
	root_directory = parse_path(exectable_directory).directory;
}