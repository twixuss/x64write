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
#define X64W_VALIDATE(condition, message) do { if (!(condition)) return message; } while (0)
	To override default validation check. Here you can insert logging and whatnot.
	
	Example (no prefixes):
uint8_t *buf = malloc(10 * X64W_MAX_INSTRUCTION_SIZE);
uint8_t *c = buf;
push64r(&c, rbp);                    // push rbp
mov64rr(&c, rbp, rsp);               // mov rbp, rsp
sub64ri(&c, rsp, 16);                // sub rsp, 16
mov64mr(&c, mem64_b(rsp), rcx);      // mov [rsp], rcx
mov64mr(&c, mem64_bd(rsp, +8), rdx); // mov [rsp+8], rdx
add64ri(&c, rsp, 16);                // add rsp, 16
pop64r(&c, rbp);                     // pop rbp
ret(&c);                             // ret

	Instruction naming:
<instr>[<operand size in _bits_>][<r/m/i - register or memory or immediate> ...]

	Memory operand naming: suffix of mem_* determines argument type and count
b - base register
i - index register, index scale (1/2/4/8)
d - 32-bit displacement

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

bool x64w_gpr8_compatible_rr(x64w_Gpr8 a, x64w_Gpr8 b);
bool x64w_gpr8_compatible_rm(x64w_Gpr8 a, x64w_Mem b);

x64w_Result x64w_inc8r (uint8_t **c, x64w_Gpr8  r);
x64w_Result x64w_inc16r(uint8_t **c, x64w_Gpr16 r);
x64w_Result x64w_inc32r(uint8_t **c, x64w_Gpr32 r);
x64w_Result x64w_inc64r(uint8_t **c, x64w_Gpr64 r);
x64w_Result x64w_inc8m (uint8_t **c, x64w_Mem m);
x64w_Result x64w_inc16m(uint8_t **c, x64w_Mem m);
x64w_Result x64w_inc32m(uint8_t **c, x64w_Mem m);
x64w_Result x64w_inc64m(uint8_t **c, x64w_Mem m);

x64w_Result x64w_dec8r (uint8_t **c, x64w_Gpr8  r);
x64w_Result x64w_dec16r(uint8_t **c, x64w_Gpr16 r);
x64w_Result x64w_dec32r(uint8_t **c, x64w_Gpr32 r);
x64w_Result x64w_dec64r(uint8_t **c, x64w_Gpr64 r);
x64w_Result x64w_dec8m (uint8_t **c, x64w_Mem m);
x64w_Result x64w_dec16m(uint8_t **c, x64w_Mem m);
x64w_Result x64w_dec32m(uint8_t **c, x64w_Mem m);
x64w_Result x64w_dec64m(uint8_t **c, x64w_Mem m);

x64w_Result x64w_not8r (uint8_t **c, x64w_Gpr8  r);
x64w_Result x64w_not16r(uint8_t **c, x64w_Gpr16 r);
x64w_Result x64w_not32r(uint8_t **c, x64w_Gpr32 r);
x64w_Result x64w_not64r(uint8_t **c, x64w_Gpr64 r);
x64w_Result x64w_not8m (uint8_t **c, x64w_Mem m);
x64w_Result x64w_not16m(uint8_t **c, x64w_Mem m);
x64w_Result x64w_not32m(uint8_t **c, x64w_Mem m);
x64w_Result x64w_not64m(uint8_t **c, x64w_Mem m);

x64w_Result x64w_neg8r (uint8_t **c, x64w_Gpr8  r);
x64w_Result x64w_neg16r(uint8_t **c, x64w_Gpr16 r);
x64w_Result x64w_neg32r(uint8_t **c, x64w_Gpr32 r);
x64w_Result x64w_neg64r(uint8_t **c, x64w_Gpr64 r);
x64w_Result x64w_neg8m (uint8_t **c, x64w_Mem m);
x64w_Result x64w_neg16m(uint8_t **c, x64w_Mem m);
x64w_Result x64w_neg32m(uint8_t **c, x64w_Mem m);
x64w_Result x64w_neg64m(uint8_t **c, x64w_Mem m);

