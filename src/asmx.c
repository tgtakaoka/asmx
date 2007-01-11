// asmx.c - copyright 1998-2007 Bruce Tomlin

#include "asmx.h"

#define versionName "asmx multi-assembler"

//#define ENABLE_REP    // uncomment to enable REPEAT pseudo-op (still under development)
//#define DOLLAR_SYM    // allow symbols to start with '$' (incompatible with $ for hexadecimal constants!)

#define versionNum "2.0a1"
#define copyright "Copyright 1998-2007 Bruce Tomlin"
#define IHEX_SIZE   32          // max number of data bytes per line in hex object file
#define MAXSYMLEN   19          // max symbol length (only used in DumpSym())
const int symTabCols = 3;       // number of columns for symbol table dump
#define MAXMACPARMS 30          // maximum macro parameters
#define MAX_INCLUDE 10          // maximum INCLUDE nesting level
//#define MAX_BYTSTR  1024        // size of bytStr[] (moved to asmx.h)
#define MAX_COND    256         // maximum nesting level of IF blocks
#define MAX_MACRO   10          // maximum nesting level of MACRO invocations

#if 0
// these should already be defined in sys/types.h (included from stdio.h)
typedef   unsigned char   u_char;
typedef   unsigned short  u_short;
typedef   unsigned int    u_int;
typedef   unsigned long   u_long;
#endif

#if 0 // moved to asmx.h
// a few useful typedefs
typedef unsigned char  bool;    // i guess bool is a c++ thing
//enum { FALSE, TRUE };
const bool FALSE = 0;
const bool TRUE  = 1;
typedef char Str255[256];       // generic string type
#endif

// --------------------------------------------------------------

const char      *progname;      // pointer to argv[0]

struct SymRec
{
    struct SymRec   *next;      // pointer to next symtab entry
    u_long          value;     // symbol value
    bool            defined;    // TRUE if defined
    bool            multiDef;   // TRUE if multiply defined
    bool            isSet;      // TRUE if defined with SET pseudo
    bool            equ;        // TRUE if defined with EQU pseudo
    bool            known;      // TRUE if value is known
    char            name[1];    // symbol name, storage = 1 + length
} *symTab = NULL;           // pointer to first entry in symbol table
typedef struct SymRec *SymPtr;

struct MacroLine
{
    struct MacroLine    *next;      // pointer to next macro line
    char                text[1];    // macro line, storage = 1 + length
};
typedef struct MacroLine *MacroLinePtr;

struct MacroParm
{
    struct MacroParm    *next;      // pointer to next macro parameter name
    char                name[1];    // macro parameter name, storage = 1 + length
};
typedef struct MacroParm *MacroParmPtr;

struct MacroRec
{
    struct MacroRec     *next;      // pointer to next macro
    bool                def;        // TRUE after macro is defined in pass 2
    bool                toomany;    // TRUE if too many parameters in definition
    MacroLinePtr        text;       // macro text
    MacroParmPtr        parms;      // macro parms
    int                 nparms;     // number of macro parameters
    char                name[1];    // macro name, storage = 1 + length
} *macroTab = NULL;             // pointer to first entry in macro table
typedef struct MacroRec *MacroPtr;

struct SegRec
{
    struct SegRec       *next;      // pointer to next segment
//  bool                gen;        // FALSE to supress code output (not currently implemented)
    u_long              loc;       // locptr for this segment
    u_long              cod;       // codptr for this segment
    char                name[1];    // segment name, storage = 1 + length
} *segTab = NULL;               // pointer to first entry in macro table
typedef struct SegRec *SegPtr;

#if 0 // moved to asmx.h
typedef char OpcdStr[maxOpcdLen+1];
struct OpcdRec
{
    OpcdStr         name;       // opcode name
    short           typ;        // opcode type
    u_short         parm;       // opcode parameter
};
typedef struct OpcdRec *OpcdPtr;
#endif

int             macroCondLevel;     // current IF nesting level inside a macro definition
int             macUniqueID;        // unique ID, incremented per macro invocation
int             macLevel;           // current macro nesting level
int             macCurrentID[MAX_MACRO]; // current unique ID
MacroPtr        macPtr[MAX_MACRO];  // current macro in use
MacroLinePtr    macLine[MAX_MACRO]; // current macro text pointer
int             numMacParms[MAX_MACRO];  // number of macro parameters
Str255          macParmsLine[MAX_MACRO]; // text of current macro parameters
char            *macParms[MAXMACPARMS * MAX_MACRO]; // pointers to current macro parameters
#ifdef ENABLE_REP
int             macRepeat[MAX_MACRO]; // repeat count for REP pseudo-op
#endif

struct AsmRec
{
    struct AsmRec   *next;          // next AsmRec
    int             endian;         // endianness for this assembler
    int             addrWid;        // address bus witdth, ADDR_16 or ADDR_32
    int             listWid;        // listing hex area width, LIST_16 or LIST_24
    OpcdPtr         opcdTab;        // opcdTab[] for this assembler
    int             (*DoCPUOpcode) (int typ, int parm);
    int             (*DoCPULabelOp) (int typ, int parm, char *labl);
    void            (*PassInit) (void);
    char            name[1];        // name of this assembler
};
typedef struct AsmRec *AsmPtr;

struct CpuRec
{
    struct CpuRec   *next;          // next CpuRec
    AsmPtr          as;             // assembler for CPU type
    int             index;          // CPU type index for assembler
    char            name[1];        // all-uppercase name of CPU
};
typedef struct CpuRec *CpuPtr;

// --------------------------------------------------------------

SegPtr          curSeg;             // current segment
SegPtr          nullSeg;            // default null segment

u_long          locPtr;             // Current program address
u_long          codPtr;             // Current program "real" address
int             pass;               // Current assembler pass
bool            warnFlag;           // TRUE if warning occurred this line
bool            errFlag;            // TRUE if error occurred this line
int             errCount;           // Total number of errors

Str255          line;               // Current line from input file
char           *linePtr;            // pointer into current line
Str255          listLine;           // Current listing line
bool            listLineFF;         // TRUE if an FF was in the current listing line
bool            listFlag;           // FALSE to suppress listing source
bool            listThisLine;       // TRUE to force listing this line
bool            sourceEnd;          // TRUE when END pseudo encountered
Str255          lastLabl;           // last label for '@' temp labels
Str255          subrLabl;           // current SUBROUTINE label for '.' temp labels
bool            listMacFlag;        // FALSE to suppress showing macro expansions
bool            macLineFlag;        // TRUE if line came from a macro
int             linenum;            // line number in main source file
bool            expandHexFlag;      // TRUE to expand long hex data to multiple listing lines
bool            symtabFlag;         // TRUE to show symbol table in listing

int             condLevel;          // current IF nesting level
int             condFail;           // highest non-failed IF nesting level
char            condElse[MAX_COND]; // TRUE if IF encountered on each nesting level

int             instrLen;           // Current instruction length (negative to display as long DB)
u_char          bytStr[MAX_BYTSTR]; // Current instruction / buffer for long DB statements
int             hexSpaces;          // flags for spaces in hex output for instructions
bool            showAddr;           // TRUE to show LocPtr on listing
u_long          xferAddr;           // Transfer address from END pseudo
bool            xferFound;          // TRUE if xfer addr defined w/ END

//  Command line parameters
Str255          cl_SrcName;         // Source file name
Str255          cl_ListName;        // Listing file name
Str255          cl_ObjName;         // Object file name
bool            cl_Err;             // TRUE for errors to screen
bool            cl_Warn;            // TRUE for warnings to screen
bool            cl_List;            // TRUE to generate listing file
bool            cl_Obj;             // TRUE to generate object file
bool            cl_S9;              // TRUE to generate an S-record object file instead of Intel hex
int             cl_S9type;          // type of S9 file: 9, 19, 28, or 37
bool            cl_Stdout;          // TRUE to send object file to stdout

FILE            *source;            // source input file
FILE            *object;            // object output file
FILE            *listing;           // listing output file
FILE            *(include[MAX_INCLUDE]);    // include files
Str255          incname[MAX_INCLUDE];       // include file names
int             incline[MAX_INCLUDE];       // include line number
int             nInclude;           // current include file index

bool            evalKnown;          // TRUE if all operands in Eval were "known"

AsmPtr          asmTab;             // list of all assemblers
CpuPtr          cpuTab;             // list of all CPU types
AsmPtr          curAsm;             // current assembler
int             curCPU;             // current CPU index for current assembler

int             endian;             // CPU endian: UNKNOWN_END, LITTLE_END, BIG_END
int             addrWid;            // CPU address width: ADDR_16, ADDR_32
int             listWid;            // listing hex area width: LIST_16, LIST_24
int             addrMax;            // maximum addrWid used
OpcdPtr         opcdTab;            // current CPU's opcode table
Str255          defCPU;             // default CPU name

// --------------------------------------------------------------

enum
{
//  0x00-0xFF = CPU-specific opcodes

//    o_Illegal = 0x100,  // opcode not found in FindOpcode

    o_DB = o_Illegal + 1,       // DB pseudo-op
    o_DW,       // DW pseudo-op
    o_DL,       // DL pseudo-op
    o_DWRE,     // reverse-endian DW
    o_DS,       // DS pseudo-op
    o_HEX,      // HEX pseudo-op
    o_FCC,      // FCC pseudo-op
    o_ZSCII,    // ZSCII pseudo-op
    o_ASCIIC,   // counted DB pseudo-op
    o_ALIGN,    // ALIGN pseudo-op
    o_ALIGN_n,  // for EVEN pseudo-op

    o_END,      // END pseudo-op
    o_Include,  // INCLUDE pseudo-op

    o_ENDM,     // ENDM pseudo-op
#ifdef ENABLE_REP
    o_REPEND,   // REPEND pseudo-op
#endif
    o_MacName,  // Macro name
    o_Processor,// CPU selection pseudo-op

//    o_LabelOp = 0x1000,   // flag to handle opcode in DoLabelOp

//  0x1000-0x10FF = CPU-specific label-ops

    // the following pseudo-ops handle the label field specially
//    o_EQU = o_LabelOp + 0x100,      // EQU and SET pseudo-ops
    o_ORG = o_EQU + 1,      // ORG pseudo-op
    o_RORG,     // RORG pseudo-op
    o_REND,     // REND pseudo-op
    o_LIST,     // LIST pseudo-op
    o_OPT,      // OPT pseudo-op
    o_ERROR,    // ERROR pseudo-op
    o_MACRO,    // MACRO pseudo-op
#ifdef ENABLE_REP
    o_REPEAT,   // REPEAT pseudo-op
#endif

    o_SEG,      // SEG pseudo-op
    o_SUBR,     // SUBROUTINE pseudo-op

    o_IF,       // IF <expr> pseudo-op
    o_ELSE,     // ELSE pseudo-op
    o_ELSIF,    // ELSIF <expr> pseudo-op
    o_ENDIF     // ENDIF pseudo-op
};

struct OpcdRec opcdTab2[] =
{
    {"DB",        o_DB,       0},
    {"FCB",       o_DB,       0},
    {"BYTE",      o_DB,       0},
    {"DC.B",      o_DB,       0},
    {"DFB",       o_DB,       0},
    {"DEFB",      o_DB,       0},

    {"DW",        o_DW,       0},
    {"FDB",       o_DW,       0},
    {"WORD",      o_DW,       0},
    {"DC.W",      o_DW,       0},
    {"DFW",       o_DW,       0},
    {"DA",        o_DW,       0},
    {"DEFW",      o_DW,       0},
    {"DRW",       o_DWRE,     0},

    {"LONG",      o_DL,       0},
    {"DL",        o_DL,       0},
    {"DC.L",      o_DL,       0},

    {"DS",        o_DS,       1},
    {"DS.B",      o_DS,       1},
    {"RMB",       o_DS,       1},
    {"BLKB",      o_DS,       1},
    {"DEFS",      o_DS,       1},
    {"DS.W",      o_DS,       2},
    {"BLKW",      o_DS,       2},
    {"DS.L",      o_DS,       4},
    {"HEX",       o_HEX,      0},
    {"FCC",       o_FCC,      0},
    {"ZSCII",     o_ZSCII,    0},
    {"ASCIIC",    o_ASCIIC,   0},
    {"END",       o_END,      0},
    {"ENDM",      o_ENDM,     0},
    {"ALIGN",     o_ALIGN,    0},
    {"EVEN",      o_ALIGN_n,  2},
#ifdef ENABLE_REP
    {"REPEND",    o_REPEND,   0},
#endif
    {"INCLUDE",   o_Include,  0},
    {"PROCESSOR", o_Processor,0},
    {"CPU",       o_Processor,0},

    {"=",         o_EQU,      0},
    {"EQU",       o_EQU,      0},
    {":=",        o_EQU,      1},
    {"SET",       o_EQU,      1},
    {"DEFL",      o_EQU,      1},
    {"ORG",       o_ORG,      0},
    {"AORG",      o_ORG,      0},
    {"RORG",      o_RORG,     0},
    {"REND",      o_REND,     0},
    {"LIST",      o_LIST,     0},
    {"OPT",       o_OPT,      0},
    {"ERROR",     o_ERROR,    0},
#ifdef ENABLE_REP
    {"REPEAT",    o_REPEAT,   0},
#endif
    {"MACRO",     o_MACRO,    0},
    {"SEG",       o_SEG,      1},
    {"RSEG",      o_SEG,      1},
    {"SEG.U",     o_SEG,      0},
    {"SUBR",      o_SUBR,     0},
    {"SUBROUTINE",o_SUBR,     0},
    {"IF",        o_IF,       0},
    {"ELSE",      o_ELSE,     0},
    {"ELSIF",     o_ELSIF,    0},
    {"ENDIF",     o_ENDIF,    0},

    {"",          o_Illegal,  0}
};


// --------------------------------------------------------------

#ifdef ENABLE_REP
void DoLine(void);          // forward declaration
#endif

// --------------------------------------------------------------

// multi-assembler call vectors

int DoCPUOpcode(int typ, int parm)
{
    if (curAsm) return curAsm->DoCPUOpcode(typ,parm);
           else return 0;
}


int DoCPULabelOp(int typ, int parm, char *labl)
{
    if (curAsm) return curAsm->DoCPULabelOp(typ,parm,labl);
           else return 0;
}


void PassInit(void)
{
    AsmPtr p;

    // for each assembler, call PassInit
    p = asmTab;
    while(p)
    {
        p -> PassInit();
        p = p -> next;
    }
}


