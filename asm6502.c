// asm6502.c - copyright 1998-2004 Bruce Tomlin

//#define DEBUG_PASS

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define version "6502 assembler version 1.7"
#define maxOpcdLen  9		// max opcode legnth (for building opcode table)
#define INSTR_MAX   5		// length of longest valid instruction
#define LITTLE_ENDIAN		// CPU is little-endian

enum instrType
{
	o_Illegal,	// opcode not found in FindOpcode
	o_Implied,	// implied instructions
//	o_Implied_6502, // implied instructions for 6502 only
	o_Implied_65C02, // implied instructions for 65C02 only
	o_Implied_6502U, // implied instructions for undocumented 6502 only
	o_Branch,	// branch instructions
	o_Branch_65C02, // branch instructions for 65C02 only
	o_Mode,		// instructions with multiple addressing modes
	o_Mode_65C02, // o_Mode for 6502
	o_Mode_6502U, // o_Mode for undocumented 6502
	o_RSMB,		// RMBn/SMBn instructions for 65C02 only
	o_BBRS,		// BBRn/BBSn instructions for 65C02 only
	o_DB,		// DB pseudo-op
	o_DWLE,		// DW pseudo-op
	o_DWBE,		// big-endian DW
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


#include "asmguts.h"


enum cputype
{
	cpu_6502, cpu_65C02, cpu_6502U
} cpu;

enum addrMode
{
	a_None = -1, // -1 addressing mode invalid
	a_Imm,		//  0 Immediate    #val
	a_Abs,		//  1 Absolute     val
	a_Zpg,		//  2 Z-Page       val
	a_Acc,		//  3 Accumulator  A   (or no operand)
	a_Inx,		//  4 Indirect X   (val,X)
	a_Iny,		//  5 Indirect Y   (val),Y
	a_Zpx,		//  6 Z-Page X     val,X
	a_Abx,		//  7 Absolute X   val,X
	a_Aby,		//  8 Absolute Y   val,Y
	a_Ind,		//  9 Indirect     [val]
	a_Zpy,		// 10 Z-Page Y     val,Y
	a_Zpi,		// 11 Z-Page Indirect  (val)
	a_Max		// 11/12
};

enum addrOps
{
	o_ORA,		//  0
	o_ASL,		//  1
	o_JSR,		//  2
	o_AND,		//  3
	o_BIT,		//  4
	o_ROL,		//  5
	o_EOR,		//  6
	o_LSR,		//  7
	o_JMP,		//  8
	o_ADC,		//  9
	o_ROR,		// 10
	o_STA,		// 11
	o_STY,		// 12
	o_STX,		// 13
	o_LDY,		// 14
	o_LDA,		// 15
	o_LDX,		// 16
	o_CPY,		// 17
	o_CMP,		// 18
	o_DEC,		// 19
	o_CPX,		// 20
	o_SBC,		// 21
	o_INC,		// 22

	o_Extra,	// 23

//  o_Mode_6502U

	o_LAX = o_Extra, // 23
	o_DCP,
	o_ISB,
	o_RLA,
	o_RRA,
	o_SAX,
	o_SLO,
	o_SRE,
	o_ANC,
	o_ARR,
	o_ASR,
	o_SBX,

//  o_Mode_65C02

