; 6309 instruction set test

DIRECT	EQU	$44
EXTEND	EQU	$4558
INDEX_B	EQU	$58
INDEX_W	EQU	$4958
IMMED_B	EQU	$49
IMMED_W	EQU	$494D
IMMED_Q	EQU	$51554144

	NEG	<DIRECT
	OIM	<DIRECT		; 6309
	AIM	<DIRECT		; 6309
	COM	<DIRECT
	LSR	<DIRECT
	EIM	<DIRECT		; 6309
	ROR	<DIRECT
	ASR	<DIRECT
	ASL	<DIRECT
	ROL	<DIRECT
	DEC	<DIRECT
	TIM	<DIRECT		; 6309
	INC	<DIRECT
	TST	<DIRECT
	JMP	<DIRECT
	CLR	<DIRECT

	FCB	$10	; 10xx
	FCB	$11	; 11xx
	NOP
	SYNC
	SEXW			; 6309
	FCB	$15	; TEST
	LBRA	*+3
	LBSR	*+3
	FCB	$18
	DAA
	ORCC	#IMMED_B
	FCB	$1B
	ANDCC	#IMMED_B
	SEX
	EXG	D,D
	TFR	D,D

	BRA	*+2
	BRN	*+2
	BHI	*+2
	BLS	*+2
	BCC	*+2
	BCS	*+2
	BNE	*+2
	BEQ	*+2
	BVC	*+2
	BVS	*+2
	BPL	*+2
	BMI	*+2
	BGE	*+2
	BLT	*+2
	BGT	*+2
	BLE	*+2

	LEAX	,X
	LEAY	,X
	LEAS	,X
	LEAU	,X
	PSHS	CC,D,DP,X,Y,U,PC
	PULS	CC,D,DP,X,Y,U,PC
	PSHU	CC,D,DP,X,Y,S,PC
	PULU	CC,D,DP,X,Y,S,PC
	FCB	$38
	RTS
	ABX
	RTI
	CWAI	#IMMED_B
	MUL
	FCB	$3E	; SWR
	SWI

	NEGA
	FCB	$41
	FCB	$42
	COMA
	LSRA
	FCB	$45
	RORA
	ASRA
	ASLA
	ROLA
	DECA
	FCB	$4B
	INCA
	TSTA
	FCB	$4E
	CLRA

	NEGB
	FCB	$51
	FCB	$52
	COMB
	LSRB
	FCB	$55
	RORB
	ASRB
	ASLB
	ROLB
	DECB
	FCB	$5B
	INCB
	TSTB
	FCB	$5E
	CLRB

	NEG	,X
	OIM	,X		; 6309
	AIM	,X		; 6309
	COM	,X
	LSR	,X
	EIM	,X		; 6309
	ROR	,X
	ASR	,X
	ASL	,X
	ROL	,X
	DEC	,X
	TIM	,X		; 6309
	INC	,X
	TST	,X
	JMP	,X
	CLR	,X

	NEG	>EXTEND
	OIM	>EXTEND		; 6309
	AIM	>EXTEND		; 6309
	COM	>EXTEND
	LSR	>EXTEND
	EIM	>EXTEND		; 6309
	ROR	>EXTEND
	ASR	>EXTEND
	ASL	>EXTEND
	ROL	>EXTEND
	DEC	>EXTEND
	TIM	>EXTEND		; 6309
	INC	>EXTEND
	TST	>EXTEND
	JMP	>EXTEND
	CLR	>EXTEND

	SUBA	#IMMED_B
	CMPA	#IMMED_B
	SBCA	#IMMED_B
	SUBD	#IMMED_W
	ANDA	#IMMED_B
	BITA	#IMMED_B
	LDA	#IMMED_B
	FCB	$87
	EORA	#IMMED_B
	ADCA	#IMMED_B
	ORA	#IMMED_B
	ADDA	#IMMED_B
	CMPX	#IMMED_W
	BSR	*+2
	LDX	#IMMED_W
	FCB	$8F

	SUBA	<DIRECT
	CMPA	<DIRECT
	SBCA	<DIRECT
	SUBD	<DIRECT
	ANDA	<DIRECT
	BITA	<DIRECT
	LDA	<DIRECT
	STA	<DIRECT
	EORA	<DIRECT
	ADCA	<DIRECT
	ORA	<DIRECT
	ADDA	<DIRECT
	CMPX	<DIRECT
	JSR	<DIRECT
	LDX	<DIRECT
	STX	<DIRECT

	SUBA	,X
	CMPA	,X
	SBCA	,X
	SUBD	,X
	ANDA	,X
	BITA	,X
	LDA	,X
	STA	,X
	EORA	,X
	ADCA	,X
	ORA	,X
	ADDA	,X
	CMPX	,X
	JSR	,X
	LDX	,X
	STX	,X

	SUBA	>EXTEND
	CMPA	>EXTEND
	SBCA	>EXTEND
	SUBD	>EXTEND
	ANDA	>EXTEND
	BITA	>EXTEND
	LDA	>EXTEND
	STA	>EXTEND
	EORA	>EXTEND
	ADCA	>EXTEND
	ORA	>EXTEND
	ADDA	>EXTEND
	CMPX	>EXTEND
	JSR	>EXTEND
	LDX	>EXTEND
	STX	>EXTEND

	SUBB	#IMMED_B
	CMPB	#IMMED_B
	SBCB	#IMMED_B
	ADDD	#IMMED_W
	ANDB	#IMMED_B
	BITB	#IMMED_B
	LDB	#IMMED_B
	FCB	$C7
	EORB	#IMMED_B
	ADCB	#IMMED_B
	ORB	#IMMED_B
	ADDB	#IMMED_B
	LDD	#IMMED_W
	LDQ	#IMMED_Q	; 6309
	LDU	#IMMED_W
	FCB	$CF

	SUBB	<DIRECT
	CMPB	<DIRECT
	SBCB	<DIRECT
	ADDD	<DIRECT
	ANDB	<DIRECT
	BITB	<DIRECT
	LDB	<DIRECT
	STB	<DIRECT
	EORB	<DIRECT
	ADCB	<DIRECT
	ORB	<DIRECT
	ADDB	<DIRECT
	LDD	<DIRECT
	STD	<DIRECT
	LDU	<DIRECT
	STU	<DIRECT

	SUBB	,X
	CMPB	,X
	SBCB	,X
	ADDD	,X
	ANDB	,X
	BITB	,X
	LDB	,X
	STB	,X
	EORB	,X
	ADCB	,X
	ORB	,X
	ADDB	,X
	LDD	,X
	STD	,X
	LDU	,X
	STU	,X

	SUBB	>EXTEND
	CMPB	>EXTEND
	SBCB	>EXTEND
	ADDD	>EXTEND
	ANDB	>EXTEND
	BITB	>EXTEND
	LDB	>EXTEND
	STB	>EXTEND
	EORB	>EXTEND
	ADCB	>EXTEND
	ORB	>EXTEND
	ADDB	>EXTEND
	LDD	>EXTEND
	STD	>EXTEND
	LDU	>EXTEND
	STU	>EXTEND