char *AddAsm(char *name,        // assembler name
              int endian,       // assembler endian
              int addrWid,      // assembler 32-bit
              int listWid,      // listing width
              struct OpcdRec opcdTab[],  // assembler opcode table
              int (*DoCPUOpcode) (int typ, int parm),
              int (*DoCPULabelOp) (int typ, int parm, char *labl),
              void (*PassInit) (void) )
{
    AsmPtr p;

    p = malloc(sizeof *p + strlen(name));

    strcpy(p -> name, name);
    p -> next     = asmTab;
    p -> endian   = endian;
    p -> addrWid  = addrWid;
    p -> listWid  = listWid;
    p -> opcdTab  = opcdTab;
    p -> DoCPUOpcode  = DoCPUOpcode;
    p -> DoCPULabelOp = DoCPULabelOp;
    p -> PassInit = PassInit;

    asmTab = p;

    return (char *) p;
}

void AddCPU(char *as,           // assembler for this CPU
            char *name,         // uppercase name of this CPU
            int index)          // index number for this CPU
{
    CpuPtr p;

    p = malloc(sizeof *p + strlen(name));

    strcpy(p -> name, name);
    p -> next  = cpuTab;
    p -> as    = (AsmPtr) as;
    p -> index = index;

    cpuTab = p;
}


CpuPtr FindCPU(char *cpuName)
{
    CpuPtr p;

    p = cpuTab;
    while (p)
    {
        if (strcmp(cpuName,p->name) == 0)
            return p;
        p = p -> next;
    }

    return NULL;
}


void CodeFlush(void);
// sets up curAsm and curCpu based on cpuName, returns non-zero if success
bool SetCPU(char *cpuName)
{
    CpuPtr p;

    p = FindCPU(cpuName);
    if (p)
    {
        curCPU  = p -> index;
        curAsm  = p -> as;
        endian  = p -> as -> endian;
        opcdTab = p -> as -> opcdTab;
        addrWid = p -> as -> addrWid;
        listWid = p -> as -> listWid;

        CodeFlush();    // make a visual change in the hex object file

        return 1;
    }

    return 0;
}


int isalphanum(char c);
void Uprcase(char *s);
void AsmInit(void)
{
    char *p;

    extern void Asm1802Init(void);
    extern void Asm6502Init(void);
    extern void Asm68KInit(void);
    extern void Asm6809Init(void);
    extern void Asm68HC11Init(void);
    extern void Asm68HC16Init(void);
    extern void Asm8051Init(void);
    extern void Asm8085Init(void);
    extern void AsmF8Init(void);
    extern void AsmZ80Init(void);

    Asm1802Init();
    Asm6502Init();
    Asm68KInit();
    Asm6809Init();
    Asm68HC11Init();
    Asm68HC16Init();
    Asm8051Init();
    Asm8085Init();
    AsmF8Init();
    AsmZ80Init();

//  strcpy(defCPU,"Z80");     // hard-coded default for testing

    strcpy(line,progname);
    Uprcase(line);

    // try to find the CPU name in the executable's name
    p = line + strlen(line);    // start at end of executable name

    while (p > line && isalphanum(p[-1]))
        p--;                    // skip back to last non alpha-numeric character
    if (!isalphanum(*p)) p++;   // advance past last non alpha-numeric character

    if (p[0] == 'A' && p[1] == 'S' && p[2] == 'M')
        p = p + 3;              // skip leading "ASM"

    // for each substring, try to find a matching CPU name
    while (*p)
    {
        if (FindCPU(p))
        {
            strcpy(defCPU,p);
            return;
        }
        p++;
    }
}


// --------------------------------------------------------------
// error messages


/*
 *  Error
 */

void Error(char *message)
{
    char *name;
    int line;

    errFlag = TRUE;
    errCount++;

    name = cl_SrcName;
    line = linenum;
    if (nInclude >= 0)
    {
        name = incname[nInclude];
        line = incline[nInclude];
    }

    if (pass == 2)
    {
        listThisLine = TRUE;
        if (cl_List)    fprintf(listing, "%s:%d: *** Error:  %s ***\n",name,line,message);
        if (cl_Err)     fprintf(stderr,  "%s:%d: *** Error:  %s ***\n",name,line,message);
    }
}


/*
 *  Warning
 */

void Warning(char *message)
{
    char *name;
    int line;

    warnFlag = TRUE;

    name = cl_SrcName;
    line = linenum;
    if (nInclude >= 0)
    {
        name = incname[nInclude];
        line = incline[nInclude];
    }

    if (pass == 2 && cl_Warn)
    {
        listThisLine = TRUE;
        if (cl_List)    fprintf(listing, "%s:%d: *** Warning:  %s ***\n",name,line,message);
        if (cl_Warn)    fprintf(stderr,  "%s:%d: *** Warning:  %s ***\n",name,line,message);
    }
}


// --------------------------------------------------------------
// string utilities


/*
 *  Debleft deblanks the string s from the left side
 */

void Debleft(char *s)
{
    char *p = s;

    while (*p == 9 || *p == ' ')
        p++;

    if (p != s)
        while ((*s++ = *p++));
}


/*
 *  Debright deblanks the string s from the right side
 */

void Debright(char *s)
{
    char *p = s + strlen(s);

    while (p>s && *--p == ' ')
        *p = 0;
}


/*
 *  Deblank removes blanks from both ends of the string s
 */

void Deblank(char *s)
{
    Debleft(s);
    Debright(s);
}


/*
 *  Uprcase converts string s to upper case
 */

void Uprcase(char *s)
{
    char *p = s;

    while ((*p = toupper(*p)))
        p++;
}


int ishex(char c)
{
    c = toupper(c);
    return isdigit(c) || ('A' <= c && c <= 'F');
}


int isalphanum(char c)
{
    c = toupper(c);
    return isdigit(c) || ('A' <= c && c <= 'Z') || c == '_';
}


u_int EvalBin(char *binStr)
{
    u_int   binVal;
    int     evalErr;
    int     c;

    evalErr = FALSE;
    binVal  = 0;

    while ((c = *binStr++))
    {
        if (c < '0' || c > '1')
            evalErr = TRUE;
        else
            binVal = binVal * 2 + c - '0';
    }

    if (evalErr)
    {
      binVal = 0;
      Error("Invalid binary number");
    }

   return binVal;
}


u_int EvalOct(char *octStr)
{
    u_int   octVal;
    int     evalErr;
    int     c;

    evalErr = FALSE;
    octVal  = 0;

    while ((c = *octStr++))
    {
        if (c < '0' || c > '7')
            evalErr = TRUE;
        else
            octVal = octVal * 8 + c - '0';
    }

    if (evalErr)
    {
      octVal = 0;
      Error("Invalid octal number");
    }

   return octVal;
}


u_int EvalDec(char *decStr)
{
    u_int   decVal;
    int     evalErr;
    int     c;

    evalErr = FALSE;
    decVal  = 0;

    while ((c = *decStr++))
    {
        if (!isdigit(c))
            evalErr = TRUE;
        else
            decVal = decVal * 10 + c - '0';
    }

    if (evalErr)
    {
      decVal = 0;
      Error("Invalid decimal number");
    }

   return decVal;
}


int Hex2Dec(c)
{
    c = toupper(c);
    if (c > '9')
        return c - 'A' + 10;
    return c - '0';
}


u_int EvalHex(char *hexStr)
{
    u_int   hexVal;
    int     evalErr;
    int     c;

    evalErr = FALSE;
    hexVal  = 0;

    while ((c = *hexStr++))
    {
        if (!ishex(c))
            evalErr = TRUE;
        else
            hexVal = hexVal * 16 + Hex2Dec(c);
    }

    if (evalErr)
    {
      hexVal = 0;
      Error("Invalid hexadecimal number");
    }

    return hexVal;
}


u_int EvalNum(char *word)
{
    int val;

    // handle C-style 0xnnnn hexadecimal constants
    if(word[0] == '0')
    {
        if (toupper(word[1]) == 'X')
        {
            return EvalHex(word+2);
        }
        // return EvalOct(word);    // 0nnn octal constants are in less demand, though
    }

    val = strlen(word) - 1;
    switch(word[val])
    {
        case 'B':
            word[val] = 0;
            val = EvalBin(word);
            break;

        case 'O':
            word[val] = 0;
            val = EvalOct(word);
            break;

        case 'D':
            word[val] = 0;
            val = EvalDec(word);
            break;

        case 'H':
            word[val] = 0;
            val = EvalHex(word);
            break;

        default:
            val = EvalDec(word);
            break;
    }

    return val;
}


// --------------------------------------------------------------
// listing hex output routines

char * ListStr(char *l, char *s)
{
    // copy string to line
    while (*s) *l++ = *s++;

    return l;
}


char * ListByte(char *p, u_char b)
{
    char s[16]; // with extra space for just in case

    sprintf(s,"%.2X",b);
    return ListStr(p,s);
}


char * ListWord(char *p, u_short w)
{
    char s[16]; // with extra space for just in case

    sprintf(s,"%.4X",w);
    return ListStr(p,s);
}


char * ListLong(char *p, u_long l)
{
    char s[16]; // with extra space for just in case

    sprintf(s,"%.8X",(unsigned int) l);
    return ListStr(p,s);
}


char * ListAddr(char *p,u_long addr)
{
    if (addrWid == ADDR_32) // you need listWid == LIST_24 with this too
        p = ListLong(p,addr);
    else
        p = ListWord(p,addr);

    return p;
}

char * ListLoc(u_long addr)
{
    char *p;

    p = ListAddr(listLine,addr);
    *p++ = ' ';
    if (listWid == LIST_24 && addrWid != ADDR_32)
        *p++ = ' ';

    return p;
}

// --------------------------------------------------------------
// ZSCII conversion routines

    u_char  zStr[MAX_BYTSTR];   // output data buffer
    int     zLen;               // length of output data
    int     zOfs,zPos;          // current output offset (in bytes) and bit position
    int     zShift;             // current shift lock status (0, 1, 2)
    char    zSpecial[] = "0123456789.,!?_#'\"/\\<-:()"; // special chars table

void InitZSCII(void)
{
    zOfs = 0;
    zPos = 0;
    zLen = 0;
    zShift = 0;
}


void PutZSCII(char nib)
{
    nib = nib & 0x1F;

    // is it time to start a new word?
    if (zPos == 3)
    {
        // check for overflow
        if (zOfs >= MAX_BYTSTR)
        {
            if (!errFlag) Error("ZSCII string length overflow");
            return;
        }
        zOfs = zOfs + 2;
        zPos = 0;
    }

    switch(zPos)
    {
        case 0:
            zStr[zOfs] = nib << 2;
            break;

        case 1:
            zStr[zOfs] |= nib >> 3;
            zStr[zOfs+1] = nib << 5;
            break;

        case 2:
            zStr[zOfs+1] |= nib;
            break;

        default:    // this shouldn't happen!
            break;
    }

    zPos++;
}


void PutZSCIIShift(char shift, char nshift)
{
    int lock;

    lock = 0;                       // only do a temp shift if next shift is different
    if (shift == nshift) lock = 2;  // generate shift lock if next shift is same

    switch ((shift - zShift + 3) % 3)
    {
        case 0: // no shift
            break;

        case 1: // shift forward
            PutZSCII(0x02 + lock);
            break;

        case 2: // shift backwards
            PutZSCII(0x03 + lock);
            break;
    }

    if (lock) zShift = shift;       // update shift lock state
}


void EndZSCII(void)
{
    // pad final word with shift nibbles
    while (zPos != 3)
        PutZSCII(0x05);

    // set high bit in final word as end-of-text marker
    zStr[zOfs] |= 0x80;

    zOfs = zOfs + 2;
}


int GetZSCIIShift(char ch)
{
        if (ch == ' ')  return -2;
        if (ch == '\n') return -1;
        if ('a' <= ch && ch <= 'z') return 0;
        if ('A' <= ch && ch <= 'Z') return 1;
        return 2;
}


void ConvertZSCII(void)
{
    //  input data is in bytStr[]
    //  input data length is -instrLen

    int     i,inpos;            // input position
    char    ch;                 // current and next input byte
    char    *p;                 // pointer for looking up special chars
    int     shift,nshift;       // current and next shift states

    InitZSCII();

    inpos = 0;
    while (inpos < -instrLen)
    {
        // get current char and shift
        ch = bytStr[inpos];
        shift = GetZSCIIShift(ch);

        // determine next char's shift (skipping blanks and newlines, stopping at end of data)
        i = inpos + 1;
        while (i < -instrLen && (nshift = GetZSCIIShift(bytStr[i])) < 0) i++;
        if (i >= -instrLen) nshift = zShift;    // if end of data, use current shift as "next" shift

        switch(shift)
        {
            case 0: // alpha lower case
            case 1: // alpha upper case
                PutZSCIIShift(shift,nshift);
                PutZSCII(ch - 'A' + 6);
                break;

            case 2: // non-alpha
                if ((p = strchr(zSpecial,ch)))
                {   // numeric and special chars
                    PutZSCIIShift(shift,nshift);
                    PutZSCII(p - zSpecial + 7);
                }
                else
                {   // extended char
                    PutZSCIIShift(shift,nshift);
                    PutZSCII(0x06);
                    PutZSCII(ch >> 5);
                    PutZSCII(ch);
                }
                break;

            default: // blank and newline
                PutZSCII(shift + 2);
                break;
        }

        inpos++;
    }

    EndZSCII();

    memcpy(bytStr,zStr,zOfs);
    instrLen = -zOfs;
}


// --------------------------------------------------------------
// token handling

// returns 0 for end-of-line, -1 for alpha-numeric, else char value for non-alphanumeric
// converts the word to uppercase, too
int GetWord(char *word)
{
    u_char  c;

    word[0] = 0;

    // skip initial whitespace
    c = *linePtr;
    while (c == 12 || c == '\t' || c == ' ')
        c = *++linePtr;

    // skip comments
    if (c == ';')
        while (c)
            c = *++linePtr;

    // test for end of line
    if (c)
    {
        // test for alphanumeric token
#ifdef DOLLAR_SYM
        if (isalphanum(c) || (c == '$' && (isalphanum(linePtr[1]) || linePtr[1]=='$')))
#else
        if (isalphanum(c))
#endif
        {
            while (isalphanum(c) || c == '$')
            {
                *word++ = toupper(c);
                c = *++linePtr;
            }
            *word = 0;
            return -1;
        }
        else
        {
            word[0] = c;
            word[1] = 0;
            linePtr++;
            return c;
        }
    }

    return 0;
}


