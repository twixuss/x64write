#define TL_IMPL
#define X64W_IMPLEMENTATION
#include <tl/main.h>
#include "common.h"


template <class T>
Span<T> same_start(Span<T> where, Span<T> what) {
	umm i = 0;
	for (; i < min(where.count, what.count); ++i) {
		if (what.data[i] != where.data[i]) {
			break;
		}
	}
	return where.subspan(0, i);
}

template <class T>
Span<T> find_most(Span<T> where, Span<T> what) {
	Span<T> result = where.subspan(0, 0);

	for (umm i = 0; i < where.count; ++i) {
		auto x = same_start(where.skip(i), what);
		if (x.count > result.count) {
			result = x;
		}
	}

	return result;

	if (what.count <= where.count) {
		for (umm i = 0; i < where.count - what.count + 1; ++i) {
			for (umm j = 0; j < what.count; ++j) {
				if (where.data[i + j] != what.data[j]) {
					if (j > result.count) {
						result = Span(where.data + i, j);
					}
					goto continue_outer;
				}
			}

			result = Span(where.data + i, what.count);
			break;
		continue_outer:;
		}
	}

	return result;
}

enum class Objective {
	generate,
	test_dumpbin
};

Objective objective;

struct CmdArg {
	char const *key;
	char const *desc;
	Variant<
		void (*)(),
		void (*)(u64),
		void (*)(String)
	> run;
};

void print_help();

CmdArg args_handlers[] = {
	{
		"-generate",
		"Generate the x64write.h header",
		+[] { objective = Objective::generate; }
	},
	{
		"-test-dumpbin",
		"Run the encoder and compare with output of dumpbin.exe",
		+[] { objective = Objective::test_dumpbin; }
	},
};

void print_help() {
	println("Usage:");
	for (auto handler : args_handlers) {
		println("  {}", handler.key);
		println("    {}", handler.desc);
	}
}

s32 tl_main(Span<String> args) {
	Span<String> possible_args = {
		u8"-generate"s,
		u8"-test-dumpbin"s,
	};

	if (args.count != 2) {
		current_logger.error("Expected exactly one argument");
		print_help();
		return 1;
	}
	
	for (umm i = 1; i < args.count; ++i) {
		for (auto handler : args_handlers) {
			auto cmd = args[i];
			if (args[i] == handler.key) {
				handler.run.visit(Combine {
					[&](void (*run)()) {
						run();
					},
					[&](void (*run)(u64 x)) {
						if (++i < args.count) {
							if (auto number = parse_u64(args[i])) {
								run(number.value());
							} else {
								current_logger.error("Could not parse number after {}. Ignoring.", cmd);
							}
						} else {
							current_logger.error("Expected a number after {}.", cmd);
						}
					},
					[&](void (*run)(String x)) {
						if (++i < args.count) {
							run(args[i]);
						} else {
							current_logger.error("Expected a string after {}.", cmd);
						}
					},
				});
				goto next_arg;
			}
		}
		current_logger.warning("Unknown command line parameter: {}", args[i]);
	next_arg:;
	}

	switch (objective) {
		case Objective::generate:
			void generate();
			generate();
			break;
		case Objective::test_dumpbin:
			void test_dumpbin();
			test_dumpbin();
			break;
	}
	return 0;
}
