; Fairchild F8 instruction set

	LR	A,KU		; 00
	LR	A,KL		; 01
	LR	A,QU		; 02
	LR	A,QL		; 03
	LR	KU,A		; 04
	LR	KL,A		; 05
	LR	QU,A		; 06
	LR	QL,A		; 07

	LR	K,PC1		; 08
	LR	PC1,K		; 09
	LR	A,IS		; 0A
	LR	IS,A		; 0B
	PK			; 0C
	LR	PC0,Q		; 0D
	LR	Q,DC0		; 0E
	LR	DC0,Q		; 0F

	LR	DC0,H		; 10
	LR	H,DC0		; 11
	SR	1		; 12
	SL	1		; 13
	SR	4		; 14
	SL	4		; 15
	LM			; 16
	ST			; 17

	COM			; 18
	LNK			; 19
	DI			; 1A
	EI			; 1B
	POP			; 1C
	LR	W,J		; 1D
	LR	J,W		; 1E
	INC			; 1F

	LI	'I'		; 20 49
	NI	'I'		; 21 49
	OI	'I'		; 22 49
	XI	'I'		; 23 49
	AI	'I'		; 24 49
	CI	'I'		; 25 49
	IN	'P'		; 26 50
	OUT	'P'		; 27 50

	PI	'AD'		; 28 41 44
	JMP	'AD'		; 29 41 44
	DCI	'AD'		; 2A 41 44
	NOP			; 2B
	XDC			; 2C
	DB	$2D		; 2D (undefined)
	DB	$2E		; 2E (undefined)
	DB	$2F		; 2F (undefined)

	DS	0		; 30
	DS	1		; 31
	DS	2		; 32
	DS	3		; 33
	DS	4		; 34
	DS	5		; 35
	DS	6		; 36
	DS	7		; 37
	DS	8		; 38
	DS	9		; 39
	DS	10		; 3A
	DS	11		; 3B
	DS	12		; 3C
	DS	13		; 3D
	DS	14		; 3E
	DS	15		; 3F

	LR	A,0		; 40
	LR	A,1		; 41
	LR	A,2		; 42
	LR	A,3		; 43
	LR	A,4		; 44
	LR	A,5		; 45
	LR	A,6		; 46
	LR	A,7		; 47
	LR	A,8		; 48
	LR	A,9		; 49
	LR	A,10		; 4A - this is why register numbers must be decimal!
	LR	A,11		; 4B
	LR	A,12		; 4C
	LR	A,13		; 4D
	LR	A,14		; 4E
	LR	A,15		; 4F

	LR	0,A		; 50
	LR	1,A		; 51
	LR	2,A		; 52
	LR	3,A		; 53
	LR	4,A		; 54
	LR	5,A		; 55
	LR	6,A		; 56
	LR	7,A		; 57
	LR	8,A		; 58
	LR	9,A		; 59
	LR	10,A		; 5A - this is why register numbers must be decimal!
	LR	11,A		; 5B
	LR	12,A		; 5C
	LR	13,A		; 5D
	LR	14,A		; 5E
	LR	15,A		; 5F

	LISU	0		; 60
	LISU	1		; 61
	LISU	2		; 62
	LISU	3		; 63
	LISU	4		; 64
	LISU	5		; 65
	LISU	6		; 66
	LISU	7		; 67

	LISL	0		; 68
	LISL	1		; 69
	LISL	2		; 6A
	LISL	3		; 6B
	LISL	4		; 6C
	LISL	5		; 6D
	LISL	6		; 6E
	LISL	7		; 6F

	LIS	0		; 70
	LIS	1		; 71
	LIS	2		; 72
	LIS	3		; 73
	LIS	4		; 74
	LIS	5		; 75
	LIS	6		; 76
	LIS	7		; 77
	LIS	8		; 78
	LIS	9		; 79
	LIS	10		; 7A
	LIS	11		; 7B
	LIS	12		; 7C
	LIS	13		; 7D
	LIS	14		; 7E
	LIS	15		; 7F

	BT	0,.		; 80 FE
	BT	1,.		; 81 FE
	BT	2,.		; 82 FE
	BT	3,.		; 83 FE
	BT	4,.		; 84 FE
	BT	5,.		; 85 FE
	BT	6,.		; 86 FE
	BT	7,.		; 87 FE

	AM			; 88
	AMD			; 89
	NM			; 8A
	OM			; 8B
	XM			; 8C
	CM			; 8D
	ADC			; 8E
	BR7	.		; 8F FE

	BF	0,.		; 90 FE
	BF	1,.		; 91 FE
	BF	2,.		; 92 FE
	BF	3,.		; 93 FE
	BF	4,.		; 94 FE
	BF	5,.		; 95 FE
	BF	6,.		; 96 FE
	BF	7,.		; 97 FE
	BF	8,.		; 98 FE
	BF	9,.		; 99 FE
	BF	10,.		; 9A FE
	BF	11,.		; 9B FE
	BF	12,.		; 9C FE
	BF	13,.		; 9D FE
	BF	14,.		; 9E FE
	BF	15,.		; 9F FE

	INS	0		; A0
	INS	1		; A1
	INS	2		; A2
	INS	3		; A3
	INS	4		; A4
	INS	5		; A5
	INS	6		; A6
	INS	7		; A7
	INS	8		; A8
	INS	9		; A9
	INS	10		; AA
	INS	11		; AB
	INS	12		; AC
	INS	13		; AD
	INS	14		; AE
	INS	15		; AF

	OUTS	0		; B0
	OUTS	1		; B1
	OUTS	2		; B2
	OUTS	3		; B3
	OUTS	4		; B4
	OUTS	5		; B5
	OUTS	6		; B6
	OUTS	7		; B7
	OUTS	8		; B8
	OUTS	9		; B9
	OUTS	10		; BA
	OUTS	11		; BB
	OUTS	12		; BC
	OUTS	13		; BD
	OUTS	14		; BE
	OUTS	15		; BF

	AS	0		; C0
	AS	1		; C1
	AS	2		; C2
	AS	3		; C3
	AS	4		; C4
	AS	5		; C5
	AS	6		; C6
	AS	7		; C7
	AS	8		; C8
	AS	9		; C9
	AS	10		; CA
	AS	11		; CB
	AS	12		; CC
	AS	13		; CD
	AS	14		; CE
	AS	15		; CF

	ASD	0		; D0
	ASD	1		; D1
	ASD	2		; D2
	ASD	3		; D3
	ASD	4		; D4
	ASD	5		; D5
	ASD	6		; D6
	ASD	7		; D7
	ASD	8		; D8
	ASD	9		; D9
	ASD	10		; DA
	ASD	11		; DB
	ASD	12		; DC
	ASD	13		; DD
	ASD	14		; DE
	ASD	15		; DF

	XS	0		; E0
	XS	1		; E1
	XS	2		; E2
	XS	3		; E3
	XS	4		; E4
	XS	5		; E5
	XS	6		; E6
	XS	7		; E7
	XS	8		; E8
	XS	9		; E9
	XS	10		; EA
	XS	11		; EB
	XS	12		; EC
	XS	13		; ED
	XS	14		; EE
	XS	15		; EF

	NS	0		; F0
	NS	1		; F1
	NS	2		; F2
	NS	3		; F3
	NS	4		; F4
	NS	5		; F5
	NS	6		; F6
	NS	7		; F7
	NS	8		; F8
	NS	9		; F9
	NS	10		; FA
	NS	11		; FB
	NS	12		; FC
	NS	13		; FD
	NS	14		; FE
	NS	15		; FF

; alternate mnemonics

	CLR			; 70
	BP	.		; 81 FE
	BC	.		; 82 FE
	BZ	.		; 84 FE
	BR	.		; 90 FE
	BM	.		; 91 FE
	BNC	.		; 92 FE
	BNZ	.		; 94 FE
	BNO	.		; 98 FE