// same as GetWord, except it allows '.' chars in alphanumerics and ":=" as a token
int GetOpcode(char *word)
{
    u_char  c;

    word[0] = 0;

    // skip initial whitespace
    c = *linePtr;
    while (c == 12 || c == '\t' || c == ' ')
        c = *++linePtr;

    // skip comments
    if (c == ';')
        while (c)
            c = *++linePtr;

    // test for ":="
    if (c == ':' && linePtr[1] == '=')
    {
        word[0] = ':';
        word[1] = '=';
        word[2] = 0;
        linePtr = linePtr + 2;
        return -1;
    }

    // test for end of line
    else if (c)
    {
        // test for alphanumeric token
        if (isalphanum(c) || c=='.')
        {
            while (isalphanum(c) || c=='.')
            {
                *word++ = toupper(c);
                c = *++linePtr;
            }
            *word = 0;
            return -1;
        }
        else
        {
            word[0] = c;
            word[1] = 0;
            linePtr++;
            return c;
        }
    }

    return 0;
}


bool Expect(char *expected)
{
    Str255 s;
    GetWord(s);
    if (strcmp(s,expected) != 0)
    {
        sprintf(s,"\"%s\" expected",expected);
        Error(s);
        return 1;
    }
    return 0;
}


bool Comma()
{
    return Expect(",");
}


bool RParen()
{
    return Expect(")");
}


void EatIt()
{
    Str255  word;
    while (GetWord(word));      // eat junk at end of line
}


/*
 *  IllegalOperand
 */

void IllegalOperand()
{
    Error("Illegal operand");
    EatIt();
}


/*
 *  MissingOperand
 */

void MissingOperand()
{
    Error("Missing operand");
    EatIt();
}


/*
 *  BadMode
 */
void BadMode()
{
    Error("Illegal addressing mode");
    EatIt();
}


// find a register name
// regList is a space-separated list of register names
// FindReg returns:
//      -1 (aka reg_None) if no register found
//      0 if regName is the first register in regList
//      1 if regName is the second register in regList
//      etc.
int FindReg(const char *regName, const char *regList)
{
    const char *p;
    int i;

    i = 0;
    while (*regList)
    {
        p = regName;
        // compare words
        while (*p && *p == *regList)
        {
            regList++;
            p++;
        }

        // if not match, skip rest of word
        if (*p || (*regList != 0 && *regList != ' '))
        {
            // skip to next whitespace
            while (*regList && *regList != ' ')
                regList++;
            // skip to next word
            while (*regList == ' ')
                regList++;
            i++;
        }
        else return i;
    }

    return reg_None;
}


// get a word and find a register name
// regList is a space-separated list of register names
// GetReg returns:
//      -2 (aka reg_EOL) and reports a "missing operand" error if end of line
//      -1 (aka reg_None) if no register found
//      0 if regName is the first register in regList
//      1 if regName is the second register in regList
//      etc.
int GetReg(const char *regList)
{
    Str255  word;

    if (!GetWord(word))
    {
        MissingOperand();
        return reg_EOL;
    }

    return FindReg(word,regList);
}


u_int GetBackslashChar(void)
{
    u_char      ch;
    Str255      s;

    if (*linePtr)
    {
        ch = *linePtr++;
            if (ch == '\\' && *linePtr != 0) // backslash
        {
            ch = *linePtr++;
            switch(ch)
            {
                case 'r':   ch = '\r';   break;
                case 'n':   ch = '\n';   break;
                case 't':   ch = '\t';   break;
                case 'x':
                    if (ishex(linePtr[0]) && ishex(linePtr[1]))
                    {
                        s[0] = linePtr[0];
                        s[1] = linePtr[1];
                        s[2] = 0;
                        linePtr = linePtr + 2;
                        ch = EvalHex(s);
                    }
                    break;
                default:   break;
            }
        }
    }
    else ch = -1;

    return ch;
}


// --------------------------------------------------------------
// macro handling


MacroPtr FindMacro(char *name)
{
    MacroPtr p = macroTab;
    bool found = FALSE;

    while (p && !found)
    {
        found = (strcmp(p -> name, name) == 0);
        if (!found)
            p = p -> next;
    }

    return p;
}


MacroPtr NewMacro(char *name)
{
    MacroPtr    p;

    p = malloc(sizeof *p + strlen(name));

    if (p)
    {
        strcpy(p -> name, name);
        p -> def = FALSE;
        p -> toomany = FALSE;
        p -> text = NULL;
        p -> next = macroTab;
        p -> parms = NULL;
        p -> nparms = 0;
    }

    return p;
}
    

MacroPtr AddMacro(char *name)
{
    MacroPtr    p;

    p = NewMacro(name);
    if (p)
        macroTab = p;

    return p;
}
    

void AddMacroParm(MacroPtr macro, char *name)
{
    MacroParmPtr    parm;
    MacroParmPtr    p;

    parm = malloc(sizeof *parm + strlen(name));
    parm -> next = NULL;
    strcpy(parm -> name, name);
    macro -> nparms++;

    p = macro -> parms;
    if (p)
    {
        while (p -> next)
            p = p -> next;
        p -> next = parm;
    }
    else macro -> parms = parm;
}


void AddMacroLine(MacroPtr macro, char *line)
{
    MacroLinePtr    m;
    MacroLinePtr    p;

    m = malloc(sizeof *m + strlen(line));
    if (m)
    {
        m -> next = NULL;
        strcpy(m -> text, line);

        p = macro -> text;
        if (p)
        {
            while (p -> next)
                p = p -> next;
            p -> next = m;
        }
        else macro -> text = m;
    }
}


void GetMacParms(MacroPtr macro)
{
    int     i;
    int     n;
    int     quote;
    char    c;
    char    *p;
    bool    done;

    macCurrentID[macLevel] = macUniqueID++;

    for (i=0; i<MAXMACPARMS; i++)
        macParms[i + macLevel * MAXMACPARMS] = NULL;

    // skip initial whitespace
    c = *linePtr;
    while (c == 12 || c == '\t' || c == ' ')
        c = *++linePtr;

    // copy rest of line for safekeeping
    strcpy(macParmsLine[macLevel], linePtr);

    n = 0;
    p = macParmsLine[macLevel];
    while (*p && *p != ';' && n<MAXMACPARMS)
    {
        // skip whitespace before current parameter
        c = *p;
        while (c == 12 || c == '\t' || c == ' ')
            c = *++p;

        // record start of parameter
        macParms[n + macLevel * MAXMACPARMS] = p;
        n++;

        quote = 0;
        done = FALSE;
        // skip to end of parameter
        while(!done)
        {
            c = *p++;
            switch(c)
            {
                case 0:
                    --p;
                    done = TRUE;
                    break;

                case ';':
                    if (quote==0)
                    {
                        *--p = 0;
                        done = TRUE;
                    }
                    break;

                case ',':
                    if (quote==0)
                    {
                        *(p-1) = 0;
                        done = TRUE;
                    }
                    break;

                case 0x27:  // quote
                case '"':
                    if (quote == 0)
                        quote = c;
                    else if (quote == c)
                        quote = 0;
            }
        }
    }

    numMacParms[macLevel] = n;

    // terminate last parameter and point remaining parameters to null strings
    *p = 0;
    for (i=n; i<MAXMACPARMS; i++)
        macParms[i + macLevel * MAXMACPARMS] = p;

    // remove whitespace from end of parameter
    for (i=0; i<MAXMACPARMS; i++)
        if (macParms[i + macLevel * MAXMACPARMS])
        {
            p = macParms[i + macLevel * MAXMACPARMS] + strlen(macParms[i + macLevel * MAXMACPARMS]) - 1;
            while (p>=macParms[i + macLevel * MAXMACPARMS] && (*p == ' ' || *p == 9))
                *p-- = 0;
        }

    if (n > macro -> nparms || n > MAXMACPARMS)
        Error("Too many macro parameters");
}


void DoMacParms()
{
    int             i;
    Str255          word,word2;
    MacroParmPtr    parm;
    char            *p;     // pointer to start of word
    char            c;
    int             token;

    // start at beginning of line
    linePtr = line;

    // skip initial whitespace
    c = *linePtr;
    while (c == 12 || c == '\t' || c == ' ')
        c = *++linePtr;

    // while not end of line
    p = linePtr;
    token = GetWord(word);
    while (token)
    {
        // if alphanumeric, search for macro parameter of the same name
        if (token == -1)
        {
            i = 0;
            parm = macPtr[macLevel] -> parms;
            while (parm && strcmp(parm -> name, word))
            {
                parm = parm -> next;
                i++;
            }

            // if macro parameter found, replace parameter name with parameter value
            if (parm)
            {
                // copy from linePtr to temp string
                strcpy(word, linePtr);
                // copy from corresponding parameter to p
                strcpy(p, macParms[i + macLevel * MAXMACPARMS]);
                // point p to end of appended text
                p = p + strlen(macParms[i + macLevel * MAXMACPARMS]);
                // copy from temp to p
                strcpy(p, word);
                // update linePtr
                linePtr = p;
            }
        }
        // handle '##' concatenation operator
        else if (token == '#' && *linePtr == '#')
        {
            p = linePtr + 1;    // skip second '#'
            linePtr--;          // skip first '#'
            // skip whitespace to the left
            while (linePtr > line && linePtr[-1] == ' ')
                linePtr--;
            // skip whitespace to the right
            while (*p == ' ') p++;
            // copy right side of chopped zone
            strcpy(word, p);
            // paste it at new linePtr
            strcpy(linePtr, word);
            // and linePtr now even points to where it should
        }
        // handle '\0' number of parameters operator
        else if (token == '\\' && *linePtr == '0')
        {
            p = linePtr + 1;    // skip '0'
            linePtr--;          // skip '\'
            // make string of number of parameters
            sprintf(word2, "%d", numMacParms[macLevel]);
            // copy right side of chopped zone
            strcpy(word, p);
            // paste number
            strcpy(linePtr, word2);
            linePtr = linePtr + strlen(word2);
            // paste right side at new linePtr
            strcpy(linePtr, word);
        }
        // handle '\n' parameter operator
        else if (token == '\\' && '1' <= *linePtr && *linePtr <= '9')
        {
            i = *linePtr - '1';
            p = linePtr + 1;    // skip 'n'
            linePtr--;          // skip '\'
            // copy right side of chopped zone
            strcpy(word, p);
            // paste parameter
            strcpy(linePtr, macParms[i + macLevel * MAXMACPARMS]);
            linePtr = linePtr + strlen(macParms[i + macLevel * MAXMACPARMS]);
            // paste right side at new linePtr
            strcpy(linePtr, word);
        }
        // handle '\?' unique ID operator
        else if (token == '\\' && *linePtr == '?')
        {
            p = linePtr + 1;    // skip '?'
            linePtr--;          // skip '\'
            // make string of number of parameters
            sprintf(word2, "%.5d", macCurrentID[macLevel]);
            // copy right side of chopped zone
            strcpy(word, p);
            // paste number
            strcpy(linePtr, word2);
            linePtr = linePtr + strlen(word2);
            // paste right side at new linePtr
            strcpy(linePtr, word);
        }
/* just use "\##" instead to avoid any confusion with \\ inside of DB pseudo-op
        // handle '\\' escape
        else if (token == '\\' && *linePtr == '\\')
        {
            p = linePtr + 1;    // skip second '\'
            // copy right side of chopped zone
            strcpy(word, p);
            // paste right side at new linePtr
            strcpy(linePtr, word);
        }
*/

        // skip initial whitespace
        c = *linePtr;
        while (c == 12 || c == '\t' || c == ' ')
            c = *++linePtr;

        p = linePtr;
        token = GetWord(word);
    }
}


void DumpMacro(MacroPtr p)
{
    MacroLinePtr    line;
    MacroParmPtr    parm;

    if (cl_List)
    {

    fprintf(listing,"--- Macro '%s' ---", p -> name);
    fprintf(listing," def = %d, nparms = %d\n", p -> def, p -> nparms);

//  dump parms here
    fprintf(listing,"Parms:");
    for (parm = p->parms; parm; parm = parm->next)
    {
        fprintf(listing," '%s'",parm->name);
    }
    fprintf(listing,"\n");

//  dump text here
    for (line = p->text; line; line = line->next)
            fprintf(listing," '%s'\n",line->text);
    }
}


void DumpMacroTab(void)
{
    struct  MacroRec *p;

    p = macroTab;
    while (p)
    {
        DumpMacro(p);
        p = p -> next;
    }
}


// --------------------------------------------------------------
// opcodes and symbol table


/*
 *  FindOpcodeTab - finds an entry in an opcode table
 */

bool FindOpcodeTab(OpcdPtr p, char *name, int *typ, int *parm)

{
    bool found = FALSE;

//  while (p -> typ != o_Illegal && !found)
    while (*(p -> name) && !found)
    {
        found = (strcmp(p -> name, name) == 0);
        if (!found)
            p++;
        else
        {
            *typ  = p -> typ;
            *parm = p -> parm;
        }
    }

    return found;
}


/*
 *  FindOpcode - finds an opcode in either the generic or CPU-specific
 *               opcode tables, or as a macro name
 */

void FindOpcode(char *name, int *typ, int *parm, MacroPtr *macro)
{
    *typ   = o_Illegal;
    *parm  = 0;
    *macro = NULL;

    if (opcdTab && FindOpcodeTab(opcdTab,  name, typ, parm)) return;
    if (name[0] == '.') name++; // allow pseudo-ops to be invoked as ".OP"
    if (FindOpcodeTab((OpcdPtr) &opcdTab2, name, typ, parm)) return;
// if name[0] == '.' try calling SetCPU

    *macro = FindMacro(name);
    if (*macro)
        *typ = o_MacName;
}


/*
 *  FindSym
 */

SymPtr FindSym(char *symName)
{
    SymPtr p = symTab;
    bool found = FALSE;

    while (p && !found)
    {
        found = (strcmp(p -> name, symName) == 0);
        if (!found)
            p = p -> next;
    }

    return p;
}


/*
 *  AddSym
 */

SymPtr AddSym(char *symName)
{
    SymPtr p;

    p = malloc(sizeof *p + strlen(symName));

    strcpy(p -> name, symName);
    p -> value = 0;
    p -> next = symTab;
    p -> defined = FALSE;
    p -> multiDef = FALSE;
    p -> isSet = FALSE;
    p -> equ = FALSE;
    p -> known = FALSE;

    symTab = p;

    return p;
}



/*
 *  RefSym
 */

