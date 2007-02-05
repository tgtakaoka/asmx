ASMX semi-generic assembler

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
">" before an absolute or absolute-indexed address operand.  (Note
that this usage is different from "<" and ">" as a high/low byte
of a word value.)

Some assemblers like to output code in binary.  This might be nice
if you're making a video game cartridge ROM, but it's really not
very flexible.  Intel and Motorola both came up with very nice text
file formats which don't require any kind of padding when you do an
ORG instruction, and don't require silly "segment" definitions
just to keep DS instructions from generating object code.  Then,
following the Unix philosophy of making tools that can connect to
other tools, you can pipe the object code to another utility which
makes the ROM image.

Anyhow, it works pretty well for what I want it to do.

 - Bruce -

 - - -

BUILDING IT
===========

Version 2.0 now has a make file that should work on Unix/Linux
type operating systems.  Just run the "make" command from the src
sub-directory and it will create the asmx binary.  To install
it, type "make install" and it will install to ~/bin (unless you
change the makefile to install it somewhere else).  Symbolic links
are generated so that each CPU assembler can be used with a
separate command, as in version 1.

If you can't use the make file, the simplest way is this:

  gcc *.c -o asmx

That's it.  Now you will probably want to copy it to your
/usr/local/bin or ~/bin directory, but that's your choice.

 - - -

RUNNING IT
==========

Just give it the name of your assembler source file, and
whatever options you want.

  asm6502 [options] srcfile

Here are the command line options:

    --                  end of options
    -e                  show errors to screen
    -w                  show warnings to screen
    -l [filename]       make a listing file, default is srcfile.lst
    -o [filename]       make an object file, default is srcfile.hex or srcfile.s9
    -d label[[:]=value] define a label, and assign an optional value
    -s9                 output object file in Motorola S9 format (16-bit address)
    -s19                output object file in Motorola S9 format (16-bit address)
    -s28                output object file in Motorola S9 format (24-bit address)
    -s37                output object file in Motorola S9 format (32-bit address)
    -c                  send object code to stdout
    -C cputype          specify default CPU type (currently 6502)

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

  The value in -d must be a number.  No expressions are allowed.  The
  valid forms are:
    -d label          defines the label as EQU 0
    -d label=value    defines the label as EQU value
    -d label:=value   defines the label as SET value

  The -c and -o options are incompatible.  Only the last one on the
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

NOTE: with the Z-80, (expr), H(val), and L(val) will likely not work at
the start of an expression because of Z-80 operand syntax.  Likewise
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
set of the CPU.  All pseudo-ops can be preceeded with a "." (example:
".BYTE" works the same as "BYTE")

NOTE:
  All of the data pseudo-ops like DB, DW, and DS have a limit of 1023
  bytes of initialized data.  (This can be changed in asmx.h if
  you really need it bigger.)

.6502 / .68HC11 / etc.

  The CPU type can be specified this way in addition to the CPU and
  PROCESSOR pseudo-ops.

ASCIC

  Creates a text string preceeded by a single byte indicating the length
  of the string.  This is equivalent to a Pascal-style string.

ALIGN

  This ensures that the next instruction or data will be located on a
  power-of-two boundary.  The parameter must be a power of two (2, 4,
  8, 16, etc.)

CPU

  This is an alias for PROCESSOR.

