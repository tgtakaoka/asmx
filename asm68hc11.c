// asm68HC11.c - copyright 1998-2004 Bruce Tomlin

//#define DEBUG_PASS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define version "68HC11 assembler version 1.7.1"
#define maxOpcdLen  9		// max opcode legnth (for building opcode table)
#define INSTR_MAX   5		// length of longest valid instruction
#define BIG_ENDIAN			// CPU is big-endian

enum instrType
{
	o_Illegal,	// opcode not found in FindOpcode
	o_Inherent,	// implied instructions
	o_Inherent_01,	// implied instructions, 6801/6803/6811
	o_Inherent_03,	// implied instructions, 6803 only
	o_Inherent_11,	// implied instructions, 6811 only
	o_Relative,	// branch instructions
	o_Bit_03,   // 6303 AIM OIM EIM TIM instructions
	o_Bit,		// 6811 BSET/BCLR
	o_BRelative, // 6811 BRSET/BRCLR
	o_Logical,  // instructions with multiple addressing modes
	o_Arith,	// arithmetic instructions with multiple addressing modes
	o_LArith,	// o_Arith instructions with 16-bit immediate modes
	o_LArith_01,	// o_Arith instructions with 16-bit immediate modes, 6801/6803/6811
	o_LArith_11,	// o_Arith instructions with 16-bit immediate modes, 6811 only

	o_DB,		// DB pseudo-op
	o_DWLE,		// DW pseudo-op (little endian)
	o_DWBE,		// DW pseudo-op (big endian)
	o_DS,		// DS pseudo-op
	o_HEX,		// HEX pseudo-op
	o_FCC,		// FCC pseudo-op
	o_ALIGN,	// ALIGN pseudo-op

	o_END,		// END pseudo-op
	o_Include,  // INCLUDE pseudo-op
	o_Processor,// PROCESSOR pseudo-op
	o_CPUtype,  // cpu type pseudo-ops (.6502, etc.)

	o_ENDM,		// ENDM pseudo-op
	o_MacName,	// Macro name

	o_LabelOp,	// the following pseudo-ops handle the label field specially
	o_EQU,		// EQU and SET pseudo-ops
	o_ORG,		// ORG pseudo-op
	o_RORG,		// RORG pseudo-op
	o_REND,		// REND pseudo-op
	o_LIST,		// LIST pseudo-op
	o_OPT,		// OPT pseudo-op
	o_ERROR,	// ERROR pseudo-op
	o_MACRO,	// MACRO pseudo-op
	o_SEG,		// SEG pseudo-op

	o_IF,		// IF <expr> pseudo-op
	o_ELSE,		// ELSE pseudo-op
	o_ELSIF,	// ELSIF <expr> pseudo-op
	o_ENDIF		// ENDIF pseudo-op
};


enum cputype
{
	cpu_6800, cpu_6801, cpu_68HC11, cpu_6303
} cpu;


#include "asmguts.h"


