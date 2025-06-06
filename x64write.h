/*
x64write is single-file, header-only utility for encoding x86-64 instructions,
focused on static typechecking.

		Before including this file you can:

#define X64W_IMPLEMENTATION
	To include implementation. Do that in one of your .c/.cpp files.

#define X64W_NO_PREFIX
	To strip x64w_ prefixes. It makes code easier to read and write, but might introduce
	name collisions. You can selectively #undef colliding names.

#define X64W_BSWAP
	To byteswap immediate and displacement values
	
#define X64W_FORCE_INLINE
	To force inlining of every instruction function.
	When this macro is enabled, each instr function will be fully inlined, which may enable some
	speedup due to constant folding, but will increase code size by a lot.

#define X64W_NO_INLINE
	To disallow inlining of every instruction function.
	When this macro is enabled, each instr function will have just a bit of setup and a jump to a generalized function,
	which will reduce code size, but might have worse performance.

	If none of the inlining macros are defined, it's up to the compiler to decide.
	
		Errors:

	By default all of the input is validated. If encoding is successful, return value is 0,
	otherwise it's a message describing the error. You can:
#define X64W_VALIDATE(condition, message) do { if (!(condition)) { *c = restore; return message; } } while (0)
	To override default validation check. Here you can insert logging and whatnot.
	
	Example (no prefixes):
uint8_t *buf = malloc(10 * X64W_MAX_INSTRUCTION_SIZE);
uint8_t *c = buf;
push_r64  (&c, rbp);                    // push rbp
mov_rr64  (&c, rbp, rsp);               // mov rbp, rsp
sub_r64i32(&c, rsp, 16);                // sub rsp, 16
mov_mr64  (&c, mem64_b(rsp), rcx);      // mov [rsp], rcx
mov_mr64  (&c, mem64_bd(rsp, +8), rdx); // mov [rsp+8], rdx
add_r64i32(&c, rsp, 16);                // add rsp, 16
pop_r64(&c, rbp);                     // pop rbp
ret(&c);                             // ret

	Instruction naming:
<mnemonic>_[[<operand type> ...]<previous operand(s) size in _bits_> ...]
	
	Operand types:
r - general purpose register
m - memory
i - immediate
x - xmm register
y - ymm register
z - zmm register

	Size may not be required if it is obvious in the mnemonic

	Example:
sub_r64i32 - subtract 32-bit immediate from 64-bit register
addpd_xx   - add 64-bit floats in xmm registers


	Memory operand naming: suffix of mem_* determines argument type and count
b - base register
i - index register, index scale (1/2/4/8)
d - 32-bit displacement

		TODO
	Choose compact instructions when available?

*/

#ifndef X64W_H_
#define X64W_H_

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:\
	4668 /* Expanding undefined macro to 0 */ \
	4820 /* Struct padding */ \
	4244 /* Conversion, possible loss of data */ \
	4711 /* Inlining logs */ \
)
#endif

#include <stdint.h>
#include <stdbool.h>

#ifndef X64W_DEF
#define X64W_DEF extern
#endif

#if __STDC_VERSION__ >= 202311L || defined(__cplusplus)
	#define X64W_UNDERLYING(x) :x
#else
	#define X64W_UNDERLYING(x)
#endif

#ifdef __cplusplus
	#define X64W_LIT(type) type
#else
	#define X64W_LIT(type) (type)
#endif

#define X64W_MAX_INSTRUCTION_SIZE 15

