// asm8085.c

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define version "8085 assembler version 1.7"
#define maxOpcdLen  7		// max opcode legnth (for building opcode table)
#define INSTR_MAX   5		// length of longest valid instruction (really 4 for Z-80)
#define LITTLE_ENDIAN		// CPU is little-endian

enum instrType
{
	o_Illegal,	// Opcode not found in FindOpcode
	o_None,		// No operands

	o_Immediate,	// one-byte immediate operand
	o_LImmediate,   // two-byte immediate operand (mostly JMPs)
	o_MOV,		// MOV r,r opcode
	o_RST,		// RST instruction
	o_Arith,	// Arithmetic instructions
	o_PushPop,	// PUSH and POP instructions
	o_MVI,		// MVI instruction
	o_LXI,		// LXI instruciton
	o_INR,		// INR/DCR instructions
	o_INX,		// INX/DCX/DAD instructions
	o_STAX,		// STAX/LDAX instructions

	o_DB,		// DB pseudo-op
	o_DWLE,		// DW pseudo-op
	o_DWBE,		// big-endian DW
	o_DS,		// DS pseudo-op
	o_END,		// END pseudo-op
	o_HEX,		// HEX pseudo-op
	o_FCC,		// FCC pseudo-op
	o_Include,  // INCLUDE pseudo-op
	o_ALIGN,	// ALIGN pseudo-op

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

#include "asmguts.h"

const char regs1[]		= "B  C  D  E  H  L  M  A  ";
const char regs2[]		= "B  D  H  SP ";
const char pushregs[]   = "B  D  H  PSW";
const char staxregs[]   = "B  D  ";

struct OpcdRec opcdTab[] =
{
	{"NOP", o_None, 0x00},
	{"RLC", o_None, 0x07},
	{"RRC", o_None, 0x0F},
	{"RAL", o_None, 0x17},
	{"RAR", o_None, 0x1F},
	{"RIM", o_None, 0x20},
	{"DAA", o_None, 0x27},
	{"CMA", o_None, 0x2F},
	{"SIM", o_None, 0x30},
	{"STC", o_None, 0x37},
	{"CMC", o_None, 0x3F},
	{"HLT", o_None, 0x76},
	{"RNZ", o_None, 0xC0},
	{"RZ",  o_None, 0xC8},
	{"RET", o_None, 0xC9},
	{"RNC", o_None, 0xD0},
	{"RC",  o_None, 0xD8},
	{"RPO", o_None, 0xE0},
	{"XTHL",o_None, 0xE3},
	{"RPE", o_None, 0xE8},
	{"PCHL",o_None, 0xE9},
	{"XCHG",o_None, 0xEB},
	{"RP",  o_None, 0xF0},
	{"DI",  o_None, 0xF3},
	{"RM",  o_None, 0xF8},
	{"SPHL",o_None, 0xF9},
	{"EI",  o_None, 0xFB},

	{"ADI", o_Immediate, 0xC6},
	{"ACI", o_Immediate, 0xCE},
	{"OUT", o_Immediate, 0xD3},
	{"SUI", o_Immediate, 0xD6},
	{"IN",  o_Immediate, 0xDB},
	{"SBI", o_Immediate, 0xDE},
	{"ANI", o_Immediate, 0xE6},
	{"XRI", o_Immediate, 0xEE},
	{"ORI", o_Immediate, 0xF6},
	{"CPI", o_Immediate, 0xFE},

	{"SHLD",o_LImmediate, 0x22},
	{"LHLD",o_LImmediate, 0x2A},
	{"STA", o_LImmediate, 0x32},
	{"LDA", o_LImmediate, 0x3A},
	{"JNZ", o_LImmediate, 0xC2},
	{"JMP", o_LImmediate, 0xC3},
	{"CNZ", o_LImmediate, 0xC4},
	{"JZ",  o_LImmediate, 0xCA},
	{"CZ",  o_LImmediate, 0xCC},
	{"CALL",o_LImmediate, 0xCD},
	{"JNC", o_LImmediate, 0xD2},
	{"CNC", o_LImmediate, 0xD4},
	{"JC",  o_LImmediate, 0xDA},
	{"CC",  o_LImmediate, 0xDC},
	{"JPO", o_LImmediate, 0xE2},
	{"CPO", o_LImmediate, 0xE4},
	{"JPE", o_LImmediate, 0xEA},
	{"CPE", o_LImmediate, 0xEC},
	{"JP",  o_LImmediate, 0xF2},
	{"CP",  o_LImmediate, 0xF4},
	{"JM",  o_LImmediate, 0xFA},
	{"CM",  o_LImmediate, 0xFC},