int RefSym(char *symName, bool *known)
{
    SymPtr p;
    int i;
    Str255 s;

    if ((p = FindSym(symName)))
    {
        if (!p -> defined)
        {
            sprintf(s, "Symbol '%s' undefined", symName);
            Error(s);
        }
        switch(pass)
        {
            case 1:
                if (!p -> defined) *known = FALSE;
                break;
            case 2:
                if (!p -> known) *known = FALSE;
                break;
        }
#if 0 // FIXME: possible fix that may be needed for 16-bit address
        if (addrWid == ADDR_16)
            return (short) p -> value;    // sign-extend from 16 bits
#endif
        return p -> value;
    }

    {   // check for 'FFH' style constants here

        i = strlen(symName)-1;
        if (toupper(symName[i]) != 'H')
            i = -1;
        else
            while (i>0 && ishex(symName[i-1]))
                i--;

        if (i == 0)
        {
            strncpy(s, symName, 255);
            s[strlen(s)-1] = 0;
            return EvalHex(s);
        }
        else
        {
            p = AddSym(symName);
            *known = FALSE;
//          sprintf(s, "Symbol '%s' undefined", symName);
//          Error(s);
        }
    }

    return 0;
}


/*
 *  DefSym
 */

void DefSym(char *symName, u_long val, bool setSym, bool equSym)
{
    SymPtr p;
    Str255 s;

    if (symName[0]) // ignore null string symName
    {
        p = FindSym(symName);
        if (p == NULL)
            p = AddSym(symName);

        if (!p -> defined || (p -> isSet && setSym))
        {
            p -> value = val;
            p -> defined = TRUE;
            p -> isSet = setSym;
            p -> equ = equSym;
        }
        else if (p -> value != val)
        {
            p -> multiDef = TRUE;
            if (pass == 2 && !p -> known)
                 sprintf(s, "Phase error");
            else sprintf(s, "Symbol '%s' multiply defined",symName);
            Error(s);
        }

        if (pass == 0 || pass == 2) p -> known = TRUE;
    }
}


void DumpSym(SymPtr p, char *s, int *w)
{
//
// #######
//
    char *s2;
    int n,len,max;

    n = 0;
    *w = 1;
    max = MAXSYMLEN;

    s2 = p->name;
    len = strlen(s2);

    while (max-1 < len)
    {
        *w = *w + 1;
        max = max + MAXSYMLEN + 8; // 8 = number of extra chars between symbol names
    }

    while(*s2 && n < max)
    {
        *s++ = *s2++;
        n++;
    }

    while (n < max)
    {
        *s++ = ' ';
        n++;
    }

    if (addrMax == ADDR_32)
        sprintf(s, "%.8lX ", p->value);
    else
        sprintf(s, "%.4lX ", p->value & 0xFFFF);

    s = s + strlen(s);

    n = 0;
    if (!p->defined)    {*s++ = 'U'; n++;}  // Undefined
    if ( p->multiDef)   {*s++ = 'M'; n++;}  // Multiply defined
    if ( p->isSet)      {*s++ = 'S'; n++;}  // Set
    if ( p->equ)        {*s++ = 'E'; n++;}  // Equ
    while (n < 3)
    {
        *s++ = ' ';
        n++;
    }
    *s = 0;
}


void DumpSymTab(void)
{
    struct  SymRec *p;
    int     i,w;
    Str255  s;

    i = 0;
    p = symTab;
    while (p)
    {
        DumpSym(p,s,&w);
        p = p -> next;

        // force a newline if new symbol won't fit on current line
        if (i+w > symTabCols)
        {
            if (cl_List)
                fprintf(listing, "\n");
            i = 0;
        }
        // if last symbol or if symbol fills line, deblank and print it
        if (p == NULL || i+w >= symTabCols)
        {
            Debright(s);
            if (cl_List)
                fprintf(listing, "%s\n", s);
            i = 0;
        }
        // otherwise just print it and count its width
        else
        {
            if (cl_List)
                fprintf(listing, "%s", s);
            i = i + w;
        }
    }
}


void SortSymTab()
{
    SymPtr          i,j;    // pointers to current elements
    SymPtr          ip,jp;  // pointers to previous elements
    SymPtr          t;      // temp for swapping

    // yes, it's a linked-list bubble sort

    if (symTab != NULL)
    {
        ip = NULL;  i = symTab;
        jp = i;     j = i -> next;

        while (j != NULL)
        {
            while (j != NULL)
            {
                if (strcmp(i->name,j->name) > 0)    // (i->name > j->name)
                {
                    if (ip != NULL) ip -> next = j;
                               else symTab     = j;
                    if (i == jp)
                    {
                        i -> next = j -> next;
                        j -> next = i;
                    }
                    else
                    {
                        jp -> next = i;

                        t         = i -> next;
                        i -> next = j -> next;
                        j -> next = t;
                    }
                    t = i; i = j; j = t;
                }
                jp = j;     j = j -> next;
            }
            ip = i;     i = i -> next;
            jp = i;     j = i -> next;
        }
    }
}


// --------------------------------------------------------------
// expression evaluation


int Eval0(void);        // forward declaration


int Factor(void)
{
    Str255      word,s;
    int         token;
    int         val;
    char        *oldLine;
    SymPtr      p;

    token = GetWord(word);
    val = 0;

    switch(token)
    {
        case 0:
            MissingOperand();
            break;

        case '%':
            GetWord(word);
            val = EvalBin(word);
            break;

        case '$':
            if (ishex(*linePtr))
            {
                GetWord(word);
                val = EvalHex(word);
                break;
            }
            // fall-through...
        case '*':
            val = locPtr;
#if 0 // FIXME: possible fix that may be needed for 16-bit address
        if (addrWid == ADDR_16)
            val = (short) val;            // sign-extend from 16 bits
#endif
            break;

        case '+':
            val = Factor();
            break;

        case '-':
            val = -Factor();
            break;

        case '~':
            val = ~Factor();
            break;

        case '!':
            val = !Factor();
            break;

        case '<':
            val = Factor() & 0xFF;
            break;

        case '>':
            val = (Factor() >> 8) & 0xFF;
            break;

        case '(':
            val = Eval0();
            RParen();
            break;

        case '[':
            val = Eval0();
            Expect("]");
            break;

        case 0x27:  // single quote
#if 1 // enable multi-char single-quote constants
            val = 0;
            while (*linePtr != 0x27 && *linePtr != 0)
            {
                val = val * 256 + GetBackslashChar();
            }
            if (*linePtr == 0x27)
                linePtr++;
            else
                Error("Missing close quote");
#else
            if ((val = GetBackslashChar()) >= 0)
            {
                if (*linePtr && *linePtr != 0x27)
                    val = val * 256 + GetBackslashChar();
                if (*linePtr == 0x27)
                    linePtr++;
                else
                    Error("Missing close quote");
            }
            else MissingOperand();
#endif
            break;

        case '.':
            // check for ".."
            oldLine = linePtr;
            val = GetWord(word);
            if (val == '.')
            {
                GetWord(word);
                // check for "..DEF" operator
                if (strcmp(word,"DEF") == 0)
                {
                    val = 0;
                    if (GetWord(word) == -1)
                    {
                        p = FindSym(word);
                        val = (p && p -> known);
                    }
                    else IllegalOperand();
                    break;
                }

                // check for "..UNDEF" operator
                if (strcmp(word,"UNDEF") == 0)
                {
                    val = 0;
                    if (GetWord(word) == -1)
                    {
                        p = FindSym(word);
                        val = !(p && p -> known);
                    }
                    else IllegalOperand();
                    break;
                }

                // invalid ".." operator
                // rewind and return "current location"
                linePtr = oldLine;
                break;
            }

            // check for '.' as "current location"
            else if (val != -1)
            {
                linePtr = oldLine;
                val = locPtr;
#if 0 // FIXME: possible fix that may be needed for 16-bit address
                if (addrWid == ADDR_16)
                    val = (short) val;    // sign-extend from 16 bits
#endif
                break;
            }

            // now try it as a local ".symbol"
            linePtr = oldLine;
            // fall-through...
        case '@':
            GetWord(word);
            if (token == '.' && subrLabl[0])    strcpy(s,subrLabl);
                                        else    strcpy(s,lastLabl);
            s[strlen(s)+1] = 0;
            s[strlen(s)]   = token;
            strcat(s,word);
            val = RefSym(s, &evalKnown);
            break;

        case -1:
            if ((word[0] == 'H' || word[0] == 'L') && word[1] == 0 && *linePtr == '(')
            {   // handle H() and L() from vintage Atari 7800 source code
                // note: no whitespace allowed before the open paren!
                token = word[0];    // save 'H' or 'L'
                GetWord(word);      // eat left paren
                val = Eval0();      // evaluate sub-expression
                RParen();           // check for right paren
                if (token == 'H') val = (val >> 8) & 0xFF;
                if (token == 'L') val = val & 0xFF;
                break;
            }
            if (isdigit(word[0]))   val = EvalNum(word);
                            else    val = RefSym(word,&evalKnown);
            break;

        default:
            // ignore anything else
            break;
    }

    return val;
}


int Term(void)
{
    Str255  word;
    int     token;
    int     val,val2;
    char    *oldLine;

    val = Factor();

    oldLine = linePtr;
    token = GetWord(word);
    while (token == '*' || token == '/' || token == '%')
    {
        switch(token)
        {
            case '*':   val = val * Factor();   break;
            case '/':   val2 = Factor();
                        if (val2)
                            val = val / val2;
                        else
                        {
                            Warning("Division by zero");
                            val = 0;
                        }
                        break;
            case '%':   val2 = Factor();
                        if (val2)
                            val = val % val2;
                        else
                        {
                            Warning("Division by zero");
                            val = 0;
                        }
                        break;
        }
        oldLine = linePtr;
        token = GetWord(word);
    }
    linePtr = oldLine;

    return val;
}


int Eval2(void)
{
    Str255  word;
    int     token;
    int     val;
    char    *oldLine;

    val = Term();

    oldLine = linePtr;
    token = GetWord(word);
    while (token == '+' || token == '-')
    {
        switch(token)
        {
            case '+':   val = val + Term();     break;
            case '-':   val = val - Term();     break;
        }
        oldLine = linePtr;
        token = GetWord(word);
    }
    linePtr = oldLine;

    return val;
}


int Eval1(void)
{
    Str255  word;
    int     token;
    int     val;
    char    *oldLine;

    val = Eval2();

    oldLine = linePtr;
    token = GetWord(word);
    while ((token == '<' && *linePtr != token) ||
            (token == '>' && *linePtr != token) ||
            token == '=' ||
            (token == '!' && *linePtr == '='))
    {
        switch(token)
        {
            case '<':   if (*linePtr == '=')    {linePtr++; val = (val <= Eval2());}
                                        else                val = (val <  Eval2());
                        break;
            case '>':   if (*linePtr == '=')    {linePtr++; val = (val >= Eval2());}
                                        else                val = (val >  Eval2());
                        break;
            case '=':   if (*linePtr == '=') linePtr++; // allow either one or two '=' signs
                        val = (val == Eval2());     break;
            case '!':   linePtr++;  val = (val != Eval2());     break;
        }
        oldLine = linePtr;
        token = GetWord(word);
    }
    linePtr = oldLine;

    return val;
}


int Eval0(void)
{
    Str255  word;
    int     token;
    int     val;
    char    *oldLine;

    val = Eval1();

    oldLine = linePtr;
    token = GetWord(word);
    while (token == '&' || token == '|' || token == '^' ||
            (token == '<' && *linePtr == '<') ||
            (token == '>' && *linePtr == '>'))
    {
        switch(token)
        {
            case '&':   if (*linePtr == '&') {linePtr++; val = ((val & Eval1()) != 0);}
                                        else             val =   val & Eval1();
                        break;
            case '|':   if (*linePtr == '|') {linePtr++; val = ((val | Eval1()) != 0);}
                                        else             val =   val | Eval1();
                        break;
            case '^':   val = val ^ Eval1();
                        break;
            case '<':   linePtr++;  val = val << Eval1();   break;
            case '>':   linePtr++;  val = val >> Eval1();   break;
        }
        oldLine = linePtr;
        token = GetWord(word);
    }
    linePtr = oldLine;

    return val;
}


int Eval(void)
{
    evalKnown = TRUE;

    return Eval0();
}


void CheckByte(int val)
{
    if (!errFlag && (val < -128 || val > 255))
        Warning("Byte out of range");
}


void CheckWord(int val)
{
    if (!errFlag && (val < -32768 || val > 65535))
        Warning("Word out of range");
}


int EvalByte(void)
{
    long val;

    val = Eval();
    CheckByte(val);

    return val & 0xFF;
}


int EvalBranch(int instrLen)
{
    long val;

    val = Eval();
    val = val - locPtr - instrLen;
    if (!errFlag && (val < -128 || val > 127))
        Error("Short branch out of range");

    return val & 0xFF;
}


int EvalWBranch(int instrLen)
{
    long val;

    val = Eval();
    val = val - locPtr - instrLen;
    if (!errFlag && (val < -32768 || val > 32767))
        Error("Word branch out of range");
    return val;
}


int EvalLBranch(int instrLen)
{
    long val;

    val = Eval();
    val = val - locPtr - instrLen;
    return val;
}


// --------------------------------------------------------------
// object file generation


    u_char ihex_buf[IHEX_SIZE];
    u_long ihex_len;   // length of current intel hex line
    u_long ihex_base;  // current address in intel hex buffer
    u_long ihex_addr;  // start address of current intel hex line
    u_short ihex_page;

// Intel hex format:
//
// :aabbbbccdddd...ddee
//
// aa    = record data length (the number of dd bytes)
// bbbb  = address for this record
// cc    = record type
//       00 = data (data is in the dd bytes)
//       01 = end of file (bbbb is transfer address)
//       02 = extended segment address record
//            dddd (two bytes) represents the segment address
//       03 = Start segment address record
//            dddd (two bytes) represents the segment of the transfer address
//       04 = extended linear address record
//            dddd (two bytes) represents the high address word
//       05 = Start linear address record
//            dddd (two bytes) represents the high word of the transfer address
// dd... = data bytes if record type needs it
// ee    = checksum byte: add all bytes aa through dd
//                        and subtract from 256 (2's complement negate)


void write_ihex(u_long addr, u_char *buf, int len, int rectype)
{
    int i,chksum;

    if (cl_Obj || cl_Stdout)
    {
        if (rectype == 1 && (addr & 0xFFFF0000))
            write_ihex(addr >> 16, buf, 0, 5);
        if (rectype == 0 && (addr >> 16) != ihex_page)
        {
            write_ihex(addr >> 16, buf, 0, 4);
            ihex_page = addr >> 16;
        }

        chksum = len + (addr >> 8) + addr + rectype;
        fprintf(object,":%.2X%.4lX%.2X",len,addr & 0xFFFF,rectype);
        for (i=0; i<len; i++)
        {
            fprintf(object,"%.2X",buf[i]);
            chksum = chksum + buf[i];
        }
        fprintf(object,"%.2X\n",(-chksum) & 0xFF);
    }
}