#ifdef __cplusplus
extern "C" {
#endif
	
// 0    - Success
// else - Error message
typedef char const *x64w_Result;

typedef struct { uint8_t i; } x64w_Gpr8;
#define x64w_al   (X64W_LIT(x64w_Gpr8) { 0x00 })
#define x64w_cl   (X64W_LIT(x64w_Gpr8) { 0x01 })
#define x64w_dl   (X64W_LIT(x64w_Gpr8) { 0x02 })
#define x64w_bl   (X64W_LIT(x64w_Gpr8) { 0x03 })
#define x64w_ah   (X64W_LIT(x64w_Gpr8) { 0x04 })
#define x64w_ch   (X64W_LIT(x64w_Gpr8) { 0x05 })
#define x64w_dh   (X64W_LIT(x64w_Gpr8) { 0x06 })
#define x64w_bh   (X64W_LIT(x64w_Gpr8) { 0x07 })
#define x64w_r8b  (X64W_LIT(x64w_Gpr8) { 0x08 })
#define x64w_r9b  (X64W_LIT(x64w_Gpr8) { 0x09 })
#define x64w_r10b (X64W_LIT(x64w_Gpr8) { 0x0a })
#define x64w_r11b (X64W_LIT(x64w_Gpr8) { 0x0b })
#define x64w_r12b (X64W_LIT(x64w_Gpr8) { 0x0c })
#define x64w_r13b (X64W_LIT(x64w_Gpr8) { 0x0d })
#define x64w_r14b (X64W_LIT(x64w_Gpr8) { 0x0e })
#define x64w_r15b (X64W_LIT(x64w_Gpr8) { 0x0f })
#define x64w_spl  (X64W_LIT(x64w_Gpr8) { 0x14 })
#define x64w_bpl  (X64W_LIT(x64w_Gpr8) { 0x15 })
#define x64w_sil  (X64W_LIT(x64w_Gpr8) { 0x16 })
#define x64w_dil  (X64W_LIT(x64w_Gpr8) { 0x17 })

typedef struct { uint8_t i; } x64w_Gpr16;
#define x64w_ax   (X64W_LIT(x64w_Gpr16) { 0x00 })
#define x64w_cx   (X64W_LIT(x64w_Gpr16) { 0x01 })
#define x64w_dx   (X64W_LIT(x64w_Gpr16) { 0x02 })
#define x64w_bx   (X64W_LIT(x64w_Gpr16) { 0x03 })
#define x64w_sp   (X64W_LIT(x64w_Gpr16) { 0x04 })
#define x64w_bp   (X64W_LIT(x64w_Gpr16) { 0x05 })
#define x64w_si   (X64W_LIT(x64w_Gpr16) { 0x06 })
#define x64w_di   (X64W_LIT(x64w_Gpr16) { 0x07 })
#define x64w_r8w  (X64W_LIT(x64w_Gpr16) { 0x08 })
#define x64w_r9w  (X64W_LIT(x64w_Gpr16) { 0x09 })
#define x64w_r10w (X64W_LIT(x64w_Gpr16) { 0x0a })
#define x64w_r11w (X64W_LIT(x64w_Gpr16) { 0x0b })
#define x64w_r12w (X64W_LIT(x64w_Gpr16) { 0x0c })
#define x64w_r13w (X64W_LIT(x64w_Gpr16) { 0x0d })
#define x64w_r14w (X64W_LIT(x64w_Gpr16) { 0x0e })
#define x64w_r15w (X64W_LIT(x64w_Gpr16) { 0x0f })

typedef struct { uint8_t i; } x64w_Gpr32;
#define x64w_eax  (X64W_LIT(x64w_Gpr32) { 0x00 })
#define x64w_ecx  (X64W_LIT(x64w_Gpr32) { 0x01 })
#define x64w_edx  (X64W_LIT(x64w_Gpr32) { 0x02 })
#define x64w_ebx  (X64W_LIT(x64w_Gpr32) { 0x03 })
#define x64w_esp  (X64W_LIT(x64w_Gpr32) { 0x04 })
#define x64w_ebp  (X64W_LIT(x64w_Gpr32) { 0x05 })
#define x64w_esi  (X64W_LIT(x64w_Gpr32) { 0x06 })
#define x64w_edi  (X64W_LIT(x64w_Gpr32) { 0x07 })
#define x64w_r8d  (X64W_LIT(x64w_Gpr32) { 0x08 })
#define x64w_r9d  (X64W_LIT(x64w_Gpr32) { 0x09 })
#define x64w_r10d (X64W_LIT(x64w_Gpr32) { 0x0a })
#define x64w_r11d (X64W_LIT(x64w_Gpr32) { 0x0b })
#define x64w_r12d (X64W_LIT(x64w_Gpr32) { 0x0c })
#define x64w_r13d (X64W_LIT(x64w_Gpr32) { 0x0d })
#define x64w_r14d (X64W_LIT(x64w_Gpr32) { 0x0e })
#define x64w_r15d (X64W_LIT(x64w_Gpr32) { 0x0f })

typedef struct { uint8_t i; } x64w_Gpr64;
#define x64w_rax  (X64W_LIT(x64w_Gpr64) { 0x00 })
#define x64w_rcx  (X64W_LIT(x64w_Gpr64) { 0x01 })
#define x64w_rdx  (X64W_LIT(x64w_Gpr64) { 0x02 })
#define x64w_rbx  (X64W_LIT(x64w_Gpr64) { 0x03 })
#define x64w_rsp  (X64W_LIT(x64w_Gpr64) { 0x04 })
#define x64w_rbp  (X64W_LIT(x64w_Gpr64) { 0x05 })
#define x64w_rsi  (X64W_LIT(x64w_Gpr64) { 0x06 })
#define x64w_rdi  (X64W_LIT(x64w_Gpr64) { 0x07 })
#define x64w_r8   (X64W_LIT(x64w_Gpr64) { 0x08 })
#define x64w_r9   (X64W_LIT(x64w_Gpr64) { 0x09 })
#define x64w_r10  (X64W_LIT(x64w_Gpr64) { 0x0a })
#define x64w_r11  (X64W_LIT(x64w_Gpr64) { 0x0b })
#define x64w_r12  (X64W_LIT(x64w_Gpr64) { 0x0c })
#define x64w_r13  (X64W_LIT(x64w_Gpr64) { 0x0d })
#define x64w_r14  (X64W_LIT(x64w_Gpr64) { 0x0e })
#define x64w_r15  (X64W_LIT(x64w_Gpr64) { 0x0f })

typedef struct { uint8_t i; } x64w_Xmm;
#define x64w_xmm0  (X64W_LIT(x64w_Xmm) { 0x00 })
#define x64w_xmm1  (X64W_LIT(x64w_Xmm) { 0x01 })
#define x64w_xmm2  (X64W_LIT(x64w_Xmm) { 0x02 })
#define x64w_xmm3  (X64W_LIT(x64w_Xmm) { 0x03 })
#define x64w_xmm4  (X64W_LIT(x64w_Xmm) { 0x04 })
#define x64w_xmm5  (X64W_LIT(x64w_Xmm) { 0x05 })
#define x64w_xmm6  (X64W_LIT(x64w_Xmm) { 0x06 })
#define x64w_xmm7  (X64W_LIT(x64w_Xmm) { 0x07 })
#define x64w_xmm8  (X64W_LIT(x64w_Xmm) { 0x08 })
#define x64w_xmm9  (X64W_LIT(x64w_Xmm) { 0x09 })
#define x64w_xmm10 (X64W_LIT(x64w_Xmm) { 0x0a })
#define x64w_xmm11 (X64W_LIT(x64w_Xmm) { 0x0b })
#define x64w_xmm12 (X64W_LIT(x64w_Xmm) { 0x0c })
#define x64w_xmm13 (X64W_LIT(x64w_Xmm) { 0x0d })
#define x64w_xmm14 (X64W_LIT(x64w_Xmm) { 0x0e })
#define x64w_xmm15 (X64W_LIT(x64w_Xmm) { 0x0f })
#define x64w_xmm16 (X64W_LIT(x64w_Xmm) { 0x10 })
#define x64w_xmm17 (X64W_LIT(x64w_Xmm) { 0x11 })
#define x64w_xmm18 (X64W_LIT(x64w_Xmm) { 0x12 })
#define x64w_xmm19 (X64W_LIT(x64w_Xmm) { 0x13 })
#define x64w_xmm20 (X64W_LIT(x64w_Xmm) { 0x14 })
#define x64w_xmm21 (X64W_LIT(x64w_Xmm) { 0x15 })
#define x64w_xmm22 (X64W_LIT(x64w_Xmm) { 0x16 })
#define x64w_xmm23 (X64W_LIT(x64w_Xmm) { 0x17 })
#define x64w_xmm24 (X64W_LIT(x64w_Xmm) { 0x18 })
#define x64w_xmm25 (X64W_LIT(x64w_Xmm) { 0x19 })
#define x64w_xmm26 (X64W_LIT(x64w_Xmm) { 0x1a })
#define x64w_xmm27 (X64W_LIT(x64w_Xmm) { 0x1b })
#define x64w_xmm28 (X64W_LIT(x64w_Xmm) { 0x1c })
#define x64w_xmm29 (X64W_LIT(x64w_Xmm) { 0x1d })
#define x64w_xmm30 (X64W_LIT(x64w_Xmm) { 0x1e })
#define x64w_xmm31 (X64W_LIT(x64w_Xmm) { 0x1f })

typedef struct { uint8_t i; } x64w_Ymm;
#define x64w_ymm0  (X64W_LIT(x64w_Ymm) { 0x00 })
#define x64w_ymm1  (X64W_LIT(x64w_Ymm) { 0x01 })
#define x64w_ymm2  (X64W_LIT(x64w_Ymm) { 0x02 })
#define x64w_ymm3  (X64W_LIT(x64w_Ymm) { 0x03 })
#define x64w_ymm4  (X64W_LIT(x64w_Ymm) { 0x04 })
#define x64w_ymm5  (X64W_LIT(x64w_Ymm) { 0x05 })
#define x64w_ymm6  (X64W_LIT(x64w_Ymm) { 0x06 })
#define x64w_ymm7  (X64W_LIT(x64w_Ymm) { 0x07 })
#define x64w_ymm8  (X64W_LIT(x64w_Ymm) { 0x08 })
#define x64w_ymm9  (X64W_LIT(x64w_Ymm) { 0x09 })
#define x64w_ymm10 (X64W_LIT(x64w_Ymm) { 0x0a })
#define x64w_ymm11 (X64W_LIT(x64w_Ymm) { 0x0b })
#define x64w_ymm12 (X64W_LIT(x64w_Ymm) { 0x0c })
#define x64w_ymm13 (X64W_LIT(x64w_Ymm) { 0x0d })
#define x64w_ymm14 (X64W_LIT(x64w_Ymm) { 0x0e })
#define x64w_ymm15 (X64W_LIT(x64w_Ymm) { 0x0f })
#define x64w_ymm16 (X64W_LIT(x64w_Ymm) { 0x10 })
#define x64w_ymm17 (X64W_LIT(x64w_Ymm) { 0x11 })
#define x64w_ymm18 (X64W_LIT(x64w_Ymm) { 0x12 })
#define x64w_ymm19 (X64W_LIT(x64w_Ymm) { 0x13 })
#define x64w_ymm20 (X64W_LIT(x64w_Ymm) { 0x14 })
#define x64w_ymm21 (X64W_LIT(x64w_Ymm) { 0x15 })
#define x64w_ymm22 (X64W_LIT(x64w_Ymm) { 0x16 })
#define x64w_ymm23 (X64W_LIT(x64w_Ymm) { 0x17 })
#define x64w_ymm24 (X64W_LIT(x64w_Ymm) { 0x18 })
#define x64w_ymm25 (X64W_LIT(x64w_Ymm) { 0x19 })
#define x64w_ymm26 (X64W_LIT(x64w_Ymm) { 0x1a })
#define x64w_ymm27 (X64W_LIT(x64w_Ymm) { 0x1b })
#define x64w_ymm28 (X64W_LIT(x64w_Ymm) { 0x1c })
#define x64w_ymm29 (X64W_LIT(x64w_Ymm) { 0x1d })
#define x64w_ymm30 (X64W_LIT(x64w_Ymm) { 0x1e })
#define x64w_ymm31 (X64W_LIT(x64w_Ymm) { 0x1f })

typedef struct { uint8_t i; } x64w_Zmm;
#define x64w_zmm0  (X64W_LIT(x64w_Zmm) { 0x00 })
#define x64w_zmm1  (X64W_LIT(x64w_Zmm) { 0x01 })
#define x64w_zmm2  (X64W_LIT(x64w_Zmm) { 0x02 })
#define x64w_zmm3  (X64W_LIT(x64w_Zmm) { 0x03 })
#define x64w_zmm4  (X64W_LIT(x64w_Zmm) { 0x04 })
#define x64w_zmm5  (X64W_LIT(x64w_Zmm) { 0x05 })
#define x64w_zmm6  (X64W_LIT(x64w_Zmm) { 0x06 })
#define x64w_zmm7  (X64W_LIT(x64w_Zmm) { 0x07 })
#define x64w_zmm8  (X64W_LIT(x64w_Zmm) { 0x08 })
#define x64w_zmm9  (X64W_LIT(x64w_Zmm) { 0x09 })
#define x64w_zmm10 (X64W_LIT(x64w_Zmm) { 0x0a })
#define x64w_zmm11 (X64W_LIT(x64w_Zmm) { 0x0b })
#define x64w_zmm12 (X64W_LIT(x64w_Zmm) { 0x0c })
#define x64w_zmm13 (X64W_LIT(x64w_Zmm) { 0x0d })
#define x64w_zmm14 (X64W_LIT(x64w_Zmm) { 0x0e })
#define x64w_zmm15 (X64W_LIT(x64w_Zmm) { 0x0f })
#define x64w_zmm16 (X64W_LIT(x64w_Zmm) { 0x10 })
#define x64w_zmm17 (X64W_LIT(x64w_Zmm) { 0x11 })
#define x64w_zmm18 (X64W_LIT(x64w_Zmm) { 0x12 })
#define x64w_zmm19 (X64W_LIT(x64w_Zmm) { 0x13 })
#define x64w_zmm20 (X64W_LIT(x64w_Zmm) { 0x14 })
#define x64w_zmm21 (X64W_LIT(x64w_Zmm) { 0x15 })
#define x64w_zmm22 (X64W_LIT(x64w_Zmm) { 0x16 })
#define x64w_zmm23 (X64W_LIT(x64w_Zmm) { 0x17 })
#define x64w_zmm24 (X64W_LIT(x64w_Zmm) { 0x18 })
#define x64w_zmm25 (X64W_LIT(x64w_Zmm) { 0x19 })
#define x64w_zmm26 (X64W_LIT(x64w_Zmm) { 0x1a })
#define x64w_zmm27 (X64W_LIT(x64w_Zmm) { 0x1b })
#define x64w_zmm28 (X64W_LIT(x64w_Zmm) { 0x1c })
#define x64w_zmm29 (X64W_LIT(x64w_Zmm) { 0x1d })
#define x64w_zmm30 (X64W_LIT(x64w_Zmm) { 0x1e })
#define x64w_zmm31 (X64W_LIT(x64w_Zmm) { 0x1f })

// Use x64w_mem_* macros to construct this.
// This will ensure correct initialization.
typedef struct x64w_Mem {
	uint8_t base : 4;
	uint8_t index : 4;
	uint8_t base_scale : 1;
	uint8_t index_scale : 4; // allowed 0, 1, 2, 4 or 8
	uint8_t size_override : 1;
	int32_t displacement;
} x64w_Mem;

inline uint8_t x64w_ensure_arg_is_gpr32(x64w_Gpr32 r) { return r.i; }
inline uint8_t x64w_ensure_arg_is_gpr64(x64w_Gpr64 r) { return r.i; }

// Suffix determines argument type and count:
//     b - base register
//     i - index register + index scale constant
//     d - constant displacement
#define _x64w_mem_b(size, so, b)                                     \
	X64W_LIT(x64w_Mem) {                                             \
		.base = x64w_ensure_arg_is_gpr##size(b),                     \
		.base_scale = 1,                                             \
		.size_override = so,                                         \
	}

#define _x64w_mem_i(size, so, i, is)                                 \
	X64W_LIT(x64w_Mem) {                                             \
		.index = x64w_ensure_arg_is_gpr##size(i),                    \
		.index_scale = is,                                           \
		.size_override = so,                                         \
	}

#define _x64w_mem_d(size, so, d)                                     \
	X64W_LIT(x64w_Mem) {                                             \
		.displacement = d,                                           \
	}

#define _x64w_mem_bi(size, so, b, i, is)                             \
	X64W_LIT(x64w_Mem) {                                             \
		.base = x64w_ensure_arg_is_gpr##size(b),                     \
		.index = x64w_ensure_arg_is_gpr##size(i),                    \
		.base_scale = 1,                                             \
		.index_scale = is,                                           \
		.size_override = so,                                         \
	}

#define _x64w_mem_bd(size, so, b, d)                                 \
	X64W_LIT(x64w_Mem) {                                             \
		.base = x64w_ensure_arg_is_gpr##size(b),                     \
		.base_scale = 1,                                             \
		.size_override = so,                                         \
		.displacement = d,                                           \
	}

#define _x64w_mem_id(size, so, i, is, d)                             \
	X64W_LIT(x64w_Mem) {                                             \
		.index = x64w_ensure_arg_is_gpr##size(i),                    \
		.index_scale = is,                                           \
		.size_override = so,                                         \
		.displacement = d,                                           \
	}

#define _x64w_mem_bid(size, so, b, i, is, d)                         \
	X64W_LIT(x64w_Mem) {                                             \
		.base = x64w_ensure_arg_is_gpr##size(b),                     \
		.index = x64w_ensure_arg_is_gpr##size(i),                    \
		.base_scale = 1,                                             \
		.index_scale = is,                                           \
		.size_override = so,                                         \
		.displacement = d,                                           \
	}

#define x64w_mem32_b(b)             _x64w_mem_b(32, 1, b)
#define x64w_mem32_i(i, is)         _x64w_mem_i(32, 1, i, is)
#define x64w_mem32_d(d)             _x64w_mem_d(32, 1, d)
#define x64w_mem32_bi(b, i, is)     _x64w_mem_bi(32, 1, b, i, is)
#define x64w_mem32_bd(b, d)         _x64w_mem_bd(32, 1, b, d)
#define x64w_mem32_id(i, is, d)     _x64w_mem_id(32, 1, i, is, d)
#define x64w_mem32_bid(b, i, is, d) _x64w_mem_bid(32, 1, b, i, is, d)

#define x64w_mem64_b(b)             _x64w_mem_b(64, 0, b)
#define x64w_mem64_i(i, is)         _x64w_mem_i(64, 0, i, is)
#define x64w_mem64_d(d)             _x64w_mem_d(64, 0, d)
#define x64w_mem64_bi(b, i, is)     _x64w_mem_bi(64, 0, b, i, is)
#define x64w_mem64_bd(b, d)         _x64w_mem_bd(64, 0, b, d)
#define x64w_mem64_id(i, is, d)     _x64w_mem_id(64, 0, i, is, d)
#define x64w_mem64_bid(b, i, is, d) _x64w_mem_bid(64, 0, b, i, is, d)

enum x64w_DisplacementForm {
	x64w_df_no    = 0,
	x64w_df_8bit  = 1,
	x64w_df_32bit = 2,
};

// Returns:
//     0 - no displacement
//     1 - 8 bit displacement
//     2 - 32 bit displacement
X64W_DEF x64w_DisplacementForm x64w_displacement_form(x64w_Mem m);

X64W_DEF bool x64w_gpr8_compatible_rr(x64w_Gpr8 a, x64w_Gpr8 b);
X64W_DEF bool x64w_gpr8_compatible_rm(x64w_Gpr8 a, x64w_Mem b);

X64W_DEF x64w_Result x64w_push8i (uint8_t **c, int8_t   i);
X64W_DEF x64w_Result x64w_push32i(uint8_t **c, int32_t  i);
X64W_DEF x64w_Result x64w_push_r16(uint8_t **c, x64w_Gpr16 r);
X64W_DEF x64w_Result x64w_push_r64(uint8_t **c, x64w_Gpr64 r);
X64W_DEF x64w_Result x64w_push_m16(uint8_t **c, x64w_Mem m);
X64W_DEF x64w_Result x64w_push_m64(uint8_t **c, x64w_Mem m);

X64W_DEF x64w_Result x64w_pop_r16(uint8_t **c, x64w_Gpr16 r);
X64W_DEF x64w_Result x64w_pop_r64(uint8_t **c, x64w_Gpr64 r);
X64W_DEF x64w_Result x64w_pop_m16(uint8_t **c, x64w_Mem m);
X64W_DEF x64w_Result x64w_pop_m64(uint8_t **c, x64w_Mem m);

X64W_DEF x64w_Result x64w_mov_ri8 (uint8_t **c, x64w_Gpr8  r, int8_t  i);
X64W_DEF x64w_Result x64w_mov_ri16(uint8_t **c, x64w_Gpr16 r, int16_t i);
X64W_DEF x64w_Result x64w_mov_ri32(uint8_t **c, x64w_Gpr32 r, int32_t i);
X64W_DEF x64w_Result x64w_mov_ri64(uint8_t **c, x64w_Gpr64 r, int64_t i);
X64W_DEF x64w_Result x64w_mov_rr8 (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_mov_rr16(uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_mov_rr32(uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_mov_rr64(uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_mov_rm8 (uint8_t **c, x64w_Gpr8  r, x64w_Mem m);
X64W_DEF x64w_Result x64w_mov_rm16(uint8_t **c, x64w_Gpr16 r, x64w_Mem m);
X64W_DEF x64w_Result x64w_mov_rm32(uint8_t **c, x64w_Gpr32 r, x64w_Mem m);
X64W_DEF x64w_Result x64w_mov_rm64(uint8_t **c, x64w_Gpr64 r, x64w_Mem m);
X64W_DEF x64w_Result x64w_mov_mi8 (uint8_t **c, x64w_Mem m, int8_t  i);
X64W_DEF x64w_Result x64w_mov_mi16(uint8_t **c, x64w_Mem m, int16_t i);
X64W_DEF x64w_Result x64w_mov_mi32(uint8_t **c, x64w_Mem m, int32_t i);
X64W_DEF x64w_Result x64w_mov_m64i32(uint8_t **c, x64w_Mem m, int32_t i);
X64W_DEF x64w_Result x64w_mov_mr8 (uint8_t **c, x64w_Mem m, x64w_Gpr8  r);
X64W_DEF x64w_Result x64w_mov_mr16(uint8_t **c, x64w_Mem m, x64w_Gpr16 r);
X64W_DEF x64w_Result x64w_mov_mr32(uint8_t **c, x64w_Mem m, x64w_Gpr32 r);
X64W_DEF x64w_Result x64w_mov_mr64(uint8_t **c, x64w_Mem m, x64w_Gpr64 r);

X64W_DEF x64w_Result x64w_adcx_rr32(uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_adcx_rr64(uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s);

X64W_DEF x64w_Result x64w_adc_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i);
X64W_DEF x64w_Result x64w_adc_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i);
X64W_DEF x64w_Result x64w_adc_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i);
X64W_DEF x64w_Result x64w_adc_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i);
X64W_DEF x64w_Result x64w_adc_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i);
X64W_DEF x64w_Result x64w_adc_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i);
X64W_DEF x64w_Result x64w_adc_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i);
X64W_DEF x64w_Result x64w_adc_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_adc_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_adc_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_adc_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_adc_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_adc_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_adc_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_adc_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_adc_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i);
X64W_DEF x64w_Result x64w_adc_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_adc_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_adc_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_adc_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_adc_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_adc_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_adc_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_adc_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_adc_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_adc_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_add_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i);
X64W_DEF x64w_Result x64w_add_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i);
X64W_DEF x64w_Result x64w_add_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i);
X64W_DEF x64w_Result x64w_add_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i);
X64W_DEF x64w_Result x64w_add_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i);
X64W_DEF x64w_Result x64w_add_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i);
X64W_DEF x64w_Result x64w_add_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i);
X64W_DEF x64w_Result x64w_add_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_add_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_add_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_add_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_add_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_add_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_add_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_add_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_add_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i);
X64W_DEF x64w_Result x64w_add_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_add_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_add_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_add_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_add_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_add_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_add_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_add_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_add_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_add_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_xor_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i);
X64W_DEF x64w_Result x64w_xor_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i);
X64W_DEF x64w_Result x64w_xor_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i);
X64W_DEF x64w_Result x64w_xor_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i);
X64W_DEF x64w_Result x64w_xor_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i);
X64W_DEF x64w_Result x64w_xor_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i);
X64W_DEF x64w_Result x64w_xor_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i);
X64W_DEF x64w_Result x64w_xor_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_xor_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_xor_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_xor_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_xor_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_xor_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_xor_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_xor_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_xor_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i);
X64W_DEF x64w_Result x64w_xor_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_xor_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_xor_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_xor_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_xor_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_xor_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_xor_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_xor_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_xor_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_xor_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_and_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i);
X64W_DEF x64w_Result x64w_and_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i);
X64W_DEF x64w_Result x64w_and_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i);
X64W_DEF x64w_Result x64w_and_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i);
X64W_DEF x64w_Result x64w_and_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i);
X64W_DEF x64w_Result x64w_and_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i);
X64W_DEF x64w_Result x64w_and_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i);
X64W_DEF x64w_Result x64w_and_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_and_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_and_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_and_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_and_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_and_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_and_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_and_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_and_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i);
X64W_DEF x64w_Result x64w_and_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_and_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_and_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_and_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_and_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_and_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_and_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_and_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_and_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_and_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_or_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i);
X64W_DEF x64w_Result x64w_or_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i);
X64W_DEF x64w_Result x64w_or_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i);
X64W_DEF x64w_Result x64w_or_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i);
X64W_DEF x64w_Result x64w_or_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i);
X64W_DEF x64w_Result x64w_or_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i);
X64W_DEF x64w_Result x64w_or_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i);
X64W_DEF x64w_Result x64w_or_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_or_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_or_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_or_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_or_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_or_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_or_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_or_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_or_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i);
X64W_DEF x64w_Result x64w_or_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_or_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_or_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_or_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_or_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_or_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_or_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_or_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_or_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_or_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_sub_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i);
X64W_DEF x64w_Result x64w_sub_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i);
X64W_DEF x64w_Result x64w_sub_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i);
X64W_DEF x64w_Result x64w_sub_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i);
X64W_DEF x64w_Result x64w_sub_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i);
X64W_DEF x64w_Result x64w_sub_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i);
X64W_DEF x64w_Result x64w_sub_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i);
X64W_DEF x64w_Result x64w_sub_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_sub_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_sub_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_sub_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_sub_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_sub_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_sub_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_sub_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s);
X64W_DEF x64w_Result x64w_sub_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i);
X64W_DEF x64w_Result x64w_sub_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_sub_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_sub_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_sub_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i);
X64W_DEF x64w_Result x64w_sub_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_sub_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i);
X64W_DEF x64w_Result x64w_sub_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s);
X64W_DEF x64w_Result x64w_sub_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s);
X64W_DEF x64w_Result x64w_sub_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s);
X64W_DEF x64w_Result x64w_sub_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s);
X64W_DEF x64w_Result x64w_inc_r8   (uint8_t **c, x64w_Gpr8  d);
X64W_DEF x64w_Result x64w_inc_r16  (uint8_t **c, x64w_Gpr16 d);
X64W_DEF x64w_Result x64w_inc_r32  (uint8_t **c, x64w_Gpr32 d);
X64W_DEF x64w_Result x64w_inc_r64  (uint8_t **c, x64w_Gpr64 d);
X64W_DEF x64w_Result x64w_inc_m8   (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_inc_m16  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_inc_m32  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_inc_m64  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_dec_r8   (uint8_t **c, x64w_Gpr8  d);
X64W_DEF x64w_Result x64w_dec_r16  (uint8_t **c, x64w_Gpr16 d);
X64W_DEF x64w_Result x64w_dec_r32  (uint8_t **c, x64w_Gpr32 d);
X64W_DEF x64w_Result x64w_dec_r64  (uint8_t **c, x64w_Gpr64 d);
X64W_DEF x64w_Result x64w_dec_m8   (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_dec_m16  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_dec_m32  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_dec_m64  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_not_r8   (uint8_t **c, x64w_Gpr8  d);
X64W_DEF x64w_Result x64w_not_r16  (uint8_t **c, x64w_Gpr16 d);
X64W_DEF x64w_Result x64w_not_r32  (uint8_t **c, x64w_Gpr32 d);
X64W_DEF x64w_Result x64w_not_r64  (uint8_t **c, x64w_Gpr64 d);
X64W_DEF x64w_Result x64w_not_m8   (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_not_m16  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_not_m32  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_not_m64  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_neg_r8   (uint8_t **c, x64w_Gpr8  d);
X64W_DEF x64w_Result x64w_neg_r16  (uint8_t **c, x64w_Gpr16 d);
X64W_DEF x64w_Result x64w_neg_r32  (uint8_t **c, x64w_Gpr32 d);
X64W_DEF x64w_Result x64w_neg_r64  (uint8_t **c, x64w_Gpr64 d);
X64W_DEF x64w_Result x64w_neg_m8   (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_neg_m16  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_neg_m32  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_neg_m64  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_mul_r8   (uint8_t **c, x64w_Gpr8  d);
X64W_DEF x64w_Result x64w_mul_r16  (uint8_t **c, x64w_Gpr16 d);
X64W_DEF x64w_Result x64w_mul_r32  (uint8_t **c, x64w_Gpr32 d);
X64W_DEF x64w_Result x64w_mul_r64  (uint8_t **c, x64w_Gpr64 d);
X64W_DEF x64w_Result x64w_mul_m8   (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_mul_m16  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_mul_m32  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_mul_m64  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_div_r8   (uint8_t **c, x64w_Gpr8  d);
X64W_DEF x64w_Result x64w_div_r16  (uint8_t **c, x64w_Gpr16 d);
X64W_DEF x64w_Result x64w_div_r32  (uint8_t **c, x64w_Gpr32 d);
X64W_DEF x64w_Result x64w_div_r64  (uint8_t **c, x64w_Gpr64 d);
X64W_DEF x64w_Result x64w_div_m8   (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_div_m16  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_div_m32  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_div_m64  (uint8_t **c, x64w_Mem   d);
X64W_DEF x64w_Result x64w_shl_r8_1  (uint8_t **c, x64w_Gpr8  r           );
X64W_DEF x64w_Result x64w_shl_r16_1 (uint8_t **c, x64w_Gpr16 r           );
X64W_DEF x64w_Result x64w_shl_r32_1 (uint8_t **c, x64w_Gpr32 r           );
X64W_DEF x64w_Result x64w_shl_r64_1 (uint8_t **c, x64w_Gpr64 r           );
X64W_DEF x64w_Result x64w_shl_ri8   (uint8_t **c, x64w_Gpr8  r, uint8_t i);
X64W_DEF x64w_Result x64w_shl_r16i8 (uint8_t **c, x64w_Gpr16 r, uint8_t i);
X64W_DEF x64w_Result x64w_shl_r32i8 (uint8_t **c, x64w_Gpr32 r, uint8_t i);
X64W_DEF x64w_Result x64w_shl_r64i8 (uint8_t **c, x64w_Gpr64 r, uint8_t i);
X64W_DEF x64w_Result x64w_shl_r8_cl (uint8_t **c, x64w_Gpr8  r           );
X64W_DEF x64w_Result x64w_shl_r16_cl(uint8_t **c, x64w_Gpr16 r           );
X64W_DEF x64w_Result x64w_shl_r32_cl(uint8_t **c, x64w_Gpr32 r           );
X64W_DEF x64w_Result x64w_shl_r64_cl(uint8_t **c, x64w_Gpr64 r           );
X64W_DEF x64w_Result x64w_shl_m8_1  (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shl_m16_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shl_m32_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shl_m64_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shl_mi8   (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_shl_m16i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_shl_m32i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_shl_m64i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_shl_m8_cl (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shl_m16_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shl_m32_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shl_m64_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shr_r8_1  (uint8_t **c, x64w_Gpr8  r           );
X64W_DEF x64w_Result x64w_shr_r16_1 (uint8_t **c, x64w_Gpr16 r           );
X64W_DEF x64w_Result x64w_shr_r32_1 (uint8_t **c, x64w_Gpr32 r           );
X64W_DEF x64w_Result x64w_shr_r64_1 (uint8_t **c, x64w_Gpr64 r           );
X64W_DEF x64w_Result x64w_shr_ri8   (uint8_t **c, x64w_Gpr8  r, uint8_t i);
X64W_DEF x64w_Result x64w_shr_r16i8 (uint8_t **c, x64w_Gpr16 r, uint8_t i);
X64W_DEF x64w_Result x64w_shr_r32i8 (uint8_t **c, x64w_Gpr32 r, uint8_t i);
X64W_DEF x64w_Result x64w_shr_r64i8 (uint8_t **c, x64w_Gpr64 r, uint8_t i);
X64W_DEF x64w_Result x64w_shr_r8_cl (uint8_t **c, x64w_Gpr8  r           );
X64W_DEF x64w_Result x64w_shr_r16_cl(uint8_t **c, x64w_Gpr16 r           );
X64W_DEF x64w_Result x64w_shr_r32_cl(uint8_t **c, x64w_Gpr32 r           );
X64W_DEF x64w_Result x64w_shr_r64_cl(uint8_t **c, x64w_Gpr64 r           );
X64W_DEF x64w_Result x64w_shr_m8_1  (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shr_m16_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shr_m32_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shr_m64_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shr_mi8   (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_shr_m16i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_shr_m32i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_shr_m64i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_shr_m8_cl (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shr_m16_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shr_m32_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_shr_m64_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sal_r8_1  (uint8_t **c, x64w_Gpr8  r           );
X64W_DEF x64w_Result x64w_sal_r16_1 (uint8_t **c, x64w_Gpr16 r           );
X64W_DEF x64w_Result x64w_sal_r32_1 (uint8_t **c, x64w_Gpr32 r           );
X64W_DEF x64w_Result x64w_sal_r64_1 (uint8_t **c, x64w_Gpr64 r           );
X64W_DEF x64w_Result x64w_sal_ri8   (uint8_t **c, x64w_Gpr8  r, uint8_t i);
X64W_DEF x64w_Result x64w_sal_r16i8 (uint8_t **c, x64w_Gpr16 r, uint8_t i);
X64W_DEF x64w_Result x64w_sal_r32i8 (uint8_t **c, x64w_Gpr32 r, uint8_t i);
X64W_DEF x64w_Result x64w_sal_r64i8 (uint8_t **c, x64w_Gpr64 r, uint8_t i);
X64W_DEF x64w_Result x64w_sal_r8_cl (uint8_t **c, x64w_Gpr8  r           );
X64W_DEF x64w_Result x64w_sal_r16_cl(uint8_t **c, x64w_Gpr16 r           );
X64W_DEF x64w_Result x64w_sal_r32_cl(uint8_t **c, x64w_Gpr32 r           );
X64W_DEF x64w_Result x64w_sal_r64_cl(uint8_t **c, x64w_Gpr64 r           );
X64W_DEF x64w_Result x64w_sal_m8_1  (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sal_m16_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sal_m32_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sal_m64_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sal_mi8   (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_sal_m16i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_sal_m32i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_sal_m64i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_sal_m8_cl (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sal_m16_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sal_m32_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sal_m64_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sar_r8_1  (uint8_t **c, x64w_Gpr8  r           );
X64W_DEF x64w_Result x64w_sar_r16_1 (uint8_t **c, x64w_Gpr16 r           );
X64W_DEF x64w_Result x64w_sar_r32_1 (uint8_t **c, x64w_Gpr32 r           );
X64W_DEF x64w_Result x64w_sar_r64_1 (uint8_t **c, x64w_Gpr64 r           );
X64W_DEF x64w_Result x64w_sar_ri8   (uint8_t **c, x64w_Gpr8  r, uint8_t i);
X64W_DEF x64w_Result x64w_sar_r16i8 (uint8_t **c, x64w_Gpr16 r, uint8_t i);
X64W_DEF x64w_Result x64w_sar_r32i8 (uint8_t **c, x64w_Gpr32 r, uint8_t i);
X64W_DEF x64w_Result x64w_sar_r64i8 (uint8_t **c, x64w_Gpr64 r, uint8_t i);
X64W_DEF x64w_Result x64w_sar_r8_cl (uint8_t **c, x64w_Gpr8  r           );
X64W_DEF x64w_Result x64w_sar_r16_cl(uint8_t **c, x64w_Gpr16 r           );
X64W_DEF x64w_Result x64w_sar_r32_cl(uint8_t **c, x64w_Gpr32 r           );
X64W_DEF x64w_Result x64w_sar_r64_cl(uint8_t **c, x64w_Gpr64 r           );
X64W_DEF x64w_Result x64w_sar_m8_1  (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sar_m16_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sar_m32_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sar_m64_1 (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sar_mi8   (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_sar_m16i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_sar_m32i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_sar_m64i8 (uint8_t **c, x64w_Mem   m, uint8_t i);
X64W_DEF x64w_Result x64w_sar_m8_cl (uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sar_m16_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sar_m32_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_sar_m64_cl(uint8_t **c, x64w_Mem   m           );
X64W_DEF x64w_Result x64w_lea_rm16(uint8_t **c, x64w_Gpr16 r, x64w_Mem m);
X64W_DEF x64w_Result x64w_lea_rm32(uint8_t **c, x64w_Gpr32 r, x64w_Mem m);
X64W_DEF x64w_Result x64w_lea_rm64(uint8_t **c, x64w_Gpr64 r, x64w_Mem m);


#ifdef X64W_IMPLEMENTATION

#define X64W_GPR8_NEEDS_REX(gpr) (!!((gpr) & 0x10))

#ifndef X64W_VALIDATE
#define X64W_VALIDATE(condition, message) do { if (!(condition)) { *c = restore; return message; } } while (0)
#endif

bool x64w_gpr8_compatible_rr(x64w_Gpr8 a, x64w_Gpr8 b) {
	if (x64w_ah.i <= a.i && a.i <= x64w_bh.i) return x64w_al.i <= b.i && b.i <= x64w_bh.i;
	if (x64w_ah.i <= b.i && b.i <= x64w_bh.i) return x64w_al.i <= a.i && a.i <= x64w_bh.i;
	return true;
}
bool x64w_gpr8_compatible_rm(x64w_Gpr8 a, x64w_Mem b) {
	if (x64w_ah.i <= a.i && a.i <= x64w_bh.i) return x64w_al.i <= b.base && b.base <= x64w_bh.i && x64w_al.i <= b.index && b.index <= x64w_bh.i;
	return true;
}

#ifdef X64W_DISABLE_VALIDATION
	#define X64W_VALIDATE(condition, message)
	#define X64W_VALIDATE_R(r)
	#define X64W_VALIDATE_M(m)
#else

#define X64W_VALIDATE_R(r)                                                           \
	do {                                                                             \
		if (size == 1) {                                                             \
			X64W_VALIDATE(r < 0x10 || (0x14 <= r && r < 0x18), "invalid register");  \
		} else {                                                                     \
			X64W_VALIDATE(r < 0x10, "invalid register");                             \
		}                                                                            \
	} while (0)

#define X64W_VALIDATE_M(m)                                                                     \
	do {                                                                                       \
		if (m.base_scale == 0) {                                                               \
			X64W_VALIDATE(m.base == 0, "base register should be zero if its scale is zero");   \
		}                                                                                      \
		X64W_VALIDATE(                                                                         \
			m.index_scale == 0 ||                                                              \
			m.index_scale == 1 ||                                                              \
			m.index_scale == 2 ||                                                              \
			m.index_scale == 4 ||                                                              \
			m.index_scale == 8, "invalid index scale"                                          \
		);                                                                                     \
		if (m.index_scale) {                                                                   \
			X64W_VALIDATE(m.index != 4, "stack pointer register cannot be used as index");     \
		} else {                                                                               \
			X64W_VALIDATE(m.index == 0, "index register should be zero if its scale is zero"); \
		}                                                                                      \
	} while (0)

#define X64W_VALIDATE_RR(a, b)                                                                     \
	do {                                                                                           \
		X64W_VALIDATE_R(a);                                                                        \
		X64W_VALIDATE_R(b);                                                                        \
		if (size == 1) {                                                                           \
			X64W_VALIDATE(x64w_gpr8_compatible_rr(X64W_LIT(x64w_Gpr8){a}, X64W_LIT(x64w_Gpr8){b}), \
				"ah,ch,dh,bh cannot be used with r8-15,spl,bpl,sil,dil");                          \
		}                                                                                          \
	} while (0)

#define X64W_VALIDATE_RM(r, m)                                                \
	do {                                                                      \
		X64W_VALIDATE_R(r);                                                   \
		X64W_VALIDATE_M(m);                                                   \
		if (size == 1) {                                                      \
			X64W_VALIDATE(x64w_gpr8_compatible_rm(X64W_LIT(x64w_Gpr8){r}, m), \
				"ah,ch,dh,bh cannot be used with r8-15,spl,bpl,sil,dil");     \
		}                                                                     \
	} while (0)

#define X64W_VALIDATE_X(x)                                           \
	do {                                                             \
		if (size == 64) X64W_VALIDATE(x < 0x10, "invalid register"); \
		else            X64W_VALIDATE(x < 0x20, "invalid register"); \
	} while (0)

#endif

#define x64w_fits_in_8(x)  ((x) == (int8_t )(x))
#define x64w_fits_in_16(x) ((x) == (int16_t)(x))
#define x64w_fits_in_32(x) ((x) == (int32_t)(x))

#ifdef X64W_BSWAP
	#define W2(c, x)                                               \
		do {                                                       \
			uint8_t *check = c;                                    \
			(void)check;                                           \
			uint16_t y = x;                                        \
			y = ((y & 0xff00ff00) >> 8) | ((y << 8) & 0xff00ff00); \
			*(uint16_t *)(c) = y;                                  \
		} while (0)
	#define W4(c, x)                                                 \
		do {                                                         \
			uint8_t *check = c;                                      \
			(void)check;                                             \
			uint32_t y = x;                                          \
			y = ((y & 0xffff0000) >> 16) | ((y << 16) & 0xffff0000); \
			y = ((y & 0xff00ff00) >>  8) | ((y <<  8) & 0xff00ff00); \
			*(uint32_t *)(c) = y;                                    \
		} while (0)
	#define W8(c, x)                                                                 \
		do {                                                                         \
			uint8_t *check = c;                                                      \
			(void)check;                                                             \
			uint64_t y = x;                                                          \
			y = ((y & 0xffffffff00000000) >> 32) | ((y << 32) & 0xffffffff00000000); \
			y = ((y & 0xffff0000ffff0000) >> 16) | ((y << 16) & 0xffff0000ffff0000); \
			y = ((y & 0xff00ff00ff00ff00) >>  8) | ((y <<  8) & 0xff00ff00ff00ff00); \
			*(uint64_t *)(c) = y;                                                    \
		} while (0)
#else
	#define W2(c, x) do { uint8_t *check = c; (void)check; *(uint16_t *)(c) = x; } while (0)
	#define W4(c, x) do { uint8_t *check = c; (void)check; *(uint32_t *)(c) = x; } while (0)
	#define W8(c, x) do { uint8_t *check = c; (void)check; *(uint64_t *)(c) = x; } while (0)
#endif

#define REXW     0x1
#define OSO      0x2 // operand size override
#define ASO      0x4 // address size override
#define NO_MODRM 0x8

#define vex_m_0f   1
#define vex_m_0f38 2
#define vex_m_0f3a 3

#define vex_p_none 0
#define vex_p_66   1
#define vex_p_f3   2
#define vex_p_f2   3

static const uint8_t index_scale_table[] = {
	0,    // 0
	0x00, // 1
	0x40, // 2
	0,    // 3
	0x80, // 4
	0,    // 5
	0,    // 6
	0,    // 7
	0xc0, // 8
};

x64w_DisplacementForm x64w_displacement_form(x64w_Mem m) {
	if (m.displacement == 0 && ((m.base & 7) != 5)) {
		return x64w_df_no;
	} else if (x64w_fits_in_8(m.displacement)) {
		return x64w_df_8bit;
	} else {
		return x64w_df_32bit;
	}
}

static void write_rex(uint8_t **c, bool w, bool r, bool i, bool b, bool force) {
	**c = 0x40 | (w << 3) | (r << 2) | (i << 1) | (int)b;
	*c += w | r | i | b | force;
}
static void write_vex2(uint8_t **c, bool r, uint8_t v, bool l, uint8_t p) {
	*(*c)++ = 0xc5;
	*(*c)++ = (!r << 7) | ((v ^ 0xf) << 3) | (l << 2) | p;
}
static void write_vex3(uint8_t **c, bool r, bool x, bool b, uint8_t m, bool w, uint8_t v, bool l, uint8_t p) {
	*(*c)++ = 0xc4;
	*(*c)++ = (!r << 7) | (!x << 6) | (!b << 5) | m;
	*(*c)++ = (!w << 7) | ((v ^ 0xf) << 3) | (l << 2) | p;
}
static void write_vex(uint8_t **c, bool r, bool x, bool b, uint8_t m, bool w, uint8_t v, bool l, uint8_t p) {
	if (x | b | w) {
		write_vex3(c, r, x, b, m, w, v, l, p);
	} else {
		write_vex2(c, r, v, l, p);
	}
}
static void write_evex(uint8_t **c, bool R, bool X, bool B, bool Rh, uint8_t m, bool W, uint8_t v, uint8_t p, bool z, uint8_t L, bool b, bool vh, uint8_t a) {
	*(*c)++ = 0x62;
	*(*c)++ = (!R << 7) | (!X << 6) | (!B << 5) | (!Rh << 4) | m;
	*(*c)++ = (W << 7) | ((v ^ 0xf) << 3) | 0x04 | p;
	*(*c)++ = (z << 7) | (L << 5) | (b << 4) | (!vh << 3) | a;
}
static void write_opcode(uint8_t **c, uint32_t opcode) {
	if (opcode <= 0xff) {
		*(*c)++ = opcode;
	} else if (opcode <= 0xffff) {
		*(*c)++ = opcode >> 8;
		*(*c)++ = opcode & 0xff;
	} else {
		*(*c)++ = opcode >> 16;
		*(*c)++ = (opcode >> 8) & 0xff;
		*(*c)++ = opcode & 0xff;
	}
}
static void write_displacement(uint8_t **c, int displacement_form, int32_t displacement) {
	**c = (uint8_t)displacement;
	*c += displacement_form == 1;
	W4(*c, displacement);
	*c += 4 * (displacement_form == 2);
}
static void write_m(uint8_t **c, x64w_Mem m, uint8_t mod, unsigned r7, unsigned i7, unsigned b7) {
	unsigned s = index_scale_table[m.index_scale];

	if (m.base_scale) {
		int df = x64w_displacement_form(m);
		if (m.index_scale) {
			*(*c)++ = mod | (df << 6) | (r7 << 3) | 0x04;
			*(*c)++ = s | (i7 << 3) | b7;
		} else {
			*(*c)++ = mod | (df << 6) | (r7 << 3) | b7;
			**c = 0x24;
			*c += b7 == 4;
		}
		write_displacement(c, df, m.displacement);
	} else {
		*(*c)++ = mod | (r7 << 3) | 0x04;
		if (m.index_scale) {
			*(*c)++ = s | (i7 << 3) | 0x05;
		} else {
			*(*c)++ = 0x25;
		}
		W4(*c, m.displacement);
		*c += 4;
	}
}
static void write_immediate(uint8_t **c, int64_t i, unsigned size) {
	switch (size) {
		case 1: **c = (uint8_t)i; break;
		case 2: W2(*c, (uint16_t)i); break;
		case 4: W4(*c, (uint32_t)i); break;
		case 8: W8(*c, (uint64_t)i); break;
	}
	*c += size;
}

#ifdef _MSC_VER
#define no_inline __declspec(noinline)
#else
#define no_inline __attribute__((noinline))
#endif

#ifdef _MSC_VER
#define force_inline __forceinline
#else
#define force_inline __attribute__((always_inline))
#endif

#if defined(X64W_NO_INLINE)
#define instr_inline no_inline
#elif defined(X64W_FORCE_INLINE)
#define instr_inline force_inline
#else
#define instr_inline
#endif

static instr_inline x64w_Result instr_i1(uint8_t **c, int8_t i, uint32_t opcode) {
	write_opcode(c, opcode);
	*(*c)++ = i;
	return 0;
}
static instr_inline x64w_Result instr_i4(uint8_t **c, int32_t i, uint32_t opcode) {
	write_opcode(c, opcode);
	W4(*c, i);
	*c += 4;
	return 0;
}
static instr_inline x64w_Result instr_r(uint8_t **c, uint8_t r, unsigned size, uint32_t opcode, uint8_t mod, uint64_t flags) {
	uint8_t *restore = *c;
	X64W_VALIDATE_R(r);

	mod <<= 3;
	
	unsigned rexb = !!(r & 8);
	
	unsigned rexw          = !!(flags & REXW);
	unsigned size_override = !!(flags & OSO);
	unsigned no_modrm      = !!(flags & NO_MODRM);
	
	**c = 0x66;
	*c += size_override;

	write_rex(c, rexw, 0, 0, rexb, X64W_GPR8_NEEDS_REX(r));

	if (no_modrm) {
		*(*c)++ = opcode | (r & 7);
	} else {
		*(*c)++ = opcode;
		*(*c)++ = 0xc0 | mod | (r & 7);
	}

	return 0;
}
static instr_inline x64w_Result instr_ri(uint8_t **c, uint8_t r, int64_t i, unsigned size, uint32_t opcode, uint8_t mod, uint64_t flags) {
	uint8_t *restore = *c;
	X64W_VALIDATE_R(r);

	mod <<= 3;
	
	unsigned rexb = !!(r & 8);
	
	unsigned rexw          = !!(flags & REXW);
	unsigned size_override = !!(flags & OSO);
	unsigned no_modrm      = !!(flags & NO_MODRM);
	
	**c = 0x66;
	*c += size_override;

	write_rex(c, rexw, 0, 0, rexb, X64W_GPR8_NEEDS_REX(r));

	if (no_modrm) {
		*(*c)++ = opcode | (r & 7);
	} else {
		write_opcode(c, opcode);
		*(*c)++ = 0xc0 | mod | (r & 7);
	}
	
	write_immediate(c, i, size);

	return 0;
}
static instr_inline x64w_Result instr_m(uint8_t **c, x64w_Mem d, uint32_t opcode, uint8_t mod, uint64_t flags) {
	uint8_t *restore = *c;
	X64W_VALIDATE_M(d);
	
	mod <<= 3;
	
	unsigned rexw          = !!(flags & REXW);
	unsigned size_override = !!(flags & OSO);
	
	unsigned b7 = d.base & 7;
	unsigned i7 = d.index & 7;
	unsigned rexb = d.base >> 3;
	unsigned rexi = d.index >> 3;
	
	**c = 0x67;
	*c += d.size_override;

	**c = 0x66;
	*c += size_override;

	write_rex(c, rexw, 0, rexi, rexb, 0);

	write_opcode(c, opcode);

	write_m(c, d, mod, 0, i7, b7);

	return 0;
}
static instr_inline x64w_Result instr_rr(uint8_t **c, uint8_t d, uint8_t s, unsigned size, uint32_t opcode, uint64_t flags) {
	uint8_t *restore = *c;
	X64W_VALIDATE_RR(d, s);
	
	unsigned rexw          = !!(flags & REXW);
	unsigned size_override = !!(flags & OSO);
	
	unsigned rexb = !!(s & 8);
	unsigned rexr = !!(d & 8);
	
	**c = 0x66;
	*c += size_override;

	write_rex(c, rexw, rexr, 0, rexb, X64W_GPR8_NEEDS_REX(d) | X64W_GPR8_NEEDS_REX(s));

	write_opcode(c, opcode);

	*(*c)++ = 0xc0 | (s & 7) | ((d & 7) << 3);

	return 0;
}
static instr_inline x64w_Result instr_rm(uint8_t **c, uint8_t r, x64w_Mem m, unsigned size, uint32_t opcode, uint64_t flags) {
	uint8_t *restore = *c;
	X64W_VALIDATE_RM(r, m);
	
	unsigned rexw          = !!(flags & REXW);
	unsigned size_override = !!(flags & OSO);
	
	unsigned r7 = r & 7;
	unsigned b7 = m.base & 7;
	unsigned i7 = m.index & 7;
	unsigned rexb = m.base >> 3;
	unsigned rexi = m.index >> 3;
	unsigned rexr = !!(r & 8);
	
	**c = 0x67;
	*c += m.size_override;

	**c = 0x66;
	*c += size_override;

	write_rex(c, rexw, rexr, rexi, rexb, X64W_GPR8_NEEDS_REX(r));
	
	write_opcode(c, opcode);
	
	write_m(c, m, 0, r7, i7, b7);

	return 0;
}
static instr_inline x64w_Result instr_mi(uint8_t **c, x64w_Mem m, int64_t i, unsigned size, uint32_t opcode, uint8_t mod, uint64_t flags) {
	uint8_t *restore = *c;
	X64W_VALIDATE_M(m);
	
	mod <<= 3;
	
	unsigned rexw          = !!(flags & REXW);
	unsigned size_override = !!(flags & OSO);
	
	unsigned b7 = m.base & 7;
	unsigned i7 = m.index & 7;
	unsigned rexb = m.base >> 3;
	unsigned rexi = m.index >> 3;
	
	**c = 0x67;
	*c += m.size_override;

	**c = 0x66;
	*c += size_override;

	write_rex(c, rexw, 0, rexi, rexb, 0);

	write_opcode(c, opcode);
	
	write_m(c, m, mod, 0, i7, b7);

	write_immediate(c, i, size);

	return 0;
}
static instr_inline x64w_Result instr_xxx(uint8_t **c, uint8_t d, uint8_t a, uint8_t b, unsigned size, uint32_t opcode) {
	uint8_t *restore = *c;
	X64W_VALIDATE_X(d);
	X64W_VALIDATE_X(a);
	X64W_VALIDATE_X(b);
	
	unsigned rexr = !!(d & 8);
	unsigned rexb = !!(b & 8);
	unsigned rexrh = !!(d & 16);
	
	if (size == 64) {
		write_evex(c, rexr, 0, rexb, rexrh, vex_m_0f, 1, a & 15, vex_p_66, 0, 2, 0, !!(a & 16), 0);
	} else {
		write_vex(c, rexr, 0, rexb, vex_m_0f, 0, a, size == 32, vex_p_66);
	}

	write_opcode(c, 0x58);

	*(*c)++ = 0xc0 | (b & 7) | ((d & 7) << 3);

	return 0;
}
static instr_inline x64w_Result instr_xxm(uint8_t **c, uint8_t d, uint8_t a, x64w_Mem b, unsigned size, uint32_t opcode) {
	uint8_t *restore = *c;
	X64W_VALIDATE_X(d);
	X64W_VALIDATE_X(a);
	X64W_VALIDATE_M(b);

	unsigned r7 = d & 7;
	unsigned b7 = b.base & 7;
	unsigned i7 = b.index & 7;
	unsigned rexb = b.base >> 3;
	unsigned rexi = b.index >> 3;
	unsigned rexr = !!(d & 8);
	unsigned rexrh = !!(d & 16);
	
	**c = 0x67;
	*c += b.size_override;

	if (size == 64) {
		write_evex(c, rexr, rexi, rexb, rexrh, vex_m_0f, 1, a & 15, vex_p_66, 0, 2, 0, !!(a & 16), 0);
	} else {
		write_vex(c, rexr, rexi, rexb, vex_m_0f, 0, a, size == 32, vex_p_66);
	}

	write_opcode(c, 0x58);
	
	if (size == 64)
		return "not implemented";

	// for zmm, mod has to be something different

  // 62 F1 FD 48 58 04 25 00 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [0000000000000000h]
  // 62 F1 FD 48 58 00                 vaddpd zmm0,zmm0,zmmword ptr [rax]
  // 62 F1 FD 48 58 04 24              vaddpd zmm0,zmm0,zmmword ptr [rsp]
  // 62 F1 FD 48 58 45 00              vaddpd zmm0,zmm0,zmmword ptr [rbp]
  // 62 D1 FD 48 58 00                 vaddpd zmm0,zmm0,zmmword ptr [r8]
  // 62 F1 FD 48 58 80 01 00 00 00     vaddpd zmm0,zmm0,zmmword ptr [rax+0000000000000001h]
  // 62 F1 FD 48 58 84 24 01 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rsp+0000000000000001h]
  // 62 F1 FD 48 58 85 01 00 00 00     vaddpd zmm0,zmm0,zmmword ptr [rbp+0000000000000001h]
  // 62 D1 FD 48 58 80 01 00 00 00     vaddpd zmm0,zmm0,zmmword ptr [r8+0000000000000001h]
  // 62 F1 FD 48 58 04 C0              vaddpd zmm0,zmm0,zmmword ptr [rax+rax*8]
  // 62 F1 FD 48 58 04 C4              vaddpd zmm0,zmm0,zmmword ptr [rsp+rax*8]
  // 62 F1 FD 48 58 44 C5 00           vaddpd zmm0,zmm0,zmmword ptr [rbp+rax*8]
  // 62 D1 FD 48 58 04 C0              vaddpd zmm0,zmm0,zmmword ptr [r8+rax*8]
  // 62 F1 FD 48 58 84 C0 01 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rax+rax*8+0000000000000001h]
  // 62 F1 FD 48 58 84 C4 01 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rsp+rax*8+0000000000000001h]
  // 62 F1 FD 48 58 84 C5 01 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rbp+rax*8+0000000000000001h]
  // 62 D1 FD 48 58 84 C0 01 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [r8+rax*8+0000000000000001h]
  // 62 F1 FD 48 58 04 C5 00 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rax*8+0000000000000000h]
  // 62 F1 FD 48 58 04 C5 00 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rax*8+0000000000000000h]
  // 62 F1 FD 48 58 04 C5 00 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rax*8+0000000000000000h]
  // 62 F1 FD 48 58 04 C5 00 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rax*8+0000000000000000h]
  // 62 F1 FD 48 58 04 C5 01 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rax*8+0000000000000001h]
  // 62 F1 FD 48 58 04 C5 01 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rax*8+0000000000000001h]
  // 62 F1 FD 48 58 04 C5 01 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rax*8+0000000000000001h]
  // 62 F1 FD 48 58 04 C5 01 00 00 00  vaddpd zmm0,zmm0,zmmword ptr [rax*8+0000000000000001h]
                    

	write_m(c, b, 0, r7, i7, b7);
	
	return 0;
}


#undef no_inline
#undef force_inline
#undef instr_inline

x64w_Result x64w_push8i (uint8_t **c, int8_t   i) { return instr_i1(c, i, 0x6a); }
x64w_Result x64w_push32i(uint8_t **c, int32_t  i) { return instr_i4(c, i, 0x68); }
x64w_Result x64w_push_r16(uint8_t **c, x64w_Gpr16 s) { return instr_r(c, s.i, 2, 0x50, 0, NO_MODRM | OSO); }
x64w_Result x64w_push_r64(uint8_t **c, x64w_Gpr64 s) { return instr_r(c, s.i, 8, 0x50, 0, NO_MODRM); }
x64w_Result x64w_push_m16(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xff, 6, OSO); }
x64w_Result x64w_push_m64(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xff, 6, 0); }

x64w_Result x64w_pop_r16(uint8_t **c, x64w_Gpr16 s) { return instr_r(c, s.i, 2, 0x58, 0, NO_MODRM | OSO); }
x64w_Result x64w_pop_r64(uint8_t **c, x64w_Gpr64 s) { return instr_r(c, s.i, 8, 0x58, 0, NO_MODRM); }
x64w_Result x64w_pop_m16(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0x8f, 0, OSO); }
x64w_Result x64w_pop_m64(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0x8f, 0, 0); }

x64w_Result x64w_mov_ri8 (uint8_t **c, x64w_Gpr8  r, int8_t   i) { return instr_ri(c, r.i, i, 1, 0xb0, 0, NO_MODRM); }
x64w_Result x64w_mov_ri16(uint8_t **c, x64w_Gpr16 r, int16_t  i) { return instr_ri(c, r.i, i, 2, 0xb8, 0, NO_MODRM|OSO); }
x64w_Result x64w_mov_ri32(uint8_t **c, x64w_Gpr32 r, int32_t  i) { return instr_ri(c, r.i, i, 4, 0xb8, 0, NO_MODRM); }
x64w_Result x64w_mov_ri64(uint8_t **c, x64w_Gpr64 r, int64_t  i) { return instr_ri(c, r.i, i, 8, 0xb8, 0, NO_MODRM|REXW); }
x64w_Result x64w_mov_rr8 (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s) { return instr_rr(c, d.i, s.i, 1, 0x8a, 0); }
x64w_Result x64w_mov_rr16(uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s) { return instr_rr(c, d.i, s.i, 2, 0x8b, OSO); }
x64w_Result x64w_mov_rr32(uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s) { return instr_rr(c, d.i, s.i, 4, 0x8b, 0); }
x64w_Result x64w_mov_rr64(uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s) { return instr_rr(c, d.i, s.i, 8, 0x8b, REXW); }
x64w_Result x64w_mov_rm8 (uint8_t **c, x64w_Gpr8  d, x64w_Mem s) { return instr_rm(c, d.i, s, 1, 0x8a, 0); }
x64w_Result x64w_mov_rm16(uint8_t **c, x64w_Gpr16 d, x64w_Mem s) { return instr_rm(c, d.i, s, 2, 0x8b, OSO); }
x64w_Result x64w_mov_rm32(uint8_t **c, x64w_Gpr32 d, x64w_Mem s) { return instr_rm(c, d.i, s, 4, 0x8b, 0); }
x64w_Result x64w_mov_rm64(uint8_t **c, x64w_Gpr64 d, x64w_Mem s) { return instr_rm(c, d.i, s, 8, 0x8b, REXW); }
x64w_Result x64w_mov_mr8 (uint8_t **c, x64w_Mem d, x64w_Gpr8  s) { return instr_rm(c, s.i, d, 1, 0x88, 0); }
x64w_Result x64w_mov_mr16(uint8_t **c, x64w_Mem d, x64w_Gpr16 s) { return instr_rm(c, s.i, d, 2, 0x89, OSO); }
x64w_Result x64w_mov_mr32(uint8_t **c, x64w_Mem d, x64w_Gpr32 s) { return instr_rm(c, s.i, d, 4, 0x89, 0); }
x64w_Result x64w_mov_mr64(uint8_t **c, x64w_Mem d, x64w_Gpr64 s) { return instr_rm(c, s.i, d, 8, 0x89, REXW); }
x64w_Result x64w_mov_mi8 (uint8_t **c, x64w_Mem m, int8_t  i) { return instr_mi(c, m, i, 1, 0xc6, 0, 0); }
x64w_Result x64w_mov_mi16(uint8_t **c, x64w_Mem m, int16_t i) { return instr_mi(c, m, i, 2, 0xc7, 0, OSO); }
x64w_Result x64w_mov_mi32(uint8_t **c, x64w_Mem m, int32_t i) { return instr_mi(c, m, i, 4, 0xc7, 0, 0); }
x64w_Result x64w_mov_m64i32(uint8_t **c, x64w_Mem m, int32_t i) { return instr_mi(c, m, i, 4, 0xc7, 0, REXW); }

x64w_Result x64w_adcx_rr32(uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s) { return instr_rr(c, d.i, s.i, 4, 0x0f38f6, OSO); }
x64w_Result x64w_adcx_rr64(uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s) { return instr_rr(c, d.i, s.i, 8, 0x0f38f6, OSO | REXW); }

x64w_Result x64w_addpd_xx(uint8_t **c, x64w_Xmm d, x64w_Xmm s) { return instr_rr(c, d.i, s.i, 16, 0x0f58, OSO); }
x64w_Result x64w_addpd_xm(uint8_t **c, x64w_Xmm d, x64w_Mem s) { return instr_rm(c, d.i, s, 16, 0x0f58, OSO); }

x64w_Result x64w_vaddpd_xxx(uint8_t **c, x64w_Xmm d, x64w_Xmm a, x64w_Xmm b) { return instr_xxx(c, d.i, a.i, b.i, 16, 0x58); }
x64w_Result x64w_vaddpd_xxm(uint8_t **c, x64w_Xmm d, x64w_Xmm a, x64w_Mem b) { return instr_xxm(c, d.i, a.i, b,   16, 0x58); }
x64w_Result x64w_vaddpd_yyy(uint8_t **c, x64w_Ymm d, x64w_Ymm a, x64w_Ymm b) { return instr_xxx(c, d.i, a.i, b.i, 32, 0x58); }
x64w_Result x64w_vaddpd_yym(uint8_t **c, x64w_Ymm d, x64w_Ymm a, x64w_Mem b) { return instr_xxm(c, d.i, a.i, b,   32, 0x58); }
x64w_Result x64w_vaddpd_zzz(uint8_t **c, x64w_Zmm d, x64w_Zmm a, x64w_Zmm b) { return instr_xxx(c, d.i, a.i, b.i, 64, 0x58); }
x64w_Result x64w_vaddpd_zzm(uint8_t **c, x64w_Zmm d, x64w_Zmm a, x64w_Mem b) { return instr_xxm(c, d.i, a.i, b,   64, 0x58); }

x64w_Result x64w_adc_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x80, 2,    0); }
x64w_Result x64w_adc_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i) { return instr_ri(c, r.i,   i, 2, 0x81, 2,  OSO); }
x64w_Result x64w_adc_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 2,    0); }
x64w_Result x64w_adc_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 2, REXW); }
x64w_Result x64w_adc_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 2,  OSO); }
x64w_Result x64w_adc_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 2,    0); }
x64w_Result x64w_adc_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 2, REXW); }
x64w_Result x64w_adc_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s) { return instr_rr(c, d.i, s.i, 1, 0x12,       0); }
x64w_Result x64w_adc_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s) { return instr_rr(c, d.i, s.i, 2, 0x13,     OSO); }
x64w_Result x64w_adc_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s) { return instr_rr(c, d.i, s.i, 4, 0x13,       0); }
x64w_Result x64w_adc_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s) { return instr_rr(c, d.i, s.i, 8, 0x13,    REXW); }
x64w_Result x64w_adc_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 1, 0x12,       0); }
x64w_Result x64w_adc_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 2, 0x13,     OSO); }
x64w_Result x64w_adc_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 4, 0x13,       0); }
x64w_Result x64w_adc_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 8, 0x13,    REXW); }
x64w_Result x64w_adc_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i) { return instr_mi(c,   m,   i, 1, 0x80, 2,    0); }
x64w_Result x64w_adc_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 2, 0x81, 2,  OSO); }
x64w_Result x64w_adc_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 2,    0); }
x64w_Result x64w_adc_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 2, REXW); }
x64w_Result x64w_adc_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 2,  OSO); }
x64w_Result x64w_adc_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 2,    0); }
x64w_Result x64w_adc_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 2, REXW); }
x64w_Result x64w_adc_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s) { return instr_rm(c, s.i,   d, 1, 0x10,       0); }
x64w_Result x64w_adc_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s) { return instr_rm(c, s.i,   d, 2, 0x11,     OSO); }
x64w_Result x64w_adc_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s) { return instr_rm(c, s.i,   d, 4, 0x11,       0); }
x64w_Result x64w_adc_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s) { return instr_rm(c, s.i,   d, 8, 0x11,    REXW); }
x64w_Result x64w_add_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x80, 0,    0); }
x64w_Result x64w_add_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i) { return instr_ri(c, r.i,   i, 2, 0x81, 0,  OSO); }
x64w_Result x64w_add_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 0,    0); }
x64w_Result x64w_add_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 0, REXW); }
x64w_Result x64w_add_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 0,  OSO); }
x64w_Result x64w_add_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 0,    0); }
x64w_Result x64w_add_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 0, REXW); }
x64w_Result x64w_add_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s) { return instr_rr(c, d.i, s.i, 1, 0x02,       0); }
x64w_Result x64w_add_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s) { return instr_rr(c, d.i, s.i, 2, 0x03,     OSO); }
x64w_Result x64w_add_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s) { return instr_rr(c, d.i, s.i, 4, 0x03,       0); }
x64w_Result x64w_add_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s) { return instr_rr(c, d.i, s.i, 8, 0x03,    REXW); }
x64w_Result x64w_add_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 1, 0x02,       0); }
x64w_Result x64w_add_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 2, 0x03,     OSO); }
x64w_Result x64w_add_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 4, 0x03,       0); }
x64w_Result x64w_add_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 8, 0x03,    REXW); }
x64w_Result x64w_add_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i) { return instr_mi(c,   m,   i, 1, 0x80, 0,    0); }
x64w_Result x64w_add_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 2, 0x81, 0,  OSO); }
x64w_Result x64w_add_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 0,    0); }
x64w_Result x64w_add_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 0, REXW); }
x64w_Result x64w_add_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 0,  OSO); }
x64w_Result x64w_add_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 0,    0); }
x64w_Result x64w_add_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 0, REXW); }
x64w_Result x64w_add_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s) { return instr_rm(c, s.i,   d, 1, 0x00,       0); }
x64w_Result x64w_add_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s) { return instr_rm(c, s.i,   d, 2, 0x01,     OSO); }
x64w_Result x64w_add_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s) { return instr_rm(c, s.i,   d, 4, 0x01,       0); }
x64w_Result x64w_add_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s) { return instr_rm(c, s.i,   d, 8, 0x01,    REXW); }
x64w_Result x64w_xor_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x80, 6,    0); }
x64w_Result x64w_xor_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i) { return instr_ri(c, r.i,   i, 2, 0x81, 6,  OSO); }
x64w_Result x64w_xor_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 6,    0); }
x64w_Result x64w_xor_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 6, REXW); }
x64w_Result x64w_xor_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 6,  OSO); }
x64w_Result x64w_xor_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 6,    0); }
x64w_Result x64w_xor_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 6, REXW); }
x64w_Result x64w_xor_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s) { return instr_rr(c, d.i, s.i, 1, 0x32,       0); }
x64w_Result x64w_xor_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s) { return instr_rr(c, d.i, s.i, 2, 0x33,     OSO); }
x64w_Result x64w_xor_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s) { return instr_rr(c, d.i, s.i, 4, 0x33,       0); }
x64w_Result x64w_xor_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s) { return instr_rr(c, d.i, s.i, 8, 0x33,    REXW); }
x64w_Result x64w_xor_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 1, 0x32,       0); }
x64w_Result x64w_xor_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 2, 0x33,     OSO); }
x64w_Result x64w_xor_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 4, 0x33,       0); }
x64w_Result x64w_xor_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 8, 0x33,    REXW); }
x64w_Result x64w_xor_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i) { return instr_mi(c,   m,   i, 1, 0x80, 6,    0); }
x64w_Result x64w_xor_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 2, 0x81, 6,  OSO); }
x64w_Result x64w_xor_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 6,    0); }
x64w_Result x64w_xor_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 6, REXW); }
x64w_Result x64w_xor_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 6,  OSO); }
x64w_Result x64w_xor_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 6,    0); }
x64w_Result x64w_xor_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 6, REXW); }
x64w_Result x64w_xor_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s) { return instr_rm(c, s.i,   d, 1, 0x30,       0); }
x64w_Result x64w_xor_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s) { return instr_rm(c, s.i,   d, 2, 0x31,     OSO); }
x64w_Result x64w_xor_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s) { return instr_rm(c, s.i,   d, 4, 0x31,       0); }
x64w_Result x64w_xor_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s) { return instr_rm(c, s.i,   d, 8, 0x31,    REXW); }
x64w_Result x64w_and_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x80, 4,    0); }
x64w_Result x64w_and_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i) { return instr_ri(c, r.i,   i, 2, 0x81, 4,  OSO); }
x64w_Result x64w_and_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 4,    0); }
x64w_Result x64w_and_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 4, REXW); }
x64w_Result x64w_and_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 4,  OSO); }
x64w_Result x64w_and_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 4,    0); }
x64w_Result x64w_and_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 4, REXW); }
x64w_Result x64w_and_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s) { return instr_rr(c, d.i, s.i, 1, 0x22,       0); }
x64w_Result x64w_and_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s) { return instr_rr(c, d.i, s.i, 2, 0x23,     OSO); }
x64w_Result x64w_and_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s) { return instr_rr(c, d.i, s.i, 4, 0x23,       0); }
x64w_Result x64w_and_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s) { return instr_rr(c, d.i, s.i, 8, 0x23,    REXW); }
x64w_Result x64w_and_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 1, 0x22,       0); }
x64w_Result x64w_and_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 2, 0x23,     OSO); }
x64w_Result x64w_and_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 4, 0x23,       0); }
x64w_Result x64w_and_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 8, 0x23,    REXW); }
x64w_Result x64w_and_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i) { return instr_mi(c,   m,   i, 1, 0x80, 4,    0); }
x64w_Result x64w_and_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 2, 0x81, 4,  OSO); }
x64w_Result x64w_and_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 4,    0); }
x64w_Result x64w_and_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 4, REXW); }
x64w_Result x64w_and_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 4,  OSO); }
x64w_Result x64w_and_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 4,    0); }
x64w_Result x64w_and_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 4, REXW); }
x64w_Result x64w_and_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s) { return instr_rm(c, s.i,   d, 1, 0x20,       0); }
x64w_Result x64w_and_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s) { return instr_rm(c, s.i,   d, 2, 0x21,     OSO); }
x64w_Result x64w_and_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s) { return instr_rm(c, s.i,   d, 4, 0x21,       0); }
x64w_Result x64w_and_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s) { return instr_rm(c, s.i,   d, 8, 0x21,    REXW); }
x64w_Result x64w_or_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x80, 1,    0); }
x64w_Result x64w_or_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i) { return instr_ri(c, r.i,   i, 2, 0x81, 1,  OSO); }
x64w_Result x64w_or_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 1,    0); }
x64w_Result x64w_or_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 1, REXW); }
x64w_Result x64w_or_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 1,  OSO); }
x64w_Result x64w_or_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 1,    0); }
x64w_Result x64w_or_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 1, REXW); }
x64w_Result x64w_or_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s) { return instr_rr(c, d.i, s.i, 1, 0x0a,       0); }
x64w_Result x64w_or_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s) { return instr_rr(c, d.i, s.i, 2, 0x0b,     OSO); }
x64w_Result x64w_or_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s) { return instr_rr(c, d.i, s.i, 4, 0x0b,       0); }
x64w_Result x64w_or_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s) { return instr_rr(c, d.i, s.i, 8, 0x0b,    REXW); }
x64w_Result x64w_or_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 1, 0x0a,       0); }
x64w_Result x64w_or_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 2, 0x0b,     OSO); }
x64w_Result x64w_or_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 4, 0x0b,       0); }
x64w_Result x64w_or_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 8, 0x0b,    REXW); }
x64w_Result x64w_or_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i) { return instr_mi(c,   m,   i, 1, 0x80, 1,    0); }
x64w_Result x64w_or_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 2, 0x81, 1,  OSO); }
x64w_Result x64w_or_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 1,    0); }
x64w_Result x64w_or_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 1, REXW); }
x64w_Result x64w_or_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 1,  OSO); }
x64w_Result x64w_or_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 1,    0); }
x64w_Result x64w_or_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 1, REXW); }
x64w_Result x64w_or_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s) { return instr_rm(c, s.i,   d, 1, 0x08,       0); }
x64w_Result x64w_or_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s) { return instr_rm(c, s.i,   d, 2, 0x09,     OSO); }
x64w_Result x64w_or_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s) { return instr_rm(c, s.i,   d, 4, 0x09,       0); }
x64w_Result x64w_or_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s) { return instr_rm(c, s.i,   d, 8, 0x09,    REXW); }
x64w_Result x64w_sub_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x80, 5,    0); }
x64w_Result x64w_sub_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i) { return instr_ri(c, r.i,   i, 2, 0x81, 5,  OSO); }
x64w_Result x64w_sub_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 5,    0); }
x64w_Result x64w_sub_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i) { return instr_ri(c, r.i,   i, 4, 0x81, 5, REXW); }
x64w_Result x64w_sub_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 5,  OSO); }
x64w_Result x64w_sub_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 5,    0); }
x64w_Result x64w_sub_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i) { return instr_ri(c, r.i,   i, 1, 0x83, 5, REXW); }
x64w_Result x64w_sub_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s) { return instr_rr(c, d.i, s.i, 1, 0x2a,       0); }
x64w_Result x64w_sub_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s) { return instr_rr(c, d.i, s.i, 2, 0x2b,     OSO); }
x64w_Result x64w_sub_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s) { return instr_rr(c, d.i, s.i, 4, 0x2b,       0); }
x64w_Result x64w_sub_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s) { return instr_rr(c, d.i, s.i, 8, 0x2b,    REXW); }
x64w_Result x64w_sub_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 1, 0x2a,       0); }
x64w_Result x64w_sub_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 2, 0x2b,     OSO); }
x64w_Result x64w_sub_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 4, 0x2b,       0); }
x64w_Result x64w_sub_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s) { return instr_rm(c, d.i,   s, 8, 0x2b,    REXW); }
x64w_Result x64w_sub_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i) { return instr_mi(c,   m,   i, 1, 0x80, 5,    0); }
x64w_Result x64w_sub_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 2, 0x81, 5,  OSO); }
x64w_Result x64w_sub_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 5,    0); }
x64w_Result x64w_sub_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 4, 0x81, 5, REXW); }
x64w_Result x64w_sub_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 5,  OSO); }
x64w_Result x64w_sub_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 5,    0); }
x64w_Result x64w_sub_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i) { return instr_mi(c,   m,   i, 1, 0x83, 5, REXW); }
x64w_Result x64w_sub_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s) { return instr_rm(c, s.i,   d, 1, 0x28,       0); }
x64w_Result x64w_sub_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s) { return instr_rm(c, s.i,   d, 2, 0x29,     OSO); }
x64w_Result x64w_sub_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s) { return instr_rm(c, s.i,   d, 4, 0x29,       0); }
x64w_Result x64w_sub_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s) { return instr_rm(c, s.i,   d, 8, 0x29,    REXW); }
x64w_Result x64w_inc_r8 (uint8_t **c, x64w_Gpr8  d) { return instr_r(c, d.i, 1, 0xfe, 0,    0); }
x64w_Result x64w_inc_r16(uint8_t **c, x64w_Gpr16 d) { return instr_r(c, d.i, 2, 0xff, 0,  OSO); }
x64w_Result x64w_inc_r32(uint8_t **c, x64w_Gpr32 d) { return instr_r(c, d.i, 4, 0xff, 0,    0); }
x64w_Result x64w_inc_r64(uint8_t **c, x64w_Gpr64 d) { return instr_r(c, d.i, 8, 0xff, 0, REXW); }
x64w_Result x64w_inc_m8 (uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xfe, 0,    0); }
x64w_Result x64w_inc_m16(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xff, 0,  OSO); }
x64w_Result x64w_inc_m32(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xff, 0,    0); }
x64w_Result x64w_inc_m64(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xff, 0, REXW); }
x64w_Result x64w_dec_r8 (uint8_t **c, x64w_Gpr8  d) { return instr_r(c, d.i, 1, 0xfe, 1,    0); }
x64w_Result x64w_dec_r16(uint8_t **c, x64w_Gpr16 d) { return instr_r(c, d.i, 2, 0xff, 1,  OSO); }
x64w_Result x64w_dec_r32(uint8_t **c, x64w_Gpr32 d) { return instr_r(c, d.i, 4, 0xff, 1,    0); }
x64w_Result x64w_dec_r64(uint8_t **c, x64w_Gpr64 d) { return instr_r(c, d.i, 8, 0xff, 1, REXW); }
x64w_Result x64w_dec_m8 (uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xfe, 1,    0); }
x64w_Result x64w_dec_m16(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xff, 1,  OSO); }
x64w_Result x64w_dec_m32(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xff, 1,    0); }
x64w_Result x64w_dec_m64(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xff, 1, REXW); }
x64w_Result x64w_not_r8 (uint8_t **c, x64w_Gpr8  d) { return instr_r(c, d.i, 1, 0xf6, 2,    0); }
x64w_Result x64w_not_r16(uint8_t **c, x64w_Gpr16 d) { return instr_r(c, d.i, 2, 0xf7, 2,  OSO); }
x64w_Result x64w_not_r32(uint8_t **c, x64w_Gpr32 d) { return instr_r(c, d.i, 4, 0xf7, 2,    0); }
x64w_Result x64w_not_r64(uint8_t **c, x64w_Gpr64 d) { return instr_r(c, d.i, 8, 0xf7, 2, REXW); }
x64w_Result x64w_not_m8 (uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf6, 2,    0); }
x64w_Result x64w_not_m16(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 2,  OSO); }
x64w_Result x64w_not_m32(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 2,    0); }
x64w_Result x64w_not_m64(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 2, REXW); }
x64w_Result x64w_neg_r8 (uint8_t **c, x64w_Gpr8  d) { return instr_r(c, d.i, 1, 0xf6, 3,    0); }
x64w_Result x64w_neg_r16(uint8_t **c, x64w_Gpr16 d) { return instr_r(c, d.i, 2, 0xf7, 3,  OSO); }
x64w_Result x64w_neg_r32(uint8_t **c, x64w_Gpr32 d) { return instr_r(c, d.i, 4, 0xf7, 3,    0); }
x64w_Result x64w_neg_r64(uint8_t **c, x64w_Gpr64 d) { return instr_r(c, d.i, 8, 0xf7, 3, REXW); }
x64w_Result x64w_neg_m8 (uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf6, 3,    0); }
x64w_Result x64w_neg_m16(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 3,  OSO); }
x64w_Result x64w_neg_m32(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 3,    0); }
x64w_Result x64w_neg_m64(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 3, REXW); }
x64w_Result x64w_mul_r8 (uint8_t **c, x64w_Gpr8  d) { return instr_r(c, d.i, 1, 0xf6, 4,    0); }
x64w_Result x64w_mul_r16(uint8_t **c, x64w_Gpr16 d) { return instr_r(c, d.i, 2, 0xf7, 4,  OSO); }
x64w_Result x64w_mul_r32(uint8_t **c, x64w_Gpr32 d) { return instr_r(c, d.i, 4, 0xf7, 4,    0); }
x64w_Result x64w_mul_r64(uint8_t **c, x64w_Gpr64 d) { return instr_r(c, d.i, 8, 0xf7, 4, REXW); }
x64w_Result x64w_mul_m8 (uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf6, 4,    0); }
x64w_Result x64w_mul_m16(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 4,  OSO); }
x64w_Result x64w_mul_m32(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 4,    0); }
x64w_Result x64w_mul_m64(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 4, REXW); }
x64w_Result x64w_div_r8 (uint8_t **c, x64w_Gpr8  d) { return instr_r(c, d.i, 1, 0xf6, 6,    0); }
x64w_Result x64w_div_r16(uint8_t **c, x64w_Gpr16 d) { return instr_r(c, d.i, 2, 0xf7, 6,  OSO); }
x64w_Result x64w_div_r32(uint8_t **c, x64w_Gpr32 d) { return instr_r(c, d.i, 4, 0xf7, 6,    0); }
x64w_Result x64w_div_r64(uint8_t **c, x64w_Gpr64 d) { return instr_r(c, d.i, 8, 0xf7, 6, REXW); }
x64w_Result x64w_div_m8 (uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf6, 6,    0); }
x64w_Result x64w_div_m16(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 6,  OSO); }
x64w_Result x64w_div_m32(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 6,    0); }
x64w_Result x64w_div_m64(uint8_t **c, x64w_Mem   d) { return instr_m(c,   d,    0xf7, 6, REXW); }
x64w_Result x64w_shl_r8_1  (uint8_t **c, x64w_Gpr8  r           ) { return instr_r (c, r.i,    1, 0xd0, 4,    0); }
x64w_Result x64w_shl_r16_1 (uint8_t **c, x64w_Gpr16 r           ) { return instr_r (c, r.i,    2, 0xd1, 4,  OSO); }
x64w_Result x64w_shl_r32_1 (uint8_t **c, x64w_Gpr32 r           ) { return instr_r (c, r.i,    4, 0xd1, 4,    0); }
x64w_Result x64w_shl_r64_1 (uint8_t **c, x64w_Gpr64 r           ) { return instr_r (c, r.i,    8, 0xd1, 4, REXW); }
x64w_Result x64w_shl_ri8   (uint8_t **c, x64w_Gpr8  r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc0, 4,    0); }
x64w_Result x64w_shl_r16i8 (uint8_t **c, x64w_Gpr16 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 4,  OSO); }
x64w_Result x64w_shl_r32i8 (uint8_t **c, x64w_Gpr32 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 4,    0); }
x64w_Result x64w_shl_r64i8 (uint8_t **c, x64w_Gpr64 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 4, REXW); }
x64w_Result x64w_shl_r8_cl (uint8_t **c, x64w_Gpr8  r           ) { return instr_r (c, r.i,    1, 0xd2, 4,    0); }
x64w_Result x64w_shl_r16_cl(uint8_t **c, x64w_Gpr16 r           ) { return instr_r (c, r.i,    2, 0xd3, 4,  OSO); }
x64w_Result x64w_shl_r32_cl(uint8_t **c, x64w_Gpr32 r           ) { return instr_r (c, r.i,    4, 0xd3, 4,    0); }
x64w_Result x64w_shl_r64_cl(uint8_t **c, x64w_Gpr64 r           ) { return instr_r (c, r.i,    8, 0xd3, 4, REXW); }
x64w_Result x64w_shl_m8_1  (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd0, 4,    0); }
x64w_Result x64w_shl_m16_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 4,  OSO); }
x64w_Result x64w_shl_m32_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 4,    0); }
x64w_Result x64w_shl_m64_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 4, REXW); }
x64w_Result x64w_shl_mi8   (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc0, 4,    0); }
x64w_Result x64w_shl_m16i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 4,  OSO); }
x64w_Result x64w_shl_m32i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 4,    0); }
x64w_Result x64w_shl_m64i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 4, REXW); }
x64w_Result x64w_shl_m8_cl (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd2, 4,    0); }
x64w_Result x64w_shl_m16_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 4,  OSO); }
x64w_Result x64w_shl_m32_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 4,    0); }
x64w_Result x64w_shl_m64_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 4, REXW); }
x64w_Result x64w_shr_r8_1  (uint8_t **c, x64w_Gpr8  r           ) { return instr_r (c, r.i,    1, 0xd0, 5,    0); }
x64w_Result x64w_shr_r16_1 (uint8_t **c, x64w_Gpr16 r           ) { return instr_r (c, r.i,    2, 0xd1, 5,  OSO); }
x64w_Result x64w_shr_r32_1 (uint8_t **c, x64w_Gpr32 r           ) { return instr_r (c, r.i,    4, 0xd1, 5,    0); }
x64w_Result x64w_shr_r64_1 (uint8_t **c, x64w_Gpr64 r           ) { return instr_r (c, r.i,    8, 0xd1, 5, REXW); }
x64w_Result x64w_shr_ri8   (uint8_t **c, x64w_Gpr8  r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc0, 5,    0); }
x64w_Result x64w_shr_r16i8 (uint8_t **c, x64w_Gpr16 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 5,  OSO); }
x64w_Result x64w_shr_r32i8 (uint8_t **c, x64w_Gpr32 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 5,    0); }
x64w_Result x64w_shr_r64i8 (uint8_t **c, x64w_Gpr64 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 5, REXW); }
x64w_Result x64w_shr_r8_cl (uint8_t **c, x64w_Gpr8  r           ) { return instr_r (c, r.i,    1, 0xd2, 5,    0); }
x64w_Result x64w_shr_r16_cl(uint8_t **c, x64w_Gpr16 r           ) { return instr_r (c, r.i,    2, 0xd3, 5,  OSO); }
x64w_Result x64w_shr_r32_cl(uint8_t **c, x64w_Gpr32 r           ) { return instr_r (c, r.i,    4, 0xd3, 5,    0); }
x64w_Result x64w_shr_r64_cl(uint8_t **c, x64w_Gpr64 r           ) { return instr_r (c, r.i,    8, 0xd3, 5, REXW); }
x64w_Result x64w_shr_m8_1  (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd0, 5,    0); }
x64w_Result x64w_shr_m16_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 5,  OSO); }
x64w_Result x64w_shr_m32_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 5,    0); }
x64w_Result x64w_shr_m64_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 5, REXW); }
x64w_Result x64w_shr_mi8   (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc0, 5,    0); }
x64w_Result x64w_shr_m16i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 5,  OSO); }
x64w_Result x64w_shr_m32i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 5,    0); }
x64w_Result x64w_shr_m64i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 5, REXW); }
x64w_Result x64w_shr_m8_cl (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd2, 5,    0); }
x64w_Result x64w_shr_m16_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 5,  OSO); }
x64w_Result x64w_shr_m32_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 5,    0); }
x64w_Result x64w_shr_m64_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 5, REXW); }
x64w_Result x64w_sal_r8_1  (uint8_t **c, x64w_Gpr8  r           ) { return instr_r (c, r.i,    1, 0xd0, 4,    0); }
x64w_Result x64w_sal_r16_1 (uint8_t **c, x64w_Gpr16 r           ) { return instr_r (c, r.i,    2, 0xd1, 4,  OSO); }
x64w_Result x64w_sal_r32_1 (uint8_t **c, x64w_Gpr32 r           ) { return instr_r (c, r.i,    4, 0xd1, 4,    0); }
x64w_Result x64w_sal_r64_1 (uint8_t **c, x64w_Gpr64 r           ) { return instr_r (c, r.i,    8, 0xd1, 4, REXW); }
x64w_Result x64w_sal_ri8   (uint8_t **c, x64w_Gpr8  r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc0, 4,    0); }
x64w_Result x64w_sal_r16i8 (uint8_t **c, x64w_Gpr16 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 4,  OSO); }
x64w_Result x64w_sal_r32i8 (uint8_t **c, x64w_Gpr32 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 4,    0); }
x64w_Result x64w_sal_r64i8 (uint8_t **c, x64w_Gpr64 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 4, REXW); }
x64w_Result x64w_sal_r8_cl (uint8_t **c, x64w_Gpr8  r           ) { return instr_r (c, r.i,    1, 0xd2, 4,    0); }
x64w_Result x64w_sal_r16_cl(uint8_t **c, x64w_Gpr16 r           ) { return instr_r (c, r.i,    2, 0xd3, 4,  OSO); }
x64w_Result x64w_sal_r32_cl(uint8_t **c, x64w_Gpr32 r           ) { return instr_r (c, r.i,    4, 0xd3, 4,    0); }
x64w_Result x64w_sal_r64_cl(uint8_t **c, x64w_Gpr64 r           ) { return instr_r (c, r.i,    8, 0xd3, 4, REXW); }
x64w_Result x64w_sal_m8_1  (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd0, 4,    0); }
x64w_Result x64w_sal_m16_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 4,  OSO); }
x64w_Result x64w_sal_m32_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 4,    0); }
x64w_Result x64w_sal_m64_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 4, REXW); }
x64w_Result x64w_sal_mi8   (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc0, 4,    0); }
x64w_Result x64w_sal_m16i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 4,  OSO); }
x64w_Result x64w_sal_m32i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 4,    0); }
x64w_Result x64w_sal_m64i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 4, REXW); }
x64w_Result x64w_sal_m8_cl (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd2, 4,    0); }
x64w_Result x64w_sal_m16_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 4,  OSO); }
x64w_Result x64w_sal_m32_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 4,    0); }
x64w_Result x64w_sal_m64_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 4, REXW); }
x64w_Result x64w_sar_r8_1  (uint8_t **c, x64w_Gpr8  r           ) { return instr_r (c, r.i,    1, 0xd0, 7,    0); }
x64w_Result x64w_sar_r16_1 (uint8_t **c, x64w_Gpr16 r           ) { return instr_r (c, r.i,    2, 0xd1, 7,  OSO); }
x64w_Result x64w_sar_r32_1 (uint8_t **c, x64w_Gpr32 r           ) { return instr_r (c, r.i,    4, 0xd1, 7,    0); }
x64w_Result x64w_sar_r64_1 (uint8_t **c, x64w_Gpr64 r           ) { return instr_r (c, r.i,    8, 0xd1, 7, REXW); }
x64w_Result x64w_sar_ri8   (uint8_t **c, x64w_Gpr8  r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc0, 7,    0); }
x64w_Result x64w_sar_r16i8 (uint8_t **c, x64w_Gpr16 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 7,  OSO); }
x64w_Result x64w_sar_r32i8 (uint8_t **c, x64w_Gpr32 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 7,    0); }
x64w_Result x64w_sar_r64i8 (uint8_t **c, x64w_Gpr64 r, uint8_t i) { return instr_ri(c, r.i, i, 1, 0xc1, 7, REXW); }
x64w_Result x64w_sar_r8_cl (uint8_t **c, x64w_Gpr8  r           ) { return instr_r (c, r.i,    1, 0xd2, 7,    0); }
x64w_Result x64w_sar_r16_cl(uint8_t **c, x64w_Gpr16 r           ) { return instr_r (c, r.i,    2, 0xd3, 7,  OSO); }
x64w_Result x64w_sar_r32_cl(uint8_t **c, x64w_Gpr32 r           ) { return instr_r (c, r.i,    4, 0xd3, 7,    0); }
x64w_Result x64w_sar_r64_cl(uint8_t **c, x64w_Gpr64 r           ) { return instr_r (c, r.i,    8, 0xd3, 7, REXW); }
x64w_Result x64w_sar_m8_1  (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd0, 7,    0); }
x64w_Result x64w_sar_m16_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 7,  OSO); }
x64w_Result x64w_sar_m32_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 7,    0); }
x64w_Result x64w_sar_m64_1 (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd1, 7, REXW); }
x64w_Result x64w_sar_mi8   (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc0, 7,    0); }
x64w_Result x64w_sar_m16i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 7,  OSO); }
x64w_Result x64w_sar_m32i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 7,    0); }
x64w_Result x64w_sar_m64i8 (uint8_t **c, x64w_Mem   m, uint8_t i) { return instr_mi(c,   m, i, 1, 0xc1, 7, REXW); }
x64w_Result x64w_sar_m8_cl (uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd2, 7,    0); }
x64w_Result x64w_sar_m16_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 7,  OSO); }
x64w_Result x64w_sar_m32_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 7,    0); }
x64w_Result x64w_sar_m64_cl(uint8_t **c, x64w_Mem   m           ) { return instr_m (c,   m,       0xd3, 7, REXW); }
x64w_Result x64w_lea_rm16(uint8_t **c, x64w_Gpr16 r, x64w_Mem m) { return instr_rm(c, r.i, m, 2, 0x8d,  OSO); }
x64w_Result x64w_lea_rm32(uint8_t **c, x64w_Gpr32 r, x64w_Mem m) { return instr_rm(c, r.i, m, 4, 0x8d,    0); }
x64w_Result x64w_lea_rm64(uint8_t **c, x64w_Gpr64 r, x64w_Mem m) { return instr_rm(c, r.i, m, 8, 0x8d, REXW); }