	o_STZ = o_Extra, // 23
	o_TSB,		// 24
	o_TRB,		// 25
	o_Max		// 26
};

const unsigned char mode2op65C02[] = // [o_Max * a_Max] =
{//Imm=0 Abs=1 Zpg=2 Acc=3 Inx=4 Iny=5 Zpx=6 Abx=7 Aby=8 Ind=9 Zpy=10 Zpi=11
	0x09, 0x0D, 0x05,    0, 0x01, 0x11, 0x15, 0x1D, 0x19,    0,    0, 0x12,	//  0 ORA
	   0, 0x0E, 0x06, 0x0A,    0,    0, 0x16, 0x1E,    0,    0,    0,    0,	//  1 ASL
	   0, 0x20,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	//  2 JSR
	0x29, 0x2D, 0x25,    0, 0x21, 0x31, 0x35, 0x3D, 0x39,    0,    0, 0x32,	//  3 AND
	0x89, 0x2C, 0x24,    0,    0,    0, 0x34, 0x3C,    0,    0,    0,    0,	//  4 BIT
	   0, 0x2E, 0x26, 0x2A,    0,    0, 0x36, 0x3E,    0,    0,    0,    0,	//  5 ROL
	0x49, 0x4D, 0x45,    0, 0x41, 0x51, 0x55, 0x5D, 0x59,    0,    0, 0x52,	//  6 EOR
	   0, 0x4E, 0x46, 0x4A,    0,    0, 0x56, 0x5E,    0,    0,    0,    0,	//  7 LSR
	   0, 0x4C,    0,    0, 0x7C,    0,    0,    0,    0, 0x6C,    0,    0,	//  8 JMP note: 7C is (abs,x)
	0x69, 0x6D, 0x65,    0, 0x61, 0x71, 0x75, 0x7D, 0x79,    0,    0, 0x72,	//  9 ADC
	   0, 0x6E, 0x66, 0x6A,    0,    0, 0x76, 0x7E,    0,    0,    0,    0,	// 10 ROR
	   0, 0x8D, 0x85,    0, 0x81, 0x91, 0x95, 0x9D, 0x99,    0,    0, 0x92,	// 11 STA
	   0, 0x8C, 0x84,    0,    0,    0, 0x94,    0,    0,    0,    0,    0,	// 12 STY
	   0, 0x8E, 0x86,    0,    0,    0,    0,    0,    0,    0, 0x96,    0,	// 13 STX
	0xA0, 0xAC, 0xA4,    0,    0,    0, 0xB4, 0xBC,    0,    0,    0,    0,	// 14 LDY
	0xA9, 0xAD, 0xA5,    0, 0xA1, 0xB1, 0xB5, 0xBD, 0xB9,    0,    0, 0xB2,	// 15 LDA
	0xA2, 0xAE, 0xA6,    0,    0,    0,    0,    0, 0xBE,    0, 0xB6,    0,	// 16 LDX
	0xC0, 0xCC, 0xC4,    0,    0,    0,    0,    0,    0,    0,    0,    0,	// 17 CPY
	0xC9, 0xCD, 0xC5,    0, 0xC1, 0xD1, 0xD5, 0xDD, 0xD9,    0,    0, 0xD2,	// 18 CMP
	   0, 0xCE, 0xC6, 0x3A,    0,    0, 0xD6, 0xDE,    0,    0,    0,    0,	// 19 DEC
	0xE0, 0xEC, 0xE4,    0,    0,    0,    0,    0,    0,    0,    0,    0,	// 20 CPX
	0xE9, 0xED, 0xE5,    0, 0xE1, 0xF1, 0xF5, 0xFD, 0xF9,    0,    0, 0xF2,	// 21 SBC
	   0, 0xEE, 0xE6, 0x1A,    0,    0, 0xF6, 0xFE,    0,    0,    0,    0,	// 22 INC
// 65C02-only instructions start here
	   0, 0x9C, 0x64,    0,    0,    0, 0x74, 0x9E,    0,    0,    0,    0,	// 23 STZ
	   0, 0x0C, 0x04,    0,    0,    0,    0,    0,    0,    0,    0,    0,	// 24 TSB
	   0, 0x1C, 0x14,    0,    0,    0,    0,    0,    0,    0,    0,    0	// 25 TRB
};

const unsigned char mode2op6502[] = // [o_Max * a_Max] =
{//Imm=0 Abs=1 Zpg=2 Acc=3 Inx=4 Iny=5 Zpx=6 Abx=7 Aby=8 Ind=9 Zpy=10 Zpi=11
	0x09, 0x0D, 0x05,    0, 0x01, 0x11, 0x15, 0x1D, 0x19,    0,    0,    0,	//  0 ORA
	   0, 0x0E, 0x06, 0x0A,    0,    0, 0x16, 0x1E,    0,    0,    0,    0,	//  1 ASL
	   0, 0x20,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,	//  2 JSR
	0x29, 0x2D, 0x25,    0, 0x21, 0x31, 0x35, 0x3D, 0x39,    0,    0,    0,	//  3 AND
	   0, 0x2C, 0x24,    0,    0,    0,    0,    0,    0,    0,    0,    0,	//  4 BIT
	   0, 0x2E, 0x26, 0x2A,    0,    0, 0x36, 0x3E,    0,    0,    0,    0,	//  5 ROL
	0x49, 0x4D, 0x45,    0, 0x41, 0x51, 0x55, 0x5D, 0x59,    0,    0,    0,	//  6 EOR
	   0, 0x4E, 0x46, 0x4A,    0,    0, 0x56, 0x5E,    0,    0,    0,    0,	//  7 LSR
	   0, 0x4C,    0,    0,    0,    0,    0,    0,    0, 0x6C,    0,    0,	//  8 JMP
	0x69, 0x6D, 0x65,    0, 0x61, 0x71, 0x75, 0x7D, 0x79,    0,    0,    0,	//  9 ADC
	   0, 0x6E, 0x66, 0x6A,    0,    0, 0x76, 0x7E,    0,    0,    0,    0,	// 10 ROR
	   0, 0x8D, 0x85,    0, 0x81, 0x91, 0x95, 0x9D, 0x99,    0,    0,    0,	// 11 STA
	   0, 0x8C, 0x84,    0,    0,    0, 0x94,    0,    0,    0,    0,    0,	// 12 STY
	   0, 0x8E, 0x86,    0,    0,    0,    0,    0,    0,    0, 0x96,    0,	// 13 STX
	0xA0, 0xAC, 0xA4,    0,    0,    0, 0xB4, 0xBC,    0,    0,    0,    0,	// 14 LDY
	0xA9, 0xAD, 0xA5,    0, 0xA1, 0xB1, 0xB5, 0xBD, 0xB9,    0,    0,    0,	// 15 LDA
	0xA2, 0xAE, 0xA6,    0,    0,    0,    0,    0, 0xBE,    0, 0xB6,    0,	// 16 LDX
	0xC0, 0xCC, 0xC4,    0,    0,    0,    0,    0,    0,    0,    0,    0,	// 17 CPY
	0xC9, 0xCD, 0xC5,    0, 0xC1, 0xD1, 0xD5, 0xDD, 0xD9,    0,    0,    0,	// 18 CMP
	   0, 0xCE, 0xC6,    0,    0,    0, 0xD6, 0xDE,    0,    0,    0,    0,	// 19 DEC
	0xE0, 0xEC, 0xE4,    0,    0,    0,    0,    0,    0,    0,    0,    0,	// 20 CPX
	0xE9, 0xED, 0xE5,    0, 0xE1, 0xF1, 0xF5, 0xFD, 0xF9,    0,    0,    0,	// 21 SBC
	   0, 0xEE, 0xE6,    0,    0,    0, 0xF6, 0xFE,    0,    0,    0,    0,	// 22 INC
// undocumented 6502 instructions start here
// Imm=0 Abs=1 Zpg=2 Acc=3 Inx=4 Iny=5 Zpx=6 Abx=7 Aby=8 Ind=9 Zpy=10 Zpi=11
	   0, 0xAF, 0xA7,    0, 0xA3, 0xB3,    0,    0, 0xBF,    0, 0xB7,    0,	// 23 LAX
	   0, 0xCF, 0xC7,    0, 0xC3, 0xD3, 0xD7, 0xDF, 0xDB,    0,    0,    0, // DCP
	   0, 0xEF, 0xE7,    0, 0xE3, 0xF3, 0xF7, 0xFF, 0xFB,    0,    0,    0, // ISB
	   0, 0x2F, 0x27,    0, 0x23, 0x33, 0x37, 0x3F, 0x3B,    0,    0,    0, // RLA
	   0, 0x6F, 0x67,    0, 0x63, 0x73, 0x77, 0x7F, 0x7B,    0,    0,    0, // RRA
	   0, 0x8F, 0x87,    0, 0x83,    0,    0,    0,    0,    0, 0x97,    0, // SAX
	   0, 0x0F, 0x07,    0, 0x03, 0x13, 0x17, 0x1F, 0x1B,    0,    0,    0, // SLO
	   0, 0x4F, 0x47,    0, 0x43, 0x53, 0x57, 0x5F, 0x5B,    0,    0,    0, // SRE
	0x2B,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // ANC
	0x6B,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // ARR
	0x4B,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // ASR
	0xCB,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0, // SBX
};


struct OpcdRec opcdTab[] =
{
	{"BRK",  o_Implied, 0x00},
	{"PHP",  o_Implied, 0x08},
	{"CLC",  o_Implied, 0x18},
	{"PLP",  o_Implied, 0x28},
	{"SEC",  o_Implied, 0x38},
	{"RTI",  o_Implied, 0x40},
	{"PHA",  o_Implied, 0x48},
	{"CLI",  o_Implied, 0x58},
	{"RTS",  o_Implied, 0x60},
	{"PLA",  o_Implied, 0x68},
	{"SEI",  o_Implied, 0x78},
	{"DEY",  o_Implied, 0x88},
	{"TXA",  o_Implied, 0x8A},
	{"TYA",  o_Implied, 0x98},
	{"TXS",  o_Implied, 0x9A},
	{"TAY",  o_Implied, 0xA8},
	{"TAX",  o_Implied, 0xAA},
	{"CLV",  o_Implied, 0xB8},
	{"TSX",  o_Implied, 0xBA},
	{"INY",  o_Implied, 0xC8},
	{"DEX",  o_Implied, 0xCA},
	{"CLD",  o_Implied, 0xD8},
	{"INX",  o_Implied, 0xE8},
	{"NOP",  o_Implied, 0xEA},
	{"SED",  o_Implied, 0xF8},