DB / FCB / BYTE / DC.B

  Defines one or more constant bytes in the code.  You can use as many
  comma-separated values as you like.  Strings use either single or
  double quotes.  Doubled quotes inside a string assemble to a quote
  character.  The backslash ("\") can escape a quote, or it can
  represent a tab ("\t"), linefeed ("\n"), or carriage return ("\r")
  character.  Hex escapes ("\xFF") are also supported.

DW / FDB / WORD / DC.W

  Defines one or more constant 16-bit words in the code, using the
  native endian-ness of the CPU.  With the 6502, Z-80, and 8080, the
  low word comes first; with the 6809, the high word comes first.
  Quoted text strings are padded to a multiple of two bytes.  The
  data is not aligned to a 2-byte address.

DL / LONG / DC.L

  Defines one or more constant 32-bit words in the code, using the
  native endian-ness of the CPU.  With the 6502, Z-80, and 8080, the
  low word comes first; with the 6809, the high word comes first.
  Quoted text strings are padded to a multiple of four bytes.  The
  data is not aligned to a 4-byte address.

DRW

  Define Reverse Word - just like DW, only the bytes are reversed.

DS / RMB / BLKB

  Skips a number of bytes, optionally initialized.

  Examples:
     DS 5     ; skip 5 bytes (generates no object code)
     DS 6,"*" ; assemble 6 asterisks

   Note that no forward-reference values are allowed for the length
   because this would cause phase errors.

ERROR

  This prints a custom error message.

EVEN

  This is an alias for ALIGN 2.

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

END

  This marks the end of code.  After the END statement, all input is 
  ignored except for LIST and OPT lines.

EQU / = / SET / :=

  Sets a label to a value.  The difference between EQU and SET is that
  a SET label is allowed to be redefined later in the source code.
  EQU and '=' are equivalent, and SET and ':=' are equivalent.

HEX

  Defines raw hexadecimal data.  Individual hex bytes may be separated by
  spaces.

  Examples:
     HEX 123456     ; assembles to hex bytes 12, 34, and 56
     HEX 78 9ABC DE ;  assembles to hex bytes 78, 9A, BC and DE
     HEX 1 2 3 4    ; Error: hexadecimal digits must be in pairs

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
  of 10 levels.  (This can be changed in asmx.c if you really need
  it bigger.)

LIST / OPT

  These set assembler options.  Currently, the options are:

    LIST ON / OPT LIST            Turn on listing
    LIST OFF / OPT NOLIST         Turn off listing
    LIST MACRO / OPT MACRO        Turn on macro expansion in listing
    LIST NOMACRO / OPT NOMACRO    Turn off macro expansion in listing
    LIST EXPAND / OPT EXPAND      Turn on data expansion in listing
    LIST NOEXPAND / OPT NOEXPAND  Turn off data expansion in listing
    LIST SYM / OPT SYM            Turn on symbol table in listing
    LIST NOSYM / OPT NOSYM        Turn off symbol table in listing

  The default is listing on, macro expansion off, data expansion on,
  symbol table on.

MACRO / ENDM

  Defines a macro.  This macro is used whenver the macro name is
  used as an opcode.  Parameters are defined on the MACRO line,
  and replace values used inside the macro.

  Macro calls can be nested to a maximum of 10 levels.  (This can
  be changed in asmx.c if you really need it bigger.)

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

  When a macro is invoked with insufficient parameters, the remaining
  parameters are replaced with a null string.  It is an error to invoke
  a macro with too many parameters.

  Macro parameters can be inserted without surrounding whitespace
  by using the '##' concatenation operator.

     TEST      MACRO labl
     labl ## 1 DB    1
     labl ## 2 DB    2
               ENDM

               TEST  HERE ; labl ## 1 gets replaced with "HERE1"
                          ; labl ## 2 gets replaced with "HERE2"

  Macro parameters can also be inserted by using the backslash ("\")
  character.  This method also includes a way to access the actual
  number of macro parameters supplied, and a unique identifier for
  creating temporary labels.

  \0 = number of macro parameters
  \1..\9 = nth macro parameter
  \? = unique ID per macro invocation (padded with leading zeros to five digits)

  NOTE:  The line with the ENDM may have a label, and that will be included in the
  macro definition.  However if you include a backslash escape before the ENDM, the
  ENDM will not be recognized, and the macro definition will not end.  Be careful!

ORG

  Sets the origin address of the following code.  This defaults to
  zero at the start of each assembler pass.

PROCESSOR

  This selects a specific CPU type to assemble code for.  Some assemblers
  support multiple CPU sub-types.  Currently supported CPU types are:

     1802          RCA 1802
     6502          MOS Technology 6502
     6502U         MOS Technology 6502 with undocumented instructions
     65C02         Rockwell 65C02
     6809          Motorola 6809
     6800 6802     Motorola 6800
     6801 6803     Motorola 6801
     6805          Motorola 6805
     6303          Hitachi 6303 (6800 variant)
     6811 68HC11   Motorola 68HC11 variants
     68HC711 68HC811
     68HC99
     68HC16        Motorola 68HC16
     68K 68000     Motorola 68000
     68010         Motorola 68010
     8051          Intel 8051 variants
     8080          Intel 8080
     8085          Intel 8085
     8085U         Intel 8085 with undocumented instructions
     F8            Fairchild F8
     JERRY         Atari Jaguar DSP
     TOM           Atari Jaguar GPU
     Z80           Zilog Z-80
     GBZ80         Gameboy Z-80 variant

  At the start of each pass, this defaults to the assembler specified
  in the "-C" command line option, if any, or the assembler type determined
  from the name of the executable used on the command line.  The latter is
  useful with soft-links when using Unix-type systems.  In that case, the
  default assembler name can be determined by looking at the end of the
  executable name used to invoke asmx, then selecting that CPU type.

  If no default assembler is specified, the DW/WORD and DL/LONG pseudo-ops
  will generate errors because they do not know which endian order to use.

  Opcodes for the selected processor will have priority over generic
  pseudo-ops.  However, assemblers for CPUs which have a "SET" opcode have
  been specifically designed to pass control to the generic "SET" pseudo-op.

REND

  Ends an RORG block.  A label in front of REND receives the relocated
  address + 1 of the last relocated byte in the RORG / REND block.

RORG

  Sets the relocated origin address of the following code.  Code
  in the object file still goes to the same addresses that follow
  the previous ORG, but labels and branches are handled as though
  the code were being assembled starting at the RORG address.

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

SUBROUTINE / SUBR name

  This sets the scope for temporary labels beginning with a dot.
  At the start of each pass, and when this pseudo-op is used with
  no name specified, temporary labels beginning with a dot use the
  previous non-temporary label, just as the temporary labels
  beginning with an '@'.

  Example:

       START
       .LABEL  NOP        ; this becomes "START.LABEL"
               SUBROUTINE foo
       .LABEL  NOP        ; this becomes "FOO.LABEL"
               SUBROUTINE bar
       .LABEL  NOP        ; this becomes "BAR.LABEL"
               SUBROUTINE
       LABEL
       .LABEL  NOP        ; this becomes "LABEL.LABEL"

ZSCII

  Creates a compressed text string in the version 1 Infocom format.
  Otherwise, this works exactly like the DB pseudo-op.  Note that this
  will always generate a multiple of two bytes of data.

  WARNING: using a forward-referenced value could cause phase errors!

  See http://www.wolldingwacht.de/if/z-spec.html for more information
  on the compressed text format.


There is also one CPU-specific pseudo-op:

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

Version 1.7.2 changes (2005-08-21)

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

* Standard pseudo-ops can now start with a period in column 1.

* Symbol table output handles long symbols better by stretching really long symbol
  names into multiple columns.

 - - -

Version 1.7.3 changes (2006-01-23)

* Added "\0".."\9" macro parameter substitution as per the ASnn series of
  assemblers at http://www.kingswood-consulting.co.uk/assemblers/.  Even though
  it seems a bit silly to have more than one way to access macro parameters,
  this does provide a way to get the number of macro parameters, and to generate
  unique labels inside a macro.

  \0 = number of macro parameters
  \1..\9 = nth macro parameter
  \? = unique ID per macro invocation (padded with leading zeros to five digits)

* 6809 SETDP pseudo-op was broken by the opcode table rearrangement in 1.7.2.
  It has been fixed.

* Added the ":=" pseudo-op as an alias for the "SET" pseudo-op.

* Multi-level nested macros have been implemented.

* The "SET" pseudo-op now works properly for Z-80.  The SET opcode was hiding
  access to the pseudo-op, though other variations (".SET" and ":=") would still
  work.  This fix is tricky enough that there may be some wierd side-effects in
  error situations.  Some bugs with Z-80 SET opcode error handling were fixed as
  well.

* Added backslash hex escapes ("\xFF") for strings in DB pseudo-op.

* "Short branch out of range" was changed from a warning to an error.

* Added an 8051 assembler back-end.

 - - -

Version 1.7.4 changes (2006-11-09)

* Updated Z-80 assembler with improved parsing techniques from the 8051
  assembler.

* In Z-80 assembler, "LD BC,(foo - 1) * 256" wouldn't assemble properly.

* Added ZSCII pseudo-op to generate Infocom-style compressed text strings
  in the version 1 format.  (versions 2 and later use a dictionary table
  which is impractical to supply or generate)

* Fixed a bug that caused long DB, DW, etc. statements to list most of their
  lines in a macro when macro listing was turned off.

* Fixed a bug that could cause a negative result to appear as "FFFF" in the
  listing for the EQU and RORG pseudo-ops.

* Added LIST SYM / LIST NOSYM to disable symbol table in listing file.

* Added "FLAG.BIT" format to EQU / SET pseudo-op in 8051 assembler.

* ELSIF pseudo-op would cause a "Too many operands" error if its matching IF
  expression evaluated to true.

 - - -

Version 1.8 changes (2007-01-11)

* made changes to allow for unified assembler
  * got rid of instr[] vs bytStr[] distinction and INSTR_MAX
  * new common error calls: BadMode() MissingOperand()
  * InstrFoo() calls renamed to be a bit more descriptive (Instr5W -> InstrBBBW, etc.)
  * made endian a variable, not a #define
  * removed usage() from invidivual assemblers
  * NUKED HARD TABS in C source code
  * updated other assemblers to use FindReg/GetReg from Z80
  * listing files now use eight more columns for hex data, and with new InstrFoo() calls,
    spaces can now be put between opcodes and operands in hex data
  * common assembler code is now mostly 32-bit clean

* added a 68000 assembler. 68010-only instructions can be enabled with a #define, except
  BKPT was left enabled because it is still semi-valid for 68000/68008

* 68HC16 referred to PSHM as "PUSHM" in a few places

* 68HC16 was mis-assembling ANDP and ORP as 8-bit immediate instructions

* 8051 was mis-assembling ORL/ANL/XRL dir,#imm instructions

* 8051 was mis-assembling ORL/ANL/XRL dir,A instructions

* added new ASCIIC pseudo-op for counted-length (Pascal-style) strings

* single-quoted character constants can now use the same backslash escapes as
  the double-quoted strings in the DB pesudo-op

* added support for "three-tab" listing format with hex data in 24 columns
  which puts blanks between parts of instructions

* added support for 32-bit addresses and symbols in listings (requires three-tab)

* symbols can now start with a $ if you set a #define, but this prevents
  hexadecimal constants starting with a $ from working

* single-quoted constants are no longer limited to two bytes

* CR-terminated source files would do bad things

* object code output for Intel hex records now supports 32-bit addresses using record
  types 04 and 05

* object code output for Motorola S9 records now supports 32-bit addresses using new
  "-s##" command line option, where ## is the type of file: 9, 19, 28, or 37.  The number
  is used for the file name of the object code output file.  9 and 19 are identical except
  for the name of the object code output file.

* fixed a really subtle bug that would cause ALIGN/EVEN to use up bytes when they weren't
  supposed to (this bug dates back to the first appearance of ALIGN!)

* DS pseudo-op no longer allows forward-declared lengths, which cause phase errors

* DC.W and DC.L now suport text strings with null padding alignment after every string literal.
  Note that alignment is internal-only, and any alignment error at the start will be preserved.

 - - -

Version 2.0a1 changes (2007-01-12)

* unified all assemblers into one binary

  CPU type selection is as follows:

  - if the CPU type can be determined from the executable name (either by renaming the
    executable or creating a Unix file link to it), that becomes the default CPU type at
    the start of each assembler pass
  - using the CPU or PROCESSOR pseudo-op selects a new CPU type
  - using a "." followed by the CPU type (".68000") also selects a new CPU type
  - note that if no CPU has been selected and there is no default, the DW and DL pseudo-ops
    will generate an error because they don't know what the current endian setting should be
  - symbol table dump is in the widest address width used

* 8080 and 8085 are now broken up in to separate CPU types along with 8085U for the
  undocumented 8085 instructions

* SUB pseudo-op alias was changed to SUBR because so many CPUs already have a SUB opcode

* added an 8048 assembler, with no specific support for 8041/8021/8022 subsets yet

 - - -

Version 2.0a2 changes (2006-01-13)

* Z80 would allow LD r,I or LD r,R with destination registers other than A

* added Gameboy variant to Z80 assembler

* fix from 2.0a1: ".CPUTYPE" in column 1 now works

* Z80 ADD/ADC/SBC can be made to not require the "A," if you #define NICE_ADD

 - - -

Version 2.0a3 changes (2007-01-14)

* Fixed a bug with 68K CMP "Dn,Dn" that got the registers reversed

 - - -

Version 2.0a4 changes (2007-02-02)

* added a 6805 assembler

* fix from 2.0a1: symbol table dump was always 32-bits, now fixed

* fix from 2.0a2: Gameboy Z80 support had CPU type flags moved from opcode table parm
  to opcode type instead.

* fixed a problem with ..DEF/..UNDEF which would cause them to return incorrect results
  during the first assembler pass, resulting in phase errors if they were used for
  conditional assembly

* 68K: EA of "(operand)" or "(operand.W)" or "(operand.L)" did not work

* 68K: EA of "0(An)" or "(0,An)" now assembles as "(An)" if offset contains no forward defs

* 68K: "TST (1)*4(A1)" didn't work but "MOVE.L (A0,D0.W),1*4(A1)" did

* 68K: ADD/CMP/SUB Dn,An now assembles as ADDA/CMPA/SUBA, but EA,An still reports an error

* -d command line option now does a SET rather than an EQU if you use ":=" instead of "="

* added 65C816 support to 6502 assembler

* rearranged multi-CPU handling so that each CPU type could have its own endian and listing
  parameters, and its own opcode table, too.  This was primarily done because of 65C816.

* added a new ADDR_24 listing parameter for CPUs with 24-bit address such as 68000 and 65C816

* Macros could be used before they are defined.  In the first pass there would be an error,
  but in the second pass code would be generated and phase errors would happen.  Now an
  error is generated if a macro is used before it is defined.

 - - -

Version 2.0b1 changes (2007-02-05)

* 68K: "EOR Dn,Dn" did not assemble properly

* ADDR_24 list mode was one character too wide for DB data

* Added Atari Jaguar GPU/DSP support.
  - Bonuses: ROLQ instruction equivalent to RORQ 32-n,Rn
             pre-defined JR/JUMP condition codes
  - Caveat: labels are byte addresses, so if you use MOVE PC,Rn and compare the address,
    or if you make a table of addresses, you need to divide by two.  This needs some way
    to inherently divide Tom/Jerry code addresses by 2. I'll probably figure something
    out before the final 2.0 version.

 - - -

To do:

* need to test what happens with 32-bit symbols on 16-bit and 24-bit address CPUs
  - negative symbols vs $8000-$FFFF symbols?  maybe RefSym should sign-extend from 16 bits?
  - masking or wraparound of values for location counter?  (sign-extend locPtr as "." too?)

* 65C816: need addressing mode force characters

* add Z80 undocumented instructions

* double-check 8048 instruction set, and add support for 8048 variants

* need an option to not list temp symbols (containing a "." or "@") in DumpSymTab

* see if it's possible to get labels starting with "$" compatible with $xxxx hex constants,
  maybe in RefSym?

* The SUBROUTINE pseudo-op needs to be tweaked.  It should either define the subroutine name
  as a label, or use the label on the left side of the line as the name of the subroutine.

* Implement REP (or REPEAT) pseudo-op (currently under construction).

* Implement ".FOO." operators? (.SHL. .AND. .OR., etc.)  If I do this, I will probably change
  ..DEF and ..UNDEF to .DEF. and .UNDEF.

* 6809 WARNDP pseudo-op? (now I can't remember what this was supposed to be)

* Linkable/relocatable object code files (long term 3.0 goal).

 - - -

BUGS:

(none currently known)

