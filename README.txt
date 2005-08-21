ASMX semi-generic 8-bit assembler

 - - -

FOREWORD
========

Okay, so it's not really generic, just semi generic.  This started
from an 8080 assembler I wrote in Turbo Pascal back in my college days
when I had a class where I had to write an 8080 emulator.  The assembler
wasn't part of the class, but no way was I going to hand assemble code
again like I did back in my early TRS-80 days.  Then when I started
dabbling with ColecoVision and Atari 2600 code, I made a Z-80 and a 6502
assembler from it.

But Turbo Pascal and MS-DOS are way out of style now, so I ported the
code to plain standard C.  It should run on any Unix-like operating
system.  It should also run on any operating system that provides
Unix-style file and command-line handling.  In particular, it runs fine
under Mac OS X, which is what I use on my laptop.

Porting it to C wasn't enough, though.  I had added some nice features
like macros to the 6502 assembler and I wanted them in the Z-80
assembler too.  But I didn't want to have to copy and paste code every
time I added a new feature.  So I turned the code inside out, and made
the common code into a gigantic .h file.  This made writing an
assembler for a new CPU easy enough that I was able to write a 6809
assembler in one day, plus another day for debugging.

Unlike most "generic" assemblers, I make an effort to conform to the
standard mnemonics and syntax for a CPU, as you'd find them in the chip
manufacturer's documentation.  I'm a bit looser on the pseudo-ops,
trying to be inclusive whenever possible so that existing code has a
better chance of working with fewer changes, especially code written
back in the '80s.

This is a two-pass assembler.  That means that on the first pass it
figures out where all the labels go, then on the second pass it
generates code.  I know there are popular multi-pass assemblers out
there (like DASM for 6502), and they have their own design philosophy.
I'm sticking with the philosopy that was used by the old EDTASM
assemblers for the TRS-80.  There are a few EDTASM-isms that you might
notice, if you know what to look for.

But being a two-pass assembler, there are some things you can't do.
You can't ORG to a label that hasn't been defined yet, because on
the second pass it'll have a value, and your code will go into a
different location, and all your labels will be at the wrong
address.  This is called a "phase error".  You also can't use a
label that hasn't been defined yet with DS or ALIGN because they
affect the current location.

Some CPUs like the 6502 and 6809 have different instructions which
can provide smaller faster code based on the size of an operand.
To make this work, the assembler keeps an extra flag in the symbol
table during the second pass, which tells if the symbol was known
yet at this point in the first pass.  Then the assembler can know
to use the longer form to avoid a phase error.  The 6809 assembler
syntax uses "<" (force 8-bits) and ">" (force 16-bits) to override
this decision.  The 6502 assembler can also override this with a
">" before an absolute or absolute-indexed address operand.  Note
that this usage is different from "<" and ">" as a high/low byte
of a word value.

Some assemblers like to output code in binary.  This might be nice
if you're making a video game cartridge ROM, but it's really not
very flexible.  Intel and Motorola both came up with very nice text
file formats which don't require any kind of padding when you do an
ORG instruction, and don't require a silly "segment" definitions
just to keep DS instructions from generating object code.  Then,
following the Unix philosophy of making tools that can connect to
other tools, you can pipe the object code to another utility which
makes the ROM image.

Anyhow, it works pretty well for what I want it to do.

 - Bruce -

 - - -

BUILDING IT
===========

While I normally compile this using Apple's XCode development
environment, that's completely unnecessary if all you want to do
is use it.  All it takes is a single command to GCC to compile it
No makefile, no autoconf, no nothing else.  For example, to
compile the 6502 assembler, just do this:

  gcc asm6502.c -o asm6502

That's it.  Now you will probably want to copy it to your
/usr/local/bin or ~/bin directory, but that's your choice.

 - - -

RUNNING IT
==========

Just give it the name of your assembler source file, and
whatever options you want.

  asm6502 [options] srcfile

