// asmf8.c

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define version "Fairchild F8 assembler version 1.7.1"
#define maxOpcdLen  7		// max opcode legnth (for building opcode table)
#define INSTR_MAX   3		// length of longest valid instruction
#define BIG_ENDIAN		// CPU is big-endian

enum instrType
{
	o_Illegal,	// Opcode not found in FindOpcode
	o_None,		// No operands
	o_Immediate,	// 8-bit immediate operand
	o_Register,	// Embedded register instruction
	o_Relative,	// Relative branch
	o_RegRel,	// Relative branch with embedded register
	o_Absolute,	// 16-bit absolute address
	o_SLSR,		// SL and SR instructions
	o_LR,		// LR instruction

	o_DB,		// DB pseudo-op
	o_DWBE,		// DW pseudo-op
	o_DWLE,		// little-endian DW
	o_DS,		// DS pseudo-op
	o_HEX,		// HEX pseudo-op
	o_FCC,		// FCC pseudo-op
	o_ALIGN,	// ALIGN pseudo-op

	o_END,		// END pseudo-op
	o_Include,  // INCLUDE pseudo-op

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

const char regs[] = "0  1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 "
					"A  W  J  H  K  Q  KU KL QU QL IS DC0PC0PC1"
					;

enum regType	// these are keyed to regs[] above
{
	reg_None = -1,	// -1
	reg_0,			// 0..15 = scratchpad registers
	reg_15 = 15,
	reg_A,			// 16 - accumulator
	reg_W,			// 17 - status register
	reg_J,			// 18 - 09
	reg_H,			// 19  - 0A 0B
	reg_K,			// 20  - 0C 0D
	reg_Q,			// 21  - 0E 0F
	reg_KU,			// 22  - 0C
	reg_KL,			// 23  - 0D
	reg_QU,			// 24  - 0E
	reg_QL,			// 25  - 0F
	reg_IS,			// 26 - indirect scratchpad address register
	reg_DC0,		// 27
	reg_PC0,		// 28 - program counter
	reg_PC1			// 29 - "stack pointer"
};

#define IS_REG_BYTE(x) (reg_B  <= (x) && (x) <= reg_A)
#define IS_REG_WORD(x) (reg_BC <= (x) && (x) <= reg_SP)

struct OpcdRec opcdTab[] =
{
	{"PK",  o_None,     0x0C},
	{"LM",  o_None,		0x16},
	{"ST",  o_None,		0x17},
	{"COM", o_None,		0x18},
	{"LNK", o_None,		0x19},
	{"DI",  o_None,		0x1A},
	{"EI",  o_None,		0x1B},
	{"POP", o_None,		0x1C},
	{"INC", o_None,		0x1F},
	{"NOP", o_None,		0x2B},
	{"XDC", o_None,		0x2C},
	{"CLR", o_None,		0x70},
	{"AM",  o_None,		0x88},
	{"AMD", o_None,		0x89},
	{"NM",  o_None,		0x8A},
	{"OM",  o_None,		0x8B},
	{"XM",  o_None,		0x8C},
	{"CM",  o_None,		0x8D},
	{"ADC", o_None,		0x8E},

	{"LR",  o_LR,		0x00},	// LR handles lots of instructions

	{"SR",  o_SLSR,		0x12},
	{"SL",  o_SLSR,		0x13},

	{"LI",  o_Immediate,0x20},
	{"NI",  o_Immediate,0x21},
	{"OI",  o_Immediate,0x22},
	{"XI",  o_Immediate,0x23},
	{"AI",  o_Immediate,0x24},
	{"CI",  o_Immediate,0x25},
	{"IN",  o_Immediate,0x26},
	{"OUT", o_Immediate,0x27},

	{"PI",  o_Absolute,	0x28},
	{"JMP", o_Absolute,	0x29},
	{"DCI", o_Absolute,	0x2A},

	{"DS",  o_Register,	0x30},
	{"LISU",o_Register,	0x60},	// reg must be 0..7
	{"LISL",o_Register,	0x68},	// reg must be 0..7
	{"LIS", o_Register,	0x70},
	{"INS", o_Register,	0xA0},
	{"OUTS",o_Register,	0xB0},
	{"AS",  o_Register,	0xC0},
	{"ASD", o_Register,	0xD0},
	{"XS",  o_Register,	0xE0},
	{"NS",  o_Register,	0xF0},

	{"BP",  o_Relative,	0x81},
	{"BC",  o_Relative,	0x82},
	{"BZ",  o_Relative,	0x84},
	{"BR7", o_Relative,	0x8F},
	{"BR",  o_Relative,	0x90},
	{"BM",  o_Relative,	0x91},
	{"BNC", o_Relative,	0x92},
	{"BNZ", o_Relative,	0x94},
	{"BNO", o_Relative,	0x98},

	{"BT",  o_RegRel,	0x80},	// reg must be 0..7
	{"BF",  o_RegRel,	0x90},

