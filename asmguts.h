// asmguts.h - copyright 1998-2004 Bruce Tomlin

//#include <stdio.h>
//#include <ctype.h>
//#include <string.h>
//#include <stdlib.h>
//#include <unistd.h>
//#include "asmcfg.h"
//#include "asmguts.h"

#define IHEX_SIZE	32			// max number of data bytes per line in intel hex format
#define maxSymLen	16			// max symbol length (only used in DumpSym())
const int symTabCols = 3;		// number of columns for symbol table dump
#define maxMacParms	30			// maximum macro parameters
#define MAX_INCLUDE 10			// maximum INCLUDE nesting level
#define MAX_BYTSTR  1024		// size of bytStr[]
#define MAX_COND	256			// maximum nesting level of IF blocks

typedef unsigned char Str255[256];	// generic string type
typedef char bool;				// i guess bool is a c++ thing
const bool FALSE = 0;
const bool TRUE = 1;

const char		*progname;		// pointer to argv[0]

struct SymRec
{
	struct SymRec	*next;		// pointer to next symtab entry
	unsigned short	value;		// symbol value
	bool			defined;	// TRUE if defined
	bool			multiDef;	// TRUE if multiply defined
	bool			isSet;		// TRUE if defined with SET pseudo
	bool			equ;		// TRUE if defined with EQU pseudo
	bool			known;		// TRUE if value is known
	char			name[1];	// symbol name, storage = 1 + length
} *symTab = NULL;			// pointer to first entry in symbol table
typedef struct SymRec *SymPtr;

struct MacroLine
{
	struct MacroLine	*next;		// pointer to next macro line
	char				text[1];	// macro line, storage = 1 + length
};
typedef struct MacroLine *MacroLinePtr;

struct MacroParm
{
	struct MacroParm	*next;		// pointer to next macro parameter name
	char				name[1];	// macro parameter name, storage = 1 + length
};
typedef struct MacroParm *MacroParmPtr;

struct MacroRec
{
	struct MacroRec		*next;		// pointer to next macro
	bool				def;		// TRUE after macro is defined in pass 2
	bool				toomany;	// TRUE if too many parameters in definition
	MacroLinePtr		text;		// macro text
	MacroParmPtr		parms;		// macro parms
	int					nparms;		// number of macro parameters
	char				name[1];	// macro name, storage = 1 + length
} *macroTab = NULL;				// pointer to first entry in macro table
typedef struct MacroRec *MacroPtr;

struct SegRec
{
	struct SegRec		*next;		// pointer to next segment
//	bool				gen;		// FALSE to supress code output (not currently implemented)
	unsigned short		loc;		// locptr for this segment
	unsigned short		cod;		// codptr for this segment
	char				name[1];	// segment name, storage = 1 + length
} *segTab = NULL;				// pointer to first entry in macro table
typedef struct SegRec *SegPtr;

typedef char OpcdStr[maxOpcdLen+1];
struct OpcdRec
{
	OpcdStr			name;		// opcode name
	short			typ;		// opcode type
	unsigned short	parm;		// opcode parameter
};
typedef struct OpcdRec *OpcdPtr;

MacroPtr		macPtr;				// current macro in use
MacroLinePtr	macLine;			// current macro text pointer
char			*macParms[maxMacParms];	// pointers to current macro parameters
Str255			macParmsLine;		// text of current macro parameters
int				macroCondLevel;		// current IF nesting level inside a macro definition

SegPtr			curSeg;				// current segment
SegPtr			nullSeg;			// default null segment

unsigned short  locPtr;				// Current program address
unsigned short	codPtr;				// Current program "real" address
int				pass;				// Current assembler pass
bool			warnFlag;			// TRUE if warning occurred this line
bool			errFlag;			// TRUE if error occurred this line
int				errCount;			// Total number of errors

Str255			line;				// Current line from input file
unsigned char	*linePtr;			// pointer into current line
Str255			listLine;			// Current listing line
bool			listLineFF;			// TRUE if an FF was in the current listing line
bool			listFlag;			// FALSE to suppress listing source
bool			listThisLine;		// TRUE to force listing this line
bool			sourceEnd;			// TRUE when END pseudo encountered
Str255			lastLabl;			// last label for @ temps
bool			listMacFlag;		// FALSE to suppress showing macro expansions
bool			macLineFlag;		// TRUE if line came from a macro
int				linenum;			// line number in main source file

int				condLevel;			// current IF nesting level
int				condFail;			// highest non-failed IF nesting level
char			condElse[MAX_COND]; // TRUE if IF encountered on each nesting level

unsigned char   instr[INSTR_MAX];   // Current instruction word
int				instrLen;			// Current instruction length (negative to use bytStr)

unsigned char   bytStr[MAX_BYTSTR]; // Buffer for long DB statements
bool			showAddr;			// TRUE to show LocPtr on listing
unsigned short  xferAddr;			// Transfer address from END pseudo
bool			xferFound;			// TRUE if xfer addr defined w/ END