Here are the command line options:

  -l [filename]     make a listing file, default is srcfile.lst
  -o [filename]     make an object file, default is srcfile.hex
                       or srcfile.s9 (if -s option specified)
  -d label[=value]  define a label, and assign an optional
                       value to it, default is zero
  -e                show errors to screen
  -w                show warnings to screen
  -9                create S9 object code instead of Intel hex
  -p                pipe mode: object file goes to stdout
  --                end of options

Example:

  asm6502 -l -o -w -e program.asm

This assembles the 6502 source file "program.asm", shows warnings and
errors to the screen, creates a listing file "program.asm.lst", and
puts the object code in an Intel hex format file named "program.asm.hex".

Notes:

  The '--' option is needed when you use -l or -o as the last option
  on the command line, so that they don't try to eat up your source
  file name.  It's really better to just put -l and -o first in the
  options.

  The value in -d must be a number.  No expressions.

  The -p and -o options are incompatible.  Only the last one on the
  command line will be used.  Normal screen output (pass number, total
  errors, error messages, etc.) always goes to stderr.

 - - -

EXPRESSIONS
===========

Whenever a value is needed, it goes through the expression evaluator.
The expression evaluator will attempt to do the arithmetic needed to
get a result.

Unary operations take a single value and do something with it.  The
supported unary operations are:

  + val        positive of val
  - val        negative of val
  ~ val        bitwise NOT of val
  ! val        logical NOT of val (returns 1 if val is zero, else 0)
  < val        low 8 bits of val
  > val        high 8 bits of val
  ..DEF sym    returns 1 if symbol 'sym' has already been defined
  ..UNDEF sym  returns 1 if symbol 'sym' has not been defined yet
  ( expr )     parentheses for grouping sub-expressions
  [ expr ]     square brackets can be used as parentheses when necessary
  'c'          One or two character constants, equal to the ASCII value
  'cc'           of c or cc.  In the two-byte case, the first character
                 is the high byte.
  H(val)       high 8 bits of val; whitespace not allowed before '('
  L(val)       low 8 bits of val; whitespace not allowed before '('

NOTE: with the Z-80, (expr), H(val), and L(val) are likely to not work
at the start of an expression because of Z-80 operand syntax.  Likewise
with the 6809, <val and >val may have special meaning at the start of
an operand.

Binary operations take two values and do something with them.  The
supported binary operations are:

  x * y        x multipled by y
  x / y        x divided by y
  x % y        x modulo y
  x + y        x plus y
  x - y        x minus y
  x << y       x shifted left by y bits
  x >> y       x shifted right by y bits
  x & y        bitwise x AND y
  x | y        bitwise x OR y
  x = y        comparison operators, return 1 if condition is true
  x == y          (note that = and == are the same)'
  x < y
  x <= y
  x > y
  x >= y
  x && y       logical AND of x and y (returns 1 if x !=0 and y != 0)
  x || y       logical OR of x and y (returns 1 if x != 0 or y != 0)

Numbers:

  .            current program counter
  *            current program counter
  $            current program counter
  $nnnn        hexadecimal constant
  nnnnH        hexadecimal constant
  0xnnnn       hexadecimal constant
  nnnn         decimal constant
  nnnnD        decimal constant
  nnnnO        octal constant
  %nnnn        binary constant
  nnnnB        binary constant

  Hexadecimal constants of the form "nnnnH" don't need a leading zero if
  there is no label defined with that name.

Operator precedence:

  ( ) [ ]
  unary operators: + - ~ ! < > ..DEF ..UNDEF
  * / %
  + -
  < <= > >= = == !=
  & & || || << >>

WARNING:
  Shifts and bitwise AND and OR have a lower precedence than the comparison
  operators!  You must use parentheses in cases like this!

  Example:
    Use "(OPTIONS & 3) = 2", not "OPTIONS & 3 = 2".  The former checks the
    lowest two bits of the label OPTIONS, the latter compares "3 = 2"
    first, which always results in zero.

  Also, unary operators have higher precedence, so if X = 255, "<X + 1" is
  256, but "<(X + 1)" is 0.

  With the 6809 assembler, a leading "<" or ">" often refers to an addressing
  mode.  If you really want to use the low-byte or high-byte operator, surround
  the whole thing with parentheses, like "(<LABEL)".  This does not apply to
  immediate mode, so "LDA #<LABEL" will use the low byte of LABEL.