#undef REXW
#undef OSO
#undef ASO
#undef NO_MODRM

#undef x64w_fits_in_8
#undef x64w_fits_in_16
#undef x64w_fits_in_32
#undef W2
#undef W4
#undef W8
#undef X64W_VALIDATE
#undef X64W_VALIDATE_R
#undef X64W_VALIDATE_RR
#undef X64W_VALIDATE_M
#undef X64W_VALIDATE_RM

#undef vex_p_none
#undef vex_p_66
#undef vex_p_f3
#undef vex_p_f2

#undef vex_m_0f
#undef vex_m_0f38
#undef vex_m_0f3a

#endif // X64W_IMPLEMENTATION

#ifdef __cplusplus
} // extern "C"
#endif

#ifdef __cplusplus
inline constexpr bool operator==(x64w_Gpr8 a, x64w_Gpr8 b) { return a.i == b.i; }
inline constexpr bool operator==(x64w_Gpr16 a, x64w_Gpr16 b) { return a.i == b.i; }
inline constexpr bool operator==(x64w_Gpr32 a, x64w_Gpr32 b) { return a.i == b.i; }
inline constexpr bool operator==(x64w_Gpr64 a, x64w_Gpr64 b) { return a.i == b.i; }
inline constexpr bool operator==(x64w_Xmm a, x64w_Xmm b) { return a.i == b.i; }
inline constexpr bool operator==(x64w_Ymm a, x64w_Ymm b) { return a.i == b.i; }
inline constexpr bool operator==(x64w_Zmm a, x64w_Zmm b) { return a.i == b.i; }
inline constexpr bool operator==(x64w_Mem a, x64w_Mem b) {
	if (a.size_override != b.size_override) return false;
	if (a.base_scale != b.base_scale) return false;
	if (a.index_scale != b.index_scale) return false;
	if (a.base != b.base) return false;
	if (a.index != b.index) return false;
	if (a.displacement != b.displacement) return false;
	return true;
}
#endif