//	Command line parameters
Str255			cl_SrcName;			// Source file name
Str255			cl_ListName;		// Listing file name
Str255			cl_ObjName;			// Object file name
bool			cl_Err;				// TRUE for errors to screen
bool			cl_Warn;			// TRUE for warnings to screen
bool			cl_List;			// TRUE to generate listing file
bool			cl_Obj;				// TRUE to generate object file
bool			cl_S9;				// TRUE to generate an S-record object file instead of Intel hex
bool			cl_Stdout;			// TRUE to send object file to stdout

FILE			*source;			// source input file
FILE			*object;			// object output file
FILE			*listing;			// listing output file
FILE			*(include[MAX_INCLUDE]);	// include files
Str255			incname[MAX_INCLUDE];		// include file names
int				incline[MAX_INCLUDE];		// include line number
int				nInclude;			// current include file index

bool			evalKnown;			// TRUE if all operands in Eval were "known"


void DoOpcode(int typ, int parm);
void DoLabelOp(int typ, int parm, char *labl);
void usage(void);
void PassInit(void);
void AsmInit(void);


// --------------------------------------------------------------
// error messages


/*
 *	Error
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
		if (cl_List)	fprintf(listing, "%s:%d: *** Error:  %s ***\n",name,line,message);
		if (cl_Err)		fprintf(stderr,  "%s:%d: *** Error:  %s ***\n",name,line,message);
	}
}


/*
 *	Warning
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
		if (cl_List)	fprintf(listing, "%s:%d: *** Warning:  %s ***\n",name,line,message);
		if (cl_Warn)	fprintf(stderr,  "%s:%d: *** Warning:  %s ***\n",name,line,message);
	}
}


// --------------------------------------------------------------
// string utilities


/*
 *	Debleft deblanks the string s from the left side
 */

void Debleft(char *s)
{
	char *p = s;

	while (*p == 9 || *p == ' ')
		p++;

	while (*s++ = *p++);
}


/*
 *	Debright deblanks the string s from the right side
 */

void Debright(char *s)
{
	char *p = s + strlen(s);

	while (p>s && *--p == ' ')
		*p = 0;
}


/*
 *	Deblank removes blanks from both ends of the string s
 */

void Deblank(char *s)
{
	Debleft(s);
	Debright(s);
}


/*
 *	Uprcase converts string s to upper case
 */