// Motorola S-record format:
//
// Sabbcc...ccdd...ddee
//
// a     = record type
//         0 starting record (optional)
//         1 data record with 16-bit address
//         2 data record with 24-bit address
//         3 data record with 32-bit address
//         4 symbol record (LSI extension) - S4<length><address><name>,<checksum>
//         5 number of data records in preceeding block
//         6 unused
//         7 ending record for S3 records
//         8 ending record for S2 records
//         9 ending record for S1 records
// bb    = record data length (from bb through ee)
// cc... = address for this record, 2 bytes for S1/S9, 3 bytes for S2/S8, 4 bytes for S3/S7
// dd... = data bytes if record type needs it
// ee    = checksum byte: add all bytes bb through dd
//                        and subtract from 255 (1's complement)

void write_srec(u_long addr, u_char *buf, int len, int rectype)
{
    int i,chksum;

    if (cl_Obj || cl_Stdout)
    {
        chksum = len+3 + ((addr >> 8) & 0xFF) + (addr & 0xFF);
        if (rectype)    i = cl_S9type % 10;  // xfer record = S9/S8/S7
                else    i = cl_S9type / 10;  // code record = S1/S2/S3
        switch(cl_S9type)
        {
            case 37:
                fprintf(object,"S%d%.2X%.8lX", i, len+5, addr);
                chksum = chksum + ((addr >> 24) & 0xFF) + ((addr >> 16) & 0xFF) + 2;
                break;

            case 28:
                fprintf(object,"S%d%.2X%.6lX", i, len+4, addr & 0xFFFFFF) + 1;
                chksum = chksum + ((addr >> 16) & 0xFF);
                break;

            default:
                if (i == 0) i = 1; // handle "-s9" option
                fprintf(object,"S%d%.2X%.4lX", i, len+3, addr & 0xFFFF);
        }
        for (i=0; i<len; i++)
        {
            fprintf(object,"%.2X",buf[i]);
            chksum = chksum + buf[i];
        }
        fprintf(object,"%.2X\n",~chksum & 0xFF);
    }
}


// rectype 0 = code, rectype 1 = xfer
void write_hex(u_long addr, u_char *buf, int len, int rectype)
{
    if (cl_S9)  write_srec(addr, buf, len, rectype);
        else    write_ihex(addr, buf, len, rectype);
}


void CodeInit(void)
{
    ihex_len  = 0;
    ihex_base = 0;
    ihex_addr = 0;
    ihex_page = 0;
}


void CodeFlush(void)
{
    if (ihex_len)
    {
        write_hex(ihex_base, ihex_buf, ihex_len, 0);
        ihex_len  = 0;
        ihex_base = ihex_base + ihex_len;
        ihex_addr = ihex_base;
    }
}


void CodeOut(int byte)
{
    if (pass == 2)
    {
        if (codPtr != ihex_addr)
        {
            CodeFlush();
            ihex_base = codPtr;
            ihex_addr = codPtr;
        }

        ihex_buf[ihex_len++] = byte;
        ihex_addr++;

        if (ihex_len == IHEX_SIZE)
            CodeFlush();
    }
    locPtr++;
    codPtr++;
}


void CodeEnd(void)
{
    CodeFlush();

    if (pass == 2)
    {
        if (xferFound)
            write_hex(xferAddr, ihex_buf, 0, 1);
    }
}


void CodeAbsOrg(int addr)
{
    codPtr = addr;
    locPtr = addr;
}


void CodeRelOrg(int addr)
{
    locPtr = addr;
}


void CodeXfer(int addr)
{
    xferAddr  = addr;
    xferFound = TRUE;
}


void AddLocPtr(int ofs)
{
    codPtr = codPtr + ofs;
    locPtr = locPtr + ofs;
}


// --------------------------------------------------------------
// instruction format calls

// clear the length of the instruction
void InstrClear(void)
{
    instrLen = 0;
    hexSpaces = 0;
}


// add a byte to the instruction
void InstrAddB(u_char b)
{
    bytStr[instrLen++] = b;
    hexSpaces |= 1<<instrLen;
}


// add a byte/word opcode to the instruction
// a big-endian word is added if the opcode is > 255
// this is for opcodes with pre-bytes
void InstrAddX(u_long op)
{
//  if ((op & 0xFFFFFF00) == 0) hexSpaces |= 1; // to indent single-byte instructions
//  if (op & 0xFF000000) bytStr[instrLen++] = op >> 24;
//  if (op & 0xFFFF0000) bytStr[instrLen++] = op >> 16;
    if (op & 0xFFFFFF00) bytStr[instrLen++] = op >>  8;
    bytStr[instrLen++] = op & 255;
    hexSpaces |= 1<<instrLen;
}


// add a word to the instruction in the CPU's endianness
void InstrAddW(u_short w)
{
    if (endian == LITTLE_END)
    {
        bytStr[instrLen++] = w & 255;
        bytStr[instrLen++] = w >> 8;
    }
    else if (endian == BIG_END)
    {
        bytStr[instrLen++] = w >> 8;
        bytStr[instrLen++] = w & 255;
    }
    else Error("CPU endian not defined");
    hexSpaces |= 1<<instrLen;
}


// add a 3-byte word to the instruction in the CPU's endianness
void InstrAdd3(u_long l)
{
    if (endian == LITTLE_END)
    {
        bytStr[instrLen++] =  l        & 255;
        bytStr[instrLen++] = (l >>  8) & 255;
        bytStr[instrLen++] = (l >> 16) & 255;
    }
    else if (endian == BIG_END)
    {
        bytStr[instrLen++] = (l >> 16) & 255;
        bytStr[instrLen++] = (l >>  8) & 255;
        bytStr[instrLen++] =  l        & 255;
    }
    else Error("CPU endian not defined");
    hexSpaces |= 1<<instrLen;
}


// add a longword to the instruction in the CPU's endianness
void InstrAddL(u_long l)
{
    if (endian == LITTLE_END)
    {
        bytStr[instrLen++] =  l        & 255;
        bytStr[instrLen++] = (l >>  8) & 255;
        bytStr[instrLen++] = (l >> 16) & 255;
        bytStr[instrLen++] = (l >> 24) & 255;
    }
    else if (endian == BIG_END)
    {
        bytStr[instrLen++] = (l >> 24) & 255;
        bytStr[instrLen++] = (l >> 16) & 255;
        bytStr[instrLen++] = (l >>  8) & 255;
        bytStr[instrLen++] =  l        & 255;
    }
    else Error("CPU endian not defined");
    hexSpaces |= 1<<instrLen;
}


void InstrB(u_char b1)
{
    InstrClear();
    InstrAddB(b1);
}


void InstrBB(u_char b1, u_char b2)
{
    InstrClear();
    InstrAddB(b1);
    InstrAddB(b2);
}


void InstrBBB(u_char b1, u_char b2, u_char b3)
{
    InstrClear();
    InstrAddB(b1);
    InstrAddB(b2);
    InstrAddB(b3);
}


void InstrBBBB(u_char b1, u_char b2, u_char b3, u_char b4)
{
    InstrClear();
    InstrAddB(b1);
    InstrAddB(b2);
    InstrAddB(b3);
    InstrAddB(b4);
}


void InstrBBBBB(u_char b1, u_char b2, u_char b3, u_char b4, u_char b5)
{
    InstrClear();
    InstrAddB(b1);
    InstrAddB(b2);
    InstrAddB(b3);
    InstrAddB(b4);
    InstrAddB(b5);
}


void InstrBW(u_char b1, u_short w1)
{
    InstrClear();
    InstrAddB(b1);
    InstrAddW(w1);
}


void InstrBBW(u_char b1, u_char b2, u_short w1)
{
    InstrClear();
    InstrAddB(b1);
    InstrAddB(b2);
    InstrAddW(w1);
}


void InstrBBBW(u_char b1, u_char b2, u_char b3, u_short w1)
{
    InstrClear();
    InstrAddB(b1);
    InstrAddB(b2);
    InstrAddB(b3);
    InstrAddW(w1);
}


void InstrX(u_long op)
{
    InstrClear();
    InstrAddX(op);
}


void InstrXB(u_long op, u_char b1)
{
    InstrClear();
    InstrAddX(op);
    InstrAddB(b1);
}


void InstrXBB(u_long op, u_char b1, u_char b2)
{
    InstrClear();
    InstrAddX(op);
    InstrAddB(b1);
    InstrAddB(b2);
}


void InstrXBBB(u_long op, u_char b1, u_char b2, u_char b3)
{
    InstrClear();
    InstrAddX(op);
    InstrAddB(b1);
    InstrAddB(b2);
    InstrAddB(b3);
}


void InstrXBBBB(u_long op, u_char b1, u_char b2, u_char b3, u_char b4)
{
    InstrClear();
    InstrAddX(op);
    InstrAddB(b1);
    InstrAddB(b2);
    InstrAddB(b3);
    InstrAddB(b4);
}


void InstrXW(u_long op, u_short w1)
{
    InstrClear();
    InstrAddX(op);
    InstrAddW(w1);
}


void InstrXBW(u_long op, u_char b1, u_short w1)
{
    InstrClear();
    InstrAddX(op);
    InstrAddB(b1);
    InstrAddW(w1);
}


void InstrXBWB(u_long op, u_char b1, u_short w1, u_char b2)
{
    InstrClear();
    InstrAddX(op);
    InstrAddB(b1);
    InstrAddW(w1);
    InstrAddB(b2);
}


void InstrXWW(u_long op, u_short w1, u_short w2)
{
    InstrClear();
    InstrAddX(op);
    InstrAddW(w1);
    InstrAddW(w2);
}


void InstrX3(u_long op, u_long l1)
{
    InstrClear();
    InstrAddX(op);
    InstrAdd3(l1);
}


void InstrW(u_short w1)
{
    InstrClear();
    InstrAddW(w1);
}

void InstrWW(u_short w1, u_short w2)
{
    InstrClear();
    InstrAddW(w1);
    InstrAddW(w2);
}

void InstrWL(u_short w1, u_long l1)
{
    InstrClear();
    InstrAddW(w1);
    InstrAddL(l1);
}


void InstrL(u_long l1)
{
    InstrClear();
    InstrAddL(l1);
}


// --------------------------------------------------------------
// segment handling


SegPtr FindSeg(char *name)
{
    SegPtr p = segTab;
    bool found = FALSE;

    while (p && !found)
    {
        found = (strcmp(p -> name, name) == 0);
        if (!found)
            p = p -> next;
    }

    return p;
}


SegPtr AddSeg(char *name)
{
    SegPtr  p;

    p = malloc(sizeof *p + strlen(name));

    p -> next = segTab;
//  p -> gen = TRUE;
    p -> loc = 0;
    p -> cod = 0;
    strcpy(p -> name, name);

    segTab = p;

    return p;
}


void SwitchSeg(SegPtr seg)
{
    CodeFlush();
    curSeg -> cod = codPtr;
    curSeg -> loc = locPtr;

    curSeg = seg;
    codPtr = curSeg -> cod;
    locPtr = curSeg -> loc;
}


// --------------------------------------------------------------
// text I/O


int OpenInclude(char *fname)
{
    if (nInclude == MAX_INCLUDE - 1)
        return -1;

    nInclude++;
    include[nInclude] = NULL;
    incline[nInclude] = 0;
    strcpy(incname[nInclude],fname);
    include[nInclude] = fopen(fname, "r");
    if (include[nInclude])
        return 1;

    nInclude--;
    return 0;
}


void CloseInclude(void)
{
    if (nInclude < 0)
        return;

    fclose(include[nInclude]);
    include[nInclude] = NULL;
    nInclude--;
}


int ReadLine(FILE *file, char *line, int max)
{
    int c = 0;
    int len = 0;

    macLineFlag = TRUE;

    // if at end of macro and inside a nested macro, pop the stack
    while (macLevel > 0 && macLine[macLevel] == NULL)
    {
#ifdef ENABLE_REP
        if (macRepeat[macLevel] > 0)
        {
            if (macRepeat[macLevel]--)
                macLine = macPtr[macLevel] -> text;
            else
            {
                free(macPtr[macLevel]);
                macLevel--;
            }
        }
        else
#endif
        macLevel--;
    }

    // if there is still another macro line to process, get it
    if (macLine[macLevel] != NULL)
    {
        strcpy(line, macLine[macLevel] -> text);
        macLine[macLevel] = macLine[macLevel] -> next;
        DoMacParms();
    }
    else
    {   // else we weren't in a macro or we just ran out of macro
        macLineFlag = FALSE;

        if (nInclude >= 0)
            incline[nInclude]++;
        else
            linenum++;

        macPtr[macLevel] = NULL;

        while (max > 1)
        {
            c = fgetc(file);
            *line = 0;
            switch(c)
            {
                case EOF:
                    if (len == 0) return 0;
                case '\n':
                    return 1;
                case '\r':
                    c = fgetc(file);
                    if (c != '\n')
                    {
                        ungetc(c,file);
                        c = '\r';
                    }
                    return 1;
                default:
                    *line++ = c;
                    max--;
                    len++;
                    break;
            }
        }
        while (c != EOF && c != '\n')
            c = fgetc(file);
    }
    return 1;
}


int ReadSourceLine(char *line, int max)
{
    int i;

    while (nInclude >= 0)
    {
        i = ReadLine(include[nInclude], line, max);
        if (i) return i;

        CloseInclude();
    }

    return ReadLine(source, line, max);
}


void ListOut(void)
{
/* uncomment this block if you want form feeds to be sent to the listing file
    if (listLineFF && cl_List)
        fputc(12,listing);
*/
    Debright(listLine);

    if (cl_List)
        fprintf(listing,"%s\n",listLine);

    if ((errFlag && cl_Err) || (warnFlag && cl_Warn))
        fprintf(stderr,"%s\n",listLine);
}


void CopyListLine(void)
{
    int n;
    char c,*p,*q;

    p = listLine;
    q = line;
    listLineFF = FALSE;

    // the old version
//  strcpy(listLine, "                ");       // 16 blanks
//  strncat(listLine, line, 255-16);

    if (listWid == LIST_24)
        for (n=24; n; n--) *p++ = ' ';  // 24 blanks at start of line
    else
        for (n=16; n; n--) *p++ = ' ';  // 16 blanks at start of line

    while (n < 255-16 && (c = *q++))    // copy rest of line, stripping out form feeds
    {
        if (c == 12) listLineFF = TRUE; // if a form feed was found, remember it for later
        else
        {
            *p++ = c;
            n++;
        }
    }
    *p = 0;   // string terminator
}


