// asmz80.c - copyright 1998-2007 Bruce Tomlin

#define versionName "Z-80 assembler"
#define THREE_TAB   // use three-tab data area in listings
#include "asmguts.h"

enum
{
    o_None,     // No operands
    o_LD,       // Generic LD opcode
    o_EX,       // Generic EX opcode
    o_ADD,      // Generic ADD opcode
    o_ADC_SBC,  // Generic ADC and SBC opcodes
    o_INC_DEC,  // Generic INC and DEC opcodes
    o_JP_CALL,  // Generic JP and CALL opcodes
    o_JR,       // Generic JR opcode
    o_RET,      // Generic RET opcode
    o_IN,       // Generic IN opcode
    o_OUT,      // Generic OUT opcode

    o_PushPop,  // PUSH and POP instructions
    o_Arith,    // Arithmetic instructions
    o_Rotate,   // Z-80 rotate instructions
//  o_Bit,      // BIT, RES, and SET instructions
    o_IM,       // IM instruction
    o_DJNZ,     // DJNZ instruction
    o_RST,      // RST instruction

    o_Bit = o_LabelOp   // BIT, RES, and SET instructions need to be pseudo-op to allow SET fallback
};

const char conds[] = "NZ Z NC C PO PE P M";
// NZ=0 Z=1 NC=2 C=3 PO=4 PE=5 P=6 M=7

// L is in regs[] twice as a placeholder for (HL)
const char regs[] = "B C D E H L L A I R BC DE HL SP IX IY AF (";

enum regType    // these are keyed to regs[] above
{
    reg_B,          //  0
    reg_C,          //  1
    reg_D,          //  2
    reg_E,          //  3
    reg_H,          //  4
    reg_L,          //  5
    reg_M,          //  6
    reg_A,          //  7
    reg_I,          //  8
    reg_R,          //  9
    reg_BC,         // 10
    reg_DE,         // 11
    reg_HL,         // 12
    reg_SP,         // 13
    reg_IX,         // 14
    reg_IY,         // 15
    reg_AF,         // 16
    reg_Paren       // 17
};

struct OpcdRec opcdTab[] =
{
    {"EXX", o_None,     0xD9},
    {"LDI", o_None,     0xEDA0},
    {"LDIR",o_None,     0xEDB0},
    {"LDD", o_None,     0xEDA8},
    {"LDDR",o_None,     0xEDB8},
    {"CPI", o_None,     0xEDA1},
    {"CPIR",o_None,     0xEDB1},
    {"CPD", o_None,     0xEDA9},
    {"CPDR",o_None,     0xEDB9},
    {"DAA", o_None,     0x27},
    {"CPL", o_None,     0x2F},
    {"NEG", o_None,     0xED44},
    {"CCF", o_None,     0x3F},
    {"SCF", o_None,     0x37},
    {"NOP", o_None,     0x00},
    {"HALT",o_None,     0x76},
    {"DI",  o_None,     0xF3},
    {"EI",  o_None,     0xFB},
    {"RLCA",o_None,     0x07},
    {"RLA", o_None,     0x17},
    {"RRCA",o_None,     0x0F},
    {"RRA", o_None,     0x1F},
    {"RLD", o_None,     0xED6F},
    {"RRD", o_None,     0xED67},
    {"RETI",o_None,     0xED4D},
    {"RETN",o_None,     0xED45},
    {"INI", o_None,     0xEDA2},
    {"INIR",o_None,     0xEDB2},
    {"IND", o_None,     0xEDAA},
    {"INDR",o_None,     0xEDBA},
    {"OUTI",o_None,     0xEDA3},
    {"OTIR",o_None,     0xEDB3},
    {"OUTD",o_None,     0xEDAB},
    {"OTDR",o_None,     0xEDBB},

    {"LD",  o_LD,       0},
    {"EX",  o_EX,       0},
    {"ADD", o_ADD,      0},
    {"ADC", o_ADC_SBC,  0},
    {"SBC", o_ADC_SBC,  1},
    {"INC", o_INC_DEC,  0},
    {"DEC", o_INC_DEC,  1},
    {"JP",  o_JP_CALL,  0xC3C2},
    {"CALL",o_JP_CALL,  0xCDC4},
    {"JR",  o_JR,       0},
    {"RET", o_RET,      0},

