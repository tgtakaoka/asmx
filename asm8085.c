// asm8085.c - copyright 1998-2006 Bruce Tomlin

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define versionName "8085 assembler"
#define INSTR_MAX   3		// length of longest valid instruction
#define CPU_LITTLE_ENDIAN   // CPU is little-endian

#include "asmguts.h"

enum instrType
{
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
	o_STAX		// STAX/LDAX instructions

//  o_Foo = o_LabelOp,
};

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


int DoCPUOpcode(int typ, int parm)
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
			return 0;
			break;
	}
    return 1;
}


int DoCPULabelOp(int typ, int parm, char *labl)
{
//	int		i,val;
//	Str255  word;

	switch(typ)
	{
		default:
			return 0;
			break;
	}
    return 1;
}


void usage(void)
{
	stdversion();
	stdusage();
	exit(1);
}


void PassInit(void)
{
}


void AsmInit(void)
{
}
