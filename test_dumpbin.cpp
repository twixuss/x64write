
template <class ...Args>
void assertion_failure(char const *cause, char const *expr, Args...args);

#define ASSERTION_FAILURE(cause_string, expression_string, ...) (assertion_failure(cause_string, expression_string __VA_OPT__(,) __VA_ARGS__), debug_break())
#define TL_IMPL
#include <tl/string.h>
#include <tl/variant.h>
#include <tl/array.h>
#include <tl/console.h>
#include <tl/main.h>

#include <Windows.h>

#pragma push_macro("assert")
#define X64W_IMPLEMENTATION
#define X64W_ENABLE_VALIDATION
#define X64W_ASSERT assert
#define X64W_NO_PREFIX
#include "x64write.h"
#pragma pop_macro("assert")

using namespace tl;

using String = Span<utf8>;

template <class ...Args>
void assertion_failure(char const *cause_string, char const *expression_string, Args...args) {
	println("Assertion failed");
	println("Cause: {}", cause_string);
	println("Expression: {}", expression_string);
	print("Message: ");
	println(args...);
}

Array regnames8  { "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh", "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b", "", "", "", "", "spl", "bpl", "sil", "dil"};
Array regnames16 { "ax", "cx", "dx", "bx", "sp", "bp", "si", "di", "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w", };
Array regnames32 { "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi", "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d", };
Array regnames64 { "rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", };

inline umm append(StringBuilder &builder, Gpr8 r) { return append(builder, regnames8[r.i]); }
inline umm append(StringBuilder &builder, Gpr16 r) { return append(builder, regnames16[r.i]); }
inline umm append(StringBuilder &builder, Gpr32 r) { return append(builder, regnames32[r.i]); }
inline umm append(StringBuilder &builder, Gpr64 r) { return append(builder, regnames64[r.i]); }
inline umm append(StringBuilder &builder, Xmm r) { return append_format(builder, "xmm{}", r.i); }

inline umm append(StringBuilder &builder, Mem m) {
	umm result = 0;
	result += append(builder, '[');
	if (m.base_scale) {
		if (m.size_override)
			result += append(builder, Gpr32{m.base});
		else
			result += append(builder, Gpr64{m.base});
	}
	if (m.index_scale) {
		if (result != 1)
			result += append(builder, '+');
		if (m.size_override)
			result += append(builder, Gpr32{m.index});
		else
			result += append(builder, Gpr64{m.index});
		result += append(builder, '*');
		result += append(builder, m.index_scale);
	}
	if (m.base_scale || m.index_scale) {
		if (m.displacement) {
			result += append(builder, '+');
			result += append(builder, m.displacement);
		}
	} else {
		result += append(builder, m.displacement);
	}
	result += append(builder, ']');
	return result;
}

// Microsoft 64 bit calling convention - saved registers
// +-----+----------+
// | reg | volatile |
// +-----+----------+
// | rax |    +     |
// | rbx |          |
// | rcx |    +     |
// | rdx |    +     |
// | rsi |          |
// | rdi |          |
// | rsp |          |
// | rbp |          |
// | r8  |    +     |
// | r9  |    +     |
// | r10 |    +     |
// | r11 |    +     |
// | r12 |          |
// | r13 |          |
// | r14 |          |
// | r15 |          |
// +-----+----------+

#include <tl/process.h>

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
#pragma pack(push, 1) // Ensure no padding in structures

// COFF Header
typedef struct {
    uint16_t Machine;          // Machine type
    uint16_t NumberOfSections; // Number of sections
    uint32_t TimeDateStamp;    // Timestamp
    uint32_t PointerToSymbolTable; // Pointer to symbol table
    uint32_t NumberOfSymbols;  // Number of symbols
    uint16_t SizeOfOptionalHeader; // Size of optional header
    uint16_t Characteristics;   // Characteristics
} COFFHeader;

// Section Header
typedef struct {
    char Name[8];              // Section name
    // chatgpt is stupid uint32_t PhysicalAddress;  // Physical address
    uint32_t VirtualSize;      // Virtual size
    uint32_t VirtualAddress;    // Virtual address
    uint32_t SizeOfRawData;    // Size of raw data
    uint32_t PointerToRawData; // Pointer to raw data
    uint32_t PointerToRelocations; // Pointer to relocations
    uint32_t PointerToLinenumbers; // Pointer to line numbers
    uint16_t NumberOfRelocations; // Number of relocations
    uint16_t NumberOfLinenumbers; // Number of line numbers
    uint32_t Characteristics;   // Characteristics
} SectionHeader;

#pragma pack(pop)