    {"PUSH",o_PushPop,  0xC5},
    {"POP", o_PushPop,  0xC1},

    {"SUB", o_Arith,    0xD690},
    {"AND", o_Arith,    0xE6A0},
    {"XOR", o_Arith,    0xEEA8},
    {"OR",  o_Arith,    0xF6B0},
    {"CP",  o_Arith,    0xFEB8},

    {"RLC", o_Rotate,   0x00},
    {"RRC", o_Rotate,   0x08},
    {"RL",  o_Rotate,   0x10},
    {"RR",  o_Rotate,   0x18},
    {"SLA", o_Rotate,   0x20},
    {"SRA", o_Rotate,   0x28},
    {"SRL", o_Rotate,   0x38},

    {"BIT", o_Bit,      0x40},
    {"RES", o_Bit,      0x80},
    {"SET", o_Bit,      0xC0},

    {"IM",  o_IM,       0},

    {"DJNZ",o_DJNZ,     0},

    {"IN",  o_IN,       0},
    {"OUT", o_OUT,      0},

    {"RST", o_RST,      0},

    {"",    o_Illegal,  0}
};


// --------------------------------------------------------------


int IXOffset()
{
    char    *oldLine;
    Str255  word;
    int     token;
    int     val;

    oldLine = linePtr;
    token = GetWord(word);

    val = 0;

    if (token == '+' || token == '-')
    {
        val = Eval();
        if (token == '-')
            val = -val;
    }
    else linePtr = oldLine;

    RParen();

    return val;
}


inline int DDFD(int reg)
{
    if (reg == reg_IX)
        return 0xDD;
    return 0xFD;
}


void DoArith(int imm, int reg)
{

    char        *oldLine;
    int         reg2;
    int         val;

    oldLine = linePtr;
    switch((reg2 = GetReg(regs)))
    {
        case reg_EOL:
            break;

        case reg_None:  // ADD A,nn
            linePtr = oldLine;
            val = Eval();
            InstrBB(imm,val);
            break;

        case reg_B:
        case reg_C:
        case reg_D:
        case reg_E:
        case reg_H:
        case reg_L:
        case reg_A:     // ADD A,r
            InstrB(reg + reg2);
            break;

        case reg_Paren: // ADD A,(
            switch((reg2 = GetReg(regs)))
            {
                case reg_EOL:
                    break;

                case reg_HL:
                    if (RParen()) break;
                    InstrB(reg+reg_M);
                    break;

                case reg_IX:
                case reg_IY:
                    val = IXOffset();
                    InstrXB(DDFD(reg2) * 256 + reg + reg_M,val);
                    break;

                default:
                    // must be constant "(expression)" so try again that way
                    linePtr = oldLine;
                    val = Eval();
                    InstrBB(imm,val);
            }
            break;

         default:
            IllegalOperand();
    }
}