#undef X64W_UNDERLYING

#ifdef X64W_NO_PREFIX

// Short names might collide, so they need to be #undef'able.
// No typedefs.
#define Result x64w_Result
#define Gpr8  x64w_Gpr8
#define Gpr16 x64w_Gpr16
#define Gpr32 x64w_Gpr32
#define Gpr64 x64w_Gpr64
#define Xmm x64w_Xmm
#define Ymm x64w_Ymm
#define Zmm x64w_Zmm
#define al   x64w_al
#define cl   x64w_cl
#define dl   x64w_dl
#define bl   x64w_bl
#define ah   x64w_ah
#define ch   x64w_ch
#define dh   x64w_dh
#define bh   x64w_bh
#define r8b  x64w_r8b
#define r9b  x64w_r9b
#define r10b x64w_r10b
#define r11b x64w_r11b
#define r12b x64w_r12b
#define r13b x64w_r13b
#define r14b x64w_r14b
#define r15b x64w_r15b
#define spl  x64w_spl
#define bpl  x64w_bpl
#define sil  x64w_sil
#define dil  x64w_dil
#define ax   x64w_ax
#define cx   x64w_cx
#define dx   x64w_dx
#define bx   x64w_bx
#define sp   x64w_sp
#define bp   x64w_bp
#define si   x64w_si
#define di   x64w_di
#define r8w  x64w_r8w
#define r9w  x64w_r9w
#define r10w x64w_r10w
#define r11w x64w_r11w
#define r12w x64w_r12w
#define r13w x64w_r13w
#define r14w x64w_r14w
#define r15w x64w_r15w
#define eax  x64w_eax
#define ecx  x64w_ecx
#define edx  x64w_edx
#define ebx  x64w_ebx
#define esp  x64w_esp
#define ebp  x64w_ebp
#define esi  x64w_esi
#define edi  x64w_edi
#define r8d  x64w_r8d
#define r9d  x64w_r9d
#define r10d x64w_r10d
#define r11d x64w_r11d
#define r12d x64w_r12d
#define r13d x64w_r13d
#define r14d x64w_r14d
#define r15d x64w_r15d
#define rax  x64w_rax
#define rcx  x64w_rcx
#define rdx  x64w_rdx
#define rbx  x64w_rbx
#define rsp  x64w_rsp
#define rbp  x64w_rbp
#define rsi  x64w_rsi
#define rdi  x64w_rdi
#define r8   x64w_r8
#define r9   x64w_r9
#define r10  x64w_r10
#define r11  x64w_r11
#define r12  x64w_r12
#define r13  x64w_r13
#define r14  x64w_r14
#define r15  x64w_r15
#define xmm0 x64w_xmm0
#define xmm1 x64w_xmm1
#define xmm2 x64w_xmm2
#define xmm3 x64w_xmm3
#define xmm4 x64w_xmm4
#define xmm5 x64w_xmm5
#define xmm6 x64w_xmm6
#define xmm7 x64w_xmm7
#define xmm8 x64w_xmm8
#define xmm9 x64w_xmm9
#define xmm10 x64w_xmm10
#define xmm11 x64w_xmm11
#define xmm12 x64w_xmm12
#define xmm13 x64w_xmm13
#define xmm14 x64w_xmm14
#define xmm15 x64w_xmm15
#define xmm16 x64w_xmm16
#define xmm17 x64w_xmm17
#define xmm18 x64w_xmm18
#define xmm19 x64w_xmm19
#define xmm20 x64w_xmm20
#define xmm21 x64w_xmm21
#define xmm22 x64w_xmm22
#define xmm23 x64w_xmm23
#define xmm24 x64w_xmm24
#define xmm25 x64w_xmm25
#define xmm26 x64w_xmm26
#define xmm27 x64w_xmm27
#define xmm28 x64w_xmm28
#define xmm29 x64w_xmm29
#define xmm30 x64w_xmm30
#define xmm31 x64w_xmm31
#define ymm0 x64w_ymm0
#define ymm1 x64w_ymm1
#define ymm2 x64w_ymm2
#define ymm3 x64w_ymm3
#define ymm4 x64w_ymm4
#define ymm5 x64w_ymm5
#define ymm6 x64w_ymm6
#define ymm7 x64w_ymm7
#define ymm8 x64w_ymm8
#define ymm9 x64w_ymm9
#define ymm10 x64w_ymm10
#define ymm11 x64w_ymm11
#define ymm12 x64w_ymm12
#define ymm13 x64w_ymm13
#define ymm14 x64w_ymm14
#define ymm15 x64w_ymm15
#define ymm16 x64w_ymm16
#define ymm17 x64w_ymm17
#define ymm18 x64w_ymm18
#define ymm19 x64w_ymm19
#define ymm20 x64w_ymm20
#define ymm21 x64w_ymm21
#define ymm22 x64w_ymm22
#define ymm23 x64w_ymm23
#define ymm24 x64w_ymm24
#define ymm25 x64w_ymm25
#define ymm26 x64w_ymm26
#define ymm27 x64w_ymm27
#define ymm28 x64w_ymm28
#define ymm29 x64w_ymm29
#define ymm30 x64w_ymm30
#define ymm31 x64w_ymm31
#define zmm0 x64w_zmm0
#define zmm1 x64w_zmm1
#define zmm2 x64w_zmm2
#define zmm3 x64w_zmm3
#define zmm4 x64w_zmm4
#define zmm5 x64w_zmm5
#define zmm6 x64w_zmm6
#define zmm7 x64w_zmm7
#define zmm8 x64w_zmm8
#define zmm9 x64w_zmm9
#define zmm10 x64w_zmm10
#define zmm11 x64w_zmm11
#define zmm12 x64w_zmm12
#define zmm13 x64w_zmm13
#define zmm14 x64w_zmm14
#define zmm15 x64w_zmm15
#define zmm16 x64w_zmm16
#define zmm17 x64w_zmm17
#define zmm18 x64w_zmm18
#define zmm19 x64w_zmm19
#define zmm20 x64w_zmm20
#define zmm21 x64w_zmm21
#define zmm22 x64w_zmm22
#define zmm23 x64w_zmm23
#define zmm24 x64w_zmm24
#define zmm25 x64w_zmm25
#define zmm26 x64w_zmm26
#define zmm27 x64w_zmm27
#define zmm28 x64w_zmm28
#define zmm29 x64w_zmm29
#define zmm30 x64w_zmm30
#define zmm31 x64w_zmm31
#define Mem x64w_Mem
#define mem32_b x64w_mem32_b
#define mem32_i x64w_mem32_i
#define mem32_d x64w_mem32_d
#define mem32_bi x64w_mem32_bi
#define mem32_bd x64w_mem32_bd
#define mem32_id x64w_mem32_id
#define mem32_bid x64w_mem32_bid
#define mem64_b x64w_mem64_b
#define mem64_i x64w_mem64_i
#define mem64_d x64w_mem64_d
#define mem64_bi x64w_mem64_bi
#define mem64_bd x64w_mem64_bd
#define mem64_id x64w_mem64_id
#define mem64_bid x64w_mem64_bid
#define gpr8_compatible_rr x64w_gpr8_compatible_rr
#define gpr8_compatible_rm x64w_gpr8_compatible_rm
#define push8i  x64w_push8i 
#define push32i x64w_push32i
#define push_r16 x64w_push_r16
#define push_r64 x64w_push_r64
#define push_m16 x64w_push_m16
#define push_m64 x64w_push_m64
#define pop_r16 x64w_pop_r16
#define pop_r64 x64w_pop_r64
#define pop_m16 x64w_pop_m16
#define pop_m64 x64w_pop_m64
#define mov_ri8  x64w_mov_ri8 
#define mov_ri16 x64w_mov_ri16
#define mov_ri32 x64w_mov_ri32
#define mov_ri64 x64w_mov_ri64
#define mov_rr8  x64w_mov_rr8
#define mov_rr16 x64w_mov_rr16
#define mov_rr32 x64w_mov_rr32
#define mov_rr64 x64w_mov_rr64
#define mov_rm8  x64w_mov_rm8 
#define mov_rm16 x64w_mov_rm16
#define mov_rm32 x64w_mov_rm32
#define mov_rm64 x64w_mov_rm64
#define mov_mi8  x64w_mov_mi8 
#define mov_mi16 x64w_mov_mi16
#define mov_mi32 x64w_mov_mi32
#define mov_m64i32 x64w_mov_m64i32
#define mov_mr8  x64w_mov_mr8 
#define mov_mr16 x64w_mov_mr16
#define mov_mr32 x64w_mov_mr32
#define mov_mr64 x64w_mov_mr64
#define adcx_rr32 x64w_adcx_rr32
#define adcx_rr64 x64w_adcx_rr64
#define addpd_xx  x64w_addpd_xx
#define addpd_xm  x64w_addpd_xm
#define vaddpd_xxx x64w_vaddpd_xxx
#define vaddpd_xxm x64w_vaddpd_xxm
#define vaddpd_yyy x64w_vaddpd_yyy
#define vaddpd_yym x64w_vaddpd_yym
#define vaddpd_zzz x64w_vaddpd_zzz
#define vaddpd_zzm x64w_vaddpd_zzm