void write_instructions_to_obj(Span<u8> instructions, Span<utf8> filename) {
	List<u8> bytes;

    // Create and write the COFF header
    COFFHeader coffHeader = {0};
    coffHeader.Machine = 0x8664; // x86-64
    coffHeader.NumberOfSections = 1;
    coffHeader.TimeDateStamp = 0; // Set to 0 for now
    coffHeader.PointerToSymbolTable = 0; // No symbols
    coffHeader.NumberOfSymbols = 0; // No symbols
    coffHeader.SizeOfOptionalHeader = 0; // No optional header
    coffHeader.Characteristics = 0 ; // Characteristics

    bytes.add(value_as_bytes(coffHeader));

    // Create and write the section header
	SectionHeader sectionHeader = {};
	memcpy(sectionHeader.Name, ".text$mn", 8);
    sectionHeader.VirtualSize = 0; // Size of the section
    sectionHeader.SizeOfRawData = instructions.count; // Size of raw data
    sectionHeader.PointerToRawData = sizeof(COFFHeader) + sizeof(SectionHeader); // Offset to raw data
    sectionHeader.Characteristics = IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE;

    bytes.add(value_as_bytes(sectionHeader));
    bytes.add(instructions);

    write_entire_file(filename, bytes);
}

using Operand = Variant<Gpr8, Gpr16, Gpr32, Gpr64, Xmm, Mem, s64>;
		
struct InstrDesc {
	String mnemonic;
	StaticList<Operand, 4> operands;
};

inline umm append(StringBuilder &builder, InstrDesc i) {
	append_format(builder, "{} ", i.mnemonic);
	for (auto &op : i.operands) {
		if (&op != i.operands.begin()) append(builder, ", ");
		append(builder, op);
	}
	return 0;
}

struct InstrInfo : InstrDesc {
	Span<u8> span;
	u8 operand_size;
};