int DoCPUOpcode(int typ, int parm)
{
    int     val;
    int     reg1;
    int     reg2;
    Str255  word;
    char    *oldLine;
    int     token;

    switch(typ)
    {

        case o_None:
            InstrX(parm);
            break;

        case o_LD:
            switch((reg1 = GetReg(regs)))
            {
                case reg_EOL:
                    break;

                case reg_None:  // LD nnnn,?
                    IllegalOperand();
                    break;

                case reg_B:
                case reg_C:
                case reg_D:
                case reg_E:
                case reg_H:
                case reg_L:
                case reg_A:     // LD r,?
                    if (Comma()) break;
                    oldLine = linePtr;
                    switch((reg2 = GetReg(regs)))
                    {
                        case reg_EOL:
                            break;

                        case reg_B:
                        case reg_C:
                        case reg_D:
                        case reg_E:
                        case reg_H:
                        case reg_L:
                        case reg_A:     // LD r,r
                           InstrB(0x40 + reg1*8 + reg2);
                           break;

                        case reg_I:     // LD A,I
                           InstrX(0xED57);
                           break;

                        case reg_R:     // LD A,R
                           InstrX(0xED5F);
                           break;

                        case reg_Paren:     // LD r,(?)
                            oldLine = linePtr;
                            switch((reg2 = GetReg(regs)))
                            {
                                case reg_EOL:
                                    break;

                                case reg_BC:    // LD A,(BC)
                                case reg_DE:    // LD A,(DE)
                                    if (reg1 != reg_A)
                                        IllegalOperand();
                                    else
                                    {
                                        if (RParen()) break;
                                        InstrB(0x0A + (reg2-reg_BC)*16);
                                    }
                                    break;

                                case reg_HL:    // LD r,(HL)
                                    if (RParen()) break;
                                    InstrB(0x40 + reg1*8 + reg_M);
                                    break;

                                case reg_IX:    // LD r,(IX+d)
                                case reg_IY:    // LD r,(IY+d)
                                    val = IXOffset();
                                    InstrXB(DDFD(reg2) * 256 + 0x46 + reg1*8,val);
                                    break;

                                case reg_None:  // LD A,(nnnn)
                                    if (reg1 != reg_A)
                                        IllegalOperand();
                                    else
                                    {
                                        linePtr = oldLine;
                                        val = Eval();
                                        if (RParen()) break;
                                        InstrBW(0x3A,val);
                                    }
                                    break;

                                default:
                                    IllegalOperand();
                            }
                            break;

                        case reg_None:  // LD r,nn
                            linePtr = oldLine;
                            InstrBB(0x06 + reg1*8,Eval());
                            break;

                        default:
                            IllegalOperand();
                    }
                    break;

                case reg_I:     // LD I,A
                    if (Comma()) break;
                    if (Expect("A")) break;
                    InstrX(0xED47);
                    break;

                case reg_R:     // LD R,A
                    if (Comma()) break;
                    if (Expect("A")) break;
                    InstrX(0xED4F);
                    break;

                case reg_BC:
                case reg_DE:
                case reg_HL:
                case reg_SP:    // LD rr,?
                    if (Comma()) break;
                    oldLine = linePtr;
                    switch((reg2 = GetReg(regs)))
                    {
                        case reg_EOL:
                            break;

                        case reg_HL:    // LD SP,HL/IX/IY
                        case reg_IX:
                        case reg_IY:
                            if (reg1 != reg_SP)
                                IllegalOperand();
                            else switch(reg2)
                            {
                                case reg_HL: InstrB(  0xF9); break;
                                case reg_IX: InstrX(0xDDF9); break;
                                case reg_IY: InstrX(0xFDF9); break;
                            }
                            break;

                        case reg_Paren:
                            if (reg1 == reg_HL)
                            {
                                val = Eval();   // LD HL,(nnnn)
                                if (RParen()) break;
                                InstrBW(0x2A,val);
                            }
                            else
                            {
                                val = Eval();   // LD BC/DE/SP,(nnnn)
                                if (RParen()) break;
                                InstrXW(0xED4B + (reg1-reg_BC)*16,val);
                            }

                            // at this point, if there is any extra stuff on the line,
                            // backtrack and try again with reg_None case
                            // to handle the case of LD BC,(foo + 1) * 256, etc.
                            token = GetWord(word);
                            if (token == 0) break;
                            // note that if an error occurs, it will be repeated twice,
                            // and we have to return to avoid that, but we need to make sure
                            // the instruction length is the same in both passes or there
                            // will be a phase error problem
                            instrLen = 3;
                            if (errFlag) break;
                            // now fall through...

                        case reg_None:  // LD rr,nnnn
                            linePtr = oldLine;
                            val = Eval();
                            InstrBW(0x01 + (reg1-reg_BC)*16,val);
                            break;

                        default:
                            IllegalOperand();
                    }
                    break;

                case reg_IX:    // LD IX,?
                case reg_IY:    // LD IY,?
                    if (Comma()) break;
                    oldLine = linePtr;
                    switch((reg2 = GetReg(regs)))
                    {
                        case reg_EOL:
                            break;

                        case reg_Paren: // LD IX,(nnnn)
                            val = Eval();
                            if (RParen()) break;
                            InstrXW(DDFD(reg1) * 256 + 0x2A,val);

                            // at this point, if there is any extra stuff on the line,
                            // backtrack and try again with reg_None case
                            // to handle the case of LD IX,(foo + 1) * 256, etc.
                            token = GetWord(word);
                            if (token == 0) break;
                            // note that if an error occurs, it will be repeated twice,
                            // and we have to return to avoid that
                            if (errFlag) break;
                            // now fall through...

                        case reg_None:  // LD IX,nnnn
                            linePtr = oldLine;
                            val = Eval();
                            InstrXW(DDFD(reg1) * 256 + 0x21,val);
                            break;

                        default:
                            IllegalOperand();
                    }
                    break;

                case reg_Paren:     // LD (?),?
                    oldLine = linePtr;
                    switch((reg1 = GetReg(regs)))
                    {
                        case reg_EOL:
                            break;

                        case reg_None:  // LD (nnnn),?
                            linePtr = oldLine;
                            val = Eval();
                            if (RParen()) break;
                            if (Comma()) break;
                            switch((reg2 = GetReg(regs)))
                            {
                                case reg_EOL:
                                    break;

                                case reg_A:  InstrBW(0x32,val); break;
                                case reg_HL: InstrBW(0x22,val); break;
                                case reg_BC:
                                case reg_DE:
                                case reg_SP: InstrXW(0xED43+(reg2-reg_BC)*16,val); break;
                                case reg_IX: InstrXW(0xDD22,val); break;
                                case reg_IY: InstrXW(0xFD22,val); break;

                                default:
                                    IllegalOperand();
                            }
                            break;

                            case reg_BC:
                            case reg_DE:
                                if (RParen()) break;
                                if (Comma()) break;
                                if (Expect("A")) break;
                                InstrB(0x02+(reg1-reg_BC)*16);
                                break;

                            case reg_HL: // LD (HL),?
                                if (RParen()) break;
                                if (Comma()) break;
                                oldLine = linePtr;
                                switch((reg2 = GetReg(regs)))
                                {
                                    case reg_EOL:
                                        break;

                                    case reg_None:
                                        linePtr = oldLine;
                                        val = Eval();
                                        InstrBB(0x36,val);
                                        break;

                                    case reg_B:
                                    case reg_C:
                                    case reg_D:
                                    case reg_E:
                                    case reg_H:
                                    case reg_L:
                                    case reg_A:
                                        InstrB(0x70 + reg2);
                                        break;

                                    default:
                                        IllegalOperand();
                                }
                                break;

                            case reg_IX:
                            case reg_IY:    // LD (IX),?
                                val = IXOffset();
                                if (Comma()) break;
                                oldLine = linePtr;
                                switch((reg2 = GetReg(regs)))
                                {
                                    case reg_EOL:
                                        break;

                                    case reg_None:
                                        linePtr = oldLine;
                                        reg2 = Eval();
                                        InstrXBB(DDFD(reg1) * 256 + 0x36,val,reg2);
                                        break;

                                    case reg_B:
                                    case reg_C:
                                    case reg_D:
                                    case reg_E:
                                    case reg_H:
                                    case reg_L:
                                    case reg_A:
                                        InstrXB(DDFD(reg1) * 256 + 0x70 + reg2,val);
                                        break;

                                    default:
                                        IllegalOperand();
                                }
                                break;

                            default: IllegalOperand();
                        }
                    break;

                default: IllegalOperand();
            }
            break;

        case o_EX:
            switch((reg1 = GetReg(regs)))
            {
                case reg_EOL:
                    break;

                case reg_DE:    // EX DE,HL }
                    if (Comma()) break;
                    if (Expect("HL")) break;
                    InstrB(0xEB);
                    break;

                case reg_AF:    // EX AF,AF' }
                    if (Comma()) break;
                    if (Expect("AF")) break;
                    if (Expect("'")) break;
                    InstrB(0x08);
                    break;

                case reg_Paren: // EX (SP),? }
                    if (Expect("SP")) break;
                    if (RParen()) break;
                    if (Comma()) break;
                    switch((reg2 = GetReg(regs)))
                    {
                        case reg_EOL:
                            break;

                        case reg_HL: InstrB(  0xE3); break;
                        case reg_IX: InstrX(0xDDE3); break;
                        case reg_IY: InstrX(0xFDE3); break;

                        default:
                            IllegalOperand();
                    }
                    break;

                default:
                    IllegalOperand();
            }
            break;

        case o_ADD:
            switch((reg1 = GetReg(regs)))
            {
                case reg_EOL:
                    break;

                case reg_A:
                    if (Comma()) break;
                    DoArith(0xC6,0x80);
                    break;

                case reg_HL:
                case reg_IX:
                case reg_IY:
                    if (Comma()) break;
                    switch((reg2 = GetReg(regs)))
                    {
                        case reg_EOL:
                            break;

                        case reg_HL:
                        case reg_IX:
                        case reg_IY:
                            if (reg1 != reg2)
                            {
                                IllegalOperand();
                                break;
                            }
                            reg2 = reg_HL;
                            // fall through

                        case reg_BC:
                        case reg_DE:
                        case reg_SP:
                            switch(reg1)
                            {
                                case reg_HL: InstrB(  0x09 + (reg2-reg_BC)*16); break;
                                case reg_IX: InstrX(0xDD09 + (reg2-reg_BC)*16); break;
                                case reg_IY: InstrX(0xFD09 + (reg2-reg_BC)*16); break;
                            }
                            break;

                        default:
                            IllegalOperand();
                    }
                    break;

                default: IllegalOperand();
            }
            break;

        case o_ADC_SBC:
            switch((reg1 = GetReg(regs)))
            {
                case reg_EOL:
                    break;

                case reg_A:
                    if (Comma()) break;
                    DoArith(parm*16+0xCE,0x88+parm*16); // had to move 0xCE because GCC complained?
                    break;

                case reg_HL:
                    if (Comma()) break;
                    switch((reg2 = GetReg(regs)))
                    {
                        case reg_EOL:
                            break;

                        case reg_BC:
                        case reg_DE:
                        case reg_HL:
                        case reg_SP:
                            InstrX(0xED4A + (reg2-reg_BC)*16 - parm*8);
                            break;

                        default:
                            IllegalOperand();
                    }
                    break;

                default:
                    IllegalOperand();
            }
            break;

        case o_INC_DEC:
            switch((reg1 = GetReg(regs)))
            {
                case reg_EOL:
                    break;

                case reg_B:
                case reg_C:
                case reg_D:
                case reg_E:
                case reg_H:
                case reg_L:
                case reg_A: // INC r
                    InstrB(0x04 + reg1*8 + parm);
                    break;

                case reg_BC:
                case reg_DE:
                case reg_HL:
                case reg_SP:    // INC rr
                    InstrB(0x03 + (reg1-reg_BC)*16 + parm*8);
                    break;

                case reg_IX: InstrX(0xDD23 + parm*8); break;
                case reg_IY: InstrX(0xFD23 + parm*8); break;

                case reg_Paren: // INC (HL)
                    switch((reg1 = GetReg(regs)))
                    {
                        case reg_EOL:
                            break;

                        case reg_HL:
                            if (RParen()) break;
                            InstrB(0x34 + parm);
                            break;

                        case reg_IX:
                        case reg_IY:
                            val = IXOffset();
                            InstrXB(DDFD(reg1)*256 + 0x34 + parm,val);
                            break;

                        default:
                            IllegalOperand();
                    }
                    break;

                default:
                    IllegalOperand();
            }
            break;

        case o_JP_CALL:
            oldLine = linePtr;
            token = GetWord(word);
            if (token == '(')
            {   // JP (HL/IX/IY)
                if (parm >> 8 != 0xC3)
                    IllegalOperand();
                else
                {
                    reg1 = GetReg(regs);
                    if (RParen()) break;
                    switch(reg1)
                    {
                        case reg_EOL:
                            break;

                        case reg_HL: InstrB(  0xE9); break;
                        case reg_IX: InstrX(0xDDE9); break;
                        case reg_IY: InstrX(0xFDE9); break;

                        default:
                            IllegalOperand();
                    }
                }
            }
            else
            {
                reg1 = FindReg(word,conds);
                if (reg1 == reg_None)
                {
                    linePtr = oldLine;
                    val = Eval();
                    InstrBW(parm >> 8,val);
                }
                else
                {
                    if (Comma()) return 1;
                    val = Eval();
                    InstrBW((parm & 255) + reg1*8,val);
                }
                if ((parm >> 8) == 0xC3 && reg1 <= 3)
                {
                    val = locPtr + 2 - val;
                    if (-128 <= val && val <= 128)
                        Warning("JR instruction could be used here");
                }
            }
            break;

        case o_JR:
            oldLine = linePtr;
            switch((reg1 = GetReg(conds)))
            {
                case reg_EOL:
                    break;

                case reg_None:
                    linePtr = oldLine;
                    val = EvalBranch(2);
                    InstrBB(0x18,val);
                    break;

                case 0:
                case 1:
                case 2:
                case 3:
                    if (Comma()) break;
                    val = EvalBranch(2);
                    InstrBB(0x20 + reg1*8,val);
                    break;

                default:
                    IllegalOperand();
            }
            break;

        case o_RET:
            if (GetWord(word) == 0) InstrB(0xC9);
            else
            {
                reg1 = FindReg(word,conds);
                if (reg1 < 0) IllegalOperand();
                else          InstrB(0xC0 + reg1*8);
            }
            break;

        case o_IN:
            switch((reg1 = GetReg(regs)))
            {
                case reg_EOL:
                    break;

                case reg_B:
                case reg_C:
                case reg_D:
                case reg_E:
                case reg_H:
                case reg_L:
                case reg_A:
                    if (Comma()) break;
                    if (Expect("(")) break;
                    oldLine = linePtr;
                    switch((reg2 = GetReg(regs)))
                    {
                        case reg_EOL:
                            break;

                        case reg_None:
                            if (reg1 != reg_A)
                                IllegalOperand();
                            else
                            {
                                linePtr = oldLine;
                                val = Eval();
                                if (RParen()) break;
                                InstrBB(0xDB,val);
                            }
                            break;

                        case reg_C:
                            if (RParen()) break;
                            InstrX(0xED40 + reg1*8);
                            break;

                        default:
                            IllegalOperand();
                    }
                    break;

                default:
                    IllegalOperand();
            }
            break;

        case o_OUT:
            if (Expect("(")) break;
            oldLine = linePtr;
            switch((reg1 = GetReg(regs)))
            {
                case reg_EOL:
                    break;

                case reg_None:
                    linePtr = oldLine;
                    val = Eval();
                    if (RParen()) break;
                    if (Comma()) break;
                    if (Expect("A")) break;
                    InstrBB(0xD3,val);
                    break;

                case reg_C:
                    if (RParen()) break;
                    if (Comma()) break;
                    switch((reg2 = GetReg(regs)))
                    {
                        case reg_EOL:
                            break;

                        case reg_B:
                        case reg_C:
                        case reg_D:
                        case reg_E:
                        case reg_H:
                        case reg_L:
                        case reg_A:
                            InstrX(0xED41 + reg2*8);
                            break;

                        default:
                            IllegalOperand();
                    }
                    break;

                default:
                    IllegalOperand();
            }
            break;

        case o_PushPop:
            switch((reg1 = GetReg(regs)))
            {
                case reg_EOL:
                    break;

                case reg_BC:
                case reg_DE:
                case reg_HL: InstrB(parm + (reg1-reg_BC)*16); break;
                case reg_AF: InstrB(parm + 0x30);             break;
                case reg_IX: InstrX(0xDD20 + parm);           break;
                case reg_IY: InstrX(0xFD20 + parm);           break;

                default:
                    IllegalOperand();
            }
            break;

        case o_Arith:
            DoArith(parm >> 8, parm & 255);
            break;

        case o_Rotate:
            switch((reg1 = GetReg(regs)))
            {
                case reg_EOL:
                    break;

                case reg_B:
                case reg_C:
                case reg_D:
                case reg_E:
                case reg_H:
                case reg_L:
                case reg_A:     // RLC r
                    InstrX(0xCB00 + parm + reg1);
                    break;

                case reg_Paren:
                    switch((reg1 = GetReg(regs)))
                    {
                        case reg_EOL:
                            break;

                        case reg_HL:
                            if (RParen()) break;
                            InstrX(0xCB00 + parm + reg_M);
                            break;

                        case reg_IX:
                        case reg_IY:
                            val = IXOffset();
                            InstrXBB(DDFD(reg1) * 256 + 0xCB,val,parm+reg_M);
                            break;

                        default:
                            IllegalOperand();
                    }
                    break;

                default:
                    IllegalOperand();
            }
            break;

        case o_IM:
            GetWord(word);
            if (word[1])
                IllegalOperand();
            else
                switch(word[0])
                {
                    case '0': InstrX(0xED46); break;
                    case '1': InstrX(0xED56); break;
                    case '2': InstrX(0xED5E); break;
                    default:
                        IllegalOperand();
                }
            break;

        case o_DJNZ:
            val = EvalBranch(2);
            InstrBB(0x10,val);
            break;

        case o_RST:
            val = Eval();
            if (0 <= val && val <= 7)
                InstrB(0xC7 + val*8);
            else if ((val & 0xC7) == 0) // [$00,$08,$10,$18,$20,$28,$30,$38]
                InstrB(0xC7 + val);
            else
            {
                IllegalOperand();
                break;
            }

            // allow bytes to follow the RST address because
            // RST vectors are used this way so much
            oldLine = line;
            token = GetWord(word);
            if (token == ',')
            {
                while (token == ',' && instrLen < MAX_BYTSTR)
                {
                    bytStr[instrLen++] = Eval();
                    oldLine = line;
                    token = GetWord(word);
                }
                instrLen = -instrLen;   // negative means we're using long DB listing format
            }
            if (token) linePtr = oldLine;  // ensure a too many operands error
            break;

        default:
            return 0;
            break;
    }
    return 1;
}


