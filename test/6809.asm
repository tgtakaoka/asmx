	NEG	<$44
	FCB	$01
	FCB	$02
	COM	<$44
	LSR	<$44
	FCB	$05
	ROR	<$44
	ASR	<$44
	ASL	<$44
	ROL	<$44
	DEC	<$44
	FCB	$0B
	INC	<$44
	TST	<$44
	JMP	<$44
	CLR	<$44

	FCB	$10	; 10xx
	FCB	$11	; 11xx
	NOP
	SYNC
	FCB	$14	; TEST
	FCB	$15	; TEST
	LBRA	*+3
	LBSR	*+3
	FCB	$18
	DAA
	ORCC	#$49
	FCB	$1B
	ANDCC	#$49
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
	CWAI	#$49
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
	FCB	$61
	FCB	$62
	COM	,X
	LSR	,X
	FCB	$65
	ROR	,X
	ASR	,X
	ASL	,X
	ROL	,X
	DEC	,X
	FCB	$6B
	INC	,X
	TST	,X
	JMP	,X
	CLR	,X

	NEG	>$4558
	FCB	$71
	FCB	$72
	COM	>$4558
	LSR	>$4558
	FCB	$75
	ROR	>$4558
	ASR	>$4558
	ASL	>$4558
	ROL	>$4558
	DEC	>$4558
	FCB	$7B
	INC	>$4558
	TST	>$4558
	JMP	>$4558
	CLR	>$4558

	SUBA	#$49
	CMPA	#$49
	SBCA	#$49
	SUBD	#$494D
	ANDA	#$49
	BITA	#$49
	LDA	#$49
	FCB	$87
	EORA	#$49
	ADCA	#$49
	ORA	#$49
	ADDA	#$49
	CMPX	#$494D
	BSR	*+2
	LDX	#$494D
	FCB	$8F

	SUBA	<$44
	CMPA	<$44
	SBCA	<$44
	SUBD	<$44
	ANDA	<$44
	BITA	<$44
	LDA	<$44
	STA	<$44
	EORA	<$44
	ADCA	<$44
	ORA	<$44
	ADDA	<$44
	CMPX	<$44
	JSR	<$44
	LDX	<$44
	STX	<$44

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

	SUBA	>$4558
	CMPA	>$4558
	SBCA	>$4558
	SUBD	>$4558
	ANDA	>$4558
	BITA	>$4558
	LDA	>$4558
	STA	>$4558
	EORA	>$4558
	ADCA	>$4558
	ORA	>$4558
	ADDA	>$4558
	CMPX	>$4558
	JSR	>$4558
	LDX	>$4558
	STX	>$4558

	SUBB	#$49
	CMPB	#$49
	SBCB	#$49
	ADDD	#$494D
	ANDB	#$49
	BITB	#$49
	LDB	#$49
	FCB	$C7
	EORB	#$49
	ADCB	#$49
	ORB	#$49
	ADDB	#$49
	LDD	#$494D
	FCB	$CD
	LDU	#$494D
	FCB	$CF

	SUBB	<$44
	CMPB	<$44
	SBCB	<$44
	ADDD	<$44
	ANDB	<$44
	BITB	<$44
	LDB	<$44
	STB	<$44
	EORB	<$44
	ADCB	<$44
	ORB	<$44
	ADDB	<$44
	LDD	<$44
	STD	<$44
	LDU	<$44
	STU	<$44

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

	SUBB	>$4558
	CMPB	>$4558
	SBCB	>$4558
	ADDD	>$4558
	ANDB	>$4558
	BITB	>$4558
	LDB	>$4558
	STB	>$4558
	EORB	>$4558
	ADCB	>$4558
	ORB	>$4558
	ADDB	>$4558
	LDD	>$4558
	STD	>$4558
	LDU	>$4558
	STU	>$4558

; 10XX