Optional<InstrDesc> parse_dumpbin_disasm_line(String line) {

	line = line.skip(20); // skip whitespace, index and colon
	
	if (!line.count)
		return {};

	auto first_space = find(line, u8' ');
	if (!first_space)
		return {};

	InstrDesc result;
	result.mnemonic = {line.begin(), first_space};

	line.set_begin(first_space);
	line = trim(line, [](auto x) { return is_whitespace((ascii)x); });

	auto parse_any_register = [&]() -> Variant<Empty, Gpr8, Gpr16, Gpr32, Gpr64, Xmm> {
		
		switch (*(u64 *)line.data & 0xffffffffff) {
			case 'x' | ('m'<<8) | ('m'<<16) | ('1'<<24) | ((u64)'0'<<32): line.data += 5; line.count -= 5; return xmm10;
			case 'x' | ('m'<<8) | ('m'<<16) | ('1'<<24) | ((u64)'1'<<32): line.data += 5; line.count -= 5; return xmm11;
			case 'x' | ('m'<<8) | ('m'<<16) | ('1'<<24) | ((u64)'2'<<32): line.data += 5; line.count -= 5; return xmm12;
			case 'x' | ('m'<<8) | ('m'<<16) | ('1'<<24) | ((u64)'3'<<32): line.data += 5; line.count -= 5; return xmm13;
			case 'x' | ('m'<<8) | ('m'<<16) | ('1'<<24) | ((u64)'4'<<32): line.data += 5; line.count -= 5; return xmm14;
			case 'x' | ('m'<<8) | ('m'<<16) | ('1'<<24) | ((u64)'5'<<32): line.data += 5; line.count -= 5; return xmm15;
		}
		switch (*(u32 *)line.data) {
			case 'r' | ('1'<<8) | ('0'<<16) | ('b'<<24): line.data += 4; line.count -= 4; return r10b;
			case 'r' | ('1'<<8) | ('1'<<16) | ('b'<<24): line.data += 4; line.count -= 4; return r11b;
			case 'r' | ('1'<<8) | ('2'<<16) | ('b'<<24): line.data += 4; line.count -= 4; return r12b;
			case 'r' | ('1'<<8) | ('3'<<16) | ('b'<<24): line.data += 4; line.count -= 4; return r13b;
			case 'r' | ('1'<<8) | ('4'<<16) | ('b'<<24): line.data += 4; line.count -= 4; return r14b;
			case 'r' | ('1'<<8) | ('5'<<16) | ('b'<<24): line.data += 4; line.count -= 4; return r15b;
			case 'r' | ('1'<<8) | ('0'<<16) | ('w'<<24): line.data += 4; line.count -= 4; return r10w;
			case 'r' | ('1'<<8) | ('1'<<16) | ('w'<<24): line.data += 4; line.count -= 4; return r11w;
			case 'r' | ('1'<<8) | ('2'<<16) | ('w'<<24): line.data += 4; line.count -= 4; return r12w;
			case 'r' | ('1'<<8) | ('3'<<16) | ('w'<<24): line.data += 4; line.count -= 4; return r13w;
			case 'r' | ('1'<<8) | ('4'<<16) | ('w'<<24): line.data += 4; line.count -= 4; return r14w;
			case 'r' | ('1'<<8) | ('5'<<16) | ('w'<<24): line.data += 4; line.count -= 4; return r15w;
			case 'r' | ('1'<<8) | ('0'<<16) | ('d'<<24): line.data += 4; line.count -= 4; return r10d;
			case 'r' | ('1'<<8) | ('1'<<16) | ('d'<<24): line.data += 4; line.count -= 4; return r11d;
			case 'r' | ('1'<<8) | ('2'<<16) | ('d'<<24): line.data += 4; line.count -= 4; return r12d;
			case 'r' | ('1'<<8) | ('3'<<16) | ('d'<<24): line.data += 4; line.count -= 4; return r13d;
			case 'r' | ('1'<<8) | ('4'<<16) | ('d'<<24): line.data += 4; line.count -= 4; return r14d;
			case 'r' | ('1'<<8) | ('5'<<16) | ('d'<<24): line.data += 4; line.count -= 4; return r15d;
			case 'x' | ('m'<<8) | ('m'<<16) | ('0'<<24): line.data += 4; line.count -= 4; return xmm0;
			case 'x' | ('m'<<8) | ('m'<<16) | ('1'<<24): line.data += 4; line.count -= 4; return xmm1;
			case 'x' | ('m'<<8) | ('m'<<16) | ('2'<<24): line.data += 4; line.count -= 4; return xmm2;
			case 'x' | ('m'<<8) | ('m'<<16) | ('3'<<24): line.data += 4; line.count -= 4; return xmm3;
			case 'x' | ('m'<<8) | ('m'<<16) | ('4'<<24): line.data += 4; line.count -= 4; return xmm4;
			case 'x' | ('m'<<8) | ('m'<<16) | ('5'<<24): line.data += 4; line.count -= 4; return xmm5;
			case 'x' | ('m'<<8) | ('m'<<16) | ('6'<<24): line.data += 4; line.count -= 4; return xmm6;
			case 'x' | ('m'<<8) | ('m'<<16) | ('7'<<24): line.data += 4; line.count -= 4; return xmm7;
			case 'x' | ('m'<<8) | ('m'<<16) | ('8'<<24): line.data += 4; line.count -= 4; return xmm8;
			case 'x' | ('m'<<8) | ('m'<<16) | ('9'<<24): line.data += 4; line.count -= 4; return xmm9;
		}

		switch (*(u32 *)line.data & 0xffffff) {
			case 's'|('p'<<8)|('l'<<16): line.data += 3; line.count -= 3; return spl;
			case 'b'|('p'<<8)|('l'<<16): line.data += 3; line.count -= 3; return bpl;
			case 's'|('i'<<8)|('l'<<16): line.data += 3; line.count -= 3; return sil;
			case 'd'|('i'<<8)|('l'<<16): line.data += 3; line.count -= 3; return dil;
			case 'r'|('8'<<8)|('b'<<16): line.data += 3; line.count -= 3; return r8b;
			case 'r'|('9'<<8)|('b'<<16): line.data += 3; line.count -= 3; return r9b;
			case 'r'|('8'<<8)|('w'<<16): line.data += 3; line.count -= 3; return r8w;
			case 'r'|('9'<<8)|('w'<<16): line.data += 3; line.count -= 3; return r9w;
			case 'e'|('a'<<8)|('x'<<16): line.data += 3; line.count -= 3; return eax;
			case 'e'|('c'<<8)|('x'<<16): line.data += 3; line.count -= 3; return ecx;
			case 'e'|('d'<<8)|('x'<<16): line.data += 3; line.count -= 3; return edx;
			case 'e'|('b'<<8)|('x'<<16): line.data += 3; line.count -= 3; return ebx;
			case 'e'|('s'<<8)|('p'<<16): line.data += 3; line.count -= 3; return esp;
			case 'e'|('b'<<8)|('p'<<16): line.data += 3; line.count -= 3; return ebp;
			case 'e'|('s'<<8)|('i'<<16): line.data += 3; line.count -= 3; return esi;
			case 'e'|('d'<<8)|('i'<<16): line.data += 3; line.count -= 3; return edi;
			case 'r'|('8'<<8)|('d'<<16): line.data += 3; line.count -= 3; return r8d;
			case 'r'|('9'<<8)|('d'<<16): line.data += 3; line.count -= 3; return r9d;
			case 'r'|('a'<<8)|('x'<<16): line.data += 3; line.count -= 3; return rax;
			case 'r'|('c'<<8)|('x'<<16): line.data += 3; line.count -= 3; return rcx;
			case 'r'|('d'<<8)|('x'<<16): line.data += 3; line.count -= 3; return rdx;
			case 'r'|('b'<<8)|('x'<<16): line.data += 3; line.count -= 3; return rbx;
			case 'r'|('s'<<8)|('p'<<16): line.data += 3; line.count -= 3; return rsp;
			case 'r'|('b'<<8)|('p'<<16): line.data += 3; line.count -= 3; return rbp;
			case 'r'|('s'<<8)|('i'<<16): line.data += 3; line.count -= 3; return rsi;
			case 'r'|('d'<<8)|('i'<<16): line.data += 3; line.count -= 3; return rdi;
			case 'r'|('1'<<8)|('0'<<16): line.data += 3; line.count -= 3; return r10;
			case 'r'|('1'<<8)|('1'<<16): line.data += 3; line.count -= 3; return r11;
			case 'r'|('1'<<8)|('2'<<16): line.data += 3; line.count -= 3; return r12;
			case 'r'|('1'<<8)|('3'<<16): line.data += 3; line.count -= 3; return r13;
			case 'r'|('1'<<8)|('4'<<16): line.data += 3; line.count -= 3; return r14;
			case 'r'|('1'<<8)|('5'<<16): line.data += 3; line.count -= 3; return r15;
		}

		switch (*(u16 *)line.data) {
			case 'a'|('l'<<8): line.data += 2; line.count -= 2; return al;
			case 'c'|('l'<<8): line.data += 2; line.count -= 2; return cl;
			case 'd'|('l'<<8): line.data += 2; line.count -= 2; return dl;
			case 'b'|('l'<<8): line.data += 2; line.count -= 2; return bl;
			case 'a'|('h'<<8): line.data += 2; line.count -= 2; return ah;
			case 'c'|('h'<<8): line.data += 2; line.count -= 2; return ch;
			case 'd'|('h'<<8): line.data += 2; line.count -= 2; return dh;
			case 'b'|('h'<<8): line.data += 2; line.count -= 2; return bh;
			case 'a'|('x'<<8): line.data += 2; line.count -= 2; return ax;
			case 'c'|('x'<<8): line.data += 2; line.count -= 2; return cx;
			case 'd'|('x'<<8): line.data += 2; line.count -= 2; return dx;
			case 'b'|('x'<<8): line.data += 2; line.count -= 2; return bx;
			case 's'|('p'<<8): line.data += 2; line.count -= 2; return sp;
			case 'b'|('p'<<8): line.data += 2; line.count -= 2; return bp;
			case 's'|('i'<<8): line.data += 2; line.count -= 2; return si;
			case 'd'|('i'<<8): line.data += 2; line.count -= 2; return di;
			case 'r'|('8'<<8): line.data += 2; line.count -= 2; return r8;
			case 'r'|('9'<<8): line.data += 2; line.count -= 2; return r9;
		}

		return {};
	};
	
	auto parse_gpr = [&]() -> Variant<Empty, Gpr8, Gpr16, Gpr32, Gpr64> {
		auto prev = line;
		auto reg = parse_any_register();
		if (reg.is<Gpr8>()) return reg.as<Gpr8>().value();
		if (reg.is<Gpr16>()) return reg.as<Gpr16>().value();
		if (reg.is<Gpr32>()) return reg.as<Gpr32>().value();
		if (reg.is<Gpr64>()) return reg.as<Gpr64>().value();
		line = prev;
		return {};
	};

	int size = 0;

	while (line.count) {
		if (auto reg = parse_any_register(); !reg.is<Empty>()) {

			reg.visit(Combine {
				[&](auto x) { result.operands.add(x); },
				[&](Empty) {},
			});
			reg.visit(Combine {
				[&](auto x) {},
				[&](Gpr8) {size = 1;},
				[&](Gpr16) {size = 2;},
				[&](Gpr32) {size = 4;},
				[&](Gpr64) {size = 8;},
			});
		} else if (
			(starts_with(line, u8"byte ptr ["s) && (line.set_begin(line.begin() + 10), true)) ||
			(starts_with(line, u8"word ptr ["s) && (line.set_begin(line.begin() + 10), true)) ||
			(starts_with(line, u8"dword ptr ["s) && (line.set_begin(line.begin() + 11), true)) ||
			(starts_with(line, u8"qword ptr ["s) && (line.set_begin(line.begin() + 11), true)) ||
			(starts_with(line, u8"xmmword ptr ["s) && (line.set_begin(line.begin() + 13), true)) ||
			(starts_with(line, u8"ymmword ptr ["s) && (line.set_begin(line.begin() + 13), true)) ||
			(starts_with(line, u8"zmmword ptr ["s) && (line.set_begin(line.begin() + 13), true))
		) {
			Mem m = {};

			bool has_regs = false;

			if (auto reg = parse_gpr(); !reg.is<Empty>()) {
				has_regs = true;

				if (reg.is<Gpr32>()) {
					m.size_override = 1;
				}

				m.base_scale = 1;
				reg.visit(Combine {
					[&](auto x) { m.base = x.i; },
					[&](Empty) {},
				});
			}

			if (line.front() == '*') {
				m.index = m.base;
				m.base = 0;
				m.base_scale = 0;
				line.set_begin(line.begin() + 1);
				if (false) {}
				else if (line.front() == '1') { m.index_scale = 1; }
				else if (line.front() == '2') { m.index_scale = 2; }
				else if (line.front() == '4') { m.index_scale = 4; }
				else if (line.front() == '8') { m.index_scale = 8; }
				line.set_begin(line.begin() + 1);
			} else if (line.front() == '+') {
				line.set_begin(line.begin() + 1);
				if (auto reg = parse_gpr(); !reg.is<Empty>()) {
					has_regs = true;
					reg.visit(Combine {
						[&](auto x) { m.index = x.i; },
						[&](Empty) {},
					});
					if (line.front() == '*') {
						line.set_begin(line.begin() + 1);
						if (false) {}
						else if (line.front() == '1') { m.index_scale = 1; }
						else if (line.front() == '2') { m.index_scale = 2; }
						else if (line.front() == '4') { m.index_scale = 4; }
						else if (line.front() == '8') { m.index_scale = 8; }
						line.set_begin(line.begin() + 1);
					} else {
						m.index_scale = 1;
					}
				} else {
					line.set_begin(line.begin() - 1);
				}
			}
						
			if ((line.front() == '+' && (line.set_begin(line.begin() + 1), true)) || !has_regs) {
				s64 disp = 0;
				while (is_hex_digit(line.front())) {
					disp = disp * 16 + hex_digit_to_int(line.front());
					line.set_begin(line.begin() + 1);
				}
							
				assert(line.front() == 'h');
				line.set_begin(line.begin() + 1);
				m.displacement = disp;
			}

			assert(line.front() == ']');
			line.set_begin(line.begin() + 1);

			result.operands.add(m);
		} else {
			s64 imm = 0;
			while (is_hex_digit(line.front())) {
				imm = imm * 16 + hex_digit_to_int(line.front());
				line.set_begin(line.begin() + 1);
			}
							
			assert(line.front() == 'h');
			line.set_begin(line.begin() + 1);
			switch (size) {
				case 1: result.operands.add((s64)(s8)imm); break;
				case 2: result.operands.add((s64)(s16)imm); break;
				case 4: result.operands.add((s64)(s32)imm); break;
				case 8: result.operands.add(imm); break;
				default: invalid_code_path();
			}
		}
				
		line = trim(line, [](auto x) { return is_whitespace((ascii)x); });
		if (line.count && line.front() == ',') line.set_begin(line.begin() + 1);
		line = trim(line, [](auto x) { return is_whitespace((ascii)x); });
	}

	return result;
}