x64w_Result x64w_push8i (uint8_t **c, int8_t   i);
x64w_Result x64w_push32i(uint8_t **c, int32_t  i);
x64w_Result x64w_push16r(uint8_t **c, x64w_Gpr16 r);
x64w_Result x64w_push64r(uint8_t **c, x64w_Gpr64 r);
x64w_Result x64w_push16m(uint8_t **c, x64w_Mem m);
x64w_Result x64w_push64m(uint8_t **c, x64w_Mem m);

x64w_Result x64w_pop16r(uint8_t **c, x64w_Gpr16 r);
x64w_Result x64w_pop64r(uint8_t **c, x64w_Gpr64 r);
x64w_Result x64w_pop16m(uint8_t **c, x64w_Mem m);
x64w_Result x64w_pop64m(uint8_t **c, x64w_Mem m);

x64w_Result x64w_mov8ri (uint8_t **c, x64w_Gpr8  r, int8_t  i);
x64w_Result x64w_mov16ri(uint8_t **c, x64w_Gpr16 r, int16_t i);
x64w_Result x64w_mov32ri(uint8_t **c, x64w_Gpr32 r, int32_t i);
x64w_Result x64w_mov64ri(uint8_t **c, x64w_Gpr64 r, int64_t i);
x64w_Result x64w_mov8rr (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s);
x64w_Result x64w_mov16rr(uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s);
x64w_Result x64w_mov32rr(uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s);
x64w_Result x64w_mov64rr(uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s);
x64w_Result x64w_mov8rm (uint8_t **c, x64w_Gpr8  r, x64w_Mem m);
x64w_Result x64w_mov16rm(uint8_t **c, x64w_Gpr16 r, x64w_Mem m);
x64w_Result x64w_mov32rm(uint8_t **c, x64w_Gpr32 r, x64w_Mem m);
x64w_Result x64w_mov64rm(uint8_t **c, x64w_Gpr64 r, x64w_Mem m);
x64w_Result x64w_mov8mi (uint8_t **c, x64w_Mem m, int8_t  i);
x64w_Result x64w_mov16mi(uint8_t **c, x64w_Mem m, int16_t i);
x64w_Result x64w_mov32mi(uint8_t **c, x64w_Mem m, int32_t i);
x64w_Result x64w_mov64mi(uint8_t **c, x64w_Mem m, int32_t i);
x64w_Result x64w_mov8mr (uint8_t **c, x64w_Mem m, x64w_Gpr8  r);
x64w_Result x64w_mov16mr(uint8_t **c, x64w_Mem m, x64w_Gpr16 r);
x64w_Result x64w_mov32mr(uint8_t **c, x64w_Mem m, x64w_Gpr32 r);
x64w_Result x64w_mov64mr(uint8_t **c, x64w_Mem m, x64w_Gpr64 r);


#ifdef X64W_IMPLEMENTATION

#define X64W_GPR8_NEEDS_REX(gpr) (!!((gpr) & 0x10))