; 10XX

;	LBRA	*+4
	LBRN	*+4
	LBHI	*+4
	LBLS	*+4
	LBCC	*+4
	LBCS	*+4
	LBNE	*+4
	LBEQ	*+4
	LBVC	*+4
	LBVS	*+4
	LBPL	*+4
	LBMI	*+4
	LBGE	*+4
	LBLT	*+4
	LBGT	*+4
	LBLE	*+4

	ADDR	D,D		; 6309
	ADCR	D,D		; 6309
	SUBR	D,D		; 6309
	SBCR	D,D		; 6309
	ANDR	D,D		; 6309
	ORR	D,D		; 6309
	EORR	D,D		; 6309
	CMPR	D,D		; 6309
	PSHSW			; 6309
	PULSW			; 6309
	PSHUW			; 6309
	PULUW			; 6309
	SWI2

	NEGD			; 6309
	COMD			; 6309
	LSRD			; 6309
	RORD			; 6309
	ASRD			; 6309
	ASLD			; 6309
	ROLD			; 6309
	DECD			; 6309
	INCD			; 6309
	TSTD			; 6309
	CLRD			; 6309

	COMW			; 6309
	LSRW			; 6309
	RORW			; 6309
	ROLW			; 6309
	DECW			; 6309
	INCW			; 6309
	TSTW			; 6309
	CLRW			; 6309

	SUBW	#IMMED_W	; 6309
	CMPW	#IMMED_W	; 6309
	SBCD	#IMMED_W	; 6309
	CMPD	#IMMED_W
	ANDD	#IMMED_W	; 6309
	BITD	#IMMED_W	; 6309
	LDW	#IMMED_W	; 6309
	EORD	#IMMED_W	; 6309
	ADCD	#IMMED_W	; 6309
	ORD	#IMMED_W	; 6309
	ADDW	#IMMED_W	; 6309
	CMPY	#IMMED_W
	LDY	#IMMED_W

	SUBW	<DIRECT		; 6309
	CMPW	<DIRECT		; 6309
	SBCD	<DIRECT		; 6309
	CMPD	<DIRECT
	ANDD	<DIRECT		; 6309
	BITD	<DIRECT		; 6309
	LDW	<DIRECT		; 6309
	STW	<DIRECT		; 6309
	EORD	<DIRECT		; 6309
	ADCD	<DIRECT		; 6309
	ORD	<DIRECT		; 6309
	ADDW	<DIRECT		; 6309
	CMPY	<DIRECT
	LDY	<DIRECT
	STY	<DIRECT

	SUBW	,X		; 6309
	CMPW	,X		; 6309
	SBCD	,X		; 6309
	CMPD	,X
	ANDD	,X		; 6309
	BITD	,X		; 6309
	LDW	,X		; 6309
	STW	,X		; 6309
	EORD	,X		; 6309
	ADCD	,X		; 6309
	ORD	,X		; 6309
	ADDW	,X		; 6309
	CMPY	,X
	LDY	,X
	STY	,X

	SUBW	>EXTEND		; 6309
	CMPW	>EXTEND		; 6309
	SBCD	>EXTEND		; 6309
	CMPD	>EXTEND
	ANDD	>EXTEND		; 6309
	BITD	>EXTEND		; 6309
	LDW	>EXTEND		; 6309
	STW	>EXTEND		; 6309
	EORD	>EXTEND		; 6309
	ADCD	>EXTEND		; 6309
	ORD	>EXTEND		; 6309
	ADDW	>EXTEND		; 6309
	CMPY	>EXTEND
	LDY	>EXTEND
	STY	>EXTEND

	LDS	#IMMED_W

	LDQ	<DIRECT		; 6309
	STQ	<DIRECT		; 6309
	LDS	<DIRECT
	STS	<DIRECT

	LDQ	,X		; 6309
	STQ	,X		; 6309
	LDS	,X
	STS	,X

	LDQ	>EXTEND		; 6309
	STQ	>EXTEND		; 6309
	LDS	>EXTEND
	STS	>EXTEND