struct OpcdRec opcdTab[] =
{
	{"TEST",  o_Inherent_11, 0x00}, // 68HC11
	{"NOP",   o_Inherent,    0x01},
	{"IDIV",  o_Inherent_11, 0x02}, // 68HC11
	{"FDIV",  o_Inherent_11, 0x03}, // 68HC11
	{"LSRD",  o_Inherent_01, 0x04}, // 68HC11 6801 6303
	{"ASLD",  o_Inherent_01, 0x05}, // 68HC11 6801 6303
	{"TAP",   o_Inherent, 0x06},
	{"TPA",   o_Inherent, 0x07},
	{"INX",   o_Inherent, 0x08},
	{"DEX",   o_Inherent, 0x09},
	{"CLV",   o_Inherent, 0x0A},
	{"SEV",   o_Inherent, 0x0B},
	{"CLC",   o_Inherent, 0x0C},
	{"SEC",   o_Inherent, 0x0D},
	{"CLI",   o_Inherent, 0x0E},
	{"SEI",   o_Inherent, 0x0F},
	{"SBA",   o_Inherent, 0x10},
	{"CBA",   o_Inherent, 0x11},
	{"TAB",   o_Inherent, 0x16},
	{"TBA",   o_Inherent, 0x17},
	{"DAA",   o_Inherent, 0x19},
	{"ABA",   o_Inherent, 0x1B},
	{"TSX",   o_Inherent, 0x30},
	{"INS",   o_Inherent, 0x31},
	{"PULA",  o_Inherent, 0x32},
	{"PULB",  o_Inherent, 0x33},
	{"DES",   o_Inherent, 0x34},
	{"TXS",   o_Inherent, 0x35},
	{"PSHA",  o_Inherent, 0x36},
	{"PSHB",  o_Inherent, 0x37},
	{"PULX",  o_Inherent_01, 0x38}, // 68HC11 6801 6303
	{"RTS",   o_Inherent,    0x39},
	{"ABX",   o_Inherent_01, 0x3A}, // 68HC11 6801 6303
	{"RTI",   o_Inherent,    0x3B},
	{"PSHX",  o_Inherent_01, 0x3C}, // 68HC11 6801 6303
	{"MUL",   o_Inherent_01, 0x3D}, // 68HC11 6801 6303
	{"WAI",   o_Inherent, 0x3E},
	{"SWI",   o_Inherent, 0x3F},
	{"NEGA",  o_Inherent, 0x40},
	{"COMA",  o_Inherent, 0x43},
	{"LSRA",  o_Inherent, 0x44},
	{"RORA",  o_Inherent, 0x46},
	{"ASRA",  o_Inherent, 0x47},
	{"ASLA",  o_Inherent, 0x48},
	{"ROLA",  o_Inherent, 0x49},
	{"DECA",  o_Inherent, 0x4A},
	{"INCA",  o_Inherent, 0x4C},
	{"TSTA",  o_Inherent, 0x4D},
	{"CLRA",  o_Inherent, 0x4F},
	{"NEGB",  o_Inherent, 0x50},
	{"COMB",  o_Inherent, 0x53},
	{"LSRB",  o_Inherent, 0x54},
	{"RORB",  o_Inherent, 0x56},
	{"ASRB",  o_Inherent, 0x57},
	{"ASLB",  o_Inherent, 0x58},
	{"ROLB",  o_Inherent, 0x59},
	{"DECB",  o_Inherent, 0x5A},
	{"INCB",  o_Inherent, 0x5C},
	{"TSTB",  o_Inherent, 0x5D},
	{"CLRB",  o_Inherent, 0x5F},
	{"XGDX",  o_Inherent_11, 0x8F}, // 0x8F for 68HC11, 0x18 for 6303
	{"STOP",  o_Inherent_11, 0xCF}, // 68HC11
	{"SLP",   o_Inherent_03, 0x1A}, // 6303 only
	{"INY",   o_Inherent_11, 0x1808}, // 68HC11
	{"DEY",   o_Inherent_11, 0x1809}, // 68HC11
	{"TSY",   o_Inherent_11, 0x1830}, // 68HC11
	{"TYS",   o_Inherent_11, 0x1835}, // 68HC11
	{"PULY",  o_Inherent_11, 0x1838}, // 68HC11
	{"ABY",   o_Inherent_11, 0x183A}, // 68HC11
	{"PSHY",  o_Inherent_11, 0x183C}, // 68HC11
	{"XGDY",  o_Inherent_11, 0x188F}, // 68HC11

	{"BRA",   o_Relative, 0x20},
	{"BRN",   o_Relative, 0x21}, // 68HC11 6801 6303 (but probably works on 6800 anyhow)
	{"BHI",   o_Relative, 0x22},
	{"BLS",   o_Relative, 0x23},
	{"BCC",   o_Relative, 0x24},
	{"BHS",   o_Relative, 0x24},
	{"BCS",   o_Relative, 0x25},
	{"BLO",   o_Relative, 0x25},
	{"BNE",   o_Relative, 0x26},
	{"BEQ",   o_Relative, 0x27},
	{"BVC",   o_Relative, 0x28},
	{"BVS",   o_Relative, 0x29},
	{"BPL",   o_Relative, 0x2A},
	{"BMI",   o_Relative, 0x2B},
	{"BGE",   o_Relative, 0x2C},
	{"BLT",   o_Relative, 0x2D},
	{"BGT",   o_Relative, 0x2E},
	{"BLE",   o_Relative, 0x2F},
	{"BSR",   o_Relative, 0x8D},

