;*************************************************************
; RAM usage (you probably shouldn't mess with these directly)
;*************************************************************

;		EQU	7000H	; 32 unused bytes
SndArray	EQU	7020H	; music array pointer?
NoiseP		EQU	7022H	; Pointer to program for noise generator
Tone1P		EQU	7024H	; Pointer to program for tone1 generator
Tone2P		EQU	7026H	; Pointer to program for tone2 generator
Tone3P		EQU	7028H	; Pointer to program for tone3 generator
NoiseCtlShad	EQU	702AH	; Noise control register shadow

;		Free memory from 702B thru 73B9

Stack		EQU	73B9H	; Default initial stack pointer
ParmArea	EQU	73BAH	; Parameter storage for PCOPY parameters
VDP0Shad	EQU	73C3H	; VDP register 0 shadow
VDP1Shad	EQU	73C4H	; VDP register 1 shadow
;		EQU	73C5H	; unused?
D73C6		EQU	73C6H	; flag byte?
WrtRAMSprt	EQU	73C7H	; BlkWrtVRAM to RAM sprite attr table if =1
RandSeed	EQU	73C8H	; Random number seed
D73CA		EQU	73CAH	; unknown
D73CB		EQU	73CBH	; unknown
D73CC		EQU	73CCH	; unknown
D73CD		EQU	73CDH	; unknown
D73CF		EQU	73CFH	; unknown
D73D1		EQU	73D1H	; unknown
TimerList	EQU	73D3H	; Pointer to timer list
TimerAux	EQU	73D5H	; Pointer to end of timer aux storage
RawCtlState	EQU	73D7H	; Raw controller state table (2 x 10 bytes)
PulseCnt1	EQU	73EBH	; Pulse counter #1
PulseCnt2	EQU	73ECH	; Pulse counter #2
;		EQU	73EDH	; unused?
Joy1Shad	EQU	73EEH	; shadow for joystick #1
Joy2Shad	EQU	73EFH	; shadow for joystick #2
Key1Shad	EQU	73F0H	; shadow for keypad #1
Key2Shad	EQU	73F1H	; shadow for keypad #2
VDPBaseShad	EQU	73F2H	; shadow for VDP table base addresses
SprtTabShad	EQU	73F2H	;   shadow for sprite table VRAM base addr
SprtPatTabShad	EQU	73F4H	;   shadow for sprite pattern generator VRAM base addr
NameTabShad	EQU	73F6H	;   shadow for name table VRAM base address
PatGenTabShad	EQU	73F8H	;   shadow for pattern generator VRAM base addr
ClrTabShad	EQU	73FAH	;   shadow for color table VRAM base address
;		EQU	73FCH	; unused?
D73FE		EQU	73FEH	; temp3

;***************************************
; Cartridge header addresses
;***************************************

Cart_Sig	EQU	8000H	; AA55 = title screen, 55AA = no screen
RamSprtTab	EQU	8002H	; RAM sprite attribute table pointer
RAMSprtIdx	EQU	8004H	; sprite index table pointer
VDP_Temp	EQU	8006H	; pointer to temp image storage (up to 40 bytes used)
CtlState	EQU	8008H	; pointer to controller state table (2 + 2x5 bytes)
Cart_Start	EQU	800AH	; start of cart code
V_RST_08H	EQU	800CH	; RST 08H vector
V_RST_10H	EQU	800FH	; RST 10H vector
V_RST_18H	EQU	8012H	; RST 18H vector
V_RST_20H	EQU	8015H	; RST 20H vector
V_RST_28H	EQU	8018H	; RST 28H vector
V_RST_30H	EQU	801BH	; RST 30H vector
V_RST_38H	EQU	801EH	; RST 38H vector
V_NMI		EQU	8021H	; NMI vector (vertical blank interrupt)
Cart_Title	EQU	8024H	; Title string "LINE 3/LINE 2/yyyy"

;***************************************
; Offsets into data blocks
;***************************************

;		Offsets into RawCtlState

RawCtlLeft	EQU	00H		; raw left controller state
RawCtlRight	EQU	0AH		; raw right controller state

;		Offsets into RawCtlLeft and RawCtlRight

RawCtlLFBit	EQU	00H		; previous left fire bit
RawCtlLFState	EQU	01H		; left fire button state
RawCtlDBits	EQU	02H		; previous directional bits
RawCtlDState	EQU	03H		; directionals state
;		EQU	04H		; unused?
;		EQU	05H		; unused?
RawCtlRFBit	EQU	06H		; previous right fire bit
RawCtlRFState	EQU	07H		; right fire button state
RawCtlKPBit	EQU	08H		; previous keypad bits
RawCtlKPState	EQU	09H		; keypad state

;		Offsets into CtlState table

CtlStateLFlag	EQU	00H		; left controller flags
CtlStateRFlag	EQU	01H		; right controller flags
CtlStateLeft	EQU	02H		; left controller state
CtlStateRight	EQU	07H		; right controller state

;CtlStateLeftL	EQU	02H	; CtlStateLeft+CtlStateLFire
;CtlStateLeftD	EQU	03H	; CtlStateLeft+CtlStateDir
;CtlStateLeftS	EQU	04H	; CtlStateLeft+CtlStateSpin
;CtlStateLeftR	EQU	05H	; CtlStateLeft+CtlStateRFire
;CtlStateLeftK	EQU	06H	; CtlStateLeft+CtlStateKey

;CtlStateRightL	EQU	07H	; CtlStateRight+CtlStateLFire
;CtlStateRightD	EQU	08H	; CtlStateRight+CtlStateDir
;CtlStateRightS	EQU	09H	; CtlStateRight+CtlStateSpin
;CtlStateRightR	EQU	0AH	; CtlStateRight+CtlStateRFire
;CtlStateRightK	EQU	0BH	; CtlStateRight+CtlStateKey

;		CtlStateLF/CtlStateRF bits

CtlCheckMe	EQU	80H	; 7	; if =0, do not check this ctrl at all
;		EQU	40H	; 6	; unused?
;		EQU	20H	; 5	; unused?
CtlCheckKP	EQU	10H	; 4	; check keypad
CtlCheckRFire	EQU	08H	; 3	; check right fire button
CtlCheckSpinner	EQU	04H	; 2	; check spinner
CtlCheckDir	EQU	02H	; 1	; check directionals
CtlCheckLFire	EQU	01H	; 0	; check left fire button

;		Offsets into CtlStateLeft and CtlStateRight

CtlStateLFire	EQU	00H		; left fire button
CtlStateDir	EQU	01H		; directionals
CtlStateSpin	EQU	02H		; spinner value
CtlStateRFire	EQU	03H		; right fire button
CtlStateKey	EQU	04H		; key code

;***************************************
; I/O port addresses
;***************************************

IO_KP_Select	EQU	080H		; Keypad select output port
IO_Joy_Select	EQU	0C0H		; Joystick select output port
IO_Joy1		EQU	0FCH		; Joystick 1 input port
IO_Joy2		EQU	0FFH		; Joystick 2 input port
IO_Sound	EQU	0FFH		; Sound chip output port
IO_VDP_Data	EQU	0BEH		; VDP data port
IO_VDP_Addr	EQU	0BFH		; VDP VRAM address output port
IO_VDP_Status	EQU	0BFH		; VDP status input port

;***************************************
; VDP table identifiers
;***************************************

SprAttrTable	EQU	00H		; Sprite attribute table   (1B00)
SprPatTable	EQU	01H		; Sprite pattern generator (3800)
NameTable	EQU	02H		; Name table (display)     (1800)
PatTable	EQU	03H		; Pattern generator table  (0000)
ColorTable	EQU	04H		; Color table              (2000)

;********************************************************
; ROM addresses (what do they do? Use The Source, Luke!)
;********************************************************

;		EQU	0069H		; decimal 60 (vertical refresh rate?)
FontA		EQU	006AH		; pointer to font bitmap for 'A'
Font0		EQU	006CH		; pointer to font bitmap for '0'

DoSound		EQU	1F61H
L1F64		EQU	1F64H
L1F67		EQU	1F67H
FlipRL		EQU	1F6AH
FlipUD		EQU	1F6DH
Rotate		EQU	1F70H
Expand		EQU	1F73H
ReadCtlRaw	EQU	1F76H
ReadCtl		EQU	1F79H
SkillScrn	EQU	1F7CH
InitFont	EQU	1F7FH
FillVRAM	EQU	1F82H
InitVDP		EQU	1F85H
ReadSpinner	EQU	1F88H
PBaseLoad	EQU	1F8BH
PBlkReadVRAM	EQU	1F8EH
PBlkWrtVRAM	EQU	1F91H
PInitRAMSprt	EQU	1F94H
PCopyRAMSprt	EQU	1F97H
PInitTimers	EQU	1F9AH
PStopTimer	EQU	1F9DH
PStartTimer	EQU	1FA0H
PTestTimer	EQU	1FA3H
PWriteReg	EQU	1FA6H
PWrtVRAM	EQU	1FA9H
PReadVRAM	EQU	1FACH
L1FAF		EQU	1FAFH
PInitSound	EQU	1FB2H
PAddSound	EQU	1FB5H
BaseLoad	EQU	1FB8H
BlkReadVRAM	EQU	1FBBH
BlkWrtVRAM	EQU	1FBEH
InitRAMSprt	EQU	1FC1H
CopyRAMSprt	EQU	1FC4H
InitTimers	EQU	1FC7H
StopTimer	EQU	1FCAH
StartTimer	EQU	1FCDH
TestTimer	EQU	1FD0H
RunTimers	EQU	1FD3H
NoSound		EQU	1FD6H
WriteReg	EQU	1FD9H
VDP_Status	EQU	1FDCH
WrtVRAM		EQU	1FDFH
ReadVRAM	EQU	1FE2H
L1FE5		EQU	1FE5H
L1FE8		EQU	1FE8H
ReadCtlState	EQU	1FEBH
InitSound	EQU	1FEEH
AddSound	EQU	1FF1H
UpdateSound	EQU	1FF4H
L1FF7		EQU	1FF7H
L1FFA		EQU	1FFAH
Random		EQU	1FFDH