#ifndef X64W_VALIDATE
#define X64W_VALIDATE(condition, message) do { if (!(condition)) return message; } while (0)
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
			X64W_VALIDATE(r < 0x10 || (0x14 <= r && r < 0x18), "invalid 8-bit gpr"); \
		} else {                                                                     \
			X64W_VALIDATE(r < 0x10, "invalid gpr");                                  \
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

#define REXW     0x0000000000000001
#define OSO      0x0000000000000002 // operand size override
#define ASO      0x0000000000000004 // address size override
#define NO_MODRM 0x0000000000000008

static const uint8_t index_scale_table[] = {
	0, // 0
	0, // 1
	1, // 2
	0, // 3
	2, // 4
	0, // 5
	0, // 6
	0, // 7
	3, // 8
};

// Returns:
//     0 - no displacement
//     1 - 8 bit displacement
//     2 - 32 bit displacement
static int displacement_form(x64w_Mem m) {
	if (m.displacement == 0 && ((m.base & 7) != 5)) {
		return 0;
	} else if (x64w_fits_in_8(m.displacement)) {
		return 1;
	} else {
		return 2;
	}
}

static void write_displacement(uint8_t **c, int displacement_form, int32_t displacement) {
	**c = (uint8_t)displacement;
	*c += displacement_form == 1;
	W4(*c, displacement);
	*c += 4 * (displacement_form == 2);
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
static void write_m(uint8_t **c, x64w_Mem m, uint8_t mod, unsigned r7, unsigned i7, unsigned b7) {
	unsigned s = index_scale_table[m.index_scale];

	if (m.base_scale) {
		int df = displacement_form(m);
		if (m.index_scale) {
			*(*c)++ = mod | (df << 6) | (r7 << 3) | 0x04;
			*(*c)++ = (s << 6) | (i7 << 3) | b7;
		} else {
			*(*c)++ = mod | (df << 6) | (r7 << 3) | b7;
			**c = 0x24;
			*c += b7 == 4;
		}
		write_displacement(c, df, m.displacement);
	} else {
		*(*c)++ = mod | (r7 << 3) | 0x04;
		if (m.index_scale) {
			*(*c)++ = (s << 6) | (i7 << 3) | 0x05;
		} else {
			*(*c)++ = 0x25;
		}
		W4(*c, m.displacement);
		*c += 4;
	}
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

static instr_inline x64w_Result instr_i1(uint8_t **c, int8_t i, uint8_t opcode) {
	*(*c)++ = opcode;
	*(*c)++ = i;
	return 0;
}
static instr_inline x64w_Result instr_i4(uint8_t **c, int32_t i, uint8_t opcode) {
	*(*c)++ = opcode;
	W4(*c, i);
	*c += 4;
	return 0;
}
static instr_inline x64w_Result instr_o_ri(uint8_t **c, uint8_t r, int64_t i, unsigned size, uint8_t opcode, uint64_t flags) {
	X64W_VALIDATE_R(r);
	
	unsigned rexw          = !!(flags & REXW);
	unsigned size_override = !!(flags & OSO);

	**c = 0x66;
	*c += size_override;
	
	unsigned rexb = !!(r & 8);

	**c = 0x40 | (rexw << 3) | rexb;
	*c += rexb | rexw | X64W_GPR8_NEEDS_REX(r);
	*(*c)++ = opcode | (r & 7);

	write_immediate(c, i, size);

	return 0;
}
static instr_inline x64w_Result instr_r(uint8_t **c, uint8_t r, unsigned size, uint8_t opcode, uint8_t mod, uint64_t flags) {
	X64W_VALIDATE_R(r);

	mod <<= 3;
	
	unsigned rexb = !!(r & 8);
	
	unsigned rexw          = !!(flags & REXW);
	unsigned size_override = !!(flags & OSO);
	unsigned no_modrm      = !!(flags & NO_MODRM);
	
	**c = 0x66;
	*c += size_override;

	**c = 0x40 | (rexw << 3) | rexb;
	*c += rexw | rexb | X64W_GPR8_NEEDS_REX(r);

	if (no_modrm) {
		*(*c)++ = opcode | (r & 7);
	} else {
		*(*c)++ = opcode;
		*(*c)++ = 0xc0 | mod | (r & 7);
	}

	return 0;
}
static instr_inline x64w_Result instr_m(uint8_t **c, x64w_Mem d, uint8_t opcode, uint8_t mod, uint64_t flags) {
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

	// REX prefix
	**c = 0x40 | (rexw << 3) | (rexi << 1) | rexb;
	*c += rexw | rexi | rexb;

	*(*c)++ = opcode;

	write_m(c, d, mod, 0, i7, b7);

	return 0;
}
static instr_inline x64w_Result instr_rr(uint8_t **c, uint8_t d, uint8_t s, unsigned size, uint8_t opcode, uint64_t flags) {
	X64W_VALIDATE_RR(d, s);
	
	unsigned rexw          = !!(flags & REXW);
	unsigned size_override = !!(flags & OSO);
	
	unsigned rexb = !!(s & 8);
	unsigned rexr = !!(d & 8);
	
	**c = 0x66;
	*c += size_override;

	**c = 0x40 | (rexw << 3) | (rexr << 2) | rexb;
	*c += rexr | rexb | rexw | X64W_GPR8_NEEDS_REX(d) | X64W_GPR8_NEEDS_REX(s);
	*(*c)++ = opcode;
	*(*c)++ = 0xc0 | (s & 7) | ((d & 7) << 3);

	return 0;
}
static instr_inline x64w_Result instr_rm(uint8_t **c, uint8_t r, x64w_Mem m, unsigned size, uint8_t opcode, uint64_t flags) {
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

	// REX prefix
	**c = 0x40 | (rexw << 3) | (rexr << 2) | (rexi << 1) | rexb;
	*c += rexr | rexi | rexb | rexw | X64W_GPR8_NEEDS_REX(r);
	*(*c)++ = opcode;
	
	write_m(c, m, 0, r7, i7, b7);

	return 0;
}
static instr_inline x64w_Result instr_mi(uint8_t **c, x64w_Mem m, int64_t i, unsigned size, uint8_t opcode, uint64_t flags) {
	X64W_VALIDATE_M(m);
	
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

	// REX prefix
	**c = 0x40 | (rexw << 3) | (rexi << 1) | rexb;
	*c += rexi | rexb | rexw;
	*(*c)++ = opcode;
	
	write_m(c, m, 0, 0, i7, b7);

	write_immediate(c, i, size);

	return 0;
}

#undef no_inline
#undef force_inline
#undef instr_inline

x64w_Result x64w_inc8r (uint8_t **c, x64w_Gpr8  d) { return instr_r(c, d.i, 1, 0xfe, 0, 0); }
x64w_Result x64w_inc16r(uint8_t **c, x64w_Gpr16 d) { return instr_r(c, d.i, 2, 0xff, 0, OSO); }
x64w_Result x64w_inc32r(uint8_t **c, x64w_Gpr32 d) { return instr_r(c, d.i, 4, 0xff, 0, 0); }
x64w_Result x64w_inc64r(uint8_t **c, x64w_Gpr64 d) { return instr_r(c, d.i, 8, 0xff, 0, REXW); }
x64w_Result x64w_inc8m (uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xfe, 0, 0); }
x64w_Result x64w_inc16m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xff, 0, OSO); }
x64w_Result x64w_inc32m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xff, 0, 0); }
x64w_Result x64w_inc64m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xff, 0, REXW); }