NOTE:
  ..def and ..undef do not work with local labels.  (the ones that start with
  '@' or '.')

 - - -

LABELS AND COMMENTS
===================

Labels must consist of alphanumeric characters or underscores, and must
not begin with a digit.  Examples are "FOO", "_BAR", and "BAZ_99".  Labels
are limited to 255 characters.  Labels may also contain '$' characters, but
must not start with one.

Labels must begin in the first column of the source file when they are
declared, and may optionally have a ":" following them.  Opcodes with no
label must have at least one blank character before them.

Local labels are defined starting with "@" or ".".  This glues whatever
is after the "@" or "." to the last non-temporary code label defined so far,
making a unique label.  Example: "@1", "@99", ".TEMP", and "@LOOP".  These
can be used until the next non-local label, by using this short form.  They
appear in the symbol table with a long form of "LABEL@1" or "LABEL.1", but
can not be referenced by this full name.  Local labels starting with a "."
can also be defined as subroutine local, by using the SUBROUTINE pseudo-op.

Comments may either be started with a "*" as the first non-blank character
of a line, or with a ";" in the middle of the line.

Lines after the END pseudo-op are ignored as though they were comments,
except for LIST and OPT lines.

 - - -

PSEUDO-OPS
==========

These are all the opcodes that have nothing to do with the instruction
set of the CPU.

NOTE:
  All of the data pseudo-ops like DB, DW, and DS have a limit of 1023
  bytes of initialized data.  (This can be changed in asmguts.h if
  you really need it bigger.)