	{"NEG",   o_Logical,  0x00}, // o_Logical: indexed,X = $60; extended = $70; indexed,Y = $1860
	{"COM",   o_Logical,  0x03},
	{"LSR",   o_Logical,  0x04},
	{"ROR",   o_Logical,  0x06},
	{"ASR",   o_Logical,  0x07},
	{"ASL",   o_Logical,  0x08},
	{"LSL",   o_Logical,  0x08},
	{"ROL",   o_Logical,  0x09},
	{"DEC",   o_Logical,  0x0A},
	{"INC",   o_Logical,  0x0C},
	{"TST",   o_Logical,  0x0D},
	{"JMP",   o_Logical,  0x0E},
	{"CLR",   o_Logical,  0x0F},

	{"SUBA",  o_Arith,    0x80}, // o_Arith: immediate = $00, direct = $10, indexed,X = $20, indexed,Y = $1820
	{"CMPA",  o_Arith,    0x81}, //           extended = $30
	{"SBCA",  o_Arith,    0x82},
	{"SUBD",  o_LArith_01,0x83}, // 68HC11 6801 6303
	{"ANDA",  o_Arith,    0x84},
	{"BITA",  o_Arith,    0x85},
	{"LDAA",  o_Arith,    0x86},
	{"STAA",  o_Arith,    0x97},
	{"EORA",  o_Arith,    0x88},
	{"ADCA",  o_Arith,    0x89},
	{"ORAA",  o_Arith,    0x8A},
	{"ADDA",  o_Arith,    0x8B},
	{"CPX",   o_LArith,   0x8C},
	{"JSR",   o_Arith,    0x9D}, // 9D is 68HC11 6801 6303 only
	{"LDS",   o_LArith,   0x8E},
	{"STS",   o_Arith,    0x9F},
	{"SUBB",  o_Arith,    0xC0},
	{"CMPB",  o_Arith,    0xC1},
	{"SBCB",  o_Arith,    0xC2},
	{"ADDD",  o_LArith_01,0xC3}, // 68HC11 6801 6303
	{"ANDB",  o_Arith,    0xC4},
	{"BITB",  o_Arith,    0xC5},
	{"LDAB",  o_Arith,    0xC6},
	{"STAB",  o_Arith,    0xD7},
	{"EORB",  o_Arith,    0xC8},
	{"ADCB",  o_Arith,    0xC9},
	{"ORAB",  o_Arith,    0xCA},
	{"ADDB",  o_Arith,    0xCB},
	{"LDD",   o_LArith_01,0xCC}, // 68HC11 6801 6303
	{"STD",   o_LArith_01,0xDD}, // 68HC11 6801 6803
	{"LDX",   o_LArith,   0xCE},
	{"STX",   o_Arith,    0xDF},
	{"CPD",   o_LArith_11,0x1A83}, // 68HC11
	{"CPY",   o_LArith_11,0x188C}, // 68HC11
	{"LDY",   o_LArith_11,0x18CE}, // 68HC11
	{"STY",   o_LArith_11,0x18DF}, // 68HC11

	{"BSET",  o_Bit,      0},
	{"BCLR",  o_Bit,      1},
	{"BRSET", o_BRelative,0},
	{"BRCLR", o_BRelative,1},

	{"AIM", o_Bit_03, 0x61},
	{"OIM", o_Bit_03, 0x62},
	{"EIM", o_Bit_03, 0x65},
	{"TIM", o_Bit_03, 0x6B},