// --------------------------------------------------------------
// main assembler loops


void DoOpcode(int typ, int parm)
{
    int             val;
    int             i,n;
    Str255          word,s;
    char            *oldLine;
    int             token;
    int             ch;
    u_char          quote;
    char            *p;

    if (DoCPUOpcode(typ, parm)) return;

    switch(typ)
    {
        case o_ZSCII:
        case o_ASCIIC:
        case o_DB:
            instrLen = 0;
            oldLine = linePtr;
            token = GetWord(word);

            if (token == 0)
               MissingOperand();

            if (typ == o_ASCIIC)
                bytStr[instrLen++] = 0;

            while (token)
            {
                if ((token == '\'' && *linePtr && linePtr[1] != '\'') || token == '"')
                {
                    quote = token;
                    while (token == quote)
                    {
                        while (*linePtr != 0 && *linePtr != token)
                        {
                            ch = GetBackslashChar();
                            if ((ch >= 0) && (instrLen < MAX_BYTSTR))
                                bytStr[instrLen++] = ch;
                        }
                        token = *linePtr;
                        if (token)  linePtr++;
                            else    Error("Missing close quote");
                        if (token == quote && *linePtr == quote)    // two quotes together
                        {
                            if (instrLen < MAX_BYTSTR)
                                bytStr[instrLen++] = *linePtr++;
                        }
                        else
                            token = *linePtr;
                    }
                }
                else
                {
                    linePtr = oldLine;
                    val = EvalByte();
                    if (instrLen < MAX_BYTSTR)
                        bytStr[instrLen++] = val;
                }

                token = GetWord(word);
                oldLine = linePtr;

                if (token == ',')
                {
                    token = GetWord(word);
                    if (token == 0)
                        MissingOperand();
                }
                else if (token)
                {
                    linePtr = oldLine;
                    Comma();
                    token = 0;
                }
                else if (errFlag)   // this is necessary to keep EvalByte() errors
                    token = 0;      // from causing phase errors
            }

            if (instrLen >= MAX_BYTSTR || (typ == o_ASCIIC && instrLen > 256))
            {
                Error("String too long");
                instrLen = MAX_BYTSTR;
            }
            instrLen = -instrLen;

            if (typ == o_ASCIIC)
            {
                if (instrLen < -256) bytStr[0] = 255;
                                else bytStr[0] = -instrLen-1;
            }

            if (typ == o_ZSCII)
                ConvertZSCII();
            break;

        case o_DW:
        case o_DWRE:    // reverse-endian DW
            if (endian != LITTLE_END && endian != BIG_END)
            {
                Error("CPU endian not defined");
                break;
            }

            instrLen = 0;
            oldLine = linePtr;
            token = GetWord(word);

            if (token == 0)
               MissingOperand();

            while (token)
            {
#if 1 // enable padded string literals
                if ((token == '\'' && *linePtr && linePtr[1] != '\'') || token == '"')
                {
                    n = 0;
                    quote = token;
                    while (token == quote)
                    {
                        while (*linePtr != 0 && *linePtr != token)
                        {
                            ch = GetBackslashChar();
                            if ((ch >= 0) && (instrLen < MAX_BYTSTR))
                            {
                                bytStr[instrLen++] = ch;
                                n++;
                            }
                        }
                        token = *linePtr;
                        if (token)  linePtr++;
                            else    Error("Missing close quote");
                        if (token == quote && *linePtr == quote)    // two quotes together
                        {
                            if (instrLen < MAX_BYTSTR)
                            {
                                bytStr[instrLen++] = *linePtr++;
                                n++;
                            }
                        }
                        else
                            token = *linePtr;
                    }
                    // add padding nulls here
                    if ((n & 1) && instrLen < MAX_BYTSTR)
                        bytStr[instrLen++] = 0;
                }
                else
#endif
                {
                    linePtr = oldLine;
                    val = Eval();
                    if ((endian == LITTLE_END) ^ (typ == o_DWRE))
                    {   // little endian
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val;
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val >> 8;
                    }
                    else
                    {   // big endian
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val >> 8;
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val;
                    }
                }

                token = GetWord(word);
                oldLine = linePtr;

                if (token == ',')
                {
                    token = GetWord(word);
                    if (token == 0)
                        MissingOperand();
                }
                else if (token)
                {
                    linePtr = oldLine;
                    Comma();
                    token = 0;
                }
            }

            if (instrLen >= MAX_BYTSTR)
            {
                Error("String too long");
                instrLen = MAX_BYTSTR;
            }
            instrLen = -instrLen;
            break;

        case o_DL:
            if (endian != LITTLE_END && endian != BIG_END)
            {
                Error("CPU endian not defined");
                break;
            }

            instrLen = 0;
            oldLine = linePtr;
            token = GetWord(word);

            if (token == 0)
               MissingOperand();

            while (token)
            {
#if 1 // enable padded string literals
                if ((token == '\'' && *linePtr && linePtr[1] != '\'') || token == '"')
                {
                    n = 0;
                    quote = token;
                    while (token == quote)
                    {
                        while (*linePtr != 0 && *linePtr != token)
                        {
                            ch = GetBackslashChar();
                            if ((ch >= 0) && (instrLen < MAX_BYTSTR))
                            {
                                bytStr[instrLen++] = ch;
                                n++;
                            }
                        }
                        token = *linePtr;
                        if (token)  linePtr++;
                            else    Error("Missing close quote");
                        if (token == quote && *linePtr == quote)    // two quotes together
                        {
                            if (instrLen < MAX_BYTSTR)
                            {
                                bytStr[instrLen++] = *linePtr++;
                                n++;
                            }
                        }
                        else
                            token = *linePtr;
                    }
                    // add padding nulls here
                    while ((n & 3) && instrLen < MAX_BYTSTR)
                    {
                        bytStr[instrLen++] = 0;
                        n++;
                    }
                }
                else
#endif
                {
                    linePtr = oldLine;
                    val = Eval();
                    if ((endian == LITTLE_END) ^ (typ == o_DWRE))
                    {   // little endian
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val;
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val >> 8;
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val >> 16;
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val >> 24;
                    }
                    else
                    {   // big endian
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val >> 24;
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val >> 16;
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val >> 8;
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val;
                    }
                }

                token = GetWord(word);
                oldLine = linePtr;

                if (token == ',')
                {
                    token = GetWord(word);
                    if (token == 0)
                        MissingOperand();
                }
                else if (token)
                {
                    linePtr = oldLine;
                    Comma();
                    token = 0;
                }
            }

            if (instrLen >= MAX_BYTSTR)
            {
                Error("String too long");
                instrLen = MAX_BYTSTR;
            }
            instrLen = -instrLen;
            break;

        case o_DS:
            val = Eval();

            if (!evalKnown)
            {
                Error("Can't use DS pseudo-op with forward-declared length");
                break;
            }

            oldLine = linePtr;
            token = GetWord(word);
            if (token == ',')
            {
                if (parm == 1)
                    n = EvalByte();
                else
                    n = Eval();

                if (val*parm > MAX_BYTSTR)
                {
                    sprintf(s,"String too long (max %d bytes)",MAX_BYTSTR);
                    Error(s);
                    break;
                }

                if (parm == 1) // DS.B
                    for (i=0; i<val; i++)
                        bytStr[i] = n;
                else           // DS.W
                {
                    if (endian != LITTLE_END && endian != BIG_END)
                    {
                        Error("CPU endian not defined");
                        break;
                    }

                    for (i=0; i<val*parm; i+=parm)
                    {
                        if (endian == BIG_END)
                        {
                            if (parm == 4)
                            {
                                bytStr[i]   = n >> 24;
                                bytStr[i+1] = n >> 16;
                                bytStr[i+2] = n >> 8;
                                bytStr[i+3] = n;
                            }
                            else
                            {
                                bytStr[i]   = n >> 8;
                                bytStr[i+1] = n;
                            }
                        }
                        else
                        {
                            bytStr[i]   = n;
                            bytStr[i+1] = n >> 8;
                            if (parm == 4)
                            {
                                bytStr[i+2] = n >> 16;
                                bytStr[i+3] = n >> 24;
                            }
                        }
                    }
                }
                instrLen = -val * parm;

                break;
            }
            else if (token)
            {
                linePtr = oldLine;
                Comma();
                token = 0;
            }

            if (pass == 2)
            {
                showAddr = FALSE;

                // "XXXX  (XXXX)"
                p = ListLoc(locPtr);
                *p++ = ' ';
                *p++ = '(';
                p = ListAddr(p,val);
                *p++ = ')';
            }

            AddLocPtr(val*parm);
            break;

        case o_HEX:
            instrLen = 0;
            while (!errFlag && GetWord(word))
            {
                n = strlen(word);
                for (i=0; i<n; i+=2)
                {
                    if (ishex(word[i]) && ishex(word[i+1]))
                    {
                        val = Hex2Dec(word[i]) * 16 + Hex2Dec(word[i+1]);
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val;
                    }
                    else
                    {
                        Error("Invalid HEX string");
                        n = i;
                    }
                }
            }

            if (instrLen >= MAX_BYTSTR)
            {
                Error("String too long");
                instrLen = MAX_BYTSTR;
            }
            instrLen = -instrLen;
            break;

        case o_FCC:
            instrLen = 0;
            oldLine = linePtr;
            token = GetWord(word);

            if (token == 0)
               MissingOperand();
            else if (token == -1)
            {
                linePtr = oldLine;
                val = Eval();
                token = GetWord(word);
                if (val >= MAX_BYTSTR)
                    Error("String too long");
                if (!errFlag && (token == ','))
                {
                    while (*linePtr >= 0x20 && instrLen < val)
                        bytStr[instrLen++] = *linePtr++;
                    while (instrLen < val)
                        bytStr[instrLen++] = ' ';
                }
                else IllegalOperand();
            }
            else
            {
                ch = token;
                while (token)
                {
                    if (token == ch)
                    {
                        while (token == ch)
                        {   while (*linePtr != 0 && *linePtr != token)
                            {
                                if (instrLen < MAX_BYTSTR)
                                    bytStr[instrLen++] = *linePtr++;
                            }
                            if (*linePtr)   linePtr++;
                                    else    Error("FCC not terminated properly");
                            if (*linePtr == token)
                            {
                                if (instrLen < MAX_BYTSTR)
                                    bytStr[instrLen++] = *linePtr++;
                            }
                            else
                                token = *linePtr;
                        }
                    }
                    else
                    {
                        linePtr = oldLine;
                        val = EvalByte();
                        if (instrLen < MAX_BYTSTR)
                            bytStr[instrLen++] = val;
                    }

                    token = GetWord(word);
                    oldLine = linePtr;

                    if (token == ',')
                    {
                        token = GetWord(word);
                        if (token == 0)
                            MissingOperand();
                    }
                    else if (token)
                    {
                        linePtr = oldLine;
                        Comma();
                        token = 0;
                    }
                    else if (errFlag)   // this is necessary to keep EvalByte() errors
                        token = 0;      // from causing phase errors
                }
            }

            if (instrLen >= MAX_BYTSTR)
            {
                Error("String too long");
                instrLen = MAX_BYTSTR;
            }
            instrLen = -instrLen;
            break;

        case o_ALIGN_n:
            val = parm;
        case o_ALIGN:
            if (typ == o_ALIGN)
                val = Eval();

            // val must be a power of two
            if (val <= 0 || val > 65535 || (val & (val - 1)) != 0)
            {
                IllegalOperand();
                val = 0;
            }
            else
            {
                i = locPtr & ~(val - 1);
                if (i != locPtr)
                    val = val - (locPtr - i); // aka val = val + i - locPtr;
                else
                    val = 0;
            }

            if (pass == 2)
            {
                showAddr = FALSE;

                // "XXXX  (XXXX)"
                p = ListLoc(locPtr);
                *p++ = ' ';
                *p++ = '(';
                p = ListAddr(p,val);
                *p++ = ')';
            }

            AddLocPtr(val);
            break;

        case o_END:
            if (nInclude >= 0)
                CloseInclude();
            else
            {
                oldLine = linePtr;
                if (GetWord(word))
                {
                    linePtr = oldLine;
                    val = Eval();
                    CodeXfer(val);

                    // "XXXX  (XXXX)"
                    p = ListLoc(locPtr);
                    *p++ = ' ';
                    *p++ = '(';
                    p = ListAddr(p,val);
                    *p++ = ')';
                }
                sourceEnd = TRUE;
            }
            break;

        case o_Include:
            // skip whitespace
            while (*linePtr == ' ' || *linePtr == '\t')
                linePtr++;
            oldLine = word;

            quote = 0;
            if (*linePtr == '"' || *linePtr == 0x27)
                quote = *linePtr++;

            while (*linePtr !=0 && *linePtr != ' ' && *linePtr != '\t' && *linePtr != quote)
            {
                ch = *linePtr++;
                if (ch == '\\' && *linePtr != 0)
                    ch = *linePtr++;
                *oldLine++ = ch;
            }
            *oldLine++ = 0;

            if (quote)
            {
                if (*linePtr == quote)
                    *linePtr++;
                else
                    Error("Missing close quote");
            }

            switch(OpenInclude(word))
            {
                case -1:
                    Error("Too many nested INCLUDEs");
                    break;
                case 0:
                    sprintf(s,"Unable to open INCLUDE file '%s'",word);
                    Error(s);
                    break;
                default:
                    break;
            }
            break;

        case o_ENDM:
            Error("ENDM without MACRO");
            break;

#ifdef ENABLE_REP
        case o_REPEND:
            Error("REPEND without REPEAT");
            break;
#endif

        case o_Processor:
            if (!GetWord(word)) MissingOperand();
            else if (!SetCPU(word)) IllegalOperand();
            break;

        default:
            Error("Unknown opcode");
            break;
    }
}