x64w_Result x64w_dec8r (uint8_t **c, x64w_Gpr8  d) { return instr_r(c, d.i, 1, 0xfe, 1, 0); }
x64w_Result x64w_dec16r(uint8_t **c, x64w_Gpr16 d) { return instr_r(c, d.i, 2, 0xff, 1, OSO); }
x64w_Result x64w_dec32r(uint8_t **c, x64w_Gpr32 d) { return instr_r(c, d.i, 4, 0xff, 1, 0); }
x64w_Result x64w_dec64r(uint8_t **c, x64w_Gpr64 d) { return instr_r(c, d.i, 8, 0xff, 1, REXW); }
x64w_Result x64w_dec8m (uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xfe, 1, 0); }
x64w_Result x64w_dec16m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xff, 1, OSO); }
x64w_Result x64w_dec32m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xff, 1, 0); }
x64w_Result x64w_dec64m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xff, 1, REXW); }

x64w_Result x64w_not8r (uint8_t **c, x64w_Gpr8  d) { return instr_r(c, d.i, 1, 0xf6, 2, 0); }
x64w_Result x64w_not16r(uint8_t **c, x64w_Gpr16 d) { return instr_r(c, d.i, 2, 0xf7, 2, OSO); }
x64w_Result x64w_not32r(uint8_t **c, x64w_Gpr32 d) { return instr_r(c, d.i, 4, 0xf7, 2, 0); }
x64w_Result x64w_not64r(uint8_t **c, x64w_Gpr64 d) { return instr_r(c, d.i, 8, 0xf7, 2, REXW); }
x64w_Result x64w_not8m (uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xf6, 2, 0); }
x64w_Result x64w_not16m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xf7, 2, OSO); }
x64w_Result x64w_not32m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xf7, 2, 0); }
x64w_Result x64w_not64m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xf7, 2, REXW); }