	{"DB",   o_DB,  0},
	{"FCB",  o_DB,  0},
	{"BYTE", o_DB,  0},
	{".BYTE",o_DB,  0},
	{"DC.B", o_DB,  0},
	{".DC.B",o_DB,  0},
	{"DFB",  o_DB,  0},
	{"DW",   o_DWBE,0},
	{"FDB",  o_DWBE,0},
	{"WORD", o_DWBE,0},
	{".WORD",o_DWBE,0},
	{"DC.W", o_DWBE,0},
	{".DC.W",o_DWBE,0},
	{"DA",   o_DWBE,0},
	{"DRW",  o_DWLE,0},
	{"DS",   o_DS,  1},
	{"DS.B", o_DS,  1},
	{".DS.B",o_DS,  1},
	{"RMB",  o_DS,  1},
	{"BLKB", o_DS,  1},
	{"DS.W", o_DS,  2},
	{".DS.W",o_DS,  2},
	{"BLKW", o_DS,  2},
	{"HEX",  o_HEX, 0},
	{"FCC",  o_FCC, 0},
	{"ALIGN",o_ALIGN,0},

	{"=",    o_EQU, 0},
	{"EQU",  o_EQU, 0},
	{"SET",  o_EQU, 1},

	{"ORG",  o_ORG,  0},
	{".ORG", o_ORG,  0},
	{"AORG", o_ORG,  0},
	{"RORG", o_RORG, 0},
	{"REND", o_REND, 0},
	{"END",  o_END,  0},
	{".END", o_END,  0},
	{"LIST", o_LIST, 0},
	{"OPT",  o_OPT,  0},
	{"ERROR",o_ERROR,0},
	{"MACRO",o_MACRO,0},
	{".MACRO",o_MACRO,0},
	{"ENDM", o_ENDM, 0},
	{".ENDM",o_ENDM, 0},
	{"SEG",  o_SEG,  1},
	{"RSEG", o_SEG,  1},
	{"SEG.U",o_SEG,  0},

	{"IF",   o_IF,   0},
	{"ELSE", o_ELSE, 0},
	{"ELSIF",o_ELSIF,0},
	{"ENDIF",o_ENDIF,0},

	{"INCLUDE",o_Include,0},

	{"PROCESSOR",o_Processor,0},
	{"CPU",      o_Processor,0},
	{".6800",    o_CPUtype,cpu_6800},
	{".6801",    o_CPUtype,cpu_6801},
	{".6802",    o_CPUtype,cpu_6800},
	{".6803",    o_CPUtype,cpu_6801},
	{".6808",    o_CPUtype,cpu_6801},
	{".6303",    o_CPUtype,cpu_6303},
	{".6811",    o_CPUtype,cpu_68HC11},
	{".68HC11",  o_CPUtype,cpu_68HC11},
	{".68HC711", o_CPUtype,cpu_68HC11},
	{".68HC811", o_CPUtype,cpu_68HC11},
	{".68HC99",  o_CPUtype,cpu_68HC11},

	{"",     o_Illegal, 0}
};


// --------------------------------------------------------------


int FindReg(char *regName, const char *regList)
{

	const char	*p;
	char		r0,r1;
	int			reg;

	r0 = regName[0];
	r1 = regName[1];
	if (r1 == 0) r1 = ' ';
	else if (regName[2]) return -1;	// longer than 2 chars!

	reg = 0;
	p = regList;
	while (*p)
	{
		if (r0 == p[0] && r1 == p[1])
			return reg;

		p = p + 2;
		reg++;
	}

	return -1;
}


void XInstr1(int op)
{
	if (op < 256)   Instr1(op);
			else	Instr2(op >> 8, op & 255);
}


void XInstr2(int op, unsigned char b)
{
	if (op < 256)   Instr2(op, b);
			else	Instr3(op >> 8, op & 255, b);
}


void XInstr3(int op, unsigned char b1, unsigned char b2)
{
	if (op < 256)   Instr3(op, b1, b2);
			else	Instr4(op >> 8, op & 255, b1, b2);
}


void XInstr3W(int op, int w)
{
	if (op < 256)   Instr3BW(op, w);
			else	Instr4BW(op >> 8, op & 255, w);
}


void XInstr4(int op, unsigned char b1, unsigned char b2, unsigned char b3)
{
	if (op < 256)   Instr4(op, b1, b2, b3);
			else	Instr5(op >> 8, op & 255, b1, b2, b3);
}


void XInstr4W(int op, unsigned char b, int w)
{
	if (op < 256)   Instr4BW(op, b, w);
			else	Instr5BW(op >> 8, op & 255, b, w);
}