	{"ASLA", o_Implied, 0x0A},
	{"ROLA", o_Implied, 0x2A},
	{"LSRA", o_Implied, 0x4A},
	{"RORA", o_Implied, 0x6A},

	{"BPL",  o_Branch, 0x10},
	{"BMI",  o_Branch, 0x30},
	{"BVC",  o_Branch, 0x50},
	{"BVS",  o_Branch, 0x70},
	{"BCC",  o_Branch, 0x90},
	{"BCS",  o_Branch, 0xB0},
	{"BNE",  o_Branch, 0xD0},
	{"BEQ",  o_Branch, 0xF0},

	{"ORA",  o_Mode, o_ORA},
	{"ASL",  o_Mode, o_ASL},
	{"JSR",  o_Mode, o_JSR},
	{"AND",  o_Mode, o_AND},
	{"BIT",  o_Mode, o_BIT},
	{"ROL",  o_Mode, o_ROL},
	{"EOR",  o_Mode, o_EOR},
	{"LSR",  o_Mode, o_LSR},
	{"JMP",  o_Mode, o_JMP},
	{"ADC",  o_Mode, o_ADC},
	{"ROR",  o_Mode, o_ROR},
	{"STA",  o_Mode, o_STA},
	{"STY",  o_Mode, o_STY},
	{"STX",  o_Mode, o_STX},
	{"LDY",  o_Mode, o_LDY},
	{"LDA",  o_Mode, o_LDA},
	{"LDX",  o_Mode, o_LDX},
	{"CPY",  o_Mode, o_CPY},
	{"CMP",  o_Mode, o_CMP},
	{"DEC",  o_Mode, o_DEC},
	{"CPX",  o_Mode, o_CPX},
	{"SBC",  o_Mode, o_SBC},
	{"INC",  o_Mode, o_INC},