void DoLabelOp(int typ, int parm, char *labl)
{
    int         val;
    int         i;
    Str255      word;
    int         token;
    Str255      opcode;
    MacroPtr    macro;
    MacroPtr    xmacro;
    int         nparms;
    SegPtr      seg;
    char        *oldLine;
//  struct MacroRec repList;        // repeat text list
//  MacroLinePtr    rep,rep2;       // pointer into repeat text list
    char        *p;

    if (DoCPULabelOp(typ,parm,labl)) return;

    switch(typ)
    {
        case o_EQU:
            if (labl[0] == 0)
                Error("Missing label");
            else
            {
                val = Eval();

                // "XXXX  (XXXX)"
                p = listLine;
                if (addrWid == ADDR_32)
                    for (i = 0; i <= 8; i++) *p++ = ' ';
                else
                {
                    if (listWid == LIST_24)
                        for (i = 0; i <= 5; i++) *p++ = ' ';
                    else
                        for (i = 0; i <= 4; i++) *p++ = ' ';
                }
                *p++ = '=';
                *p++ = ' ';
                p = ListAddr(p,val);
				DefSym(labl,val,parm==1,parm==0);
            }
            break;

        case o_ORG:
            CodeAbsOrg(Eval());
            if (!evalKnown)
                Warning("Undefined label used in ORG statement");
            DefSym(labl,locPtr,FALSE,FALSE);
            showAddr = TRUE;
            break;

        case o_RORG:
            val = Eval();
            CodeRelOrg(val);
            DefSym(labl,codPtr,FALSE,FALSE);

            if (pass == 2)
            {
                // "XXXX = XXXX"
                p = ListLoc(codPtr);
                *p++ = '=';
                *p++ = ' ';
                p = ListAddr(p,val);
            }
            break;

        case o_SEG:
            token = GetWord(word);  // get seg name
            if (token == 0 || token == -1)  // must be null or alphanumeric
            {
                seg = FindSeg(word);    // find segment storage and create if necessary
                if (!seg) seg = AddSeg(word);
//              seg -> gen = parm;      // copy gen flag from parameter
                SwitchSeg(seg);
                DefSym(labl,locPtr,FALSE,FALSE);
                showAddr = TRUE;
            }
            break;

        case o_SUBR:
            token = GetWord(word);  // get subroutine name
            strcpy(subrLabl,word);
            break;

        case o_REND:
            if (pass == 2)
            {
                // "XXXX = XXXX"
                p = ListLoc(locPtr);
            }

            DefSym(labl,locPtr,FALSE,FALSE);

            CodeAbsOrg(codPtr);

            break;

        case o_LIST:
            listThisLine = TRUE;    // always list this line

            if (labl[0])
                Error("Label not allowed");

            GetWord(word);

                 if (strcmp(word,"ON") == 0)        listFlag = TRUE;
            else if (strcmp(word,"OFF") == 0)       listFlag = FALSE;
            else if (strcmp(word,"MACRO") == 0)     listMacFlag = TRUE;
            else if (strcmp(word,"NOMACRO") == 0)   listMacFlag = FALSE;
            else if (strcmp(word,"EXPAND") == 0)    expandHexFlag = TRUE;
            else if (strcmp(word,"NOEXPAND") == 0)  expandHexFlag = FALSE;
            else if (strcmp(word,"SYM") == 0)       symtabFlag = TRUE;
            else if (strcmp(word,"NOSYM") == 0)     symtabFlag = FALSE;
            else                                    IllegalOperand();

            break;
/*
        case o_PAGE:
            listThisLine = TRUE;    // always list this line

            if (labl[0])
                Error("Label not allowed");

            listLineFF = TRUE;
            break;
*/
        case o_OPT:
            listThisLine = TRUE;    // always list this line

            if (labl[0])
                Error("Label not allowed");

            GetWord(word);

                 if (strcmp(word,"LIST") == 0)      listFlag = TRUE;
            else if (strcmp(word,"NOLIST") == 0)    listFlag = FALSE;
            else if (strcmp(word,"MACRO") == 0)     listMacFlag = TRUE;
            else if (strcmp(word,"NOMACRO") == 0)   listMacFlag = FALSE;
            else if (strcmp(word,"EXPAND") == 0)    expandHexFlag = TRUE;
            else if (strcmp(word,"NOEXPAND") == 0)  expandHexFlag = FALSE;
            else if (strcmp(word,"SYM") == 0)       symtabFlag = TRUE;
            else if (strcmp(word,"NOSYM") == 0)     symtabFlag = FALSE;
            else                                    Error("Illegal option");

            break;

        case o_ERROR:
            if (labl[0])
                Error("Label not allowed");
            while (*linePtr == ' ' || *linePtr == '\t')
                linePtr++;
            Error(linePtr);
            break;

        case o_MACRO:
            // see if label already provided
            if (labl[0] == 0)
            {
                // get next word on line for macro name
                if (GetWord(labl) != -1)
                {
                    Error("Macro name requried");
                    break;
                }
                // optional comma after macro name
                oldLine = linePtr;
                if (GetWord(word) != ',')
                    linePtr = oldLine;
            }

            // don't allow temporary labels as macro names
            if (strchr(labl,'@') || strchr(labl,'.'))
            {
                Error("Can not use temporary labels as macro names");
                break;
            }

            macro = FindMacro(labl);
            if (macro && macro -> def)
                Error("Macro multiply defined");
            else
            {
                if (macro == NULL)
                {
                    macro = AddMacro(labl);
                    nparms = 0;

                    token = GetWord(word);
                    while (token == -1)
                    {
                        nparms++;
                        if (nparms > MAXMACPARMS)
                        {
                            Error("Too many macro parameters");
                            macro -> toomany = TRUE;
                            break;
                        }
                        AddMacroParm(macro,word);
                        token = GetWord(word);
                        if (token == ',')
                            token = GetWord(word);
                    }

                    if (word[0] && !errFlag)
                        Error("Illegal operand");
                }

                if (pass == 2)
                {
                    macro -> def = TRUE;
                    if (macro -> toomany)
                        Error("Too many macro parameters");
                }

                macroCondLevel = 0;
                i = ReadSourceLine(line, sizeof(line));
                while (i && typ != o_ENDM)
                {
                    if (pass == 2 && (listFlag || errFlag))
                        ListOut();
                    CopyListLine();

                    // skip initial formfeeds
                    linePtr = line;
                    while (*linePtr == 12)
                        linePtr++;

                    // get label
                    labl[0] = 0;
                    if (isalphanum(*linePtr) || *linePtr == '@' || *linePtr == '.')
                    {
                        token = GetWord(labl);
                        if (token)
                            showAddr = TRUE;
                            while (*linePtr == ' ' || *linePtr == '\t')
                                linePtr++;

                        if (labl[0])
                        {
                            if (token == '@' || token == '.')
                            {
                                GetWord(word);
                                if (token == '.' && subrLabl[0])    strcpy(labl,subrLabl);
                                                            else    strcpy(labl,lastLabl);
                                labl[strlen(labl)+1] = 0;
                                labl[strlen(labl)]   = token;
                                strcat(labl,word);          // labl = lastLabl + "@" + word;
                            }
                            else
                                strcpy(lastLabl,labl);
                        }

                        if (*linePtr == ':' && linePtr[1] != '=')
                            linePtr++;
                    }

                    typ = 0;
                    if (GetOpcode(opcode))
                        FindOpcode(opcode, &typ, &parm, &xmacro);

                    switch(typ)
                    {
                        case o_IF:
                            if (pass == 1)
                                AddMacroLine(macro,line);
                            macroCondLevel++;
                            break;

                        case o_ENDIF:
                            if (pass == 1)
                                AddMacroLine(macro,line);
                            if (macroCondLevel)
                                macroCondLevel--;
                            else
                                Error("ENDIF without IF in macro definition");
                            break;

                        case o_END:
                            Error("END not allowed inside a macro");
                            break;

                        case o_ENDM:
                            if (pass == 1 && labl[0])
                                AddMacroLine(macro,labl);
                            break;

                        default:
                            if (pass == 1)
                                AddMacroLine(macro,line);
                            break;
                    }
                    if (typ != o_ENDM)
                        i = ReadSourceLine(line, sizeof(line));
                }

                if (macroCondLevel)
                    Error("IF block without ENDIF in macro definition");

                if (typ != o_ENDM)
                    Error("Missing ENDM");
            }
            break;

        case o_IF:
            if (labl[0])
                Error("Label not allowed");

            condLevel++;

            if (condLevel < MAX_COND)
                condElse[condLevel] = FALSE;

            condFail = condLevel - 1;
            val = Eval();
            if (!errFlag && val != 0)
                condFail = condLevel;
            break;

        case o_ELSE:    // previous IF was true, so this section stays off
            if (labl[0])
                Error("Label not allowed");

            if (condLevel == 0)
                Error("ELSE outside of IF block");
            else
                if (condLevel < MAX_COND && condElse[condLevel])
                    Error("Multiple ELSE statements in an IF block");
                else
                {
                    if (condLevel < MAX_COND)
                        condElse[condLevel] = TRUE;

                    condFail = condLevel - 1;                   
                }
            break;

        case o_ELSIF:   // previous IF was true, so this section stays off
            if (labl[0])
                Error("Label not allowed");

            if (condLevel == 0)
                Error("ELSIF outside of IF block");
            else
                if (condLevel < MAX_COND && condElse[condLevel])
                    Error("Multiple ELSE statements in an IF block");
                else
                {
                    // i = Eval(); // evaluate condition and ignore result
                    EatIt(); // just ignore the conditional expression

                    if (condLevel < MAX_COND)
                        condElse[condLevel] = FALSE;

                    condFail = condLevel - 1;
                }
            break;

        case o_ENDIF:   // previous ELSE was true, now exiting it
            if (labl[0])
                Error("Label not allowed");

            if (condLevel == 0)
                Error("ENDIF outside of IF block");
            else
            {
                condLevel--;
                condFail = condLevel;
            }
            break;

#ifdef ENABLE_REP
// still under construction
// notes:
//      REPEAT pseudo-op should act like an inline macro
//      1) collect all lines into new macro level
//      2) copy parameters from previous macro level (if any)
//      3) set repeat count for this macro level (repeat count is set to 0 for plain macro)
//      4) when macro ends, decrement repeat count and start over if count > 0
//      5) don't forget to dispose of the temp macro and its lines when done!
        case o_REPEAT:
            if (labl[0])
            {
                DefSym(labl,locPtr,FALSE,FALSE);
                showAddr = TRUE;
            }

            // get repeat count
            val = Eval();
            if (!errFlag)
                if (val < 0) IllegalOperand();

            if (!errFlag)
            {
                repList -> text = NULL;

// *** read line
// *** while line not REPEND
// ***      add line to repeat buffer
                macroCondLevel = 0;
                i = ReadSourceLine(line, sizeof(line));
                while (i && typ != o_REPEND)
                {
                    if (pass == 2 && (listFlag || errFlag))
                        ListOut();
                    CopyListLine();

                    // skip initial formfeeds
                    linePtr = line;
                    while (*linePtr == 12)
                        linePtr++;

                    // get label
                    labl[0] = 0;
                    if (isalphanum(*linePtr) || *linePtr == '@' || *linePtr == '.')
                    {
                        token = GetWord(labl);
                        if (token)
                            showAddr = TRUE;
                            while (*linePtr == ' ' || *linePtr == '\t')
                                linePtr++;

                        if (labl[0])
                        {
                            if (token == '@' || token == '.')
                            {
                                GetWord(word);
                                if (token == '.' && subrLabl[0])    strcpy(labl,subrLabl);
                                                            else    strcpy(labl,lastLabl);
                                labl[strlen(labl)+1] = 0;
                                labl[strlen(labl)]   = token;
                                strcat(labl,word);          // labl = lastLabl + "@" + word;
                            }
                            else
                                strcpy(lastLabl,labl);
                        }

                        if (*linePtr == ':' && linePtr[1] != '=')
                            linePtr++;
                    }

                    typ = 0;
                    if (GetOpcode(opcode))
                        FindOpcode(opcode, &typ, &parm, &xmacro);

                    switch(typ)
                    {
                        case o_IF:
                            if (pass == 1)
                                AddMacroLine(&replist,line);
                            macroCondLevel++;
                            break;

                        case o_ENDIF:
                            if (pass == 1)
                                AddMacroLine(&replist,line);
                            if (macroCondLevel)
                                macroCondLevel--;
                            else
                                Error("ENDIF without IF in REPEAT block");
                            break;

                        case o_END:
                            Error("END not allowed inside REPEAT block");
                            break;

                        case o_ENDM:
                            if (pass == 1 && labl[0])
                                AddMacroLine(&replist,labl);
                            break;

                        default:
                            if (pass == 1)
                                AddMacroLine(&replist,line);
                            break;
                    }
                    if (typ != o_ENDM)
                        i = ReadSourceLine(line, sizeof(line));
                }

                if (macroCondLevel)
                    Error("IF block without ENDIF in REPEAT block");

                if (typ != o_REPEND)
                    Error("Missing REPEND");

                if (!errFlag)
                {
// *** while (val--)
// ***      for each line
// ***            doline()
                }

                // free repeat line buffer
                while (replist)
                {
                    rep = replist->next;
                    free(replist);
                    replist = rep;
                }
            }
            break;
#endif

        default:
            Error("Unknown opcode");
            break;
    }
}