	{"MOV", o_MOV,     0},

	{"RST",	o_RST,		0},

	{"ADD", o_Arith, 0x80},
	{"ADC", o_Arith, 0x88},
	{"SUB", o_Arith, 0x90},
	{"SBB", o_Arith, 0x98},
	{"ANA", o_Arith, 0xA0},
	{"XRA", o_Arith, 0xA8},
	{"ORA", o_Arith, 0xB0},
	{"CMP", o_Arith, 0xB8},

	{"PUSH",o_PushPop, 0xC5},
	{"POP",	o_PushPop, 0xC1},

	{"MVI", o_MVI,   0x06},
	{"LXI", o_LXI,   0x01},
	{"INR", o_INR,   0x04},
	{"DCR", o_INR,   0x05},
	{"INX", o_INX,   0x03},
	{"DAD", o_INX,   0x09},
	{"DCX", o_INX,   0x0B},
	{"STAX",o_STAX,  0x02},
	{"LDAX",o_STAX,  0x0A},

//  Undocumented 8085 instructions:
//  These are in most "real" 8085 versions,
//  but are likely to not be found in VHDL cores, etc.

	{"DSUB",o_None, 0x08},		// HL = HL - BC
	{"ARHL",o_None, 0x10},		// arithmetic shift right HL
	{"RDEL",o_None, 0x18},		// rotate DE left through carry
	{"LDHI",o_Immediate, 0x28}, // DE = HL + imm
	{"LDSI",o_Immediate, 0x38}, // DE = SP + imm
	{"RSTV",o_None, 0xCB},		// call 0x40 if overflow
	{"LHLX",o_None, 0xED},		// L = (DE), H = (DE+1)
	{"SHLX",o_None, 0xD9},		// (DE) = L, (DE+1) = H
	{"JNX5",o_LImmediate, 0xDD},
	{"JX5", o_LImmediate, 0xFD},

	{"DB",	o_DB,		0},
	{"FCB", o_DB,       0},
	{"BYTE",o_DB,       0},
	{".BYTE",o_DB,      0},
	{"DC.B",o_DB,       0},
	{".DC.B",o_DB,		0},
	{"DFB", o_DB,		0},
	{"DW",	o_DWLE,		0},
	{"FDB", o_DWLE,     0},
	{"WORD",o_DWLE,     0},
	{".WORD",o_DWLE,    0},
	{"DC.W",o_DWLE,     0},
	{".DC.W",o_DWLE,	0},
	{"DA",  o_DWLE,		0},
	{"DRW",	o_DWBE,		0},
	{"DS",	o_DS,		1},
	{"DS.B", o_DS,		1},
	{".DS.B",o_DS,		1},
	{"RMB", o_DS,       1},
	{"DS.W", o_DS,		2},
	{".DS.W",o_DS,		2},
	{"BLKW", o_DS,		2},
	{"HEX", o_HEX,      0},
	{"FCC", o_FCC,      0},
	{"ALIGN",o_ALIGN,   0},

	{"=",	o_EQU,		0},
	{"EQU",	o_EQU,		0},
	{"SET",	o_EQU,		1},
	{"DEFL",o_EQU,		1},

	{"ORG",	 o_ORG,		0},
	{".ORG", o_ORG,		0},
	{"AORG", o_ORG,		0},
	{"RORG", o_RORG,	0},
	{"REND", o_REND,	0},
	{"END",	 o_END,		0},
	{".END", o_END,		0},
	{"LIST", o_LIST,	0},
	{"OPT",	 o_OPT,		0},
	{"ERROR",o_ERROR,   0},
	{"MACRO",o_MACRO,   0},
	{".MACRO",o_MACRO,  0},
	{"ENDM", o_ENDM,	0},
	{".ENDM",o_ENDM,	0},
	{"SEG",  o_SEG,     1},
	{"RSEG", o_SEG,     1},
	{"SEG.U",o_SEG,     0},

	{"IF",   o_IF,		0},
	{"ELSE", o_ELSE,	0},
	{"ELSIF",o_ELSIF,   0},
	{"ENDIF",o_ENDIF,   0},

	{"INCLUDE",o_Include,0},

