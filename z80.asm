	NOP
	LD	BC,1234H
	LD	(BC),A
	INC	BC
	INC	B
	DEC	B
	LD	B,56H
	RLCA

	EX	AF,AF'
	ADD	HL,BC
	LD	A,(BC)
	DEC	BC
	INC	C
	DEC	C
	LD	C,56H
	RRCA

	DJNZ	$
	LD	DE,1234H
	LD	(DE),A
	INC	DE
	INC	D
	DEC	D
	LD	D,56H
	RLA

	JR	$
	ADD	HL,DE
	LD	A,(DE)
	DEC	DE
	INC	E
	DEC	E
	LD	E,56H
	RRA

	JR	NZ,$
	LD	HL,1234H
	LD	(1234H),HL
	INC	HL
	INC	H
	DEC	H
	LD	H,56H
	DAA

	JR	Z,$
	ADD	HL,HL
	LD	HL,(1234H)
	DEC	HL
	INC	L
	DEC	L
	LD	L,56H
	CPL

	JR	NC,$
	LD	SP,1234H
	LD	(1234H),A
	INC	SP
	INC	(HL)
	DEC	(HL)
	LD	(HL),56H
	SCF

	JR	C,$
	ADD	HL,SP
	LD	A,(1234H)
	DEC	SP
	INC	A
	DEC	A
	LD	A,56H
	CCF

	LD	B,B
	LD	B,C
	LD	B,D
	LD	B,E
	LD	B,H
	LD	B,L
	LD	B,(HL)
	LD	B,A

	LD	C,B
	LD	C,C
	LD	C,D
	LD	C,E
	LD	C,H
	LD	C,L
	LD	C,(HL)
	LD	C,A

	LD	D,B
	LD	D,C
	LD	D,D
	LD	D,E
	LD	D,H
	LD	D,L
	LD	D,(HL)
	LD	D,A

	LD	E,B
	LD	E,C
	LD	E,D
	LD	E,E
	LD	E,H
	LD	E,L
	LD	E,(HL)
	LD	E,A

	LD	H,B
	LD	H,C
	LD	H,D
	LD	H,E
	LD	H,H
	LD	H,L
	LD	H,(HL)
	LD	H,A

	LD	L,B
	LD	L,C
	LD	L,D
	LD	L,E
	LD	L,H
	LD	L,L
	LD	L,(HL)
	LD	L,A

	LD	(HL),B
	LD	(HL),C
	LD	(HL),D
	LD	(HL),E
	LD	(HL),H
	LD	(HL),L
	HALT
	LD	(HL),A

	LD	A,B
	LD	A,C
	LD	A,D
	LD	A,E
	LD	A,H
	LD	A,L
	LD	A,(HL)
	LD	A,A

	ADD	A,B
	ADD	A,C
	ADD	A,D
	ADD	A,E
	ADD	A,H
	ADD	A,L
	ADD	A,(HL)
	ADD	A,A

	ADC	A,B
	ADC	A,C
	ADC	A,D
	ADC	A,E
	ADC	A,H
	ADC	A,L
	ADC	A,(HL)
	ADC	A,A

	SUB	B
	SUB	C
	SUB	D
	SUB	E
	SUB	H
	SUB	L
	SUB	(HL)
	SUB	A

	SBC	A,B
	SBC	A,C
	SBC	A,D
	SBC	A,E
	SBC	A,H
	SBC	A,L
	SBC	A,(HL)
	SBC	A,A

	AND	B
	AND	C
	AND	D
	AND	E
	AND	H
	AND	L
	AND	(HL)
	AND	A

	XOR	B
	XOR	C
	XOR	D
	XOR	E
	XOR	H
	XOR	L
	XOR	(HL)
	XOR	A

	OR	B
	OR	C
	OR	D
	OR	E
	OR	H
	OR	L
	OR	(HL)
	OR	A

	CP	B
	CP	C
	CP	D
	CP	E
	CP	H
	CP	L
	CP	(HL)
	CP	A

	RET	NZ
	POP	BC
	JP	NZ,$
	JP	$
	CALL	NZ,$
	PUSH	BC
	ADD	A,78H
	RST	00H

	RET	Z
	RET
	JP	Z,$
	DB	0CBH	; bit & rot instructions
	CALL	Z,$
	CALL	$
	ADC	A,78H
	RST	08H

	RET	NC
	POP	DE
	JP	NC,$
	OUT	(78H),A
	CALL	NC,$
	PUSH	DE
	SUB	78H
	RST	10H

	RET	C
	EXX
	JP	C,$
	IN	A,(78H)
	CALL	C,$
	DB	0DDH	; IX instructions
	SBC	A,78H
	RST	18H

	RET	PO
	POP	HL
	JP	PO,$
	EX	(SP),HL
	CALL	PO,$
	PUSH	HL   
	AND	78H
	RST	20H

	RET	PE
	JP	(HL)
	JP	PE,$
	EX	DE,HL
	CALL	PE,$
	DB	0EDH	; misc instructions
	XOR	78H
	RST	28H

	RET	P
	POP	AF
	JP	P,$
	DI
	CALL	P,$
	PUSH	AF
	OR	78H
	RST	30H

	RET	M
	LD	SP,HL
	JP	M,$
	EI
	CALL	M,$
	DB	0FDH	; IY instructions
	CP	78H
	RST	38H

	RLC	B
	RLC	C
	RLC	D
	RLC	E
	RLC	H
	RLC	L
	RLC	(HL)
	RLC	A

	RRC	B
	RRC	C
	RRC	D
	RRC	E
	RRC	H
	RRC	L
	RRC	(HL)
	RRC	A

	RL	B
	RL	C
	RL	D
	RL	E
	RL	H
	RL	L
	RL	(HL)
	RL	A

	RR	B
	RR	C
	RR	D
	RR	E
	RR	H
	RR	L
	RR	(HL)
	RR	A

	SLA	B
	SLA	C
	SLA	D
	SLA	E
	SLA	H
	SLA	L
	SLA	(HL)
	SLA	A

	SRA	B
	SRA	C
	SRA	D
	SRA	E
	SRA	H
	SRA	L
	SRA	(HL)
	SRA	A

	SRL	B
	SRL	C
	SRL	D
	SRL	E
	SRL	H
	SRL	L
	SRL	(HL)
	SRL	A

	BIT	0,B
	BIT	0,C
	BIT	0,D
	BIT	0,E
	BIT	0,H
	BIT	0,L
	BIT	0,(HL)
	BIT	0,A

	BIT	1,B
	BIT	1,C
	BIT	1,D
	BIT	1,E
	BIT	1,H
	BIT	1,L
	BIT	1,(HL)
	BIT	1,A

	BIT	2,B
	BIT	2,C
	BIT	2,D
	BIT	2,E
	BIT	2,H
	BIT	2,L
	BIT	2,(HL)
	BIT	2,A

	BIT	3,B
	BIT	3,C
	BIT	3,D
	BIT	3,E
	BIT	3,H
	BIT	3,L
	BIT	3,(HL)
	BIT	3,A

	BIT	4,B
	BIT	4,C
	BIT	4,D
	BIT	4,E
	BIT	4,H
	BIT	4,L
	BIT	4,(HL)
	BIT	4,A

	BIT	5,B
	BIT	5,C
	BIT	5,D
	BIT	5,E
	BIT	5,H
	BIT	5,L
	BIT	5,(HL)
	BIT	5,A

	BIT	6,B
	BIT	6,C
	BIT	6,D
	BIT	6,E
	BIT	6,H
	BIT	6,L
	BIT	6,(HL)
	BIT	6,A

	BIT	7,B
	BIT	7,C
	BIT	7,D
	BIT	7,E
	BIT	7,H
	BIT	7,L
	BIT	7,(HL)
	BIT	7,A

	RES	0,B
	RES	0,C
	RES	0,D
	RES	0,E
	RES	0,H
	RES	0,L
	RES	0,(HL)
	RES	0,A

	RES	1,B
	RES	1,C
	RES	1,D
	RES	1,E
	RES	1,H
	RES	1,L
	RES	1,(HL)
	RES	1,A

	RES	2,B
	RES	2,C
	RES	2,D
	RES	2,E
	RES	2,H
	RES	2,L
	RES	2,(HL)
	RES	2,A

	RES	3,B
	RES	3,C
	RES	3,D
	RES	3,E
	RES	3,H
	RES	3,L
	RES	3,(HL)
	RES	3,A

	RES	4,B
	RES	4,C
	RES	4,D
	RES	4,E
	RES	4,H
	RES	4,L
	RES	4,(HL)
	RES	4,A

	RES	5,B
	RES	5,C
	RES	5,D
	RES	5,E
	RES	5,H
	RES	5,L
	RES	5,(HL)
	RES	5,A

	RES	6,B
	RES	6,C
	RES	6,D
	RES	6,E
	RES	6,H
	RES	6,L
	RES	6,(HL)
	RES	6,A

	RES	7,B
	RES	7,C
	RES	7,D
	RES	7,E
	RES	7,H
	RES	7,L
	RES	7,(HL)
	RES	7,A

	SET	0,B
	SET	0,C
	SET	0,D
	SET	0,E
	SET	0,H
	SET	0,L
	SET	0,(HL)
	SET	0,A

	SET	1,B
	SET	1,C
	SET	1,D
	SET	1,E
	SET	1,H
	SET	1,L
	SET	1,(HL)
	SET	1,A

	SET	2,B
	SET	2,C
	SET	2,D
	SET	2,E
	SET	2,H
	SET	2,L
	SET	2,(HL)
	SET	2,A

	SET	3,B
	SET	3,C
	SET	3,D
	SET	3,E
	SET	3,H
	SET	3,L
	SET	3,(HL)
	SET	3,A

	SET	4,B
	SET	4,C
	SET	4,D
	SET	4,E
	SET	4,H
	SET	4,L
	SET	4,(HL)
	SET	4,A

	SET	5,B
	SET	5,C
	SET	5,D
	SET	5,E
	SET	5,H
	SET	5,L
	SET	5,(HL)
	SET	5,A

	SET	6,B
	SET	6,C
	SET	6,D
	SET	6,E
	SET	6,H
	SET	6,L
	SET	6,(HL)
	SET	6,A

	SET	7,B
	SET	7,C
	SET	7,D
	SET	7,E
	SET	7,H
	SET	7,L
	SET	7,(HL)
	SET	7,A
     
	ADD	IX,BC
	ADD	IX,DE
	LD	IX,1234H
	LD	(1234H),IX
	INC	IX
	ADD	IX,IX
	LD	IX,(1234H)
	DEC	IX
	INC	(IX+12H)
	DEC	(IX+12H)
	LD	(IX+12H),56H
	ADD	IX,SP
	LD	B,(IX+12H)
	LD	C,(IX+12H)
	LD	D,(IX+12H)
	LD	E,(IX+12H)
	LD	H,(IX+12H)
	LD	L,(IX+12H)
	LD	(IX+12H),B
	LD	(IX+12H),C
	LD	(IX+12H),D
	LD	(IX+12H),E
	LD	(IX+12H),H
	LD	(IX+12H),L
	LD	(IX+12H),A
	LD	A,(IX+12H)
	ADD	A,(IX+12H)
	ADC	A,(IX+12H)
	SUB	(IX+12H)
	SBC	A,(IX+12H)
	AND	(IX+12H)
	XOR	(IX+12H)
	OR	(IX+12H)
	CP	(IX+12H)
	RLC	(IX+12H)
	RRC	(IX+12H)
	RL	(IX+12H)
	RR	(IX+12H)
	SLA	(IX+12H)
	SRA	(IX+12H)
	SRL	(IX+12H)
	BIT	0,(IX+12H)
	BIT	1,(IX+12H)
	BIT	2,(IX+12H)
	BIT	3,(IX+12H)
	BIT	4,(IX+12H)
	BIT	5,(IX+12H)
	BIT	6,(IX+12H)
	BIT	7,(IX+12H)
	RES	0,(IX+12H)
	RES	1,(IX+12H)
	RES	2,(IX+12H)
	RES	3,(IX+12H)
	RES	4,(IX+12H)
	RES	5,(IX+12H)
	RES	6,(IX+12H)
	RES	7,(IX+12H)
	SET	0,(IX+12H)
	SET	1,(IX+12H)
	SET	2,(IX+12H)
	SET	3,(IX+12H)
	SET	4,(IX+12H)
	SET	5,(IX+12H)
	SET	6,(IX+12H)
	SET	7,(IX+12H)
	POP	IX
	EX	(SP),IX
	PUSH	IX
	JP	(IX)
	LD	SP,IX

	ADD	IY,BC
	ADD	IY,DE
	LD	IY,1234H
	LD	(1234H),IY
	INC	IY
	ADD	IY,IY
	LD	IY,(1234H)
	DEC	IY
	INC	(IY+12H)
	DEC	(IY+12H)
	LD	(IY+12H),56H
	ADD	IY,SP
	LD	B,(IY+12H)
	LD	C,(IY+12H)
	LD	D,(IY+12H)
	LD	E,(IY+12H)
	LD	H,(IY+12H)
	LD	L,(IY+12H)
	LD	(IY+12H),B
	LD	(IY+12H),C
	LD	(IY+12H),D
	LD	(IY+12H),E
	LD	(IY+12H),H
	LD	(IY+12H),L
	LD	(IY+12H),A
	LD	A,(IY+12H)
	ADD	A,(IY+12H)
	ADC	A,(IY+12H)
	SUB	(IY+12H)
	SBC	A,(IY+12H)
	AND	(IY+12H)
	XOR	(IY+12H)
	OR	(IY+12H)
	CP	(IY+12H)
	RLC	(IY+12H)
	RRC	(IY+12H)
	RL	(IY+12H)
	RR	(IY+12H)
	SLA	(IY+12H)
	SRA	(IY+12H)
	SRL	(IY+12H)
	BIT	0,(IY+12H)
	BIT	1,(IY+12H)
	BIT	2,(IY+12H)
	BIT	3,(IY+12H)
	BIT	4,(IY+12H)
	BIT	5,(IY+12H)
	BIT	6,(IY+12H)
	BIT	7,(IY+12H)
	RES	0,(IY+12H)
	RES	1,(IY+12H)
	RES	2,(IY+12H)
	RES	3,(IY+12H)
	RES	4,(IY+12H)
	RES	5,(IY+12H)
	RES	6,(IY+12H)
	RES	7,(IY+12H)
	SET	0,(IY+12H)
	SET	1,(IY+12H)
	SET	2,(IY+12H)
	SET	3,(IY+12H)
	SET	4,(IY+12H)
	SET	5,(IY+12H)
	SET	6,(IY+12H)
	SET	7,(IY+12H)
	POP	IY
	EX	(SP),IY
	PUSH	IY
	JP	(IY)
	LD	SP,IY

	IN	B,(C)
	OUT	(C),B
	SBC	HL,BC
	LD	(1234H),BC
	NEG
	RETN
	IM	0
	LD	I,A
	IN	C,(C)
	OUT	(C),C
	ADC	HL,BC
	LD	BC,(1234H)
	RETI
	LD	R,A
	IN	D,(C)
	OUT	(C),D
	SBC	HL,DE
	LD	(1234H),DE
	IM	1
	LD	A,I
	IN	E,(C)
	OUT	(C),E
	ADC	HL,DE
	LD	DE,(1234H)
	IM	2
	LD	A,R
	IN	H,(C)
	OUT	(C),H
	SBC	HL,HL
	RRD
	IN	L,(C)
	OUT	(C),L
	ADC	HL,HL
	RLD
	SBC	HL,SP
	LD	(1234H),SP
	IN	A,(C)
	OUT	(C),A
	ADC	HL,SP
	LD	SP,(1234H)
	LDI
	CPI
	INI
	OUTI
	LDD
	CPD
	IND
	OUTD
	LDIR
	CPIR
	INIR
	OTIR
	LDDR
	CPDR
	INDR
	OTDR

	END