void DoLine()
{
    Str255      labl;
    Str255      opcode;
    int         typ;
    int         parm;
    int         i;
    Str255      word;
    int         token;
    MacroPtr    macro;
    char        *oldLine;
    char        *p;
    int         numhex;

    errFlag      = FALSE;
    warnFlag     = FALSE;
    instrLen     = 0;
    showAddr     = FALSE;
    listThisLine = listFlag;
    CopyListLine();

    // skip initial formfeeds
    linePtr = line;
    while (*linePtr == 12)
        linePtr++;

    // look for label at beginning of line
    labl[0] = 0;
    if (isalphanum(*linePtr) || *linePtr == '$' || *linePtr == '@' || *linePtr == '.')
    {
        token = GetWord(labl);
        oldLine = linePtr;
        if (token)
            showAddr = TRUE;
        while (*linePtr == ' ' || *linePtr == '\t')
            linePtr++;

           if (labl[0])
        {
            if (token == '@' || token == '.')
            {
                GetWord(word);
                if (token == '.' && FindOpcodeTab((OpcdPtr) &opcdTab2, word, &typ, &parm))
                {
                    linePtr = oldLine;
                }
                else
                {
                if (token == '.' && subrLabl[0])    strcpy(labl,subrLabl);
                                            else    strcpy(labl,lastLabl);
                labl[strlen(labl)+1] = 0;
                labl[strlen(labl)]   = token;
                strcat(labl,word);          // labl = lastLabl + "@" + word;
                }
            }
            else
                strcpy(lastLabl,labl);
        }

        if (*linePtr == ':' && linePtr[1] != '=')
            linePtr++;
    }

    if (condLevel > condFail)
    {
        listThisLine = FALSE;

        // inside failed IF statement
        if (GetOpcode(opcode))
        {
            FindOpcode(opcode, &typ, &parm, &macro);

            switch(typ)
            {
                case o_IF:
                    condLevel++;
                    break;

                case o_ELSE:
                    if (condLevel == condFail+1)
                    {   // previous IF was false
                        listThisLine = listFlag;

                        if (condLevel < MAX_COND && condElse[condLevel])
                            Error("Multiple ELSE statements in an IF block");
                        else
                        {
                            if (condLevel < MAX_COND)
                                condElse[condLevel] = TRUE;

                            condFail++;
                        }
                    }
                    break;

                case o_ELSIF:
                    if (condLevel == condFail+1)
                    {   // previous IF was false
                        listThisLine = listFlag;

                        if (condLevel < MAX_COND && condElse[condLevel])
                            Error("Multiple ELSE statements in an IF block");
                        else
                        {
                            i = Eval();
                            if (!errFlag && i != 0)
                                condFail++;
                        }
                    }
                    break;

                case o_ENDIF:
                    condLevel--;
                    if (condLevel == condFail)
                        listThisLine = listFlag;
                    break;

                default:    // ignore any other lines
                    break;
            }
        }

        if (pass == 2 && listThisLine && (errFlag || listMacFlag || !macLineFlag))
            ListOut();
    }
    else
    {
        token = GetOpcode(opcode);
        if (token == 0 || token == '*')         // line with label only
        {
            DefSym(labl,locPtr,FALSE,FALSE);
        }
        else
        {
            FindOpcode(opcode, &typ, &parm, &macro);

            if (typ == o_Illegal)
            {
                if (opcode[0] == '.' && SetCPU(opcode+1))
                    /* successfully changed CPU type */;
                else
                {
                    sprintf(word,"Illegal opcode '%s'",opcode);
                    Error(word);
                }
            }
            else if (typ == o_MacName)
            {
                if (macPtr[macLevel] && macLevel >= MAX_MACRO)
                    Error("Macros nested too deeply");
                else
                {
                    if (macPtr[macLevel])
                        macLevel++;

                    macPtr [macLevel] = macro;
                    macLine[macLevel] = macro -> text;
#ifdef ENABLE_REP
                    macRepeat[macLevel] = 0;
#endif

                    GetMacParms(macro);

                    showAddr = TRUE;
                    DefSym(labl,locPtr,FALSE,FALSE);
                }
            }
            else if (typ >= o_LabelOp)
            {
                showAddr = FALSE;
                DoLabelOp(typ,parm,labl);
            }
            else
            {
                showAddr = TRUE;
                DefSym(labl,locPtr,FALSE,FALSE);
                DoOpcode(typ, parm);
            }

            if (typ != o_Illegal && typ != o_MacName)
                if (!errFlag && GetWord(word))
                    Error("Too many operands");
        }

        if (pass == 1)
            AddLocPtr(abs(instrLen));
        else
        {
            p = listLine;
            if (showAddr) p = ListLoc(locPtr);
            else
            {
                if (addrWid == ADDR_32)
                    p = listLine + 9;
                else
                    p = listLine + 5;
            }
            if (listWid == LIST_24)
            {
                if (addrWid == ADDR_32)
                    numhex = 6;
                else
                    numhex = 8;
            }
            else
                numhex = 5;

            if (instrLen>0) // positive instrLen for CPU instruction formatting
            {
                if (addrWid == ADDR_32)
                    p = listLine + 9;
                else
                {
                    if (listWid == LIST_24)
                        p = listLine + 6;
                    else
                        p = listLine + 5;
                }
                if (hexSpaces & 1) { *p++ = ' '; }
                for (i = 0; i < instrLen; i++)
                {
                    if (listWid == LIST_24)
                        if (hexSpaces & (1<<i)) *p++ = ' ';

                    if (i<numhex || expandHexFlag)
                    {
                        if (i > 0 && i % numhex == 0)
                        {
                            if (listThisLine && (errFlag || listMacFlag || !macLineFlag))
                                ListOut();
                            if (listWid == LIST_24)
                                strcpy(listLine, "                        ");   // 24 blanks
                            else
                                strcpy(listLine, "                ");           // 16 blanks
                            p = ListLoc(locPtr);
                        }

                        p = ListByte(p,bytStr[i]);
                    }
                    CodeOut(bytStr[i]);
                }
            }
            else if (instrLen<0) // negative instrLen for generic data formatting
            {
                instrLen = abs(instrLen);
                for (i = 0; i < instrLen; i++)
                {
                    if (i<numhex || expandHexFlag)
                    {
                        if (i > 0 && i % numhex == 0)
                        {
                            if (listThisLine && (errFlag || listMacFlag || !macLineFlag))
                                ListOut();
                            if (listWid == LIST_24)
                                strcpy(listLine, "                        ");   // 24 blanks
                            else
                                strcpy(listLine, "                ");           // 16 blanks
                            p = ListLoc(locPtr);
                        }
                        if (numhex == 6 && (i%numhex) == 2) *p++ = ' ';
                        if (numhex == 6 && (i%numhex) == 4) *p++ = ' ';
                        if (numhex == 8 && (i%numhex) == 4) *p++ = ' ';
                        p = ListByte(p,bytStr[i]);
                        if (i>=numhex) *p = 0;
                    }
                    CodeOut(bytStr[i]);
                }
            }

            if (listThisLine && (errFlag || listMacFlag || !macLineFlag))
                ListOut();
        }
    }
}


void DoPass()
{
    Str255      opcode;
    int         typ;
    int         parm;
    int         i;
    MacroPtr    macro;
    SegPtr      seg;

    fseek(source, 0, SEEK_SET); // rewind source file
    sourceEnd = FALSE;
    lastLabl[0] = 0;
    subrLabl[0] = 0;

    fprintf(stderr,"Pass %d\n",pass);

    errCount      = 0;
    condLevel     = 0;
    condFail      = 0;
    listFlag      = TRUE;
    listMacFlag   = FALSE;
    expandHexFlag = TRUE;
    symtabFlag    = TRUE;
    linenum       = 0;
    macLevel      = 0;
    macUniqueID   = 0;
    macCurrentID[0] = 0;
    curAsm        = NULL;
    endian        = UNKNOWN_END;
    opcdTab       = NULL;
    listWid       = LIST_24;
    addrWid       = ADDR_32;
    addrMax       = addrWid;
    SetCPU(defCPU);

    // reset all code pointers
    CodeAbsOrg(0);
    seg = segTab;
    while (seg)
    {
        seg -> cod = 0;
        seg -> loc = 0;
        seg = seg -> next;
    }
    curSeg = nullSeg;

    PassInit();
    i = ReadSourceLine(line, sizeof(line));
    while (i && !sourceEnd)
    {
        DoLine();
        i = ReadSourceLine(line, sizeof(line));
    }

    if (condLevel != 0)
        Error("IF block without ENDIF");

    if (pass == 2) CodeEnd();

    // Put the lines after the END statement into the listing file
    // while still checking for listing control statements.  Ignore
    // any lines which have invalid syntax, etc., because whatever
    // is found after an END statement should esentially be ignored.

    if (pass == 2)
    {
        while (i)
        {
            listThisLine = listFlag;
            CopyListLine();

            if (line[0]==' ' || line[0]=='\t')          // ignore labels (this isn't the right way)
            {
                GetOpcode(opcode);
                FindOpcode(opcode, &typ, &parm, &macro);
                if (typ == o_LIST || typ == o_OPT)
                {
                    DoLabelOp(typ,parm,"");
                }
            }

            if (listThisLine)
                ListOut();

            i = ReadSourceLine(line, sizeof(line));
        }
    }

}

// --------------------------------------------------------------
// initialization and parameters


void stdversion(void)
{
    fprintf(stderr,"%s version %s\n",versionName,versionNum);
//  fprintf(stderr,version);
}


void usage(void)
{
    stdversion();
    fprintf(stderr,"\n");
    fprintf(stderr,"Usage:\n");
    fprintf(stderr,"    %s [options] srcfile\n",progname);
    fprintf(stderr,"\n");
    fprintf(stderr,"Options:\n");
    fprintf(stderr,"    --               end of options\n");
    fprintf(stderr,"    -e               show errors to screen\n");
    fprintf(stderr,"    -w               show warnings to screen\n");
    fprintf(stderr,"    -l [filename]    make a listing file, default is srcfile.lst\n");
    fprintf(stderr,"    -o [filename]    make an object file, default is srcfile.hex or srcfile.s9\n");
    fprintf(stderr,"    -d label[=value] define a label, and assign an optional value\n");
//  fprintf(stderr,"    -9               output object file in Motorola S9 format (16-bit address)\n");
    fprintf(stderr,"    -s9              output object file in Motorola S9 format (16-bit address)\n");
    fprintf(stderr,"    -s19             output object file in Motorola S9 format (16-bit address)\n");
    fprintf(stderr,"    -s28             output object file in Motorola S9 format (24-bit address)\n");
    fprintf(stderr,"    -s37             output object file in Motorola S9 format (32-bit address)\n");
    fprintf(stderr,"    -c               send object code to stdout\n");
    fprintf(stderr,"    -C cputype       specify default CPU type (currently ");
    if (defCPU[0]) fprintf(stderr,"%s",defCPU);
              else fprintf(stderr,"no default");
    fprintf(stderr,")\n");
    exit(1);
}


void getopts(int argc, char * const argv[])
{
    int     ch;
    int     val;
    Str255  labl,word;
    int     neg;

    while ((ch = getopt(argc, argv, "ew9cd:l:o:s:C:?")) != -1)
    {
        switch (ch)
        {
            case 'e':
                cl_Err = TRUE;
                break;

            case 'w':
                cl_Warn = TRUE;
                break;

            case '9': // -9 option is deprecated
                cl_S9 = TRUE;
                cl_S9type = 9;
                break;

            case 's':
                if (optarg[0] == '9' && optarg[1] == 0)
                    cl_S9type = 9;
                else if (optarg[0] == '1' && optarg[1] == '9' && optarg[2] == 0)
                    cl_S9type = 19;
                else if (optarg[0] == '2' && optarg[1] == '8' && optarg[2] == 0)
                    cl_S9type = 28;
                else if (optarg[0] == '3' && optarg[1] == '7' && optarg[2] == 0)
                    cl_S9type = 37;
                else usage();
                cl_S9 = TRUE;
                break;

            case 'c':
                cl_Stdout = TRUE;
                cl_Obj    = FALSE;
                break;

            case 'd':
                strncpy(line, optarg, 255);
                linePtr = line;
                GetWord(labl);
                val = 0;
                if (GetWord(word) == '=')
                {
                    neg = 1;
                    if (GetWord(word) == '-')
                    {
                        neg = -1;
                        GetWord(word);
                    }
                    val = neg * EvalNum(word);
                }
                DefSym(labl,val,FALSE,TRUE);
                break;

            case 'l':
                cl_List = TRUE;
                if (optarg[0] =='-')
                {
                    optarg = "";
                    optind--;
                }
                strncpy(cl_ListName, optarg, 255);
                break;

            case 'o':
                cl_Obj    = TRUE;
                cl_Stdout = FALSE;
                if (optarg[0] =='-')
                {
                    optarg = "";
                    optind--;
                }
                strncpy(cl_ObjName, optarg, 255);
                break;

            case 'C':
                strncpy(word, optarg, 255);
                Uprcase(word);
                if (FindCPU(word))
                    strcpy(defCPU, word);
                break;

            case '?':
            default:
                usage();
        }
    }
    argc -= optind;
    argv += optind;

    // now argc is the number of remaining arguments
    // and argv[0] is the first remaining argument

    if (argc != 1)
        usage();

    strncpy(cl_SrcName, argv[0], 255);

    // note: this won't work if there's a single-char filename in the current directory!
    if (cl_SrcName[0] == '?' && cl_SrcName[1] == 0)
        usage();

    if (cl_List && cl_ListName[0] == 0)
    {
        strncpy(cl_ListName, cl_SrcName, 255-4);
        strcat (cl_ListName, ".lst");
    }

    if (cl_Obj  && cl_ObjName [0] == 0)
    {
        if (cl_S9)
        {
            strncpy(cl_ObjName, cl_SrcName, 255-3);
            sprintf(word,".s%d",cl_S9type);
//            strcat (cl_ObjName, ".s9");
            strcat (cl_ObjName, word);
        }
        else
        {
            strncpy(cl_ObjName, cl_SrcName, 255-4);
            strcat (cl_ObjName, ".hex");
        }
    }
}


int main (int argc, char * const argv[])
{
    int i;

    // initialize and get parms

    progname   = argv[0];
    pass       = 0;
    symTab     = NULL;
    xferAddr   = 0;
    xferFound  = FALSE;

    macroTab   = NULL;
    macPtr[0]  = NULL;
    macLine[0] = NULL;
    segTab     = NULL;
    nullSeg    = AddSeg("");
    curSeg     = nullSeg;

    cl_Err     = FALSE;
    cl_Warn    = FALSE;
    cl_List    = FALSE;
    cl_Obj     = FALSE;

    asmTab     = NULL;
    cpuTab     = NULL;
    defCPU[0]  = 0;

    nInclude  = -1;
    for (i=0; i<MAX_INCLUDE; i++)
        include[i] = NULL;

    cl_SrcName [0] = 0;     source  = NULL;
    cl_ListName[0] = 0;     listing = NULL;
    cl_ObjName [0] = 0;     object  = NULL;

    AsmInit();

    getopts(argc, argv);

    // open files

    source = fopen(cl_SrcName, "r");
    if (source == NULL)
    {
        fprintf(stderr,"Unable to open source input file '%s'!\n",cl_SrcName);
        exit(1);
    }

    if (cl_List)
    {
        listing = fopen(cl_ListName, "w");
        if (listing == NULL)
        {
            fprintf(stderr,"Unable to create listing output file '%s'!\n",cl_ListName);
            if (source)
                fclose(source);
            exit(1);
        }
    }

    if (cl_Stdout)
    {
        object = stdout;
    }
    else if (cl_Obj)
    {
        object = fopen(cl_ObjName, "w");
        if (object == NULL)
        {
            fprintf(stderr,"Unable to create object output file '%s'!\n",cl_ObjName);
            if (source)
                fclose(source);
            if (listing)
                fclose(listing);
            exit(1);
        }
    }

    CodeInit();

    pass = 1;
    DoPass();

    pass = 2;
    DoPass();

    if (cl_List)    fprintf(listing, "\n%.5d Total Error(s)\n\n", errCount);
    if (cl_Err)     fprintf(stderr,  "\n%.5d Total Error(s)\n\n", errCount);

    if (symtabFlag)
    {
        SortSymTab();
        DumpSymTab();
    }
//  DumpMacroTab();

    if (source)
        fclose(source);
    if (listing)
        fclose(listing);
    if (object && object != stdout)
        fclose(object);

    return (errCount != 0);
}