	{"DB",   o_DB,  0},
	{"FCB",  o_DB,  0},
	{"BYTE", o_DB,  0},
	{".BYTE",o_DB,  0},
	{"DC.B", o_DB,  0},
	{".DC.B",o_DB,  0},
	{"DFB",  o_DB,  0},
	{"DW",   o_DWLE,0},
	{"FDB",  o_DWLE,0},
	{"WORD", o_DWLE,0},
	{".WORD",o_DWLE,0},
	{"DC.W", o_DWLE,0},
	{".DC.W",o_DWLE,0},
	{"DA",   o_DWLE,0},
	{"DRW",  o_DWBE,0},
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
	{".6502",    o_CPUtype,cpu_6502},
	{".65C02",   o_CPUtype,cpu_65C02},
	{".6502U",   o_CPUtype,cpu_6502U},

// 65C02 opcodes

	{"INCA", o_Implied_65C02, 0x1A},
	{"INA",  o_Implied_65C02, 0x1A},
	{"DECA", o_Implied_65C02, 0x3A},
	{"DEA",  o_Implied_65C02, 0x3A},
	{"PHY",  o_Implied_65C02, 0x5A},
	{"PLY",  o_Implied_65C02, 0x7A},
	{"PHX",  o_Implied_65C02, 0xDA},
	{"PLX",  o_Implied_65C02, 0xFA},

	{"BRA",  o_Branch_65C02, 0x80},

	{"STZ",  o_Mode_65C02, o_STZ},
	{"TSB",  o_Mode_65C02, o_TSB},
	{"TRB",  o_Mode_65C02, o_TRB},

	{"RMB0", o_RSMB, 0x07},
	{"RMB1", o_RSMB, 0x17},
	{"RMB2", o_RSMB, 0x27},
	{"RMB3", o_RSMB, 0x37},
	{"RMB4", o_RSMB, 0x47},
	{"RMB5", o_RSMB, 0x57},
	{"RMB6", o_RSMB, 0x67},
	{"RMB7", o_RSMB, 0x77},