x64w_Result x64w_neg8r (uint8_t **c, x64w_Gpr8  d) { return instr_r(c, d.i, 1, 0xf6, 3, 0); }
x64w_Result x64w_neg16r(uint8_t **c, x64w_Gpr16 d) { return instr_r(c, d.i, 2, 0xf7, 3, OSO); }
x64w_Result x64w_neg32r(uint8_t **c, x64w_Gpr32 d) { return instr_r(c, d.i, 4, 0xf7, 3, 0); }
x64w_Result x64w_neg64r(uint8_t **c, x64w_Gpr64 d) { return instr_r(c, d.i, 8, 0xf7, 3, REXW); }
x64w_Result x64w_neg8m (uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xf6, 3, 0); }
x64w_Result x64w_neg16m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xf7, 3, OSO); }
x64w_Result x64w_neg32m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xf7, 3, 0); }
x64w_Result x64w_neg64m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xf7, 3, REXW); }

x64w_Result x64w_push8i (uint8_t **c, int8_t   i) { return instr_i1(c, i, 0x6a); }
x64w_Result x64w_push32i(uint8_t **c, int32_t  i) { return instr_i4(c, i, 0x68); }
x64w_Result x64w_push16r(uint8_t **c, x64w_Gpr16 s) { return instr_r(c, s.i, 2, 0x50, 0, NO_MODRM | OSO); }
x64w_Result x64w_push64r(uint8_t **c, x64w_Gpr64 s) { return instr_r(c, s.i, 8, 0x50, 0, NO_MODRM); }
x64w_Result x64w_push16m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xff, 6, OSO); }
x64w_Result x64w_push64m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0xff, 6, 0); }

x64w_Result x64w_pop16r(uint8_t **c, x64w_Gpr16 s) { return instr_r(c, s.i, 2, 0x58, 0, NO_MODRM | OSO); }
x64w_Result x64w_pop64r(uint8_t **c, x64w_Gpr64 s) { return instr_r(c, s.i, 8, 0x58, 0, NO_MODRM); }
x64w_Result x64w_pop16m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0x8f, 0, OSO); }
x64w_Result x64w_pop64m(uint8_t **c, x64w_Mem d) { return instr_m(c, d, 0x8f, 0, 0); }

x64w_Result x64w_mov8ri (uint8_t **c, x64w_Gpr8  r, int8_t   i) { return instr_o_ri(c, r.i, i, 1, 0xb0, 0); }
x64w_Result x64w_mov16ri(uint8_t **c, x64w_Gpr16 r, int16_t  i) { return instr_o_ri(c, r.i, i, 2, 0xb8, OSO); }
x64w_Result x64w_mov32ri(uint8_t **c, x64w_Gpr32 r, int32_t  i) { return instr_o_ri(c, r.i, i, 4, 0xb8, 0); }
x64w_Result x64w_mov64ri(uint8_t **c, x64w_Gpr64 r, int64_t  i) { return instr_o_ri(c, r.i, i, 8, 0xb8, REXW); }
x64w_Result x64w_mov8rr (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s) { return instr_rr(c, d.i, s.i, 1, 0x8a, 0); }
x64w_Result x64w_mov16rr(uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s) { return instr_rr(c, d.i, s.i, 2, 0x8b, OSO); }
x64w_Result x64w_mov32rr(uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s) { return instr_rr(c, d.i, s.i, 4, 0x8b, 0); }
x64w_Result x64w_mov64rr(uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s) { return instr_rr(c, d.i, s.i, 8, 0x8b, REXW); }
x64w_Result x64w_mov8rm (uint8_t **c, x64w_Gpr8  d, x64w_Mem s) { return instr_rm(c, d.i, s, 1, 0x8a, 0); }
x64w_Result x64w_mov16rm(uint8_t **c, x64w_Gpr16 d, x64w_Mem s) { return instr_rm(c, d.i, s, 2, 0x8b, OSO); }
x64w_Result x64w_mov32rm(uint8_t **c, x64w_Gpr32 d, x64w_Mem s) { return instr_rm(c, d.i, s, 4, 0x8b, 0); }
x64w_Result x64w_mov64rm(uint8_t **c, x64w_Gpr64 d, x64w_Mem s) { return instr_rm(c, d.i, s, 8, 0x8b, REXW); }
x64w_Result x64w_mov8mr (uint8_t **c, x64w_Mem d, x64w_Gpr8  s) { return instr_rm(c, s.i, d, 1, 0x88, 0); }
x64w_Result x64w_mov16mr(uint8_t **c, x64w_Mem d, x64w_Gpr16 s) { return instr_rm(c, s.i, d, 2, 0x89, OSO); }
x64w_Result x64w_mov32mr(uint8_t **c, x64w_Mem d, x64w_Gpr32 s) { return instr_rm(c, s.i, d, 4, 0x89, 0); }
x64w_Result x64w_mov64mr(uint8_t **c, x64w_Mem d, x64w_Gpr64 s) { return instr_rm(c, s.i, d, 8, 0x89, REXW); }
x64w_Result x64w_mov8mi (uint8_t **c, x64w_Mem m, int8_t  i) { return instr_mi(c, m, i, 1, 0xc6, 0); }
x64w_Result x64w_mov16mi(uint8_t **c, x64w_Mem m, int16_t i) { return instr_mi(c, m, i, 2, 0xc7, OSO); }
x64w_Result x64w_mov32mi(uint8_t **c, x64w_Mem m, int32_t i) { return instr_mi(c, m, i, 4, 0xc7, 0); }
x64w_Result x64w_mov64mi(uint8_t **c, x64w_Mem m, int32_t i) { return instr_mi(c, m, i, 4, 0xc7, REXW); }

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