#define adc_ri8    x64w_adc_ri8   
#define adc_ri16   x64w_adc_ri16  
#define adc_ri32   x64w_adc_ri32  
#define adc_r64i32 x64w_adc_r64i32
#define adc_r16i8  x64w_adc_r16i8 
#define adc_r32i8  x64w_adc_r32i8 
#define adc_r64i8  x64w_adc_r64i8 
#define adc_rr8    x64w_adc_rr8   
#define adc_rr16   x64w_adc_rr16  
#define adc_rr32   x64w_adc_rr32  
#define adc_rr64   x64w_adc_rr64  
#define adc_rm8    x64w_adc_rm8   
#define adc_rm16   x64w_adc_rm16  
#define adc_rm32   x64w_adc_rm32  
#define adc_rm64   x64w_adc_rm64  
#define adc_mi8    x64w_adc_mi8   
#define adc_mi16   x64w_adc_mi16  
#define adc_mi32   x64w_adc_mi32  
#define adc_m64i32 x64w_adc_m64i32
#define adc_m16i8  x64w_adc_m16i8 
#define adc_m32i8  x64w_adc_m32i8 
#define adc_m64i8  x64w_adc_m64i8 
#define adc_mr8    x64w_adc_mr8   
#define adc_mr16   x64w_adc_mr16  
#define adc_mr32   x64w_adc_mr32  
#define adc_mr64   x64w_adc_mr64  
#define add_ri8    x64w_add_ri8   
#define add_ri16   x64w_add_ri16  
#define add_ri32   x64w_add_ri32  
#define add_r64i32 x64w_add_r64i32
#define add_r16i8  x64w_add_r16i8 
#define add_r32i8  x64w_add_r32i8 
#define add_r64i8  x64w_add_r64i8 
#define add_rr8    x64w_add_rr8   
#define add_rr16   x64w_add_rr16  
#define add_rr32   x64w_add_rr32  
#define add_rr64   x64w_add_rr64  
#define add_rm8    x64w_add_rm8   
#define add_rm16   x64w_add_rm16  
#define add_rm32   x64w_add_rm32  
#define add_rm64   x64w_add_rm64  
#define add_mi8    x64w_add_mi8   
#define add_mi16   x64w_add_mi16  
#define add_mi32   x64w_add_mi32  
#define add_m64i32 x64w_add_m64i32
#define add_m16i8  x64w_add_m16i8 
#define add_m32i8  x64w_add_m32i8 
#define add_m64i8  x64w_add_m64i8 
#define add_mr8    x64w_add_mr8   
#define add_mr16   x64w_add_mr16  
#define add_mr32   x64w_add_mr32  
#define add_mr64   x64w_add_mr64  
#define xor_ri8    x64w_xor_ri8   
#define xor_ri16   x64w_xor_ri16  
#define xor_ri32   x64w_xor_ri32  
#define xor_r64i32 x64w_xor_r64i32
#define xor_r16i8  x64w_xor_r16i8 
#define xor_r32i8  x64w_xor_r32i8 
#define xor_r64i8  x64w_xor_r64i8 
#define xor_rr8    x64w_xor_rr8   
#define xor_rr16   x64w_xor_rr16  
#define xor_rr32   x64w_xor_rr32  
#define xor_rr64   x64w_xor_rr64  
#define xor_rm8    x64w_xor_rm8   
#define xor_rm16   x64w_xor_rm16  
#define xor_rm32   x64w_xor_rm32  
#define xor_rm64   x64w_xor_rm64  
#define xor_mi8    x64w_xor_mi8   
#define xor_mi16   x64w_xor_mi16  
#define xor_mi32   x64w_xor_mi32  
#define xor_m64i32 x64w_xor_m64i32
#define xor_m16i8  x64w_xor_m16i8 
#define xor_m32i8  x64w_xor_m32i8 
#define xor_m64i8  x64w_xor_m64i8 
#define xor_mr8    x64w_xor_mr8   
#define xor_mr16   x64w_xor_mr16  
#define xor_mr32   x64w_xor_mr32  
#define xor_mr64   x64w_xor_mr64  
#define and_ri8    x64w_and_ri8   
#define and_ri16   x64w_and_ri16  
#define and_ri32   x64w_and_ri32  
#define and_r64i32 x64w_and_r64i32
#define and_r16i8  x64w_and_r16i8 
#define and_r32i8  x64w_and_r32i8 
#define and_r64i8  x64w_and_r64i8 
#define and_rr8    x64w_and_rr8   
#define and_rr16   x64w_and_rr16  
#define and_rr32   x64w_and_rr32  
#define and_rr64   x64w_and_rr64  
#define and_rm8    x64w_and_rm8   
#define and_rm16   x64w_and_rm16  
#define and_rm32   x64w_and_rm32  
#define and_rm64   x64w_and_rm64  
#define and_mi8    x64w_and_mi8   
#define and_mi16   x64w_and_mi16  
#define and_mi32   x64w_and_mi32  
#define and_m64i32 x64w_and_m64i32
#define and_m16i8  x64w_and_m16i8 
#define and_m32i8  x64w_and_m32i8 
#define and_m64i8  x64w_and_m64i8 
#define and_mr8    x64w_and_mr8   
#define and_mr16   x64w_and_mr16  
#define and_mr32   x64w_and_mr32  
#define and_mr64   x64w_and_mr64  
#define or_ri8    x64w_or_ri8   
#define or_ri16   x64w_or_ri16  
#define or_ri32   x64w_or_ri32  
#define or_r64i32 x64w_or_r64i32
#define or_r16i8  x64w_or_r16i8 
#define or_r32i8  x64w_or_r32i8 
#define or_r64i8  x64w_or_r64i8 
#define or_rr8    x64w_or_rr8   
#define or_rr16   x64w_or_rr16  
#define or_rr32   x64w_or_rr32  
#define or_rr64   x64w_or_rr64  
#define or_rm8    x64w_or_rm8   
#define or_rm16   x64w_or_rm16  
#define or_rm32   x64w_or_rm32  
#define or_rm64   x64w_or_rm64  
#define or_mi8    x64w_or_mi8   
#define or_mi16   x64w_or_mi16  
#define or_mi32   x64w_or_mi32  
#define or_m64i32 x64w_or_m64i32
#define or_m16i8  x64w_or_m16i8 
#define or_m32i8  x64w_or_m32i8 
#define or_m64i8  x64w_or_m64i8 
#define or_mr8    x64w_or_mr8   
#define or_mr16   x64w_or_mr16  
#define or_mr32   x64w_or_mr32  
#define or_mr64   x64w_or_mr64  
#define sub_ri8    x64w_sub_ri8   
#define sub_ri16   x64w_sub_ri16  
#define sub_ri32   x64w_sub_ri32  
#define sub_r64i32 x64w_sub_r64i32
#define sub_r16i8  x64w_sub_r16i8 
#define sub_r32i8  x64w_sub_r32i8 
#define sub_r64i8  x64w_sub_r64i8 
#define sub_rr8    x64w_sub_rr8   
#define sub_rr16   x64w_sub_rr16  
#define sub_rr32   x64w_sub_rr32  
#define sub_rr64   x64w_sub_rr64  
#define sub_rm8    x64w_sub_rm8   
#define sub_rm16   x64w_sub_rm16  
#define sub_rm32   x64w_sub_rm32  
#define sub_rm64   x64w_sub_rm64  
#define sub_mi8    x64w_sub_mi8   
#define sub_mi16   x64w_sub_mi16  
#define sub_mi32   x64w_sub_mi32  
#define sub_m64i32 x64w_sub_m64i32
#define sub_m16i8  x64w_sub_m16i8 
#define sub_m32i8  x64w_sub_m32i8 
#define sub_m64i8  x64w_sub_m64i8 
#define sub_mr8    x64w_sub_mr8   
#define sub_mr16   x64w_sub_mr16  
#define sub_mr32   x64w_sub_mr32  
#define sub_mr64   x64w_sub_mr64  
#define inc_r8  x64w_inc_r8   
#define inc_r16 x64w_inc_r16  
#define inc_r32 x64w_inc_r32  
#define inc_r64 x64w_inc_r64  
#define inc_m8  x64w_inc_m8   
#define inc_m16 x64w_inc_m16  
#define inc_m32 x64w_inc_m32  
#define inc_m64 x64w_inc_m64  
#define dec_r8  x64w_dec_r8   
#define dec_r16 x64w_dec_r16  
#define dec_r32 x64w_dec_r32  
#define dec_r64 x64w_dec_r64  
#define dec_m8  x64w_dec_m8   
#define dec_m16 x64w_dec_m16  
#define dec_m32 x64w_dec_m32  
#define dec_m64 x64w_dec_m64  
#define not_r8  x64w_not_r8   
#define not_r16 x64w_not_r16  
#define not_r32 x64w_not_r32  
#define not_r64 x64w_not_r64  
#define not_m8  x64w_not_m8   
#define not_m16 x64w_not_m16  
#define not_m32 x64w_not_m32  
#define not_m64 x64w_not_m64  
#define neg_r8  x64w_neg_r8   
#define neg_r16 x64w_neg_r16  
#define neg_r32 x64w_neg_r32  
#define neg_r64 x64w_neg_r64  
#define neg_m8  x64w_neg_m8   
#define neg_m16 x64w_neg_m16  
#define neg_m32 x64w_neg_m32  
#define neg_m64 x64w_neg_m64  
#define mul_r8  x64w_mul_r8   
#define mul_r16 x64w_mul_r16  
#define mul_r32 x64w_mul_r32  
#define mul_r64 x64w_mul_r64  
#define mul_m8  x64w_mul_m8   
#define mul_m16 x64w_mul_m16  
#define mul_m32 x64w_mul_m32  
#define mul_m64 x64w_mul_m64  
#define div_r8  x64w_div_r8   
#define div_r16 x64w_div_r16  
#define div_r32 x64w_div_r32  
#define div_r64 x64w_div_r64  
#define div_m8  x64w_div_m8   
#define div_m16 x64w_div_m16  
#define div_m32 x64w_div_m32  
#define div_m64 x64w_div_m64  
#define shl_r8_1   x64w_shl_r8_1  
#define shl_r16_1  x64w_shl_r16_1 
#define shl_r32_1  x64w_shl_r32_1 
#define shl_r64_1  x64w_shl_r64_1 
#define shl_ri8    x64w_shl_ri8   
#define shl_r16i8  x64w_shl_r16i8 
#define shl_r32i8  x64w_shl_r32i8 
#define shl_r64i8  x64w_shl_r64i8 
#define shl_r8_cl  x64w_shl_r8_cl 
#define shl_r16_cl x64w_shl_r16_cl
#define shl_r32_cl x64w_shl_r32_cl
#define shl_r64_cl x64w_shl_r64_cl
#define shl_m8_1   x64w_shl_m8_1  
#define shl_m16_1  x64w_shl_m16_1 
#define shl_m32_1  x64w_shl_m32_1 
#define shl_m64_1  x64w_shl_m64_1 
#define shl_mi8    x64w_shl_mi8   
#define shl_m16i8  x64w_shl_m16i8 
#define shl_m32i8  x64w_shl_m32i8 
#define shl_m64i8  x64w_shl_m64i8 
#define shl_m8_cl  x64w_shl_m8_cl 
#define shl_m16_cl x64w_shl_m16_cl
#define shl_m32_cl x64w_shl_m32_cl
#define shl_m64_cl x64w_shl_m64_cl
#define shr_r8_1   x64w_shr_r8_1  
#define shr_r16_1  x64w_shr_r16_1 
#define shr_r32_1  x64w_shr_r32_1 
#define shr_r64_1  x64w_shr_r64_1 
#define shr_ri8    x64w_shr_ri8   
#define shr_r16i8  x64w_shr_r16i8 
#define shr_r32i8  x64w_shr_r32i8 
#define shr_r64i8  x64w_shr_r64i8 
#define shr_r8_cl  x64w_shr_r8_cl 
#define shr_r16_cl x64w_shr_r16_cl
#define shr_r32_cl x64w_shr_r32_cl
#define shr_r64_cl x64w_shr_r64_cl
#define shr_m8_1   x64w_shr_m8_1  
#define shr_m16_1  x64w_shr_m16_1 
#define shr_m32_1  x64w_shr_m32_1 
#define shr_m64_1  x64w_shr_m64_1 
#define shr_mi8    x64w_shr_mi8   
#define shr_m16i8  x64w_shr_m16i8 
#define shr_m32i8  x64w_shr_m32i8 
#define shr_m64i8  x64w_shr_m64i8 
#define shr_m8_cl  x64w_shr_m8_cl 
#define shr_m16_cl x64w_shr_m16_cl
#define shr_m32_cl x64w_shr_m32_cl
#define shr_m64_cl x64w_shr_m64_cl
#define sal_r8_1   x64w_sal_r8_1  
#define sal_r16_1  x64w_sal_r16_1 
#define sal_r32_1  x64w_sal_r32_1 
#define sal_r64_1  x64w_sal_r64_1 
#define sal_ri8    x64w_sal_ri8   
#define sal_r16i8  x64w_sal_r16i8 
#define sal_r32i8  x64w_sal_r32i8 
#define sal_r64i8  x64w_sal_r64i8 
#define sal_r8_cl  x64w_sal_r8_cl 
#define sal_r16_cl x64w_sal_r16_cl
#define sal_r32_cl x64w_sal_r32_cl
#define sal_r64_cl x64w_sal_r64_cl
#define sal_m8_1   x64w_sal_m8_1  
#define sal_m16_1  x64w_sal_m16_1 
#define sal_m32_1  x64w_sal_m32_1 
#define sal_m64_1  x64w_sal_m64_1 
#define sal_mi8    x64w_sal_mi8   
#define sal_m16i8  x64w_sal_m16i8 
#define sal_m32i8  x64w_sal_m32i8 
#define sal_m64i8  x64w_sal_m64i8 
#define sal_m8_cl  x64w_sal_m8_cl 
#define sal_m16_cl x64w_sal_m16_cl
#define sal_m32_cl x64w_sal_m32_cl
#define sal_m64_cl x64w_sal_m64_cl
#define sar_r8_1   x64w_sar_r8_1  
#define sar_r16_1  x64w_sar_r16_1 
#define sar_r32_1  x64w_sar_r32_1 
#define sar_r64_1  x64w_sar_r64_1 
#define sar_ri8    x64w_sar_ri8   
#define sar_r16i8  x64w_sar_r16i8 
#define sar_r32i8  x64w_sar_r32i8 
#define sar_r64i8  x64w_sar_r64i8 
#define sar_r8_cl  x64w_sar_r8_cl 
#define sar_r16_cl x64w_sar_r16_cl
#define sar_r32_cl x64w_sar_r32_cl
#define sar_r64_cl x64w_sar_r64_cl
#define sar_m8_1   x64w_sar_m8_1  
#define sar_m16_1  x64w_sar_m16_1 
#define sar_m32_1  x64w_sar_m32_1 
#define sar_m64_1  x64w_sar_m64_1 
#define sar_mi8    x64w_sar_mi8   
#define sar_m16i8  x64w_sar_m16i8 
#define sar_m32i8  x64w_sar_m32i8 
#define sar_m64i8  x64w_sar_m64i8 
#define sar_m8_cl  x64w_sar_m8_cl 
#define sar_m16_cl x64w_sar_m16_cl
#define sar_m32_cl x64w_sar_m32_cl
#define sar_m64_cl x64w_sar_m64_cl
#define lea_rm16 x64w_lea_rm16
#define lea_rm32 x64w_lea_rm32
#define lea_rm64 x64w_lea_rm64


#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif
//     Encoding example for 64 bit increment:
//
// b - base
// i - index
// s - index scale:
//     1: 00
//     2: 01
//     4: 10
//     8: 11
//
// ----   rbp/r13 base requires disp8 even when 0
// ---- * rsp/r12 base requires 0x24 postfix before displacement when not using index
//
// inc opcode: 0xff | 0b11111111
//
//                   REX      Opcode
// inc b           - 0100100b 11111111 11000bbb
// inc [b        ] - 0100100b 11111111 00000bbb *
// inc [b  +  d8 ] - 0100100b 11111111 01000bbb * d8
// inc [b  +  d32] - 0100100b 11111111 10000bbb * d32
// inc [      d32] - 01001000 11111111 00000100 00100101 d32 <- No conflict because rsp
// inc [  i*s+d32] - 010010i0 11111111 00000100 ssiii101 d32 <- cannot be used as index
// inc [b+i*s    ] - 010010ib 11111111 00000100 ssiiibbb
// inc [b+i*s+d8 ] - 010010ib 11111111 01000100 ssiiibbb d8
// inc [b+i*s+d32] - 010010ib 11111111 10000100 ssiiibbb d32

#endif // X64W_H_