; 11XX

	BAND	B,5,2,<DIRECT	; 6309
	BIAND	B,5,2,<DIRECT	; 6309
	BOR	B,5,2,<DIRECT	; 6309
	BIOR	B,5,2,<DIRECT	; 6309
	BEOR	B,5,2,<DIRECT	; 6309
	BIEOR	B,5,2,<DIRECT	; 6309
	LDBT	B,5,2,<DIRECT	; 6309
	STBT	B,5,2,<DIRECT	; 6309
	TFM	D+,D+		; 6309
	TFM	D-,D-		; 6309
	TFM	D+,D		; 6309
	TFM	D,D+		; 6309
	BITMD	#IMMED_B	; 6309
	LDMD	#IMMED_B	; 6309
	SWI3

	COME			; 6309
	DECE			; 6309
	INCE			; 6309
	TSTE			; 6309
	CLRE			; 6309

	COMF			; 6309
	DECF			; 6309
	INCF			; 6309
	TSTF			; 6309
	CLRF			; 6309

	SUBE	#IMMED_B	; 6309
	CMPE	#IMMED_B	; 6309
	CMPU	#IMMED_W
	LDE	#IMMED_B	; 6309
	ADDE	#IMMED_B	; 6309
	CMPS	#IMMED_W
	DIVD	#IMMED_W	; 6309
	DIVQ	#IMMED_W	; 6309
	MULD	#IMMED_W	; 6309

	SUBE	<DIRECT		; 6309
	CMPE	<DIRECT		; 6309
	CMPU	<DIRECT
	LDE	<DIRECT		; 6309
	STE	<DIRECT		; 6309
	ADDE	<DIRECT		; 6309
	CMPS	<DIRECT
	DIVD	<DIRECT		; 6309
	DIVQ	<DIRECT		; 6309
	MULD	<DIRECT		; 6309

	SUBE	,X		; 6309
	CMPE	,X		; 6309
	CMPU	,X
	LDE	,X		; 6309
	STE	,X		; 6309
	ADDE	,X		; 6309
	CMPS	,X
	DIVD	,X		; 6309
	DIVQ	,X		; 6309
	MULD	,X		; 6309

	SUBE	>EXTEND		; 6309
	CMPE	>EXTEND		; 6309
	CMPU	>EXTEND
	LDE	>EXTEND		; 6309
	STE	>EXTEND		; 6309
	ADDE	>EXTEND		; 6309
	CMPS	>EXTEND
	DIVD	>EXTEND		; 6309
	DIVQ	>EXTEND		; 6309
	MULD	>EXTEND		; 6309

	SUBF	#IMMED_B	; 6309
	CMPF	#IMMED_B	; 6309
	LDF	#IMMED_B	; 6309
	ADDF	#IMMED_B	; 6309

	SUBF	<DIRECT		; 6309
	CMPF	<DIRECT		; 6309
	LDF	<DIRECT		; 6309
	STF	<DIRECT		; 6309
	ADDF	<DIRECT		; 6309

	SUBF	>EXTEND		; 6309
	CMPF	>EXTEND		; 6309
	LDF	>EXTEND		; 6309
	STF	>EXTEND		; 6309
	ADDF	>EXTEND		; 6309