	{"",	o_Illegal,  0}
};


// --------------------------------------------------------------


int FindReg(char *regName, const char *regList)
{

	const char	*p;
	char		r0,r1,r2;
	int			reg;

	r0 = regName[0];
	r1 = ' ';
	r2 = ' ';
	if (regName[1])
	{
		r1 = regName[1];
		if (regName[2])
		{
			r2 = regName[2];
			if (r2 && regName[3])
				return -1;	// longer than 3 chars!
		}
	}


	reg = 0;
	p = regList;
	while (*p)
	{
		if (r0 == p[0] && r1 == p[1] && r2 == p[2])
			return reg;

		p = p + 3;
		reg++;
	}

	return -1;
}


int IXOffset()
{
	Str255	word;
	int		token;
	int		val;

	token = GetWord(word);

	if (token == ')')
		val = 0;
	else if (token == '+' || token == '-')
	{
		val = Eval();
		if (token == '-')
			val = -val;
         RParen();
	}

	return val;
}


void DoOpcode(int typ, int parm)
{
	int		val;
	int		reg1;
	int		reg2;
	Str255	word;
	char	*oldLine;
//	int		token;

	switch(typ)
	{

		case o_None:
			if (parm > 255)	Instr2(parm >> 8, parm & 255);
			else			Instr1(parm);
			break;

		case o_Immediate:
			val = Eval();
			Instr2(parm,val);
			break;

		case o_LImmediate:
			val = Eval();
			Instr3W(parm,val);
			break;

		case o_MOV:
            GetWord(word);
            reg1 = FindReg(word,regs1);
			if (reg1 == -1)
				IllegalOperand();
			else
			{
				oldLine = linePtr;
				if (GetWord(word) != ',')
				{
					linePtr = oldLine;
					Comma();
				}
				else
				{
					GetWord(word);
					reg2 = FindReg(word,regs1);
					if (reg2 == -1 || (reg1 == 6 && reg2 == 6))
						IllegalOperand();
					else
						Instr1(0x40 + (reg1 << 3) + reg2);
				}
			}
			break;

		case o_RST:
			val = Eval();
			if (0 <= val && val <= 7)
				Instr1(0xC7 + val*8);
            else IllegalOperand();
        	break;

		case o_Arith:
            GetWord(word);
            reg1 = FindReg(word,regs1);
			if (reg1 == -1)
				IllegalOperand();
			else
				Instr1(parm + reg1);
			break;

		case o_PushPop:
			GetWord(word);
			reg1 = FindReg(word,pushregs);
			if (reg1 == -1)
				IllegalOperand();
			else
				Instr1(parm + (reg1 << 4));
			break;

		case o_MVI:
			GetWord(word);
			reg1 = FindReg(word,regs1);
			if (reg1 == -1)
				IllegalOperand();
			else
			{
				oldLine = linePtr;
				if (GetWord(word) != ',')
				{
					linePtr = oldLine;
					Comma();
				}
				else
				{
					val = Eval();
					Instr2(parm + (reg1 << 3), val);
				}
			}
			break;

		case o_LXI:
			GetWord(word);
			reg1 = FindReg(word,regs2);
			if (reg1 == -1)
				IllegalOperand();
			else
			{
				oldLine = linePtr;
				if (GetWord(word) != ',')
				{
					linePtr = oldLine;
					Comma();
				}
				else
				{
					val = Eval();
					Instr3W(parm + (reg1 << 4), val);
				}
			}
			break;

		case o_INR:
			GetWord(word);
			reg1 = FindReg(word,regs1);
			if (reg1 == -1)
				IllegalOperand();
			else
				Instr1(parm + (reg1 << 3));
			break;

		case o_INX:
			GetWord(word);
			reg1 = FindReg(word,regs2);
			if (reg1 == -1)
				IllegalOperand();
			else
				Instr1(parm + (reg1 << 4));
			break;

		case o_STAX:
			GetWord(word);
			reg1 = FindReg(word,staxregs);
			if (reg1 == -1)
				IllegalOperand();
			else
				Instr1(parm + (reg1 << 4));
			break;

		default:
			DoStdOpcode(typ, parm);
			break;
	}
}


void DoLabelOp(int typ, int parm, char *labl)
{
//	int		i,val;
//	Str255  word;

	switch(typ)
	{
		default:
			DoStdLabelOp(typ, parm, labl);
			break;
	}
}


void usage(void)
{
	fprintf(stderr,version);
	stdusage();
	exit(1);
}


void PassInit(void)
{
}


void AsmInit(void)
{
}