DB / FCB / BYTE / .BYTE / DC.B

  Defines one or more constant bytes in the code.  You can use as many
  comma-separated values as you like.  Strings use either single or
  double quotes.  Doubled quotes inside a string assemble to a quote
  character.  The backslash ("\") can escape a quote, or it can
  represent a tab ("\t"), linefeed ("\n"), or carriage return ("\r")
  character.  Hex escapes ("\0xnnn") are not supported.

DW / FDB / WORD / .WORD / DC.W

  Defines one or more constant 16-bit words in the code, using the
  native endian-ness of the CPU.  With the 6502, Z-80, and 8080, the
  low word comes first; with the 6809, the high word comes first.

DRW

  Define Reverse Word - just like DW, only the bytes are reversed.

DS / RMB / BLKB

  Skips a number of bytes, optionally initialized.

  Examples:
     DS 5     ; skip 5 bytes (generates no object code)
     DS 6,"*" ; assemble 6 asterisks

HEX

  Defines raw hex data.  Individual hex bytes may be separated by
  spaces.

  Examples:
     HEX 123456     ; assembles to hex bytes 12, 34, and 56
     HEX 78 9ABC DE ;  assembles to hex bytes 78, 9A, BC and DE
     HEX 1 2 3 4    ; Error: hex chars must be in pairs

FCC

  Motorola's equivalent to DB with a string.  Each string starts and
  ends with the same character.  The start/end character must not be
  alphanumeric or an underscore.

  Examples:
     FCC /TEXT/     ; 4 bytes "TEXT"
     FCC \TEXT\     ; 4 bytes "TEXT"


  In this assembler, FCC is extended by allowing it to work like DB
  afterward, only with a different quote character.  Also, the
  string delimiter can be repeated twice inside the string to
  include the delimiter inside the string.

  Examples:
     FCC /TEXT//TEXT/    ; 9 bytes "TEXT/TEXT"
     FCC /TEXT/,0        ; 5 bytes "TEXT" followed by a null
     FCC /TEXT/,0,/TEXT/ ; 9 bytes "TEXT", null, "TEXT"

  There is also a second mode where the length is specified, the text has
  no quotes, and the text is padded to the specified length with blanks.
  Be aware that if the text is too short, it will copy more data from your
  source line, even if you have a comment in the line!  However, it will
  stop copying when it encounters a tab character.

  Example:
     FCC 9,TEXT          <- this is 9 bytes "TEXT     "
     FCC 9,TEXT;comm     <- this is 9 bytes "TEXT;comm"
     FCC 9,TEXT;comment  <- this is 9 bytes "TEXT;comm", then an error from "ent"

ALIGN

  This ensures that the next instruction or data will be located on a
  power-of-two boundary.  The parameter must be a power of two (2, 4,
  8, 16, etc.)

END

  This marks the end of code.  After the END statement, all input is 
  ignored except for LIST and OPT lines.

EQU / = / SET

  Sets a label to a value.  The difference between EQU and SET is that
  a SET label is allowed to be redefined later in the source code.

ORG

  Sets the origin address of the following code.  This defaults to
  zero at the start of each assembler pass.

RORG

  Sets the relocated origin address of the following code.  Code
  in the object file still goes to the same addresses that follow
  the previous ORG, but labels and branches are handled as though
  the code were being assembled starting at the RORG address.

REND

  Ends an RORG block.  A label in front of REND receives the relocated
  address + 1 of the last relocated byte in the RORG / REND block.

SEG / RSEG / SEG.U segment

  Switches to a new code segment.  Code segments are simply different
  sections of code which get assembled to different addresses.  They
  remember their last location when you switch back to them.  If no 
  segment name is specified, the null segment is used.

  At the start of each assembler pass, all segment pointers are reset
  to zero, and the null segment becomes the current segment.

  SEG.U is for DASM compatibility.  DASM uses SEG.U to indicate an
  "unitialized" segment.  This is necessary because its DS pseudo-op
  always generates data even when none is specified.  Since the DS
  pseudo-op in this assembler normally doesn't generate any data,
  unitialized segments aren't supported as such.

  RSEG is for compatibility with vintage Atari 7800 source code.

LIST ON / LIST OFF / LIST MACRO / LIST NOMACRO
OPT LIST / OPT NOLIST / OPT MACRO / OPT NOMACRO

  These set assembler options.  Currently, the only options are to
  turn listing on and off, and to turn listing of macro expansion
  on and off.  The default is listing on, macro expansion off.

SUB / SUBROUTINE name

  This sets the scope for temporary labels beginning with a dot.
  If no name is specified, temporary labels beginning with a dot
  work just like the ones beginning with an '@'.

  Example:

               SUBROUTINE foo
       .LABEL  NOP        ; this one becomes "FOO.LABEL"
               SUBROUTINE bar
       .LABEL  NOP        ; this one becomes "BAR.LABEL"
               SUBROUTINE
       LABEL
       .LABEL  NOP        ; this one becomes "LABEL.LABEL"


ERROR

  This prints a custom error message.

MACRO / ENDM

  Defines a macro.  This macro is used whenver the macro name is
  used as an opcode.  Parameters are defined on the MACRO line,
  and replace values used inside the macro.

  Example:
     TWOBYTES  MACRO parm1, parm2     ; start recording the macro
               DB    parm1, parm2
               ENDM                   ; stop recording the macro

     TWOBYTES  1, 2        ; use the macro - expands to "DB 1, 2"

  An alternate form with the macro name after MACRO, instead of as
  a label, is also accepted.  A comma after the macro name is
  optional.

               MACRO plusfive parm
               DB    (parm)+5
               ENDM

  Macro parameters can be inserted without surrounding whitespace
  by using the '##' concatenation operator.

     TEST      MACRO labl
     labl ## 1 DB    1
     labl ## 2 DB    2
               ENDM

               TEST  HERE ; labl ## 1 gets replaced with "HERE1"
                          ; labl ## 2 gets replaced with "HERE2"

IF expr / ELSE / ELSIF expr / ENDIF

  Conditional assembly.  Depending on the value in the IF statement,
  code between it and the next ELSE / ELSIF / ENDIF, and code between
  an ELSE and an ENDIF, may or may not be assembled.

  ELSIF is the same as "ELSE" followed by "IF", only without the need
  for an extra ENDIF.

  Example:
     IF .undef mode
       ERROR mode not defined!
     ELSIF mode = 1
       JSR mode1
     ELSIF mode = 2
       JSR mode2
     ELSE
       ERROR Invalid value of mode!
     ENDIF

  IF statements inside a macro only work inside that macro.  When
  a macro is defined, IF statements are checked for matching ENDIF
  statements.

INCLUDE filename

  This starts reading source code from the named file.  The file is
  read once in each pass.  INCLUDE files can be nested to a maximum
  of 10 levels.  (This can be changed in asmguts.h if you really need
  it bigger.)

There are also a few CPU-specific pseudo-ops:

PROCESSOR 6502
PROCESSOR 65C02
PROCESSOR 6502U

  With the 6502 assembler, this selects either 6502 opcodes, 65C02
  opcodes, or 6502 with undocumented opcodes.  It defaults to 6502
  at the start of each assembler pass.

PROCESSOR 6800 / 6801 / 6802 / 6803 / 6808 / 6303 / 68HC11

  With the 68HC11 assembler, this selects an appropriate instruction
  subset.  It defaults to 68HC11 at the start of each assembler pass.

CPU

  This is an alias for PROCESSOR.

.6502 / .68HC11 / etc.

  The CPU type can be specified this way in addition to the CPU and
  PROCESSOR pseudo-ops.

SETDP value

  With the 6809 assembler, this sets the current value of the
  direct page register, for determining whether to use direct or
  extended mode.  It defaults to zero at the start of each
  assembler pass.

 - - -

SYMBOL TABLE DUMP
=================

The symbol table is dumped at the end of the listing file.  Each
symbol shows its name, value, and flags.  The flags are:

  U  Undefined        - this symbol was referenced but never defined
  M  Multiply defined - this symbol was defined more than once with
                        different values (only the first is kept)
  S  SET - this symbol was defined with the SET pseudo-op
  E  EQU - this symbol was defined with the EQU pseudo-op, or from
           the -d command line option

 - - -

CHANGE HISTORY
==============

Version 1.1 changes (April 1995) (this version was on the original Starpath
"Stella Gets a New Brian" CD)

* Fixed a bug that would prevent using any label beginning with the letter
  'A' as the first parameter of an opcode.

* Prevented output of unnecessary origin commands to .DAT file to avoid
  complaints from MAKEROM.

* Fixed the "data overwritten" error in MAKEROM, which was only displaying
  the low byte of the offending address.

* Improved MAKEROM's handling of out-of-range memory addresses.

* Added a basic macro feature, controlled by the following pseudo-ops:

    <label>  MACRO         start recording the macro with the name <label>
             ENDM          end the macro
             LIST MACRO    enable macro expansion in listings
             LIST NOMACRO  disable macro expansion in listings (default)
             <label>       use the macro

  Note that macros can not currently call other macros.

 - - -

Version 1.2 changes (September 1996)

* Added macro parameters

* Added FCB, FDB, RMB, BYTE, WORD, and BLKB pseudo-ops

* Added binary constants (xxxxxxxxB and %xxxxxxxx)

* Added ASLA, ROLA, LSRA, and RORA opcodes

* Added '$' by itself as the current location, in addition to '.' and '*'

* DB and DW opcodes now work properly with multiple operands

* Added &, |, <<, and >> to expression evaluator.  Each of these operators
  has the same precedence, below + and -.

* Default file extension was changed from .A65 to .ASM

* Allowed command line options to be specified in lowercase

* Added a new RORG "relative ORG" pseudo-op to allow assembling code that
  will later be moved to a different address

 - - -

Version 1.3 changes (December 1996)

* No more than one macro could be defined, because the macro list wasn't
  being linked properly.

* Errorlevel 1 is now returned if any errors occurred during assembly.

 - - -

Version 1.4 changes (February 2002)

* Added FCC pseudo-op

* Lines beginning with "*" are now comments

* Jump indirect wouldn't work correctly if the line had a comment

* No error was generated if too many macro parameters were used

* Maximum macro parameters increased from 5 to 10

* Lines from macro expansions with errors are now displayed

* Symbol table is now displayed with more than one column

* Lines that generate more than 3 bytes are flagged with a "+"

 - - -
 
Version 1.5 changes (2004-02-24)

* Converted from Pascal to C.

* Added 65C02 opcodes.  They can be disabled at compile time with a #define.

* Improved output with more than 3 bytes of object code from a line.

* Now outputs code in Intel Hex format.  Old object format still available
  by changing a #define.

 - - -

Version 1.6 changes (2004-04-30)

* Separated common code so that my Z-80 assembler could use the new features
  like macros and INCLUDE.

* Fixed a bug that would cause phase errors with one-character label names.

* Fixed a bug that would prevent control pseudo-ops from working after END
  pseudo-op.

* Added DC.B, DC.W, .BYTE, and .WORD pseudo-ops for compatibility with source
  code written for various other assemblers.

* DB now handles double quotes, \r \n \t \' and \".  A bug in the handlng of
  paired quotes ("foo""bar") was fixed.

* new DRW pseudo-op to do a DW in reverse order (such as for Atari 7800 Maria
  chip display lists)

* added INCLUDE pseudo-op.  Maximum nesting level is set to 10 by a #define.

* added PROCESSOR 6502/PROCESSOR 65C02 pseudo-op to select 65C02 instructions,
  and for improved DASM compatiblity.  The default is 6502 instructions only.

* increased the size of bytStr[] from 256 to 1024 bytes.  This will allow more
  data for long-data pseudo-ops like DB, etc.

* added support for DS pseudo-op with an initializer.  This is currently
  limited to a max of 1024 bytes.  (DB, DW, and FCC also have this limit, but
  do not yet have a bounds check for it.)

* added HEX pseudo-op.  This also currently limited to a max of 1024 bytes.

* [ and ] can now be used as parentheses in expressions, for better DASM source
  compatibility.

* REND pseudo-op added for better DASM source compatibility.  Note that a label
  on the REND line will receive the relocated value of the last relocated code
  byte + 1.

 - - -

Version 1.7 changes (2004-08-25)

* Added a 6809 assembler back-end.

* Added an 8085 assembler back-end.  Couldn't just add 8080/8085 style opcodes
  to the Z-80 assembler because the JP opcode with no condition in the Z-80
  style mnemonics conflicts with JP in the 8080/8085 style mnemonics.
  Irony: this assembler originally started out a long time ago as an 8080
  assembler, and now it is again!  I did this new 8080 code from scratch,
  rather than porting the old Turbo Pascal code to C.

* Added a 68HC11 assembler back-end, with support for 6800, 6801, and 6303
  instruction set variations.

* Added support for undocumented 6502 instructions, enabled with "PROCESSOR
  6502U".  Only the most reliable and useful instructions are implemented:
  3-cycle NOP (mnemonic NOP3), LAX, DCP, ISB, RLA, RRA, SAX, SLO, SRE, ANC,
  ARR, ASR, and SBX.

* Added pseudo-ops to 6502 assembler to select CPU type without PROCESSOR
  pseudo-op:  .6502, .6502U, .65C02

* Added conditional assembly using IF <expr> / ELSE / ELSIF <expr> / ENDIF.
  Maximum official nesting level is 255, but beyond that it will still work if
  you don't try wierd stuff like multiple ELSE statements in a row.  <expr> is
  an expression that is false if it is equal to 0 and true if it is not equal
  to 0.

* Added new operators to the expression evaluator, because IF needed them:

  ..def symbol (returns 1 if symbol is defined)
  ..undef symbol (returns 1 if symbol is not defined)
  &&, || (boolean and/or, returns either 1 or 0)
  < <= > >= = == != (returns either 1 or 0; = and == are the same)

  Beware the operator precedence when using these!

* Added new H() and L() operators for compatiblity with vintage Atari 7800
  source code, equivalent to >() and <().  Note that no whitespace is allowed
  before the left paren.

* Symbols may now contain the '$' character, but they must not start with it.
  This, combined with H() and L(), will allow the original Atari 7800 ROM
  source code to compile as-is.

* Single quote operator can now handle 2-byte constants.  The first character
  becomes the high byte of the constant.

* Added 0xnnnn hexadecimal constants (but not 0nnn octal constants).

* Fixed problems with DB, DW, DS, and FCC pseudo-ops.

* Added bounds checking to DB, DW, and FCC pseudo-ops.  They are now limited to
  MAX_BYTSTR - 1 bytes (1023).

* Added ERROR pseudo-op.

* Added ALIGN pseudo-op.

* Added SEG/RSEG/SEG.U pseudo-op.

* INCLUDE pseudo-op can now accept file names surrounded by single or double
  quotes.

* Handled division by zero.

* Improved handling of form feed characters in source files.

* Fixed some bugs in macro handling, and expanded maximum macro parameters from
  10 to 30.

* Temporary labels can now start with '.' in addition to '@'.

* Labels can be defined from the command line with "-d label=value".  If value
  is not specified, the label is set to zero.

* Added -9 command line option to generate Motorola S9-record code output.

* Added -c command line option to send object code to stdout, to allow piping
  to makerom.

* Console output now goes to stderr.

* Error messages now include file name and line number.

* Bytes out of -127..256 range now generate a warning instead of an error.

* Rewrote the documentation.

 - - -

Version 1.7.1 changes (2004-10-20)

* Added exclusive-or ("^") to the expression evaluator.  This has the same
  precedence as the existing AND and OR operators.

* DB has been modified to handle the case of arithmetic parameters which
  start with a single-quoted character, such as "DB 'X'+$80".

* 6809 conditional long branches were off by one.

* Changed 68HC11 assembler file name to all lowercase.

* Found out that asm68hc11.c wasn't being included in the zip archive anyhow.

* Added an F8 assembler back-end.  Since I have no real code to test this on,
  it should be considered experimental.

 - - -

Version 1.7.2 changes (2005-03-02)

* Added the SUBROUTINE pseudo-op.

* Added BLKB pseudo-op as an alias to DS.

* Fixed a bug that would cause errors if the line after an ENDM
  was not a blank line.

* F8 assembler relative branches were off by one.

* Other minor changes to F8 assembler, including allowing expressions for
  register numbers in more places, and 'A' and 'B' as hex register numbers
  when not inappropriate.

* Tweaked symbol table output to show up to 19 characters of symbol names,
  and to fit in 80 columns.

* ENDIF lines and the guts of MACRO declarations did not respect the
  LIST OFF setting.

* Macro parameters can now be inserted without surrounding whitespace using
  the '##' concatenation operator, similar to how it works in C macros.

* 6502 assembler can now force absolute or absolute-indexed addressing mode
  (instead of zero-page and zero-page-indexed) by using '>' in front of the
  address, similar to how the 6809 does it.

* When not using the -w option, warning lines would still print to the
  console.

* Z-80 assembler can now take parameters after a RST instruction, which are
  interpreted as DB bytes.  Example:  RST 08H,'('

* Fixed a bug that could cause phase errors if a forward-referenced label was
  used with DW.

* started work on REP pseudo-op (code still under construction)

* Added an 1802 assembler back-end.

* Added opcode table of common pseudo-ops to asmguts.h, and removed "DoStdOpcode"
  pass-thru in order to keep more generic stuff out of the CPU-specific .c files.

* CPU_BIG_ENDIAN/CPU_LITTLE_ENDIAN #define now automatically controls
  DW/DRW opcodes and Instr3W/Instr4W/Instr5W calls.

* Cleanups for GCC 4 stricter type checking: string signedness with Str255 type
  and forward declaration of OpcdTab.

* Short branch range check was allowing branches too far forward.

* Z-80 assembler couldn't do "CP (a+b)/256" (or SUB, AND, OR, XOR) because it was
  expecting "CP (HL)" etc.  Now it falls back and tries to evaluate the parens as
  an immediate operand.

* Added OPT NOEXPAND to disable expanding hex output for more than one listing line.

* All standard pseudo-ops can now optionally start with a period, such as .DB, .DW,
  .EQU, etc.

* Standard pseudo-ops can start with a period in column 1.

* Symbol table output handles long symbols better by stretching really long symbol
  names into multiple columns.

 - - -

To do:

* Implement REP (or REPEAT) pseudo-op (currently under construction).

* Implement multi-level nested macros.

 - - -

BUGS:

(none currently known)