void Uprcase(char *s)
{
	char *p = s;

	while (*p = toupper(*p))
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


unsigned int EvalBin(char *binStr)
{
	unsigned int	binVal;
	int				evalErr;
	int				c;

	evalErr = FALSE;
	binVal  = 0;

	while (c = *binStr++)
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


unsigned int EvalOct(char *octStr)
{
	unsigned int	octVal;
	int				evalErr;
	int				c;

	evalErr = FALSE;
	octVal  = 0;

	while (c = *octStr++)
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


unsigned int EvalDec(char *decStr)
{
	unsigned int	decVal;
	int				evalErr;
	int				c;

	evalErr = FALSE;
	decVal  = 0;

	while (c = *decStr++)
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


unsigned int EvalHex(char *hexStr)
{
	unsigned int	hexVal;
	int				evalErr;
	int				c;

	evalErr = FALSE;
	hexVal  = 0;

	while (c = *hexStr++)
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


unsigned int EvalNum(char *word)
{
	int val;

	// handle C-style 0xnnnn hexadecimal constants
	if(word[0] == '0')
	{
		if (toupper(word[1]) == 'X')
		{
			return EvalHex(word+2);
		}
		// return EvalOct(word);	// 0nnn octal constants are in less demand, though
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
// token handling

// returns 0 for end-of-line, -1 for alpha-numeric, else char value for non-alphanumeric
// converts the word to uppercase, too
int GetWord(char *word)
{
	unsigned char	c;

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
		if (isalphanum(c))
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


// same as GetWord, except it allows '.' chars in alphanumerics
int GetOpcode(char *word)
{
	unsigned char	c;

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


void Expect(char *expected)
{
	Str255 s;
	GetWord(s);
	if (strcmp(s,expected) != 0)
	{
		sprintf(s,"\"%s\" expected",expected);
		Error(s);
	}
}


void Comma()
{
	Expect(",");
}


void RParen()
{
	Expect(")");
}


void EatIt()
{
	Str255	word;
	while (GetWord(word));		// eat junk at end of line
}


/*
 *	IllegalOperand
 */

void IllegalOperand()
{
	Error("Illegal operand");
	EatIt();
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


MacroPtr AddMacro(char *name)
{
	MacroPtr	p;

	p = malloc(sizeof(struct MacroRec) + strlen(name));

	strcpy(p -> name, name);
	p -> def = FALSE;
	p -> toomany = FALSE;
	p -> text = NULL;
	p -> next = macroTab;
	p -> parms = NULL;
	p -> nparms = 0;

	macroTab = p;

	return p;
}
	

void AddMacroParm(MacroPtr macro, char *name)
{
	MacroParmPtr	parm;
	MacroParmPtr	p;

	parm = malloc(sizeof(struct MacroParm) + strlen(name));
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
	MacroLinePtr	m;
	MacroLinePtr	p;

	m = malloc(sizeof(struct MacroLine) + strlen(line));
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


void GetMacParms(MacroPtr macro)
{
	int		i;
	int		n;
	int		quote;
	char	c;
	char	*p;
	bool	done;

	for (i=0; i<maxMacParms; i++)
		macParms[i] = NULL;

	// skip initial whitespace
	c = *linePtr;
	while (c == 12 || c == '\t' || c == ' ')
		c = *++linePtr;

	// copy rest of line for safekeeping
	strcpy(macParmsLine, linePtr);

	n = 0;
	p = macParmsLine;
	while (*p && *p != ';' && n<maxMacParms)
	{
		// skip whitespace before current parameter
		c = *p;
		while (c == 12 || c == '\t' || c == ' ')
			c = *++p;

		// record start of parameter
		macParms[n] = p;
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

				case 0x27:	// quote
				case '"':
					if (quote == 0)
						quote = c;
					else if (quote == c)
						quote = 0;
			}
		}
	}

	// terminate last parameter and point remaining parameters to null strings
	*p = 0;
	for (i=n; i<maxMacParms; i++)
		macParms[i] = p;

	// remove whitespace from end of parameter
	for (i=0; i<maxMacParms; i++)
		if (macParms[i])
		{
			p = macParms[i] + strlen(macParms[i]) - 1;
			while (p>=macParms[i] && (*p == ' ' || *p == 9))
				*p-- = 0;
		}

	if (n > macro -> nparms || n > maxMacParms)
		Error("Too many macro parameters");
}


void DoMacParms()
{
	int				i;
	Str255			word;
	MacroParmPtr	parm;
	char			*p;		// pointer to start of word
	char			c;
	int				token;

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
			parm = macPtr -> parms;
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
				strcpy(p, macParms[i]);
				// point p to end of appended text
				p = p + strlen(macParms[i]);
				// copy from temp to p
				strcpy(p, word);
				// update linePtr
				linePtr = p;
			}
		}

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
	MacroLinePtr	line;
	MacroParmPtr	parm;

	if (cl_List)
	{

	fprintf(listing,"--- Macro '%s' ---", p -> name);
	fprintf(listing," def = %d, nparms = %d\n", p -> def, p -> nparms);

//	dump parms here
	fprintf(listing,"Parms:");
	for (parm = p->parms; parm; parm = parm->next)
	{
		fprintf(listing," '%s'",parm->name);
	}
	fprintf(listing,"\n");

//	dump text here
	for (line = p->text; line; line = line->next)
			fprintf(listing," '%s'\n",line->text);
	}
}


void DumpMacroTab(void)
{
	struct	MacroRec *p;

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
 *	FindOpcode
 */

void FindOpcode(char *name, int *typ, int *parm, MacroPtr *macro)

{
extern OpcdPtr opcdTab;
	OpcdPtr p = (OpcdPtr) &opcdTab;
	bool found = FALSE;

	while (p -> typ != o_Illegal && !found)
	{
		found = (strcmp(p -> name, name) == 0);
		if (!found)
			p++;
	}

	*typ  = p -> typ;
	*parm = p -> parm;

	*macro = NULL;
	if (!found)
	{
		*macro = FindMacro(name);
		if (*macro)
			*typ = o_MacName;
	}
}


/*
 *	FindSym
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
 *	AddSym
 */

SymPtr AddSym(char *symName)
{
	SymPtr p;

	p = malloc(sizeof(struct SymRec) + strlen(symName));

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
 *	RefSym
 */

int RefSym(char *symName, bool *known)
{
	SymPtr p;
	int i;
	Str255 s;

	if (p = FindSym(symName))
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
		return p -> value;
	}

	{	// check for 'FFH' style constants here

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
//			sprintf(s, "Symbol '%s' undefined", symName);
//			Error(s);
		}
	}

	return 0;
}


/*
 *	DefSym
 */

void DefSym(char *symName, unsigned short val, bool setSym, bool equSym)
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


void DumpSym(SymPtr p, char *s)
{
	char *s2;
	int n;

	n = 0;

	s2 = p->name;
	while(*s2 && n<maxSymLen)
	{
		*s++ = *s2++;
		n++;
	}

	while (n < maxSymLen)
	{
		*s++ = ' ';
		n++;
	}

	sprintf(s, "%.4X ", p->value);

	s = s + strlen(s);

	n = 0;
	if (!p->defined)	{*s++ = 'U'; n++;}	// Undefined
	if ( p->multiDef)	{*s++ = 'M'; n++;}	// Multiply defined
	if ( p->isSet)		{*s++ = 'S'; n++;}	// Set
	if ( p->equ)		{*s++ = 'E'; n++;}	// Equ
	while (n < 9)
	{
		*s++ = ' ';
		n++;
	}
	*s = 0;
}


void DumpSymTab(void)
{
	struct	SymRec *p;
	int		i;
	Str255	s;

	i = 1;
	p = symTab;
	while (p)
	{
		DumpSym(p,s);
		p = p -> next;

		i++;
		if (p == NULL || i > symTabCols)
		{
			i = 1;
			Debright(s);
			if (cl_List)
				fprintf(listing, "%s\n", s);
		}
		else
		{
			if (cl_List)
				fprintf(listing, "%s", s);
		}
	}
}


void SortSymTab()
{
	SymPtr			i,j;	// pointers to current elements
	SymPtr			ip,jp;	// pointers to previous elements
	SymPtr			t;		// temp for swapping

	if (symTab != NULL)
	{
		ip = NULL;	i = symTab;
		jp = i;		j = i -> next;

		while (j != NULL)
		{
			while (j != NULL)
			{
				if (strcmp(i->name,j->name) > 0)	// (i->name > j->name)
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
				jp = j;		j = j -> next;
			}
			ip = i;		i = i -> next;
			jp = i;		j = i -> next;
		}
	}
}


// --------------------------------------------------------------
// expression evaluation


int Eval0(void);		// forward declaration


int Factor(void)
{
	Str255		word,s;
	int			token;
	int			val;
	char		*oldLine;
	SymPtr		p;

	token = GetWord(word);
	val = 0;

	switch(token)
	{
		case 0:
			Error("Missing operand");
			break;

		case '%':
			GetWord(word);
			val = EvalBin(word);
			break;

		case '$':   // 6502/6809 only?
			if (ishex(*linePtr))
			{
				GetWord(word);
				val = EvalHex(word);
				break;
			}
			// fall-through...
		case '*':
			val = locPtr;
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

		case 0x27:	// single quote
			val = *linePtr;
			if (val)
			{
				linePtr++;
				if (*linePtr && *linePtr != 0x27)
					val = val * 256 + *linePtr++;
				if (*linePtr == 0x27)
					linePtr++;
				else
					Error("Missing close quote");
			}
			else Error("Missing operand");
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
				break;
			}

			// now try it as a local ".symbol"
			linePtr = oldLine;
			// fall-through...
		case '@':
			GetWord(word);
			strcpy(s,lastLabl);
			s[strlen(s)+1] = 0;
			s[strlen(s)]   = token;
			strcat(s,word);
			val = RefSym(s, &evalKnown);
			break;

		case -1:
			if ((word[0] == 'H' || word[0] == 'L') && word[1] == 0 && *linePtr == '(')
			{   // handle H() and L() from vintage Atari 7800 source code
				// note: no whitespace allowed before the open paren!
				token = word[0];	// save 'H' or 'L'
				GetWord(word);		// eat left paren
				val = Eval0();		// evaluate sub-expression
				RParen();			// check for right paren
				if (token == 'H') val = (val >> 8) & 0xFF;
				if (token == 'L') val = val & 0xFF;
				break;
			}
			if (isdigit(word[0]))   val = EvalNum(word);
							else	val = RefSym(word,&evalKnown);
			break;

		default:
			// ignore anything else
			break;
	}

  	return val;
}


int Term(void)
{
	Str255	word;
	int		token;
	int		val,val2;
	char	*oldLine;

	val = Factor();

	oldLine = linePtr;
	token = GetWord(word);
	while (token == '*' || token == '/' || token == '%')
	{
		switch(token)
		{
			case '*':	val = val * Factor();	break;
			case '/':	val2 = Factor();
						if (val2)
							val = val / val2;
						else
						{
							Warning("Division by zero");
							val = 0;
						}
						break;
			case '%':	val2 = Factor();
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
	Str255	word;
	int		token;
	int		val;
	char	*oldLine;

	val = Term();

	oldLine = linePtr;
	token = GetWord(word);
	while (token == '+' || token == '-')
	{
		switch(token)
		{
			case '+':	val = val + Term();		break;
			case '-':	val = val - Term();		break;
		}
		oldLine = linePtr;
		token = GetWord(word);
	}
	linePtr = oldLine;

	return val;
}


int Eval1(void)
{
	Str255	word;
	int		token;
	int		val;
	char	*oldLine;

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
			case '<':	if (*linePtr == '=')	{linePtr++; val = (val <= Eval2());}
										else				val = (val <  Eval2());
						break;
			case '>':	if (*linePtr == '=')	{linePtr++; val = (val >= Eval2());}
										else				val = (val >  Eval2());
						break;
			case '=':	if (*linePtr == '=') linePtr++; // allow either one or two '=' signs
						val = (val == Eval2());		break;
			case '!':	linePtr++;  val = (val != Eval2());		break;
		}
		oldLine = linePtr;
		token = GetWord(word);
	}
	linePtr = oldLine;

	return val;
}


int Eval0(void)
{
	Str255	word;
	int		token;
	int		val;
	char	*oldLine;

	val = Eval1();

	oldLine = linePtr;
	token = GetWord(word);
	while (token == '&' || token == '|' || token == '^' ||
			(token == '<' && *linePtr == '<') ||
			(token == '>' && *linePtr == '>'))
	{
		switch(token)
		{
			case '&':	if (*linePtr == '&') {linePtr++; val = (val & Eval1() != 0);}
										else			 val =  val & Eval1();
						break;
			case '|':	if (*linePtr == '|') {linePtr++; val = (val | Eval1() != 0);}
										else			 val =  val | Eval1();
						break;
			case '^':   val = val ^ Eval1();
						break;
			case '<':	linePtr++;	val = val << Eval1();	break;
			case '>':	linePtr++;	val = val >> Eval1();	break;
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


int EvalByte(void)
{
	short val;

	val = Eval();

	if (!errFlag && (val < -128 || val > 255))
		Warning("Byte out of range");

	return val & 0xFF;
}


int EvalBranch(int instrLen)
{
	short val;

	val = Eval();
	val = val - locPtr - instrLen;
	if (!errFlag && (val < -128 || val > 127))
		Error("Branch out of range");

	return val & 0xFF;
}


// --------------------------------------------------------------
// object file generation


	unsigned char ihex_buf[IHEX_SIZE];
	int ihex_len;	// length of current intel hex line
	int ihex_base;	// current address in intel hex buffer
	int ihex_addr;	// start address of current intel hex line

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


void write_ihex(int addr, unsigned char *buf, int len, int rectype)
{
	int i,chksum;

	if (cl_Obj || cl_Stdout)
		{
		chksum = len + (addr >> 8) + addr + rectype;
		fprintf(object,":%.2X%.4X%.2X",len,addr & 0xFFFF,rectype);
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

void write_srec(int addr, unsigned char *buf, int len, int rectype)
{
	int i,chksum;

	if (cl_Obj || cl_Stdout)
	{
		chksum = len+3 + (addr >> 8) + addr;
		if (rectype)	i = 9;  // xfer record = S9
				else	i = 1;  // code record = S1
		fprintf(object,"S%d%.2X%.4X", i, len+3, addr & 0xFFFF);
		for (i=0; i<len; i++)
		{
			fprintf(object,"%.2X",buf[i]);
			chksum = chksum + buf[i];
		}
		fprintf(object,"%.2X\n",~chksum & 0xFF);
	}
}


// rectype 0 = code, rectype 1 = xfer
void write_hex(int addr, unsigned char *buf, int len, int rectype)
{
	if (cl_S9)  write_srec(addr, buf, len, rectype);
		else	write_ihex(addr, buf, len, rectype);
}


void CodeInit(void)
{
	ihex_len  = 0;
	ihex_base = 0;
	ihex_addr = 0;
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


void Instr1(unsigned char b)
{
	instr[0] = b;
	instrLen = 1;
}


void Instr2(unsigned char b1, unsigned char b2)
{
	instr[0] = b1;
	instr[1] = b2;
	instrLen = 2;
}


void Instr3(unsigned char b1, unsigned char b2, unsigned char b3)
{
	instr[0] = b1;
	instr[1] = b2;
	instr[2] = b3;
	instrLen = 3;
}


void Instr3W(unsigned char b, int w)
{
	Instr3(b,w & 255,w >> 8);
}


void Instr3BW(unsigned char b, int w)
{
	Instr3(b,w >> 8,w & 255);
}


void Instr4(int b1, int b2, int b3, int b4)
{
	instr[0] = b1;
	instr[1] = b2;
	instr[2] = b3;
	instr[3] = b4;
	instrLen = 4;
}


void Instr4W(int b1, int b2, int w)
{
	Instr4(b1,b2,w & 255,w >> 8);
}


void Instr4BW(int b1, int b2, int w)
{
	Instr4(b1,b2,w >> 8,w & 255);
}


void Instr5(int b1, int b2, int b3, int b4, int b5)
{
	instr[0] = b1;
	instr[1] = b2;
	instr[2] = b3;
	instr[3] = b4;
	instr[4] = b5;
	instrLen = 5;
}


void Instr5BW(int b1, int b2, int b3, int w)
{
	Instr5(b1,b2,b3,w >> 8,w & 255);
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
	SegPtr	p;

	p = malloc(sizeof(struct SegRec) + strlen(name));

	p -> next = segTab;
//	p -> gen = TRUE;
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
	int c;
	int len = 0;

	macLineFlag = (macLine != NULL);
	if (macLineFlag)
	{
		strcpy(line, macLine -> text);
		macLine = macLine -> next;
		DoMacParms();
	}
	else
	{
		if (nInclude >= 0)
			incline[nInclude]++;
		else
			linenum++;

		macPtr = NULL;

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
					break;
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
/*
	if (listLineFF && cl_List)
		fputc(12,listing);
//		fprintf(listing,"^L");  // for debugging
*/
	Debright(listLine);

	if (cl_List)
		fprintf(listing,"%s\n",listLine);

	if ((errFlag && cl_Err) || warnFlag)
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
//	strcpy(listLine, "                ");		// 16 blanks
//	strncat(listLine, line, 255-16);

	for (n=16; n; n--) *p++ = ' ';	// 16 blanks at start of line

	while (n < 255-16 && (c = *q++))	// copy rest of line, stripping out form feeds
	{
		if (c == 12) listLineFF = TRUE;
		else
		{
			*p++ = c;
			n++;
		}
	}
	*p++ = 0;   // string terminator
}


// --------------------------------------------------------------
// main assembler loops


void DoStdOpcode(int typ, int parm)
{
	int				val;
	int				i,n;
	Str255			word,s;
	char			*oldLine;
	int				token;
	unsigned char	ch;
	unsigned char	quote;

	switch(typ)
	{
		case o_DB:
			instrLen = 0;
			oldLine = linePtr;
			token = GetWord(word);

			if (token == 0)
               Error("Missing operand");

			while (token)
			{
				if ((token == '\'' && *linePtr && linePtr[1] != '\'') || token == '"')
				{
					quote = token;
					while (token == quote)
					{
						while (*linePtr != 0 && *linePtr != token)
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
//									case 'x': // add hex, dec, etc. someday?
								}
							}
							if (instrLen < MAX_BYTSTR)
								bytStr[instrLen++] = ch;
						}
 						token = *linePtr;
						if (token)  linePtr++;
							else	Error("Missing close quote");
						if (token == quote && *linePtr == quote)	// two quotes together
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
						Error("Missing operand");
             	}
				else if (token)
				{
					linePtr = oldLine;
					Comma();
					token = 0;
				}
				else if (errFlag)   // this is necessary to keep EvalByte() errors
					token = 0;		// from causing phase errors
           	}

			if (instrLen >= MAX_BYTSTR)
			{
				Error("String too long");
				instrLen = MAX_BYTSTR;
			}
            instrLen = -instrLen;
			break;

		case o_DWLE:	// little-endian DW
		case o_DWBE:	// big-endian DW
            instrLen = 0;
            oldLine = linePtr;
            token = GetWord(word);

			if (token == 0)
               Error("Missing operand");

           	while (token && !errFlag)
			{
				linePtr = oldLine;
				val = Eval();
				if (typ == o_DWLE)
				{
					if (instrLen < MAX_BYTSTR)
						bytStr[instrLen++] = val & 255;
					if (instrLen < MAX_BYTSTR)
						bytStr[instrLen++] = val >> 8;
				}
				else
				{
					if (instrLen < MAX_BYTSTR)
						bytStr[instrLen++] = val >> 8;
					if (instrLen < MAX_BYTSTR)
						bytStr[instrLen++] = val & 255;
				}

				token = GetWord(word);
				oldLine = linePtr;

               	if (token == ',')
				{
					token = GetWord(word);
					if (token == 0)
						Error("Missing operand");
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

				if (parm == 1)
					for (i=0; i<val; i++)
						bytStr[i] = n;
				else
					for (i=0; i<val*parm; i+=parm)
					{
#ifdef BIG_ENDIAN
						bytStr[i]   = n >> 8;
						bytStr[i+1] = n & 0xFF;
#else
						bytStr[i]   = n & 0xFF;
						bytStr[i+1] = n >> 8;
#endif
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

				sprintf(word,"%.4X  (%.4X)",locPtr,val);
				for (i=0; i<12; i++)
					listLine[i] = word[i];
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
               Error("Missing operand");
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
						{	while (*linePtr != 0 && *linePtr != token)
							{
								if (instrLen < MAX_BYTSTR)
									bytStr[instrLen++] = *linePtr++;
							}
							if (*linePtr)   linePtr++;
									else	Error("FCC not terminated properly");
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
							Error("Missing operand");
					}
					else if (token)
					{
						linePtr = oldLine;
						Comma();
						token = 0;
					}
					else if (errFlag)   // this is necessary to keep EvalByte() errors
						token = 0;		// from causing phase errors
				}
			}

			if (instrLen >= MAX_BYTSTR)
			{
				Error("String too long");
				instrLen = MAX_BYTSTR;
			}
            instrLen = -instrLen;
			break;

		case o_ALIGN:
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
					val = i + val - locPtr;
			}

			if (pass == 2)
			{
				showAddr = FALSE;

				sprintf(word,"%.4X  (%.4X)",locPtr,val);
				for (i=0; i<12; i++)
					listLine[i] = word[i];
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

					sprintf(word,"----  (%.4X)",val);
					for (i=6; i<12; i++)
						listLine[i] = word[i];
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

		default:
			Error("Unknown opcode");
			break;
	}
}


void DoStdLabelOp(int typ, int parm, char *labl)
{
	int			val;
	int			i;
	Str255		word;
	int			token;
	Str255		opcode;
	MacroPtr	macro;
	MacroPtr	xmacro;
	int			nparms;
	SegPtr		seg;
	char		*oldLine;

	switch(typ)
	{
		case o_EQU:
			if (labl[0] == 0)
				Error("Missing label");
			else
			{
				val = Eval();

				sprintf(word,"---- = %.4X",val);
				for (i=5; i<11; i++)
					listLine[i] = word[i];

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
			DefSym(labl,/*locPtr?*/codPtr,FALSE,FALSE);

            if (pass == 2)
			{
				sprintf(word,"%.4X = %.4X",codPtr,val);
				for (i=0; i<11; i++)
					listLine[i] = word[i];
			}
			break;

		case o_SEG:
			token = GetWord(word);  // get seg name
			if (token == 0 || token == -1)	// must be null or alphanumeric
			{
				seg = FindSeg(word);	// find segment storage and create if necessary
				if (!seg) seg = AddSeg(word);
//				seg -> gen = parm;		// copy gen flag from parameter
				SwitchSeg(seg);
				DefSym(labl,locPtr,FALSE,FALSE);
				showAddr = TRUE;
			}
			break;

		case o_REND:
            if (pass == 2)
			{
				sprintf(word,"%.4X",locPtr);
				for (i=0; i<4; i++)
					listLine[i] = word[i];
			}

			DefSym(labl,locPtr,FALSE,FALSE);

			CodeAbsOrg(codPtr);

			break;

		case o_LIST:
			listThisLine = TRUE;

			if (labl[0])
				Error("Label not allowed");

			GetWord(word);

			     if (strcmp(word,"ON") == 0)		listFlag = TRUE;
			else if (strcmp(word,"OFF") == 0)		listFlag = FALSE;
			else if (strcmp(word,"MACRO") == 0)		listMacFlag = TRUE;
			else if (strcmp(word,"NOMACRO") == 0)	listMacFlag = FALSE;
			else									IllegalOperand();

			break;
/*
		case o_PAGE:
			listThisLine = TRUE;

			if (labl[0])
				Error("Label not allowed");

			listLineFF = TRUE;
			break;
*/
		case o_OPT:
			listThisLine = TRUE;

			if (labl[0])
				Error("Label not allowed");

			GetWord(word);

			     if (strcmp(word,"LIST") == 0)		listFlag = TRUE;
			else if (strcmp(word,"NOLIST") == 0)	listFlag = FALSE;
			else if (strcmp(word,"MACRO") == 0)		listMacFlag = TRUE;
			else if (strcmp(word,"NOMACRO") == 0)	listMacFlag = FALSE;
			else									Error("Illegal option");

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
						if (nparms > maxMacParms)
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
					if (pass == 2)
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
								strcpy(labl,lastLabl);
								labl[strlen(labl)+1] = 0;
								labl[strlen(labl)]   = token;
								strcat(labl,word);			// labl = lastLabl + "@" + word;
							}
							else
								strcpy(lastLabl,labl);
						}

						if (*linePtr == ':')
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

		case o_ELSE:	// previous IF was true, so this section stays off
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

		default:
			Error("Unknown opcode");
			break;
  	}
}


void DoPass()
{
	Str255		labl;
	Str255		opcode;
	int			typ;
	int			parm;
	int			i;
	Str255		word;
	int			token;
	MacroPtr	macro;
	SegPtr		seg;

	fseek(source, 0, SEEK_SET);	// rewind source file
	sourceEnd = FALSE;
	lastLabl[0] = 0;

	fprintf(stderr,"Pass %d\n",pass);

	errCount    = 0;
	condLevel   = 0;
	condFail    = 0;
	listFlag    = TRUE;
	listMacFlag = FALSE;
	linenum		= 0;

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
					strcpy(labl,lastLabl);
					labl[strlen(labl)+1] = 0;
					labl[strlen(labl)]   = token;
					strcat(labl,word);			// labl = lastLabl + "@" + word;
				}
				else
					strcpy(lastLabl,labl);
			}

			if (*linePtr == ':')
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
						{	// previous IF was false
							listThisLine = TRUE;

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
						{	// previous IF was false
							listThisLine = TRUE;

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
							listThisLine = TRUE;
						break;

					default:	// ignore any other lines
						break;
				}
			}

			if (pass == 2 && listThisLine && (errFlag || listMacFlag || !macLineFlag))
				ListOut();
		}
		else
		{
			token = GetOpcode(opcode);
			if (token == 0 || token == '*')			// line with label only
			{
				DefSym(labl,locPtr,FALSE,FALSE);
			}
			else
			{
				FindOpcode(opcode, &typ, &parm, &macro);

				if (typ == o_Illegal)
				{
					sprintf(word,"Illegal opcode '%s'",opcode);
					Error(word);
				}
				else if (typ == o_MacName)
				{
					if (macPtr)
						Error("Nested macros not supported");
					else
					{
						macPtr  = macro;
						macLine = macro -> text;

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
				if (showAddr)
				{
					sprintf(word,"%.4X",locPtr);	// codPtr
					for (i=0; i<4; i++)
						listLine[i] = word[i];
				}

				if (instrLen>0)
					for (i = 0; i < instrLen; i++)
					{
						sprintf(word,"%.2X",instr[i]);
						listLine[i*2+5] = word[0];
						listLine[i*2+6] = word[1];
						CodeOut(instr[i]);
				}
				else if (instrLen<0)
				{
					for (i = 0; i < -instrLen; i++)
					{
						if (i > 0 && i % 5 == 0)
						{
							if (listThisLine)
								ListOut();
							strcpy(listLine, "                ");		// 16 blanks
							sprintf(word,"%.4X",locPtr);
							listLine[0] = word[0];
							listLine[1] = word[1];
							listLine[2] = word[2];
							listLine[3] = word[3];
						}
						sprintf(word,"%.2X",bytStr[i]);
						if (i<5)
						{
							listLine[i*2+5] = word[0];
							listLine[i*2+6] = word[1];
						}
						else
						{
							listLine[(i%5)*2+5] = word[0];
							listLine[(i%5)*2+6] = word[1];
							listLine[(i%5)*2+7] = 0;
						}
						CodeOut(bytStr[i]);
					}
				}

				if (listThisLine && (errFlag || listMacFlag || !macLineFlag))
					ListOut();
			}
		}

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

			if (line[0]==' ' || line[0]=='\t')			// ignore labels (this isn't the right way)
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


void stdusage(void)
{
	fprintf(stderr,"\n");
	fprintf(stderr,"Usage:\n");
	fprintf(stderr,"    %s [options] srcfile\n",progname);
	fprintf(stderr,"\n");
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"    --             end of options\n");
	fprintf(stderr,"    -e             show errors to screen\n");
	fprintf(stderr,"    -w             show warnings to screen\n");
	fprintf(stderr,"    -l [filename]  make a listing file, default is srcfile.lst\n");
	fprintf(stderr,"    -o [filename]  make an object file, default is srcfile.hex or srcfile.s9\n");
	fprintf(stderr,"    -d label[=val] define a label, and assign an optional value\n");
	fprintf(stderr,"    -9             output object file in Motorola S9 format\n");
	fprintf(stderr,"    -c             send object code to stdout\n");
}


void getopts(int argc, char * const argv[])
{
	int ch;
	int val;
	Str255 labl,word;
	int neg;

	while ((ch = getopt(argc, argv, "ew9cd:l:o:?")) != -1)
	{
		switch (ch)
		{
			case 'e':
				cl_Err = TRUE;
				break;

			case 'w':
				cl_Warn = TRUE;
				break;

			case '9':
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
			strcat (cl_ObjName, ".s9");
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

	progname  = argv[0];
	pass      = 0;
	symTab    = NULL;
	xferAddr  = 0;
	xferFound = FALSE;

	macroTab  = NULL;
	macPtr    = NULL;
	macLine   = NULL;
	segTab    = NULL;
	nullSeg   = AddSeg("");
	curSeg    = nullSeg;

	cl_Err    = FALSE;
	cl_Warn   = FALSE;
	cl_List   = FALSE;
	cl_Obj    = FALSE;

	nInclude  = -1;
	for (i=0; i<MAX_INCLUDE; i++)
		include[i] = NULL;

	cl_SrcName [0] = 0;		source  = NULL;
	cl_ListName[0] = 0;		listing = NULL;
	cl_ObjName [0] = 0;		object  = NULL;

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
	AsmInit();

	pass = 1;
	DoPass();

	pass = 2;
	DoPass();

	if (cl_List)	fprintf(listing, "\n%.5d Total Error(s)\n\n", errCount);
	if (cl_Err)		fprintf(stderr,  "\n%.5d Total Error(s)\n\n", errCount);

	SortSymTab();
	DumpSymTab();
//	DumpMacroTab();

	if (source)
		fclose(source);
	if (listing)
		fclose(listing);
	if (object && object != stdout)
		fclose(object);

	return (errCount != 0);
}