void BadMode()
{
	Error("Illegal addressing mode");
	EatIt();
}


void DoOpcode(int typ, int parm)
{
	int		val,val2,val3;
	Str255	word;
	char	*oldLine;
	int		token;
	char	force;
	char	reg;

	switch(typ)
	{
		case o_Inherent_01:	// implied instructions, 6801/6803/6811
			if (typ == o_Inherent_01 && cpu == cpu_6800)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}
		case o_Inherent_03:	// implied instructions, 6803 only
			if (typ == o_Inherent_03 && cpu != cpu_6303)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}
		case o_Inherent_11:	// implied instructions, 6811 only
			if (parm == 0x8F && cpu == cpu_6303)	// 6303 XGDX
				parm = 0x18;
			else if (typ == o_Inherent_11 && cpu != cpu_68HC11)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}
		case o_Inherent:
			if ((parm & 0xF0FF) == 0xA0E0)	XInstr3W(0x3504,parm);
									else	XInstr1(parm);
			break;

		case o_Relative:
			if (parm == 0x21 && cpu == cpu_6800)	// BRN
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}
			val = EvalBranch(2);
			XInstr2(parm,val);
			break;

		case o_Logical:
			val = Eval();

			oldLine = linePtr;
			token = GetWord(word);
			if (token == 0)
				XInstr3W(parm + 0x70, val);
			else if (token == ',')
			{
				GetWord(word);
				if (word[1] == 0) switch(word[0])
				{
					case 'X':
						XInstr2(parm + 0x60, val);
						break;

					case 'Y':
						if (cpu == cpu_68HC11)
						{
							XInstr3(0x18, parm + 0x60, val);
							break;
						}

					default:
						BadMode();
						break;
				}
			}
			else
			{
				linePtr = oldLine;
				Comma();
			}
			break;

		case o_LArith_01:
			if (typ == o_LArith_01 && cpu == cpu_6800)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}
		case o_LArith_11:
			if (typ == o_LArith_11 && cpu != cpu_68HC11)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}
		case o_Arith:
		case o_LArith:
			oldLine = linePtr;
			token = GetWord(word);
			if (token == '#')
			{
				if (parm & 0x10)	// immediate
					Error("Invalid addressing mode");
				else
				{
					val = Eval();
					if (typ == o_Arith) XInstr2 (parm & ~0x10, val);
								else	XInstr3W(parm & ~0x10, val);
				}
			}
			else
			{
				force = 0;
				parm = parm & ~0x10;

				if (token == '<' || token == '>')
					force = token;
				else
					linePtr = oldLine;


				val = Eval();

				oldLine = linePtr;
				token = GetWord(word);
				if (token == 0)
				{
					if (((force != '>' && evalKnown && (val & 0xFF00) >> 8 == 0) || force == '<')
								&& (cpu != cpu_6800 || parm != 0x8D))
						XInstr2(parm + 0x10, val);	// <$xx
					else
						XInstr3W(parm + 0x30, val);   // >$xxxx
				}
				else if (token == ',')
				{
					GetWord(word);
					if (word[1] == 0) switch(word[0])
					{
						case 'X':
							if (parm >> 8 == 0x18)
								XInstr3(0x1A, parm + 0x20, val);
							else
								XInstr2(parm + 0x20, val);
							break;

						case 'Y':
							if (cpu == cpu_68HC11)
							{
								if (parm == 0x8C || parm == 0xCE || parm == 0xCF || (parm >> 8 == 0x1A))
									XInstr3(0xCD, parm + 0x20, val);
								else
									XInstr3(0x18, parm + 0x20, val);
								break;
							}

						default:
							BadMode();
							break;
					}
				}
				else
				{
					linePtr = oldLine;
					Comma();
				}
			}
			break;

		case o_Bit_03:
			if (cpu != cpu_6303)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}
			// need to implement AIM/OIM/EIM/TIM
			// opcode #,ofs,X (3 bytes)
			// opcode #,dir  (3 bytes)

			if (GetWord(word) != '#')
				BadMode();
			else
			{
				val = Eval();   // get immediate value
				Comma();
				val2 = Eval();  // get offset/address
				if (GetWord(word) == ',')
				{
					GetWord(word);
					if (word[0] != 'X' || word[1] != 0)
						BadMode();
					else
						XInstr3(parm, val, val2);
				}
				else // direct mode = +0x10
					XInstr3(parm + 0x10, val, val2);
			}
			break;

		case o_Bit:
			if (cpu != cpu_68HC11)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}

			val = Eval();   // direct page address or offset
			reg = 0;

			oldLine = linePtr;  // if comma present, may signal ,X or ,Y
			if (GetWord(word) == ',')
			{
				oldLine = linePtr;
				GetWord(word);
				if (word[1] == 0 && (word[0] == 'X' || word[0] == 'Y'))
				{
					reg = word[0];

					oldLine = linePtr;  // eat optional comma after ,X or ,Y
					if (GetWord(word) != ',')
						linePtr = oldLine;
				}
				else linePtr = oldLine; // not ,X or ,Y so reposition to after comma
			}
			else linePtr = oldLine; // optional comma not present, bit comes next



			val2 = Eval();  // mask

			switch(reg)
			{
				case 'X':
					XInstr3(parm + 0x1C, val, val2);
					break;

				case 'Y':
					XInstr3(parm + 0x181C, val, val2);
					break;

				default:
					XInstr3(parm + 0x14, val, val2);
					break;

			}
			break;

		case o_BRelative:
			if (cpu != cpu_68HC11)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}

			val = Eval();   // direct page address or offset
			reg = 0;

			oldLine = linePtr;  // if comma present, may signal ,X or ,Y
			if (GetWord(word) == ',')
			{
				oldLine = linePtr;;
				GetWord(word);
				if (word[1] == 0 && (word[0] == 'X' || word[0] == 'Y'))
				{
					reg = word[0];

					oldLine = linePtr;  // eat optional comma after ,X or ,Y
					if (GetWord(word) != ',')
						linePtr = oldLine;
				}
				else linePtr = oldLine; // not ,X or ,Y so reposition to after comma
			}
			else linePtr = oldLine; // optional comma not present, bit comes next

			val2 = Eval();  // bit mask

			oldLine = linePtr;  // eat optional comma after bit mask
			if (GetWord(word) != ',')
				linePtr = oldLine;

			val3 = EvalBranch(4 + (reg == 'Y'));  // offset

			switch(reg)
			{
				case 'X':
					XInstr4(parm + 0x1E, val, val2, val3);
					break;

				case 'Y':
					XInstr4(parm + 0x181E, val, val2, val3);
					break;

				default:
					XInstr4(parm + 0x12, val, val2, val3);
					break;
			}
			break;

		case o_Processor:
			if (GetWord(word))
			{
					 if (strcmp(word,"6800") == 0) cpu = cpu_6800;
				else if (strcmp(word,"6801") == 0) cpu = cpu_6801;
				else if (strcmp(word,"6802") == 0) cpu = cpu_6800;
				else if (strcmp(word,"6803") == 0) cpu = cpu_6801;
				else if (strcmp(word,"6808") == 0) cpu = cpu_6801;

				else if (strcmp(word,"6303") == 0) cpu = cpu_6303;

				else if (strcmp(word,"6811"   ) == 0) cpu = cpu_68HC11;
				else if (strcmp(word,"68HC11" ) == 0) cpu = cpu_68HC11;
				else if (strcmp(word,"68HC711") == 0) cpu = cpu_68HC11;
				else if (strcmp(word,"68HC811") == 0) cpu = cpu_68HC11;
				else if (strcmp(word,"68HC99" ) == 0) cpu = cpu_68HC11;
				else								Error("Illegal option");				
			}
			else Error("Missing operand");
			break;

		case o_CPUtype:
			cpu = parm;
			break;

		default:
			DoStdOpcode(typ, parm);
			break;
	}
}


void DoLabelOp(int typ, int parm, char *labl)
{
	DoStdLabelOp(typ, parm, labl);
}


void usage(void)
{
	fprintf(stderr,version);
	stdusage();
	exit(1);
}


void PassInit(void)
{
	cpu = cpu_68HC11;
}


void AsmInit(void)
{
}