;	LBRA	*+3
	LBRN	*+3
	LBHI	*+3
	LBLS	*+3
	LBCC	*+3
	LBCS	*+3
	LBNE	*+3
	LBEQ	*+3
	LBVC	*+3
	LBVS	*+3
	LBPL	*+3
	LBMI	*+3
	LBGE	*+3
	LBLT	*+3
	LBGT	*+3
	LBLE	*+3

	SWI2

	CMPD	#$494D
	CMPY	#$494D
	LDY	#$494D

	CMPD	<$44
	CMPY	<$44
	LDY	<$44
	STY	<$44

	CMPD	,X
	CMPY	,X
	LDY	,X
	STY	,X

	CMPD	>$4558
	CMPY	>$4558
	LDY	>$4558
	STY	>$4558

	LDS	#$494D

	LDS	<$44
	STS	<$44

	LDS	,X
	STS	,X

	LDS	>$4558
	STS	>$4558

; 11XX

	SWI3

	CMPU	#$494D
	CMPS	#$494D

	CMPU	<$44
	CMPS	<$44

	CMPU	,X
	CMPS	,X

	CMPU	>$4558
	CMPS	>$4558

; indexed

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
	FDB	$3087
	LEAX	<$58,X
	LEAX	>$4958,X
	FDB	$308A
	LEAX	D,X
	LEAX	<.,PCR
	LEAX	<$58,PC
	LEAX	>.,PCR
	LEAX	>$4958,PC
	FDB	$308E
	LEAX	$4558

	LEAX	[,X+]
	LEAX	[,X++]
	LEAX	[,-X]
	LEAX	[,--X]
	LEAX	[,X]
	LEAX	[B,X]
	LEAX	[A,X]
	FDB	$3097
	LEAX	[<$58,X]
	LEAX	[>$4958,X]
	FDB	$309A
	LEAX	[D,X]
	LEAX	[<.,PCR]
	LEAX	[<$58,PC]
	LEAX	[>.,PCR]
	LEAX	[>$4958,PC]
	FDB	$309E
	LEAX	[$495E]

	LEAX	,Y+
	LEAX	,Y++
	LEAX	,-Y
	LEAX	,--Y
	LEAX	,Y
	LEAX	B,Y
	LEAX	A,Y
	FDB	$30A7
	LEAX	$58,Y
	LEAX	$4958,Y
	FDB	$30AA
	LEAX	D,Y

	LEAX	[,Y+]
	LEAX	[,Y++]
	LEAX	[,-Y]
	LEAX	[,--Y]
	LEAX	[,Y]
	LEAX	[B,Y]
	LEAX	[A,Y]
	FDB	$30B7
	LEAX	[$58,Y]
	LEAX	[$4958,Y]
	FDB	$30BA
	LEAX	[D,Y]

	LEAX	,U+
	LEAX	,U++
	LEAX	,-U
	LEAX	,--U
	LEAX	,U
	LEAX	B,U
	LEAX	A,U
	LEAX	<$58,U
	LEAX	>$4958,U
	LEAX	D,U

	LEAX	[,U+]
	LEAX	[,U++]
	LEAX	[,-U]
	LEAX	[,--U]
	LEAX	[,U]
	LEAX	[B,U]
	LEAX	[A,U]
	LEAX	[<$58,U]
	LEAX	[>$4958,U]
	LEAX	[D,U]

	LEAX	,S+
	LEAX	,S++
	LEAX	,-S
	LEAX	,--S
	LEAX	,S
	LEAX	B,S
	LEAX	A,S
	LEAX	<$58,S
	LEAX	>$4958,S
	LEAX	D,S

	LEAX	[,S+]
	LEAX	[,S++]
	LEAX	[,-S]
	LEAX	[,--S]
	LEAX	[,S]
	LEAX	[B,S]
	LEAX	[A,S]
	LEAX	[<$58,S]
	LEAX	[>$4958,S]
	LEAX	[D,S]

; tests

	LDA	$44
	LDA	$4548

	SETDP	$45

	LDA	$44
	LDA	$4548

	LEAX	$58,X
	LEAX	$4558,X
	CMPD	$1234,PC
	CMPD	.,PCR
	CMPD	>.,PCR

	END
