// asm6805.c - copyright 1998-2007 Bruce Tomlin

#define versionName "6805 assembler"
#include "asmx.h"

enum
{
    o_Inherent,     // implied instructions
    o_Relative,     // branch instructions
    o_Bit,          // BCLR/BSET
    o_BRelative,    // BRSET/BRCLR
    o_Logical,      // instructions with multiple addressing modes
    o_Arith,        // arithmetic instructions with multiple addressing modes
    o_Store,        // STA/STX/JMP/JSR - same as o_Arith except no immediate allowed

//  o_Foo = o_LabelOp,
};

enum cputype
{
    CPU_6805,
};

struct OpcdRec M6805_opcdTab[] =
{

    {"NEGA",   o_Inherent, 0x40},
    {"COMA",   o_Inherent, 0x43},
    {"LSRA",   o_Inherent, 0x44},
    {"RORA",   o_Inherent, 0x46},
    {"ASRA",   o_Inherent, 0x47},
    {"ASLA",   o_Inherent, 0x48},
    {"LSLA",   o_Inherent, 0x48},
    {"ROLA",   o_Inherent, 0x49},
    {"DECA",   o_Inherent, 0x4A},
    {"INCA",   o_Inherent, 0x4C},
    {"TSTA",   o_Inherent, 0x4D},
    {"CLRA",   o_Inherent, 0x4F},

    {"NEGX",   o_Inherent, 0x50},
    {"COMX",   o_Inherent, 0x53},
    {"LSRX",   o_Inherent, 0x54},
    {"RORX",   o_Inherent, 0x56},
    {"ASRX",   o_Inherent, 0x57},
    {"ASLX",   o_Inherent, 0x58},
    {"LSLX",   o_Inherent, 0x58},
    {"ROLX",   o_Inherent, 0x59},
    {"DECX",   o_Inherent, 0x5A},
    {"INCX",   o_Inherent, 0x5C},
    {"TSTX",   o_Inherent, 0x5D},
    {"CLRX",   o_Inherent, 0x5F},

    {"MUL",   o_Inherent, 0x42},
    {"RTI",   o_Inherent, 0x80},
    {"RTS",   o_Inherent, 0x81},
    {"SWI",   o_Inherent, 0x83},
    {"STOP",  o_Inherent, 0x8E},
    {"WAIT",  o_Inherent, 0x8F},
    {"TAX",   o_Inherent, 0x97},
    {"CLC",   o_Inherent, 0x98},
    {"SEC",   o_Inherent, 0x99},
    {"CLI",   o_Inherent, 0x9A},
    {"SEI",   o_Inherent, 0x9B},
    {"RSP",   o_Inherent, 0x9C},
    {"NOP",   o_Inherent, 0x9D},
    {"TXA",   o_Inherent, 0x9F},

    {"BRA",   o_Relative, 0x20},
    {"BRN",   o_Relative, 0x21},
    {"BHI",   o_Relative, 0x22},
    {"BLS",   o_Relative, 0x23},
    {"BCC",   o_Relative, 0x24},
    {"BCS",   o_Relative, 0x25},
    {"BHS",   o_Relative, 0x24},
    {"BLO",   o_Relative, 0x25},
    {"BNE",   o_Relative, 0x26},
    {"BEQ",   o_Relative, 0x27},
    {"BHCC",  o_Relative, 0x28},
    {"BHCS",  o_Relative, 0x29},
    {"BPL",   o_Relative, 0x2A},
    {"BMI",   o_Relative, 0x2B},
    {"BMC",   o_Relative, 0x2C},
    {"BMS",   o_Relative, 0x2D},
    {"BIL",   o_Relative, 0x2E},
    {"BIH",   o_Relative, 0x2F},

    {"BSR",   o_Relative, 0xAD},

    {"NEG",   o_Logical,  0x00}, // o_Logical: direct = $30; ix1 = $60, idx = $70
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
    {"CLR",   o_Logical,  0x0F},


    {"SUB",   o_Arith,    0x00}, // o_Arith: immediate = $80, direct = $90, ix2 = $A0
    {"CMP",   o_Arith,    0x01}, //           ix1 = $E0, extended = $F0
    {"SBC",   o_Arith,    0x02},
    {"CPX",   o_Arith,    0x03},
    {"AND",   o_Arith,    0x04},
    {"BIT",   o_Arith,    0x05},
    {"LDA",   o_Arith,    0x06},
    {"STA",   o_Store,    0x07},
    {"EOR",   o_Arith,    0x08},
    {"ADC",   o_Arith,    0x09},
    {"ORA",   o_Arith,    0x0A},
    {"ADD",   o_Arith,    0x0B},
    {"JMP",   o_Store,    0x0C},
    {"JSR",   o_Store,    0x0D},
    {"LDX",   o_Arith,    0x0E},
    {"STX",   o_Store,    0x0F},