; indexed addressing modes

	LEAX	0,X
	LEAX	1,X
	LEAX	2,X
	LEAX	3,X
	LEAX	4,X
	LEAX	5,X
	LEAX	6,X
	LEAX	7,X
	LEAX	8,X
	LEAX	9,X
	LEAX	10,X
	LEAX	11,X
	LEAX	12,X
	LEAX	13,X
	LEAX	14,X
	LEAX	15,X

	LEAX	-16,X
	LEAX	-15,X
	LEAX	-14,X
	LEAX	-13,X
	LEAX	-12,X
	LEAX	-11,X
	LEAX	-10,X
	LEAX	-9,X
	LEAX	-8,X
	LEAX	-7,X
	LEAX	-6,X
	LEAX	-5,X
	LEAX	-4,X
	LEAX	-3,X
	LEAX	-2,X
	LEAX	-1,X

	LEAX	0,Y
	LEAX	1,Y
	LEAX	2,Y
	LEAX	3,Y
	LEAX	4,Y
	LEAX	5,Y
	LEAX	6,Y
	LEAX	7,Y
	LEAX	8,Y
	LEAX	9,Y
	LEAX	10,Y
	LEAX	11,Y
	LEAX	12,Y
	LEAX	13,Y
	LEAX	14,Y
	LEAX	15,Y

	LEAX	-16,Y
	LEAX	-15,Y
	LEAX	-14,Y
	LEAX	-13,Y
	LEAX	-12,Y
	LEAX	-11,Y
	LEAX	-10,Y
	LEAX	-9,Y
	LEAX	-8,Y
	LEAX	-7,Y
	LEAX	-6,Y
	LEAX	-5,Y
	LEAX	-4,Y
	LEAX	-3,Y
	LEAX	-2,Y
	LEAX	-1,Y

	LEAX	0,U
	LEAX	1,U
	LEAX	2,U
	LEAX	3,U
	LEAX	4,U
	LEAX	5,U
	LEAX	6,U
	LEAX	7,U
	LEAX	8,U
	LEAX	9,U
	LEAX	10,U
	LEAX	11,U
	LEAX	12,U
	LEAX	13,U
	LEAX	14,U
	LEAX	15,U

	LEAX	-16,U
	LEAX	-15,U
	LEAX	-14,U
	LEAX	-13,U
	LEAX	-12,U
	LEAX	-11,U
	LEAX	-10,U
	LEAX	-9,U
	LEAX	-8,U
	LEAX	-7,U
	LEAX	-6,U
	LEAX	-5,U
	LEAX	-4,U
	LEAX	-3,U
	LEAX	-2,U
	LEAX	-1,U

	LEAX	0,S
	LEAX	1,S
	LEAX	2,S
	LEAX	3,S
	LEAX	4,S
	LEAX	5,S
	LEAX	6,S
	LEAX	7,S
	LEAX	8,S
	LEAX	9,S
	LEAX	10,S
	LEAX	11,S
	LEAX	12,S
	LEAX	13,S
	LEAX	14,S
	LEAX	15,S

	LEAX	-16,S
	LEAX	-15,S
	LEAX	-14,S
	LEAX	-13,S
	LEAX	-12,S
	LEAX	-11,S
	LEAX	-10,S
	LEAX	-9,S
	LEAX	-8,S
	LEAX	-7,S
	LEAX	-6,S
	LEAX	-5,S
	LEAX	-4,S
	LEAX	-3,S
	LEAX	-2,S
	LEAX	<-1,S

	LEAX	,X+
	LEAX	,X++
	LEAX	,-X
	LEAX	,--X
	LEAX	,X
	LEAX	B,X
	LEAX	A,X
	LEAX	E,X		; 6309
	LEAX	<INDEX_B,X
	LEAX	>INDEX_W,X
	LEAX	F,X		; 6309
	LEAX	D,X
	LEAX	<.,PCR
	LEAX	<INDEX_B,PC
	LEAX	>.,PCR
	LEAX	>INDEX_W,PC
	LEAX	W,X		; 6309
	LEAX	,W		; 6309

	LEAX	[,W]		; 6309
	LEAX	[,X++]
	LEAX	[,--X]
	LEAX	[,X]
	LEAX	[B,X]
	LEAX	[A,X]
	LEAX	[E,X]		; 6309
	LEAX	[<INDEX_B,X]
	LEAX	[>INDEX_W,X]
	LEAX	[F,X]		; 6309
	LEAX	[D,X]
	LEAX	[<.,PCR]
	LEAX	[<INDEX_B,PC]
	LEAX	[>.,PCR]
	LEAX	[>INDEX_W,PC]
	LEAX	[W,X]		; 6309
	LEAX	[$495E]

	LEAX	,Y+
	LEAX	,Y++
	LEAX	,-Y
	LEAX	,--Y
	LEAX	,Y
	LEAX	B,Y
	LEAX	A,Y
	LEAX	E,Y		; 6309
	LEAX	INDEX_B,Y
	LEAX	INDEX_W,Y
	LEAX	F,Y		; 6309
	LEAX	D,Y
	LEAX	W,Y		; 6309
	LEAX	>INDEX_W,W	; 6309

	LEAX	[>INDEX_W,W]	; 6309
	LEAX	[,Y++]
	LEAX	[,--Y]
	LEAX	[,Y]
	LEAX	[B,Y]
	LEAX	[A,Y]
	LEAX	[E,Y]		; 6309
	LEAX	[INDEX_B,Y]
	LEAX	[INDEX_W,Y]
	LEAX	[F,Y]		; 6309
	LEAX	[D,Y]
	LEAX	[W,Y]		; 6309

	LEAX	,U+
	LEAX	,U++
	LEAX	,-U
	LEAX	,--U
	LEAX	,U
	LEAX	B,U
	LEAX	A,U
	LEAX	E,U		; 6309
	LEAX	<INDEX_B,U
	LEAX	>INDEX_W,U
	LEAX	F,U		; 6309
	LEAX	D,U
	LEAX	W,U		; 6309
	LEAX	,W++		; 6309

	LEAX	[,W++]		; 6309
	LEAX	[,U++]
	LEAX	[,--U]
	LEAX	[,U]
	LEAX	[B,U]
	LEAX	[A,U]
	LEAX	[E,U]		; 6309
	LEAX	[<INDEX_B,U]
	LEAX	[>INDEX_W,U]
	LEAX	[F,U]		; 6309
	LEAX	[D,U]
	LEAX	[W,U]		; 6309

	LEAX	,S+
	LEAX	,S++
	LEAX	,-S
	LEAX	,--S
	LEAX	,S
	LEAX	B,S
	LEAX	A,S
	LEAX	E,S		; 6309
	LEAX	<INDEX_B,S
	LEAX	>INDEX_W,S
	LEAX	F,S		; 6309
	LEAX	D,S
	LEAX	W,S		; 6309
	LEAX	,--W		; 6309

	LEAX	[,--W]		; 6309
	LEAX	[,S++]
	LEAX	[,--S]
	LEAX	[,S]
	LEAX	[B,S]
	LEAX	[A,S]
	LEAX	[E,S]		; 6309
	LEAX	[<INDEX_B,S]
	LEAX	[>INDEX_W,S]
	LEAX	[F,S]		; 6309
	LEAX	[D,S]
	LEAX	[W,S]		; 6309

; EXG/TFR registers

	TFR	D,X
	TFR	Y,U
	TFR	S,PC
	TFR	W,V	; 6309
	TFR	A,B
	TFR	CC,DP
	TFR	0,00	; 6309 - both zero registers
	TFR	E,F	; 6309

; automatic operand size test

	LEAX	$1,X	; 5-bit
	LEAX	$12,X	; 8-bit
	LEAX	$1234,X ; 16-bit

	CMPD	$1234,PC ; 16-bit
	CMPD	.,PCR	; 8-bit
	CMPD	>.,PCR	; 16-bit

; SETDP test

	LDA	DIRECT	; direct page
	LDA	EXTEND	; extended

	SETDP	EXTEND/256

	LDA	DIRECT	; extended
	LDA	EXTEND	; direct page

	END