#endif // X64W_IMPLEMENTATION

#ifdef __cplusplus
} // extern "C"
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
#define inc8r  x64w_inc8r
#define inc16r x64w_inc16r
#define inc32r x64w_inc32r
#define inc64r x64w_inc64r
#define inc8m  x64w_inc8m
#define inc16m x64w_inc16m
#define inc32m x64w_inc32m
#define inc64m x64w_inc64m
#define dec8r  x64w_dec8r
#define dec16r x64w_dec16r
#define dec32r x64w_dec32r
#define dec64r x64w_dec64r
#define dec8m  x64w_dec8m
#define dec16m x64w_dec16m
#define dec32m x64w_dec32m
#define dec64m x64w_dec64m
#define not8r  x64w_not8r
#define not16r x64w_not16r
#define not32r x64w_not32r
#define not64r x64w_not64r
#define not8m  x64w_not8m
#define not16m x64w_not16m
#define not32m x64w_not32m
#define not64m x64w_not64m
#define neg8r  x64w_neg8r
#define neg16r x64w_neg16r
#define neg32r x64w_neg32r
#define neg64r x64w_neg64r
#define neg8m  x64w_neg8m
#define neg16m x64w_neg16m
#define neg32m x64w_neg32m
#define neg64m x64w_neg64m
#define push8i  x64w_push8i 
#define push32i x64w_push32i
#define push16r x64w_push16r
#define push64r x64w_push64r
#define push16m x64w_push16m
#define push64m x64w_push64m
#define pop16r x64w_pop16r
#define pop64r x64w_pop64r
#define pop16m x64w_pop16m
#define pop64m x64w_pop64m
#define mov8ri  x64w_mov8ri 
#define mov16ri x64w_mov16ri
#define mov32ri x64w_mov32ri
#define mov64ri x64w_mov64ri
#define mov8rr  x64w_mov8rr
#define mov16rr x64w_mov16rr
#define mov32rr x64w_mov32rr
#define mov64rr x64w_mov64rr
#define mov8rm  x64w_mov8rm 
#define mov16rm x64w_mov16rm
#define mov32rm x64w_mov32rm
#define mov64rm x64w_mov64rm
#define mov8mi  x64w_mov8mi 
#define mov16mi x64w_mov16mi
#define mov32mi x64w_mov32mi
#define mov64mi x64w_mov64mi
#define mov8mr  x64w_mov8mr 
#define mov16mr x64w_mov16mr
#define mov32mr x64w_mov32mr
#define mov64mr x64w_mov64mr

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