	{"DB",	o_DB,		0},
	{"FCB", o_DB,       0},
	{"BYTE",o_DB,       0},
	{".BYTE",o_DB,      0},
	{"DC.B",o_DB,       0},
	{".DC.B",o_DB,		0},
	{"DFB", o_DB,		0},
	{"DW",	o_DWBE,		0},
	{"FDB", o_DWBE,     0},
	{"WORD",o_DWBE,     0},
	{".WORD",o_DWBE,    0},
	{"DC.W",o_DWBE,     0},
	{".DC.W",o_DWBE,	0},
	{"DA",  o_DWBE,		0},
	{"DRW",	o_DWLE,		0},
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
	r1 = regName[1];
	r2 = regName[2];
	if (r1 == 0) { r1 = ' '; r2 = ' '; }
	else if (r2 == 0) { r2 = ' '; }
	else if (regName[3]) return reg_None;	// longer than 3 chars!

	reg = 0;
	p = regList;
	while (*p)
	{
		if (r0 == p[0] && r1 == p[1] && r2 == p[2])
			return reg;

		p = p + 3;
		reg++;
	}

	return reg_None;
}


void DoOpcode(int typ, int parm)
{
	int		val;
	int		reg1;
	int		reg2;
	Str255	word;
//	char	*oldLine;
//	int		token;

	switch(typ)
	{

		case o_None:
			Instr1(parm);
			break;

		case o_Immediate:
			val = EvalByte();
			Instr2(parm,val);
			break;

		case o_Register:
			reg1 = Eval();
			if ((parm == 0x60 || parm == 0x68) && val > 7)
				IllegalOperand();
			else
				Instr1(parm+reg1);
			break;

		case o_Relative:
			val = EvalBranch(2);
			Instr2(parm,val);
			break;

		case o_RegRel:
			reg1 = Eval();
			Comma();
			val = EvalBranch(2);
			if (parm == 0x80 && reg1 > 7)
				IllegalOperand();
			else
				Instr2(parm+reg1,val);
			break;

		case o_Absolute:
			val = Eval();
			Instr3BW(parm,val);
			break;

		case o_SLSR:
			val = Eval();
			if (val == 1)
				Instr1(parm);
			else if (val == 4)
				Instr1(parm+2);
			else
				IllegalOperand();
			break;

			// NOTE: this does not use Eval() for LR A,n and LR n,A - maybe it should?
		case o_LR:
			val = -1;
            GetWord(word);
            reg1 = FindReg(word,regs);
			Comma();
			GetWord(word);
			reg2 = FindReg(word,regs);

			switch(reg1)
			{
				case reg_A:
					if (reg2 == reg_KU)		val = 0x00; // LR A,KU
					if (reg2 == reg_KL)		val = 0x01; // LR A,KL
					if (reg2 == reg_QU)		val = 0x02; // LR A,QU
					if (reg2 == reg_QL)		val = 0x03; // LR A,QL
					if (reg2 == reg_IS)		val = 0x0A; // LR A,IS
					if (reg_0 <= reg2 && reg2 <= reg_15)
											val = 0x40 + reg2; // LR A,n
					break;

				case reg_K:
					if (reg2 == reg_PC1)	val = 0x08; // LR K,PC1
					break;

				case reg_W:
					if (reg2 == reg_J)		val = 0x1D; // LR W,J
					break;

				case reg_J:
					if (reg2 == reg_W)		val = 0x1E; // LR J,W
					break;

				case reg_Q:
					if (reg2 == reg_DC0)	val = 0x0E; // LR Q,DC0
					break;

				case reg_H:
					if (reg2 == reg_DC0)	val = 0x11; // LR H,DC0
					break;

				case reg_KU:
					if (reg2 == reg_A)		val = 0x04; // LR KU,A
					break;

				case reg_KL:
					if (reg2 == reg_A)		val = 0x05; // LR KL,A
					break;

				case reg_QU:
					if (reg2 == reg_A)		val = 0x06; // LR QU,A
					break;

				case reg_QL:
					if (reg2 == reg_A)		val = 0x07; // LR QL,A
					break;

				case reg_IS:
					if (reg2 == reg_A)		val = 0x0B; // LR IS,A
					break;

				case reg_PC1:
					if (reg2 == reg_K)		val = 0x09; // LR PC1,K
					break;

				case reg_PC0:
					if (reg2 == reg_Q)		val = 0x0D; // LR PC0,Q
					break;

				case reg_DC0:
					if (reg2 == reg_Q)		val = 0x0F; // LR DC0,H
					if (reg2 == reg_H)		val = 0x10; // LR DC0,H
					break;

				default:
					if (reg_0 <= reg1 && reg1 <= reg_15 && reg2 == reg_A)
							val = 0x50 + reg1;			// LR n,A
			}

			if (val < 0)
				IllegalOperand();
			else
				Instr1(val);
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