	{"SMB0", o_RSMB, 0x87},
	{"SMB1", o_RSMB, 0x97},
	{"SMB2", o_RSMB, 0xA7},
	{"SMB3", o_RSMB, 0xB7},
	{"SMB4", o_RSMB, 0xC7},
	{"SMB5", o_RSMB, 0xD7},
	{"SMB6", o_RSMB, 0xE7},
	{"SMB7", o_RSMB, 0xF7},

	{"BBR0", o_BBRS, 0x0F},
	{"BBR1", o_BBRS, 0x1F},
	{"BBR2", o_BBRS, 0x2F},
	{"BBR3", o_BBRS, 0x3F},
	{"BBR4", o_BBRS, 0x4F},
	{"BBR5", o_BBRS, 0x5F},
	{"BBR6", o_BBRS, 0x6F},
	{"BBR7", o_BBRS, 0x7F},

	{"BBS0", o_BBRS, 0x8F},
	{"BBS1", o_BBRS, 0x9F},
	{"BBS2", o_BBRS, 0xAF},
	{"BBS3", o_BBRS, 0xBF},
	{"BBS4", o_BBRS, 0xCF},
	{"BBS5", o_BBRS, 0xDF},
	{"BBS6", o_BBRS, 0xEF},
	{"BBS7", o_BBRS, 0xFF},

//  undocumented instructions for original 6502 only
//  see http://www.s-direktnet.de/homepages/k_nadj/opcodes.html
	{"NOP3", o_Implied_6502U, 0x0400},  // 3-cycle NOP
	{"LAX",  o_Mode_6502U,    o_LAX},	// LDA + LDX
	{"DCP",  o_Mode_6502U,    o_DCP},   // DEC + CMP (also called DCM)
	{"ISB",  o_Mode_6502U,    o_ISB},   // INC + SBC (also called INS and ISC)
	{"RLA",  o_Mode_6502U,    o_RLA},   // ROL + AND
	{"RRA",  o_Mode_6502U,    o_RRA},   // ROR + ADC
	{"SAX",  o_Mode_6502U,    o_SAX},   // store (A & X) (also called AXS)
	{"SLO",  o_Mode_6502U,    o_SLO},   // ASL + ORA (also called ASO)
	{"SRE",  o_Mode_6502U,    o_SRE},   // LSR + EOR (also called LSE)
	{"ANC",  o_Mode_6502U,    o_ANC},   // AND# with bit 7 copied to C flag (also called AAC)
	{"ARR",  o_Mode_6502U,    o_ARR},   // AND# + RORA with strange flag results
	{"ASR",  o_Mode_6502U,    o_ASR},   // AND# + LSRA (also called ALR)
	{"SBX",  o_Mode_6502U,    o_SBX},   // X = (A & X) - #nn (also called ASX and SAX)

	{"",     o_Illegal, 0}
};


// --------------------------------------------------------------


void DoOpcode(int typ, int parm)
{
	int		val;
	int		i;
	Str255	word;
	char	*oldLine;
	int		token;
//	char	ch;
//	bool	done;
	int		opcode;
	int		mode;
	const unsigned char	*modes;		// pointer to current o_Mode instruction's opcodes

	switch(typ)
	{
		case o_Implied_65C02:
			if (cpu == cpu_6502)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}
		case o_Implied_6502U:
			if (typ == o_Implied_6502U && cpu != cpu_6502U)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}
/*		case o_Implied_6502:
			if (typ == o_Implied_6502 && cpu != cpu_6502)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}		
*/		case o_Implied:
			if (parm > 256) Instr2(parm >> 8, parm & 255);
					else	Instr1(parm);
			break;

