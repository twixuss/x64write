#define TL_IMPL
#define X64W_IMPLEMENTATION
#include "common.h"
#include <tl/main.h>

s32 tl_main(Span<String> args) {
	init_common(args);

	StringBuilder function_declarations;
	StringBuilder function_definitions;
	StringBuilder function_prefix_strippers;
	
	#define hex(x) tformat("0x{}", format_hex(x))

	// r/m, r/m/i
	struct E1 {
		u8 op[7];
		u8 mod;
	};
	auto I1 = [&](char const *mnem, E1 e) {
		append_format(function_declarations, "x64w_Result x64w_{}_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s);\n", mnem);
		
		append_format(function_definitions, "x64w_Result x64w_{}_ri8   (uint8_t **c, x64w_Gpr8  r, int8_t     i) {{ return instr_ri(c, r.i,   i, 1, {}, {},    0); }}\n", mnem, hex(e.op[0]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_ri16  (uint8_t **c, x64w_Gpr16 r, int16_t    i) {{ return instr_ri(c, r.i,   i, 2, {}, {},  OSO); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_ri32  (uint8_t **c, x64w_Gpr32 r, int32_t    i) {{ return instr_ri(c, r.i,   i, 4, {}, {},    0); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r64i32(uint8_t **c, x64w_Gpr64 r, int32_t    i) {{ return instr_ri(c, r.i,   i, 4, {}, {}, REXW); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r16i8 (uint8_t **c, x64w_Gpr16 r, int8_t     i) {{ return instr_ri(c, r.i,   i, 1, {}, {},  OSO); }}\n", mnem, hex(e.op[2]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r32i8 (uint8_t **c, x64w_Gpr32 r, int8_t     i) {{ return instr_ri(c, r.i,   i, 1, {}, {},    0); }}\n", mnem, hex(e.op[2]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r64i8 (uint8_t **c, x64w_Gpr64 r, int8_t     i) {{ return instr_ri(c, r.i,   i, 1, {}, {}, REXW); }}\n", mnem, hex(e.op[2]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_rr8   (uint8_t **c, x64w_Gpr8  d, x64w_Gpr8  s) {{ return instr_rr(c, d.i, s.i, 1, {}, \      0); }}\n", mnem, hex(e.op[5]));
		append_format(function_definitions, "x64w_Result x64w_{}_rr16  (uint8_t **c, x64w_Gpr16 d, x64w_Gpr16 s) {{ return instr_rr(c, d.i, s.i, 2, {}, \    OSO); }}\n", mnem, hex(e.op[6]));
		append_format(function_definitions, "x64w_Result x64w_{}_rr32  (uint8_t **c, x64w_Gpr32 d, x64w_Gpr32 s) {{ return instr_rr(c, d.i, s.i, 4, {}, \      0); }}\n", mnem, hex(e.op[6]));
		append_format(function_definitions, "x64w_Result x64w_{}_rr64  (uint8_t **c, x64w_Gpr64 d, x64w_Gpr64 s) {{ return instr_rr(c, d.i, s.i, 8, {}, \   REXW); }}\n", mnem, hex(e.op[6]));
		append_format(function_definitions, "x64w_Result x64w_{}_rm8   (uint8_t **c, x64w_Gpr8  d, x64w_Mem   s) {{ return instr_rm(c, d.i,   s, 1, {}, \      0); }}\n", mnem, hex(e.op[5]));
		append_format(function_definitions, "x64w_Result x64w_{}_rm16  (uint8_t **c, x64w_Gpr16 d, x64w_Mem   s) {{ return instr_rm(c, d.i,   s, 2, {}, \    OSO); }}\n", mnem, hex(e.op[6]));
		append_format(function_definitions, "x64w_Result x64w_{}_rm32  (uint8_t **c, x64w_Gpr32 d, x64w_Mem   s) {{ return instr_rm(c, d.i,   s, 4, {}, \      0); }}\n", mnem, hex(e.op[6]));
		append_format(function_definitions, "x64w_Result x64w_{}_rm64  (uint8_t **c, x64w_Gpr64 d, x64w_Mem   s) {{ return instr_rm(c, d.i,   s, 8, {}, \   REXW); }}\n", mnem, hex(e.op[6]));
		append_format(function_definitions, "x64w_Result x64w_{}_mi8   (uint8_t **c, x64w_Mem   m, int8_t     i) {{ return instr_mi(c,   m,   i, 1, {}, {},    0); }}\n", mnem, hex(e.op[0]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_mi16  (uint8_t **c, x64w_Mem   m, int16_t    i) {{ return instr_mi(c,   m,   i, 2, {}, {},  OSO); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_mi32  (uint8_t **c, x64w_Mem   m, int32_t    i) {{ return instr_mi(c,   m,   i, 4, {}, {},    0); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m64i32(uint8_t **c, x64w_Mem   m, int32_t    i) {{ return instr_mi(c,   m,   i, 4, {}, {}, REXW); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m16i8 (uint8_t **c, x64w_Mem   m, int16_t    i) {{ return instr_mi(c,   m,   i, 1, {}, {},  OSO); }}\n", mnem, hex(e.op[2]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m32i8 (uint8_t **c, x64w_Mem   m, int32_t    i) {{ return instr_mi(c,   m,   i, 1, {}, {},    0); }}\n", mnem, hex(e.op[2]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m64i8 (uint8_t **c, x64w_Mem   m, int32_t    i) {{ return instr_mi(c,   m,   i, 1, {}, {}, REXW); }}\n", mnem, hex(e.op[2]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_mr8   (uint8_t **c, x64w_Mem   d, x64w_Gpr8  s) {{ return instr_rm(c, s.i,   d, 1, {}, \      0); }}\n", mnem, hex(e.op[3]));
		append_format(function_definitions, "x64w_Result x64w_{}_mr16  (uint8_t **c, x64w_Mem   d, x64w_Gpr16 s) {{ return instr_rm(c, s.i,   d, 2, {}, \    OSO); }}\n", mnem, hex(e.op[4]));
		append_format(function_definitions, "x64w_Result x64w_{}_mr32  (uint8_t **c, x64w_Mem   d, x64w_Gpr32 s) {{ return instr_rm(c, s.i,   d, 4, {}, \      0); }}\n", mnem, hex(e.op[4]));
		append_format(function_definitions, "x64w_Result x64w_{}_mr64  (uint8_t **c, x64w_Mem   d, x64w_Gpr64 s) {{ return instr_rm(c, s.i,   d, 8, {}, \   REXW); }}\n", mnem, hex(e.op[4]));
		
		append_format(function_prefix_strippers, "#define {}_ri8    x64w_{}_ri8   \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_ri16   x64w_{}_ri16  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_ri32   x64w_{}_ri32  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r64i32 x64w_{}_r64i32\n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r16i8  x64w_{}_r16i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r32i8  x64w_{}_r32i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r64i8  x64w_{}_r64i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_rr8    x64w_{}_rr8   \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_rr16   x64w_{}_rr16  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_rr32   x64w_{}_rr32  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_rr64   x64w_{}_rr64  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_rm8    x64w_{}_rm8   \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_rm16   x64w_{}_rm16  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_rm32   x64w_{}_rm32  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_rm64   x64w_{}_rm64  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_mi8    x64w_{}_mi8   \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_mi16   x64w_{}_mi16  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_mi32   x64w_{}_mi32  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m64i32 x64w_{}_m64i32\n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m16i8  x64w_{}_m16i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m32i8  x64w_{}_m32i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m64i8  x64w_{}_m64i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_mr8    x64w_{}_mr8   \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_mr16   x64w_{}_mr16  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_mr32   x64w_{}_mr32  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_mr64   x64w_{}_mr64  \n", mnem, mnem);
	};
	
	// r/m
	struct E2 {
		u8 op[2];
		u8 mod;
	};
	auto I2 = [&](char const *mnem, E2 e) {
		append_format(function_declarations, "x64w_Result x64w_{}_r8   (uint8_t **c, x64w_Gpr8  d);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r16  (uint8_t **c, x64w_Gpr16 d);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r32  (uint8_t **c, x64w_Gpr32 d);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r64  (uint8_t **c, x64w_Gpr64 d);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m8   (uint8_t **c, x64w_Mem   d);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m16  (uint8_t **c, x64w_Mem   d);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m32  (uint8_t **c, x64w_Mem   d);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m64  (uint8_t **c, x64w_Mem   d);\n", mnem);
		
		append_format(function_definitions, "x64w_Result x64w_{}_r8 (uint8_t **c, x64w_Gpr8  d) {{ return instr_r(c, d.i, 1, {}, {},    0); }}\n", mnem, hex(e.op[0]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r16(uint8_t **c, x64w_Gpr16 d) {{ return instr_r(c, d.i, 2, {}, {},  OSO); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r32(uint8_t **c, x64w_Gpr32 d) {{ return instr_r(c, d.i, 4, {}, {},    0); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r64(uint8_t **c, x64w_Gpr64 d) {{ return instr_r(c, d.i, 8, {}, {}, REXW); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m8 (uint8_t **c, x64w_Mem   d) {{ return instr_m(c,   d,    {}, {},    0); }}\n", mnem, hex(e.op[0]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m16(uint8_t **c, x64w_Mem   d) {{ return instr_m(c,   d,    {}, {},  OSO); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m32(uint8_t **c, x64w_Mem   d) {{ return instr_m(c,   d,    {}, {},    0); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m64(uint8_t **c, x64w_Mem   d) {{ return instr_m(c,   d,    {}, {}, REXW); }}\n", mnem, hex(e.op[1]), e.mod);
		
		append_format(function_prefix_strippers, "#define {}_r8  x64w_{}_r8   \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r16 x64w_{}_r16  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r32 x64w_{}_r32  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r64 x64w_{}_r64  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m8  x64w_{}_m8   \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m16 x64w_{}_m16  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m32 x64w_{}_m32  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m64 x64w_{}_m64  \n", mnem, mnem);
	};
	
	// r/m, i8/cl/1
	struct E3 {
		u8 op[6];
		u8 mod;
	};
	auto I3 = [&](char const *mnem, E3 e) {
		append_format(function_declarations, "x64w_Result x64w_{}_r8_1  (uint8_t **c, x64w_Gpr8  r           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r16_1 (uint8_t **c, x64w_Gpr16 r           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r32_1 (uint8_t **c, x64w_Gpr32 r           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r64_1 (uint8_t **c, x64w_Gpr64 r           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_ri8   (uint8_t **c, x64w_Gpr8  r, uint8_t i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r16i8 (uint8_t **c, x64w_Gpr16 r, uint8_t i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r32i8 (uint8_t **c, x64w_Gpr32 r, uint8_t i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r64i8 (uint8_t **c, x64w_Gpr64 r, uint8_t i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r8_cl (uint8_t **c, x64w_Gpr8  r           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r16_cl(uint8_t **c, x64w_Gpr16 r           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r32_cl(uint8_t **c, x64w_Gpr32 r           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_r64_cl(uint8_t **c, x64w_Gpr64 r           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m8_1  (uint8_t **c, x64w_Mem   m           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m16_1 (uint8_t **c, x64w_Mem   m           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m32_1 (uint8_t **c, x64w_Mem   m           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m64_1 (uint8_t **c, x64w_Mem   m           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_mi8   (uint8_t **c, x64w_Mem   m, uint8_t i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m16i8 (uint8_t **c, x64w_Mem   m, uint8_t i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m32i8 (uint8_t **c, x64w_Mem   m, uint8_t i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m64i8 (uint8_t **c, x64w_Mem   m, uint8_t i);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m8_cl (uint8_t **c, x64w_Mem   m           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m16_cl(uint8_t **c, x64w_Mem   m           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m32_cl(uint8_t **c, x64w_Mem   m           );\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_m64_cl(uint8_t **c, x64w_Mem   m           );\n", mnem);
		
		append_format(function_definitions, "x64w_Result x64w_{}_r8_1  (uint8_t **c, x64w_Gpr8  r           ) {{ return instr_r (c, r.i,    1, {}, {},    0); }}\n", mnem, hex(e.op[0]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r16_1 (uint8_t **c, x64w_Gpr16 r           ) {{ return instr_r (c, r.i,    2, {}, {},  OSO); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r32_1 (uint8_t **c, x64w_Gpr32 r           ) {{ return instr_r (c, r.i,    4, {}, {},    0); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r64_1 (uint8_t **c, x64w_Gpr64 r           ) {{ return instr_r (c, r.i,    8, {}, {}, REXW); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_ri8   (uint8_t **c, x64w_Gpr8  r, uint8_t i) {{ return instr_ri(c, r.i, i, 1, {}, {},    0); }}\n", mnem, hex(e.op[4]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r16i8 (uint8_t **c, x64w_Gpr16 r, uint8_t i) {{ return instr_ri(c, r.i, i, 1, {}, {},  OSO); }}\n", mnem, hex(e.op[5]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r32i8 (uint8_t **c, x64w_Gpr32 r, uint8_t i) {{ return instr_ri(c, r.i, i, 1, {}, {},    0); }}\n", mnem, hex(e.op[5]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r64i8 (uint8_t **c, x64w_Gpr64 r, uint8_t i) {{ return instr_ri(c, r.i, i, 1, {}, {}, REXW); }}\n", mnem, hex(e.op[5]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r8_cl (uint8_t **c, x64w_Gpr8  r           ) {{ return instr_r (c, r.i,    1, {}, {},    0); }}\n", mnem, hex(e.op[2]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r16_cl(uint8_t **c, x64w_Gpr16 r           ) {{ return instr_r (c, r.i,    2, {}, {},  OSO); }}\n", mnem, hex(e.op[3]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r32_cl(uint8_t **c, x64w_Gpr32 r           ) {{ return instr_r (c, r.i,    4, {}, {},    0); }}\n", mnem, hex(e.op[3]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_r64_cl(uint8_t **c, x64w_Gpr64 r           ) {{ return instr_r (c, r.i,    8, {}, {}, REXW); }}\n", mnem, hex(e.op[3]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m8_1  (uint8_t **c, x64w_Mem   m           ) {{ return instr_m (c,   m,       {}, {},    0); }}\n", mnem, hex(e.op[0]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m16_1 (uint8_t **c, x64w_Mem   m           ) {{ return instr_m (c,   m,       {}, {},  OSO); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m32_1 (uint8_t **c, x64w_Mem   m           ) {{ return instr_m (c,   m,       {}, {},    0); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m64_1 (uint8_t **c, x64w_Mem   m           ) {{ return instr_m (c,   m,       {}, {}, REXW); }}\n", mnem, hex(e.op[1]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_mi8   (uint8_t **c, x64w_Mem   m, uint8_t i) {{ return instr_mi(c,   m, i, 1, {}, {},    0); }}\n", mnem, hex(e.op[4]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m16i8 (uint8_t **c, x64w_Mem   m, uint8_t i) {{ return instr_mi(c,   m, i, 1, {}, {},  OSO); }}\n", mnem, hex(e.op[5]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m32i8 (uint8_t **c, x64w_Mem   m, uint8_t i) {{ return instr_mi(c,   m, i, 1, {}, {},    0); }}\n", mnem, hex(e.op[5]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m64i8 (uint8_t **c, x64w_Mem   m, uint8_t i) {{ return instr_mi(c,   m, i, 1, {}, {}, REXW); }}\n", mnem, hex(e.op[5]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m8_cl (uint8_t **c, x64w_Mem   m           ) {{ return instr_m (c,   m,       {}, {},    0); }}\n", mnem, hex(e.op[2]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m16_cl(uint8_t **c, x64w_Mem   m           ) {{ return instr_m (c,   m,       {}, {},  OSO); }}\n", mnem, hex(e.op[3]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m32_cl(uint8_t **c, x64w_Mem   m           ) {{ return instr_m (c,   m,       {}, {},    0); }}\n", mnem, hex(e.op[3]), e.mod);
		append_format(function_definitions, "x64w_Result x64w_{}_m64_cl(uint8_t **c, x64w_Mem   m           ) {{ return instr_m (c,   m,       {}, {}, REXW); }}\n", mnem, hex(e.op[3]), e.mod);
		
		append_format(function_prefix_strippers, "#define {}_r8_1   x64w_{}_r8_1  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r16_1  x64w_{}_r16_1 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r32_1  x64w_{}_r32_1 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r64_1  x64w_{}_r64_1 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_ri8    x64w_{}_ri8   \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r16i8  x64w_{}_r16i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r32i8  x64w_{}_r32i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r64i8  x64w_{}_r64i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r8_cl  x64w_{}_r8_cl \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r16_cl x64w_{}_r16_cl\n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r32_cl x64w_{}_r32_cl\n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_r64_cl x64w_{}_r64_cl\n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m8_1   x64w_{}_m8_1  \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m16_1  x64w_{}_m16_1 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m32_1  x64w_{}_m32_1 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m64_1  x64w_{}_m64_1 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_mi8    x64w_{}_mi8   \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m16i8  x64w_{}_m16i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m32i8  x64w_{}_m32i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m64i8  x64w_{}_m64i8 \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m8_cl  x64w_{}_m8_cl \n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m16_cl x64w_{}_m16_cl\n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m32_cl x64w_{}_m32_cl\n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_m64_cl x64w_{}_m64_cl\n", mnem, mnem);
	};
	
	// r, m
	struct E4 {
		u8 op;
	};
	auto I4 = [&](char const *mnem, E4 e) {
		append_format(function_declarations, "x64w_Result x64w_{}_rm16(uint8_t **c, x64w_Gpr16 r, x64w_Mem m);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_rm32(uint8_t **c, x64w_Gpr32 r, x64w_Mem m);\n", mnem);
		append_format(function_declarations, "x64w_Result x64w_{}_rm64(uint8_t **c, x64w_Gpr64 r, x64w_Mem m);\n", mnem);
		
		append_format(function_definitions, "x64w_Result x64w_{}_rm16(uint8_t **c, x64w_Gpr16 r, x64w_Mem m) {{ return instr_rm(c, r.i, m, 2, {},  OSO); }}\n", mnem, hex(e.op));
		append_format(function_definitions, "x64w_Result x64w_{}_rm32(uint8_t **c, x64w_Gpr32 r, x64w_Mem m) {{ return instr_rm(c, r.i, m, 4, {},    0); }}\n", mnem, hex(e.op));
		append_format(function_definitions, "x64w_Result x64w_{}_rm64(uint8_t **c, x64w_Gpr64 r, x64w_Mem m) {{ return instr_rm(c, r.i, m, 8, {}, REXW); }}\n", mnem, hex(e.op));
		
		append_format(function_prefix_strippers, "#define {}_rm16 x64w_{}_rm16\n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_rm32 x64w_{}_rm32\n", mnem, mnem);
		append_format(function_prefix_strippers, "#define {}_rm64 x64w_{}_rm64\n", mnem, mnem);
	};

	I1("adc", {.op = {0x80, 0x81, 0x83, 0x10, 0x11, 0x12, 0x13}, .mod = 2,});
	I1("add", {.op = {0x80, 0x81, 0x83, 0x00, 0x01, 0x02, 0x03}, .mod = 0,});
	I1("xor", {.op = {0x80, 0x81, 0x83, 0x30, 0x31, 0x32, 0x33}, .mod = 6,});
	I1("and", {.op = {0x80, 0x81, 0x83, 0x20, 0x21, 0x22, 0x23}, .mod = 4,});
	I1("or",  {.op = {0x80, 0x81, 0x83, 0x08, 0x09, 0x0a, 0x0b}, .mod = 1,});
	I1("sub", {.op = {0x80, 0x81, 0x83, 0x28, 0x29, 0x2a, 0x2b}, .mod = 5,});
	I2("inc", {.op = {0xfe, 0xff}, .mod = 0});
	I2("dec", {.op = {0xfe, 0xff}, .mod = 1});
	I2("not", {.op = {0xf6, 0xf7}, .mod = 2});
	I2("neg", {.op = {0xf6, 0xf7}, .mod = 3});
	I2("mul", {.op = {0xf6, 0xf7}, .mod = 4});
	I2("div", {.op = {0xf6, 0xf7}, .mod = 6});
	I3("shl", {.op = {0xd0, 0xd1, 0xd2, 0xd3, 0xc0, 0xc1}, .mod = 4});
	I3("shr", {.op = {0xd0, 0xd1, 0xd2, 0xd3, 0xc0, 0xc1}, .mod = 5});
	I3("sal", {.op = {0xd0, 0xd1, 0xd2, 0xd3, 0xc0, 0xc1}, .mod = 4});
	I3("sar", {.op = {0xd0, 0xd1, 0xd2, 0xd3, 0xc0, 0xc1}, .mod = 7});
	I4("lea", {.op = 0x8d});

	auto templatee = to_list((Span<char>)read_entire_file(tformat("{}\\x64write.template.h", root_directory)));

	struct Inserter {
		Span<char> keyword;
		StringBuilder *source;
	};

	Inserter inserters[] = {
		{"INSERT_FUNCTION_DECLARATIONS"s,     &function_declarations    },
		{"INSERT_FUNCTION_DEFINITIONS"s,      &function_definitions     },
		{"INSERT_FUNCTION_PREFIX_STRIPPERS"s, &function_prefix_strippers},
	};

	for (auto &inserter : inserters) {
		auto found = find(templatee, inserter.keyword);
		templatee.replace({found, inserter.keyword.count}, (Span<char>)to_string(*inserter.source));
	}

	write_entire_file(tformat(u8"{}\\x64write.h", root_directory), as_bytes(templatee));
	println("Finished generation");

	return 0;
}