int DoCPULabelOp(int typ, int parm, char *labl)
{
    int     val;
    Str255  word;
    char    *oldLine;
    int     token;
    int     reg1;
    int     reg2;

    switch(typ)
    {
        // o_Bit needs to be implemented like a pseudo-op so that
        //        SET can fall back to the standard SET pseudo-op
        case o_Bit:
            oldLine = linePtr;
            reg1 = Eval();                  // get bit number
            token = GetWord(word);          // attempt to get comma
            // if end of line and SET opcode
            if (token == 0 && parm == 0xC0)
            {
                linePtr = oldLine;
                if (!errFlag || pass < 2)   // don't double-error on second pass
                    DoLabelOp(o_EQU, 1, labl);  // fall back to SET pseudo-op
            }
            else
            {
                DefSym(labl,locPtr,FALSE,FALSE); // define label if present
                showAddr = TRUE;
                if (token != ',')           // validate that comma is present
                {
                    Error("\",\" expected");
                    break;
                }
                switch((reg2 = GetReg(regs)))
                {
                    case reg_EOL:
                        break;

                    case reg_B:
                    case reg_C:
                    case reg_D:
                    case reg_E:
                    case reg_H:
                    case reg_L:
                    case reg_A:         // BIT n,r
                        InstrX(0xCB00 + parm + reg1*8 + reg2);
                        break;

                    case reg_Paren:     // BIT n,(HL)
                        switch((reg2 = GetReg(regs)))
                        {
                            case reg_EOL:
                                break;

                            case reg_HL:
                                if (RParen()) break;
                                InstrX(0xCB00 + parm + reg1*8 + reg_M);
                                break;

                            case reg_IX:
                            case reg_IY:
                                val = IXOffset();
                                InstrXBB(DDFD(reg2) * 256 + 0xCB,val,parm + reg1*8 + reg_M);
                                break;

                            default:
                                IllegalOperand();
                        }
                        break;

                    default:            // invalid or unknown register
                        IllegalOperand();
                }
            }
            break;

        default:
            return 0;
            break;
    }
    return 1;
}


void PassInit(void)
{
}


void AsmInit(void)
{
    endian = LITTLE_END;
}