    {"BSET",  o_Bit,      0x10},
    {"BCLR",  o_Bit,      0x11},
    {"BRSET", o_BRelative,0x00},
    {"BRCLR", o_BRelative,0x01},

    {"",     o_Illegal, 0}
};


// --------------------------------------------------------------


int M6805_DoCPUOpcode(int typ, int parm)
{
    int     val,val2,val3;
    Str255  word;
    char    *oldLine;
    int     token;
    char    force;
    char    reg;

    switch(typ)
    {
        case o_Inherent:
            InstrB(parm);
            break;

        case o_Relative:
            val = EvalBranch(2);
            InstrBB(parm,val);
            break;

        case o_Logical:
            oldLine = linePtr;
            token = GetWord(word);  // look for ",X"
            if (token == ',')
            {
                if (Expect("X")) break;
                InstrB(parm + 0x70);
                break;
            }

            linePtr = oldLine;
            val = Eval();

            oldLine = linePtr;
            token = GetWord(word);  // look for ",X"
            if (token == 0)
                InstrBW(parm + 0x30, val);  // no ",X", so must be direct
            else if (token == ',')
            {
                if (Expect("X")) break;

                if (evalKnown && val == 0) // 0,X
                     InstrB(parm + 0x70);
                else InstrBB(parm + 0x60, val); // ix1,X
            }
            else
            {
                linePtr = oldLine;
                Comma();
            }
            break;

        case o_Arith:
        case o_Store:
            oldLine = linePtr;
            token = GetWord(word);
            if (token == '#')   // immediate
            {
                if (typ == o_Store)
                    Error("Invalid addressing mode");
                else
                {
                    val = Eval();
                    InstrBB(parm + 0xA0, val);
                }
            }
            else if (token == ',') // ,X
            {
                if (Expect("X")) break;
                InstrB(parm + 0xF0);
                break;
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
                if (token == 0) // dir or ext
                {
                    if (((force != '>' && evalKnown && (val & 0xFF00) >> 8 == 0) || force == '<'))
// FIXME: should warn about force-byte with non-byte value
                        InstrBB(parm + 0xB0, val);  // <$xx
                    else
                        InstrBW(parm + 0xC0, val);   // >$xxxx
                }
                else if (token == ',') // ix1,X or ix2,X
                {
                    if (Expect("X")) break;

                    if (evalKnown && val == 0) // 0,X
                         InstrB(parm + 0xF0);
                    else if (((force != '>' && evalKnown && (val & 0xFF00) >> 8 == 0) || force == '<'))
// FIXME: should warn about force-byte with non-byte value
                         InstrBB(parm + 0xE0, val); // ix1,X
                    else InstrBW(parm + 0xD0, val); // ix2,X
                    break;
                }
                else
                {
                    linePtr = oldLine;
                    Comma();
                }
            }
            break;

        case o_Bit:
            val = Eval();   // bit number
            if (val < 0 || val > 7)
                Error("Bit number must be 0 - 7");

            oldLine = linePtr;  // eat optional comma after bit number
            if (GetWord(word) != ',')
                linePtr = oldLine;

            val2 = Eval();      // direct page address or offset
            reg = 0;

            InstrBB(parm + val * 2, val2);
            break;

        case o_BRelative:
            val = Eval();   // bit number
            if (val < 0 || val > 7)
                Error("Bit number must be 0 - 7");

            oldLine = linePtr;  // eat optional comma after bit number
            if (GetWord(word) != ',')
                linePtr = oldLine;

            val2 = Eval();      // direct page address or offset
            reg = 0;

            oldLine = linePtr;  // eat optional comma after direct page address
            if (GetWord(word) != ',')
                linePtr = oldLine;

            val3 = EvalBranch(2);  // offset

            InstrBBB(parm + val * 2, val2, val3);
            break;

        default:
            return 0;
            break;
    }
    return 1;
}


void Asm6805Init(void)
{
    char *p;

    p = AddAsm(versionName, &M6805_DoCPUOpcode, NULL, NULL);
    AddCPU(p, "6805", CPU_6805, BIG_END, ADDR_16, LIST_24, M6805_opcdTab);
}