		case o_Branch_65C02:
			if (cpu == cpu_6502)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}
		case o_Branch:
			val = EvalBranch(2);
			Instr2(parm,val);
			break;

		case o_Mode_6502U:
			if (cpu != cpu_6502U)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}		
		case o_Mode_65C02:
			if (typ == o_Mode_65C02 && cpu != cpu_65C02)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}		
		case o_Mode:
			instrLen = 0;
			oldLine = linePtr;
			token = GetWord(word);

			if (cpu == cpu_65C02)	modes = mode2op65C02 + parm * a_Max;
							else	modes = mode2op6502  + parm * a_Max;

            mode = a_None;
            val  = 0;

			if (token == 0)
               mode = a_Acc;	// accumulator
			else
			{
				switch(token)
				{
					case '#':	// immediate
						val  = Eval();
						mode = a_Imm;
						break;

					case '[':	// indirect (this is a 6809ism that I got used to)
						val  = Eval();
						// should probably also handle 65C02 (abs,X) here
						Expect("]");
						mode = a_Ind;
						break;

					case '(':	// indirect X,Y
						val  = Eval();
						token = GetWord(word);
						switch (token)
						{
							case ',':	// (val,X)
								Expect("X");
								Expect(")");
								mode = a_Inx;
                             	break;

							case ')':	// (val) -and- (val),Y
								token = GetWord(word);
								switch(token)
								{
									case 0:
										mode = a_Ind;
										if (val >= 0 && val < 256 && evalKnown &&
											(modes[a_Zpi] != 0))
													mode = a_Zpi;
											else	mode = a_Ind;
										break;
									case ',':
										Expect("Y");
										mode = a_Iny;
										break;
								}
                              	break;

							default:
								mode = a_None;
								break;
						}
						break;

					default:	// everything else
						if ((word[1] == 0) && (toupper(word[0]) == 'A'))
						{
							token = GetWord(word);
							if (token == 0)		// mustn't have anything after the 'A'
								mode = a_Acc;
						}
						else
						{
							linePtr = oldLine;
							val   = Eval();
							token = GetWord(word);

							switch(token)
							{
								case 0:		// abs or zpg
									if (val >= 0 && val < 256 && evalKnown &&
										(modes[a_Zpg] != 0))
												mode = a_Zpg;
										else	mode = a_Abs;
									break;

								case ',':
									token = GetWord(word);

									if ((word[1] == 0) && (toupper(word[0]) == 'X'))
									{
										if (val >= 0 && val < 256 &&
											(evalKnown || modes[a_Abx] == 0))
													mode = a_Zpx;
											else	mode = a_Abx;
									}
									else if ((word[1] == 0) && (toupper(word[0]) == 'Y'))
									{
										if (val >= 0 && val < 256 &&
											(evalKnown || modes[a_Aby] == 0)
												&& modes[a_Zpy] != 0)
													mode = a_Zpy;
											else	mode = a_Aby;
									}
							}
						}
				}
			}

			if (mode != a_None)
			{
				opcode = modes[mode];
				if (opcode == 0)
					mode = a_None;
			}

			instrLen = 0;
			switch(mode)
			{
				case a_None:  
					Error("Invalid addressing mode");
					break;

				case a_Acc:
					Instr1(opcode);
					break;

				case a_Inx:
					if (opcode == 0x7C)	// 65C02 JMP (abs,X)
					{
						Instr3W(opcode, val);
						break;
					}
					// else fall through

				case a_Zpg:
				case a_Iny:
				case a_Zpx:
				case a_Zpy:
				case a_Zpi:
					val = (short) val;
					if (!errFlag && (val < 0 || val > 255))
						Error("Byte out of range");
					Instr2(opcode, val & 0xFF);
					break;

				case a_Imm:
					val = (short) val;
					if (!errFlag && (val < -128 || val > 255))
						Error("Byte out of range");
					Instr2(opcode, val & 0xFF);
					break;

				case a_Abs:
				case a_Abx:
				case a_Aby:
				case a_Ind:
					Instr3W(opcode, val);
					break;
			}
			break;

		case o_RSMB:		//    RMBn/SMBn instructions
			if (cpu == cpu_6502)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}

			// RMB/SMB zpg
			val = Eval();
			Instr2(parm,val);
			break;

		case o_BBRS:		//    BBRn/BBSn instructions
			if (cpu == cpu_6502)
			{
				DoStdOpcode(typ, parm); // pass the buck to get "unknown opcode"
				break;
			}

			i = Eval();
			Expect(",");
			val = EvalBranch(3);
			Instr3(parm,i,val);
			break;

		case o_Processor:
			if (GetWord(word))
			{
					 if (strcmp(word,"6502")  == 0) cpu = cpu_6502;
				else if (strcmp(word,"65C02") == 0) cpu = cpu_65C02;
				else if (strcmp(word,"6502U") == 0) cpu = cpu_6502U;
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
	cpu = cpu_6502;
}


void AsmInit(void)
{
}