static u8 buf[65536 * 256] = {};

s32 tl_main(Span<String> args) {
	auto exectable_directory = parse_path(args[0]).directory;
	auto msvc_path_path = format(u8"{}\\msvc_path.txt", exectable_directory);
	auto msvc_path = as_utf8(read_entire_file(msvc_path_path));
	if (!msvc_path.count) {
		with(ConsoleColor::red, println("Could not read {}. Make sure this file exists and contains path to directory which has dumpbin.exe and ml64.exe", msvc_path_path));
		return 1;
	}
	if (!directory_exists(msvc_path)) {
		with(ConsoleColor::red, println("Could not read {}. Make sure {} contains valid path to directory which has dumpbin.exe and ml64.exe", msvc_path, msvc_path_path));
		return 1;
	}

	Array regs8  {al, cl, dl, bl, ah, ch, dh, bh, r8b,r9b,r10b,r11b,r12b,r13b,r14b,r15b,spl,bpl,sil,dil,};
	Array regs16 {ax, cx, dx, bx, sp, bp, si, di, r8w,r9w,r10w,r11w,r12w,r13w,r14w,r15w,};
	Array regs32 {eax,ecx,edx,ebx,esp,ebp,esi,edi,r8d,r9d,r10d,r11d,r12d,r13d,r14d,r15d,};
	Array regs64 {rax,rcx,rdx,rbx,rsp,rbp,rsi,rdi,r8, r9, r10, r11, r12, r13, r14, r15,};
	Array xmms {xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7,xmm8,xmm9,xmm10,xmm11,xmm12,xmm13,xmm14,xmm15};

	u8 *c = buf;
			
	List<InstrInfo> instr_infos;

	List<Mem> mems;
			
	mems.add(mem64_d(0x34));
	mems.add(mem64_d(0x3456));
	for (u8 i = 0; i < 16; ++i) mems.add(mem64_b((Gpr64{i})));
	for (u8 i = 0; i < 16; ++i) mems.add(mem64_bd((Gpr64{i}), 0x34));	
	for (u8 i = 0; i < 16; ++i) mems.add(mem64_bd((Gpr64{i}), 0x3456));	
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_i((Gpr64{i}), 1));
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_i((Gpr64{i}), 2));
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_i((Gpr64{i}), 4));
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_i((Gpr64{i}), 8));
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_id((Gpr64{i}), 1, 0x34));
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_id((Gpr64{i}), 2, 0x34));
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_id((Gpr64{i}), 4, 0x34));
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_id((Gpr64{i}), 8, 0x34));
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_id((Gpr64{i}), 1, 0x3456));
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_id((Gpr64{i}), 2, 0x3456));
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_id((Gpr64{i}), 4, 0x3456));
	for (u8 i = 0; i < 16; ++i) if (i != 4) mems.add(mem64_id((Gpr64{i}), 8, 0x3456));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bi((Gpr64{i}), (Gpr64{j}), 1));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bi((Gpr64{i}), (Gpr64{j}), 2));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bi((Gpr64{i}), (Gpr64{j}), 4));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bi((Gpr64{i}), (Gpr64{j}), 8));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bid((Gpr64{i}), (Gpr64{j}), 1, 0x34));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bid((Gpr64{i}), (Gpr64{j}), 2, 0x34));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bid((Gpr64{i}), (Gpr64{j}), 4, 0x34));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bid((Gpr64{i}), (Gpr64{j}), 8, 0x34));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bid((Gpr64{i}), (Gpr64{j}), 1, 0x3456));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bid((Gpr64{i}), (Gpr64{j}), 2, 0x3456));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bid((Gpr64{i}), (Gpr64{j}), 4, 0x3456));
	for (u8 i = 0; i < 16; ++i) for (u8 j = 0; j < 16; ++j) if (j != 4) mems.add(mem64_bid((Gpr64{i}), (Gpr64{j}), 8, 0x3456));
			
	{
		umm count = mems.count;
		for (umm i = 0; i < count; ++i) {
			if (mems[i].base_scale || mems[i].index_scale) {
				mems.add(mems[i] withx { it.size_override = 1; });
			}
		}
	}

	List<std::pair<Gpr8, Gpr8>> reg8_pairs;
			
	for (auto a : regs8) {
		for (auto b : regs8) {
			if (!gpr8_compatible_rr(a, b)) {
				continue;
			}
			reg8_pairs.add({a, b});
		}
	}
			
	println("Encoding...");

	StringBuilder ml64_builder;
	append(ml64_builder, ".code\nmain proc\n");

	auto test = [&](String name, int size, auto instr, auto ...args) {
		auto start = c;
				
		InstrInfo i = {};
		i.mnemonic = name;
		i.operand_size = size;

		append_format(ml64_builder, "\t{} ", name);

		int operand_index = 0;

		auto add_operand = [&](auto op) {
			defer {++operand_index;};
			if constexpr (OneOf<decltype(op), s8, s16, s32>) {
				i.operands.add((s64)op);
			} else {
				i.operands.add(op);
			}

			if (operand_index) append(ml64_builder, ",");

			if (false) {}
			else if constexpr (std::is_same_v<decltype(op), s8>) append_format(ml64_builder, "byte ptr {}", op);
			else if constexpr (std::is_same_v<decltype(op), s16>) append_format(ml64_builder, "word ptr {}", op);
			else if constexpr (std::is_same_v<decltype(op), s32>) append_format(ml64_builder, "dword ptr {}", op);
			else if constexpr (std::is_same_v<decltype(op), s64>) append_format(ml64_builder, "qword ptr {}", op);
			else append(ml64_builder, op);
		};

		(add_operand(args), ...);
		
		append(ml64_builder, '\n');

		if (auto reason = instr(&c, args...)) {
			with(ConsoleColor::red, println("Failed to encode '{}'\nReason: {}", i, reason));
			exit(1);
		}

		i.span = Span(start, c);

		instr_infos.add(i);
	};

	// D = R/M
	// V = R/M/I
	#define TEST_I8(name)  test(u8###name##s,  8, name##_i8 , (int8_t )0x123456789abcdef);
	#define TEST_I16(name) test(u8###name##s, 16, name##_i16, (int16_t)0x123456789abcdef);
	#define TEST_I32(name) test(u8###name##s, 32, name##_i32, (int32_t)0x123456789abcdef);
	#define TEST_I64(name) test(u8###name##s, 64, name##_i64, (int64_t)0x123456789abcdef);
	#define TEST_R8(name)  for (auto r : regs8)  test(u8###name##s,  8, name##_r8 , r);
	#define TEST_R16(name) for (auto r : regs16) test(u8###name##s, 16, name##_r16, r);
	#define TEST_R32(name) for (auto r : regs32) test(u8###name##s, 32, name##_r32, r);
	#define TEST_R64(name) for (auto r : regs64) test(u8###name##s, 64, name##_r64, r);
	#define TEST_M8(name)  for (auto m : mems)   test(u8###name##s,  8, name##_m8 , m);
	#define TEST_M16(name) for (auto m : mems)   test(u8###name##s, 16, name##_m16, m);
	#define TEST_M32(name) for (auto m : mems)   test(u8###name##s, 32, name##_m32, m);
	#define TEST_M64(name) for (auto m : mems)   test(u8###name##s, 64, name##_m64, m);
	#define TEST_RI8(name)     for (auto r : regs8)  test(u8###name##s,  8, name##_ri8,  r, (int8_t )0x123456789abcdef);
	#define TEST_RI16(name)    for (auto r : regs16) test(u8###name##s, 16, name##_ri16, r, (int16_t)0x123456789abcdef);
	#define TEST_RI32(name)    for (auto r : regs32) test(u8###name##s, 32, name##_ri32, r, (int32_t)0x123456789abcdef);
	#define TEST_RI64(name)    for (auto r : regs64) test(u8###name##s, 64, name##_r64i32, r, (int32_t)0x123456789abcdef);
	#define TEST_RI16_8(name)  for (auto r : regs16) test(u8###name##s, 16, name##_r16i8, r, (int8_t)0x123456789abcdef);
	#define TEST_RI32_8(name)  for (auto r : regs32) test(u8###name##s, 32, name##_r32i8, r, (int8_t)0x123456789abcdef);
	#define TEST_RI64_8(name)  for (auto r : regs64) test(u8###name##s, 64, name##_r64i8, r, (int8_t)0x123456789abcdef);
	#define TEST_RI64_64(name) for (auto r : regs64) test(u8###name##s, 64, name##_ri64, r, (int64_t)0x123456789abcdef);
	#define TEST_RR8(name)  for (auto a : regs8)  for (auto b : regs8) if (gpr8_compatible_rr(a, b)) test(u8###name##s,  8, name##_rr8,  a, b);
	#define TEST_RR16(name) for (auto a : regs16) for (auto b : regs16)                              test(u8###name##s, 16, name##_rr16, a, b);
	#define TEST_RR32(name) for (auto a : regs32) for (auto b : regs32)                              test(u8###name##s, 32, name##_rr32, a, b);
	#define TEST_RR64(name) for (auto a : regs64) for (auto b : regs64)                              test(u8###name##s, 64, name##_rr64, a, b);
	#define TEST_RM8(name)  for (auto a : regs8 ) for (auto b : mems) if (gpr8_compatible_rm(a, b)) test(u8###name##s,  8, name##_rm8,  a, b);
	#define TEST_RM16(name) for (auto a : regs16) for (auto b : mems)                               test(u8###name##s, 16, name##_rm16, a, b);
	#define TEST_RM32(name) for (auto a : regs32) for (auto b : mems)                               test(u8###name##s, 32, name##_rm32, a, b);
	#define TEST_RM64(name) for (auto a : regs64) for (auto b : mems)                               test(u8###name##s, 64, name##_rm64, a, b);
	#define TEST_MR8(name)  for (auto a : mems) for (auto b : regs8 ) if (gpr8_compatible_rm(b, a)) test(u8###name##s,  8, name##_mr8,  a, b);
	#define TEST_MR16(name) for (auto a : mems) for (auto b : regs16)                               test(u8###name##s, 16, name##_mr16, a, b);
	#define TEST_MR32(name) for (auto a : mems) for (auto b : regs32)                               test(u8###name##s, 32, name##_mr32, a, b);
	#define TEST_MR64(name) for (auto a : mems) for (auto b : regs64)                               test(u8###name##s, 64, name##_mr64, a, b);
	#define TEST_MI8(name)     for (auto a : mems)  test(u8###name##s,  8, name##_mi8,  a, (int8_t )0x123456789abcdef);
	#define TEST_MI16(name)    for (auto a : mems)  test(u8###name##s, 16, name##_mi16, a, (int16_t)0x123456789abcdef);
	#define TEST_MI32(name)    for (auto a : mems)  test(u8###name##s, 32, name##_mi32, a, (int32_t)0x123456789abcdef);
	#define TEST_MI64(name)    for (auto a : mems)  test(u8###name##s, 64, name##_m64i32, a, (int32_t)0x123456789abcdef);
	#define TEST_MI16_8(name)  for (auto a : mems)  test(u8###name##s, 16, name##_m16i8, a, (int8_t)0x123456789abcdef);
	#define TEST_MI32_8(name)  for (auto a : mems)  test(u8###name##s, 32, name##_m32i8, a, (int8_t)0x123456789abcdef);
	#define TEST_MI64_8(name)  for (auto a : mems)  test(u8###name##s, 64, name##_m64i8, a, (int8_t)0x123456789abcdef);
	#define TEST_XX(name) for (auto a : xmms) for (auto b : xmms) test(u8###name##s, 128, name##_xx, a, b);
	#define TEST_XM(name) for (auto a : xmms) for (auto b : mems) test(u8###name##s, 128, name##_xm, a, b);
	#define TEST_XXX(name) for (auto a : xmms) for (auto b : xmms) for (auto c : xmms) test(u8###name##s, 128, name##_xxx, a, b, c);
			
	#define TEST_R(name) \
		TEST_R8(name) \
		TEST_R16(name) \
		TEST_R32(name) \
		TEST_R64(name) \

	#define TEST_M(name) \
		TEST_M8(name) \
		TEST_M16(name) \
		TEST_M32(name) \
		TEST_M64(name) \

	#define TEST_D(name) \
		TEST_R(name); \
		TEST_M(name); \

	#define TEST_RI(name) \
		TEST_RI8(name) \
		TEST_RI16(name) \
		TEST_RI32(name) \
		TEST_RI64(name) \

	#define TEST_RR(name) \
		TEST_RR8(name) \
		TEST_RR16(name) \
		TEST_RR32(name) \
		TEST_RR64(name) \

	#define TEST_RM(name) \
		TEST_RM8(name) \
		TEST_RM16(name) \
		TEST_RM32(name) \
		TEST_RM64(name) \

	#define TEST_MI(name) \
		TEST_MI8(name) \
		TEST_MI16(name) \
		TEST_MI32(name) \
		TEST_MI64(name) \

	#define TEST_MR(name) \
		TEST_MR8(name) \
		TEST_MR16(name) \
		TEST_MR32(name) \
		TEST_MR64(name) \

	// TEST_RI(add);
	// TEST_RR(add);
	// TEST_RM(add);
	// TEST_MI(add);
	// TEST_MR(add);
	// TEST_RI16_8(add);
	// TEST_RI32_8(add);
	// TEST_RI64_8(add);
	// TEST_MI16_8(add);
	// TEST_MI32_8(add);
	// TEST_MI64_8(add);

	//TEST_RI8(mov)
	//TEST_RI16(mov)
	//TEST_RI32(mov)
	//TEST_RI64_64(mov)

	//TEST_XX(addpd);
	//TEST_XM(addpd);

	TEST_XXX(vaddpd);
	//test(u8"vaddpd"s, 128, vaddpd_xxx, xmm0, xmm0, xmm0);
	//test(u8"vaddpd"s, 128, vaddpd_xxx, xmm8, xmm0, xmm0);
	//test(u8"vaddpd"s, 128, vaddpd_xxx, xmm0, xmm8, xmm0);
	//test(u8"vaddpd"s, 128, vaddpd_xxx, xmm8, xmm8, xmm0);
	//test(u8"vaddpd"s, 128, vaddpd_xxx, xmm0, xmm0, xmm8);
	//test(u8"vaddpd"s, 128, vaddpd_xxx, xmm8, xmm0, xmm8);
	//test(u8"vaddpd"s, 128, vaddpd_xxx, xmm0, xmm8, xmm8);
	//test(u8"vaddpd"s, 128, vaddpd_xxx, xmm8, xmm8, xmm8);

	println("Writing obj...");

	if (false) {
		StringBuilder builder;
		for (auto c : Span(buf, c)) {
			append(builder, format_hex(c));
		}
		auto hex_path = format("{}\\check.txt"s, exectable_directory);
		write_entire_file(hex_path, as_bytes(to_string(builder)));
	}

	auto my_obj_path = format(u8"{}\\check.obj"s, exectable_directory);
	auto ml64_obj_path = format(u8"{}\\ml64.obj"s, exectable_directory);
	auto asm_path = format(u8"{}\\check.asm"s, exectable_directory);
			
	write_instructions_to_obj(Span(buf, (umm)(c - buf)), my_obj_path);
	
	append(ml64_builder, "main endp\nend");
	println("Assembling...");
	write_entire_file(asm_path, ml64_builder);
	wait(start_process(tformat(u8"{}\\ml64.exe /Fo {} /c {}"s, msvc_path, ml64_obj_path, asm_path)));
	auto assembled_obj = read_entire_file(ml64_obj_path);

	println("Disassembling...");

	auto disasm_output = (String)start_process_and_get_output(tformat(u8"{}\\dumpbin.exe /disasm:nobytes /nologo {}"s, msvc_path, my_obj_path));
	
	println("Parsing...");

	smm line_index = -5;
	split_by_seq(disasm_output, u8"\r\n"s, [&](String line) {
		defer{++line_index;};

		if (line_index < 0)
			return ForEach_continue;

		if (line_index >= instr_infos.count)
			return ForEach_break;

		auto i = instr_infos[line_index];

		String reason = {};
				
		auto parsed_ = parse_dumpbin_disasm_line(line);

		auto parsed = parsed_.value_or({});

		auto encoding_is_right = [&]() {
			if (!parsed_) {
				reason = u8"Could not parse"s;
				return false;
			}

			if (i.mnemonic != parsed.mnemonic) {
				reason = format(u8"invalid mnemonic. expected: {}, actual: {}"s, i.mnemonic, parsed.mnemonic);
				return false;
			}
			if (i.operands.count != parsed.operands.count) {
				reason = format(u8"operand count mismatch. expected: {}, actual: {}"s, i.operands.count, parsed.operands.count);
				return false;
			}
			for (int j = 0; j < i.operands.count; ++j) {
				auto normalize_mem = [&](Mem &m) {
					if (m.base_scale == 0 && m.index_scale == 1) {
						m.base_scale = 1;
						m.index_scale = 0;
						m.base = m.index;
						m.index = 0;
					}
				};
				i.operands[j].visit(Combine{
					[](auto &){},
					[&](Mem &m) {
						normalize_mem(m);
					}
				});
				parsed.operands[j].visit(Combine{
					[](auto &){},
					[&](Mem &m) {
						normalize_mem(m);
					}
				});

				bool ok = true;

				if (i.operands[j].is<s64>() && parsed.operands[j].is<s64>()) {
					switch (i.operand_size) {
						case 8: ok = (s8)i.operands[j].as<s64>().value() == (s8)parsed.operands[j].as<s64>().value(); break;
						case 16: ok = (s16)i.operands[j].as<s64>().value() == (s16)parsed.operands[j].as<s64>().value(); break;
						case 32: ok = (s32)i.operands[j].as<s64>().value() == (s32)parsed.operands[j].as<s64>().value(); break;
						case 64: ok = (s64)i.operands[j].as<s64>().value() == (s64)parsed.operands[j].as<s64>().value(); break;
					}
				} else {
					ok = i.operands[j] == parsed.operands[j];
				}

				if (!ok) {
					reason = format(u8"operand {} mismatch. expected: {}, actual: {}"s, j, i.operands[j], parsed.operands[j]);
					return false;
				}
			}
			return true;
		};

		if (!encoding_is_right()) {
			with(ConsoleColor::red, println("Failed to encode instruction #{}", line_index));
			println("Encoded:      {}", i);
			println("Disassembled: {}", parsed);
			println("x64w bytes: {}", format_hex(i.span));
			println("ml64 bytes: {}", format_hex(Span(assembled_obj.data + 0x8c + (i.span.data - buf), i.span.count)));
			println("Reason:   {}", reason);

			exit(1);
		}

		return ForEach_continue;
	});
			
	with(ConsoleColor::green, println("Successfully encoded {} bytes of {} instructions", c - buf, instr_infos.count));
	return 0;
}
