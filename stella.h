* TIA write registers

VSync	EQU	$00	; D1 = start vertical sync
VBlank	EQU	$01	; D7 = ground paddle inputs Inpt0, Inpt1, Inpt2, Inpt3
			; D6 = enable Inpt4, Inpt5 latches
			; D1 = start vertical blank
WSync	EQU	$02	; write to wait for horizontal sync
RSync	EQU	$03	; write to reset horizontal sync counter
NuSiz0	EQU	$04	; D4-D5 = P0 size, D2-D0 = P0 number
NuSiz1	EQU	$05	; D4-D5 = P1 size, D2-D0 = P1 number
CoLuP0	EQU	$06	; D7-D1 = player 0 color
CoLuP1	EQU	$07	; D7-D1 = player 1 color
CoLuPf	EQU	$08	; D7-D1 = playfield color
CoLuBk	EQU	$09	; D7-D1 = background color
CtrlPf  EQU     $0A	; D5-D4 = ball size
			; D2 = playfield priority
			; D1 = use P0/P1 colors for playfield
			; D0 = reflect playfield
RefP0	EQU	$0B	; D3 = reflect player 0
RefP1	EQU	$0C	; D3 = reflect player 1
Pf0	EQU	$0D	; D7-D4 = playfield 0 graphics
Pf1	EQU	$0E	; D7-D0 = playfield 1 graphics
Pf2	EQU	$0F	; D7-D0 = playfield 2 graphics
ResP0	EQU	$10	; write to reset player 0
ResP1	EQU	$11	; write to reset player 1
ResM0	EQU	$12	; write to reset missile 0
ResM1	EQU	$13	; write to reset missile 1
ResBl	EQU	$14	; write to reset ball
AudC0	EQU	$15	; D3-D0 = audio control 1
AudC1	EQU	$16	; D3-D0 = audio control 0
AudF0	EQU	$17	; D4-D0 = audio frequency 0
AudF1	EQU	$18	; D4-D0 = audio frequency 1
AudV0	EQU	$19	; D3-D0 = audio volume 0
AudV1	EQU	$1A	; D3-D0 = audio volume 1
GrP0	EQU	$1B	; D7-D0 = player 0 graphics
GrP1	EQU	$1C	; D7-D0 = player 1 graphics
EnaM0	EQU	$1D	; D1 = enable missile 0 graphics
EnaM1	EQU	$1E	; D1 = enable missile 1 graphics
EnaBl	EQU	$1F	; D1 = enable ball graphics
HMP0	EQU	$20	; D7-D4 = horizontal move player 0
HMP1	EQU	$21	; D7-D4 = horizontal move player 1
HMM0	EQU	$22	; D7-D4 = horizontal move missile 0
HMM1	EQU	$23	; D7-D4 = horizontal move missile 1
HMBl	EQU	$24	; D7-D4 = horizontal move ball
VDelP0	EQU	$25	; D0 = vertical delay player 0
VDelP1	EQU	$26	; D0 = vertical delay player 1
VDelBl	EQU	$27	; D0 = vertical delay ball
ResMP0	EQU	$28	; D1 = reset missile 0 to player 0
ResMP1	EQU	$29	; D1 = reset missile 1 to player 1
HMove	EQU	$2A	; write to apply horizontal motion registers
HMClr	EQU	$2B	; write to clear horizontal motion registers
CxClr	EQU	$2C	; write to clear collision latches

* TIA read registers

CxM0P	EQU	$00	; D7 = M0 x P1, D6 = M0 x P0
CxM1P	EQU	$01	; D7 = M1 x P0, D6 = M1 x P1
CxP0FB	EQU	$02	; D7 = P0 x PF, D6 = P0 x BL
CxP1FB	EQU	$03	; D7 = P1 x PF, D6 = P1 x BL
CxM0FB	EQU	$04	; D7 = M0 x PF, D6 = M0 x BL
CxM1FB	EQU	$05	; D7 = M1 x PF, D6 = M1 x BL
CxBlPf	EQU	$06	; D7 = BL x PF, D6 = unused
CxPPMM	EQU	$07	; D7 = P0 x P1, D6 = M0 x M1
Inpt0	EQU	$08	; paddle 0, keypad 0 col 1
Inpt1	EQU	$09	; paddle 1, keypad 0 col 2
Inpt2	EQU	$0A	; paddle 2, keypad 1 col 1
Inpt3	EQU	$0B	; paddle 3, keypad 1 col 2
Inpt4	EQU	$0C	; fire button 0, keypad 0 col 3
Inpt5	EQU	$0D	; fire button 1, keypad 1 col 3

* RIOT registers

SwchA	EQU	$280	; joystick I/O port
			; D7 = P0 right, paddle 0 button
			; D6 = P0 left, paddle 1 button
			; D5 = P0 down
			; D4 = P0 up
			; D3 = P1 right, paddle 2 button
			; D2 = P1 left, paddle 3 button
			; D1 = P1 down
			; D0 = P1 up
SwACnt	EQU	$281	; joystick port control, 0=output, 1=input
SwchB	EQU	$282	; option switches I/O port
			; D7 = P1 difficulty
			; D6 = P0 difficulty
			; D5/D4 not used
			; D3 = color B/W
			; D2 not used
			; D1 = select
			; D0 = reset
SwBCnt	EQU	$283	; option switches port control, 0=output, 1=input
InTim	EQU	$284	; interval timer counter (read only)
Tim1T	EQU	$294	; load timer /1 (write only)
Tim8T	EQU	$295	; load timer /8 (write only)
Tim64T	EQU	$296	; load timer /64 (write only)
T1024T	EQU	$297	; load timer /1024 (write only)
;Tim1I	EQU	$29C	; load interrupt timer /1 (not usable)
;Tim8I	EQU	$29D	; load interrupt timer /8 (not usable)
;Tim64I	EQU	$29E	; load interrupt timer /64 (not usable)
;T1024I	EQU	$29F	; load interrupt timer /1024 (not usable)
