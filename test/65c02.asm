	PROCESSOR 65C02

        BRK
        ORA ($FF,X)
        DB  $02
        DB  $03
        TSB $FF
        ORA $FF
        ASL $FF
        RMB0 $FF

        PHP
        ORA #$FF
        ASL A
        DB  $0B
        TSB $FFFF
        ORA $FFFF
        ASL $FFFF
        BBR0 $FF,*

        BPL *
        ORA ($FF),Y
        ORA ($FF)
        DB  $13
        TRB $FF
        ORA $FF,X
        ASL $FF,X
        RMB1 $FF

        CLC
        ORA $FFFF,Y
        INA
        DB  $1B
        TRB $FFFF
        ORA $FFFF,X
        ASL $FFFF,X
        BBR1 $FF,*

        JSR $FFFF
        AND ($FF,X)
        DB  $22
        DB  $23
        BIT $FF
        AND $FF
        ROL $FF
        RMB2 $FF

        PLP
        AND #$FF
        ROL A
        DB  $2B
        BIT $FFFF
        AND $FFFF
        ROL $FFFF
        BBR2 $FF,*

        BMI *
        AND ($FF),Y
        AND ($FF)
        DB  $33
        BIT $FF,X
        AND $FF,X
        ROL $FF,X
        RMB3 $FF

        SEC
        AND $FFFF,Y
        DEA
        DB  $3B
        BIT $FFFF,X
        AND $FFFF,X
        ROL $FFFF,X
        BBR3 $FF,*

        RTI
        EOR ($FF,X)
        DB  $42
        DB  $43
        DB  $44
        EOR $FF
        LSR $FF
        RMB4 $FF

        PHA
        EOR #$FF
        LSR A
        DB  $4B
        JMP $FFFF
        EOR $FFFF
        LSR $FFFF
        BBR4 $FF,*

        BVC *
        EOR ($FF),Y
        EOR ($FF)
        DB  $53
        DB  $54
        EOR $FF,X
        LSR $FF,X
        RMB5 $FF

        CLI
        EOR $FFFF,Y
        PHY
        DB  $5B
        DB  $5C
        EOR $FFFF,X
        LSR $FFFF,X
        BBR5 $FF,*

        RTS
        ADC ($FF,X)
        DB  $62
        DB  $63
        STZ $FF
        ADC $FF
        ROR $FF
        RMB6 $FF

        PLA
        ADC #$FF
        ROR A
        DB  $6B
        JMP ($FFFF)
        ADC $FFFF
        ROR $FFFF
        BBR6 $FF,*

        BVS *
        ADC ($FF),Y
        ADC ($FF)
        DB  $73
        STZ $FF,X
        ADC $FF,X
        ROR $FF,X
        RMB7 $FF

        SEI
        ADC $FFFF,Y
        PLY
        DB  $7B
        JMP ($FFFF,X)
        ADC $FFFF,X
        ROR $FFFF,X
        BBR7 $FF,*

        BRA *
        STA ($FF,X)
        DB  $82
        DB  $83
        STY $FF
        STA $FF
        STX $FF
        SMB0 $FF

        DEY
        BIT #$FF
        TXA
        DB  $8B
        STY $FFFF
        STA $FFFF
        STX $FFFF
        BBS0 $FF,*

        BCC *
        STA ($FF),Y
        STA ($FF)
        DB  $93
        STY $FF,X
        STA $FF,X
        STX $FF,Y
        SMB1 $FF

        TYA
        STA $FFFF,Y
        TXS
        DB  $9B
        STZ $FFFF
        STA $FFFF,X
        STZ $FFFF,X
        BBS1 $FF,*

        LDY #$FF
        LDA ($FF,X)
        LDX #$FF
        DB  $A3
        LDY $FF
        LDA $FF
        LDX $FF
        SMB2 $FF

        TAY
        LDA #$FF
        TAX
        DB  $AB
        LDY $FFFF
        LDA $FFFF
        LDX $FFFF
        BBS2 $FF,*

        BCS *
        LDA ($FF),Y
        LDA ($FF)
        DB  $B3
        LDY $FF,X
        LDA $FF,X
        LDX $FF,Y
        SMB3 $FF

        CLV
        LDA $FFFF,Y
        TSX
        DB  $BB
        LDY $FFFF,X
        LDA $FFFF,X
        LDX $FFFF,Y
        BBS3 $FF,*

        CPY #$FF
        CMP ($FF,X)
        DB  $C2
        DB  $C3
        CPY $FF
        CMP $FF
        DEC $FF
        SMB4 $FF

        INY
        CMP #$FF
        DEX
        DB  $CB
        CPY $FFFF
        CMP $FFFF
        DEC $FFFF
        BBS4 $FF,*

        BNE *
        CMP ($FF),Y
        CMP ($FF)
        DB  $D3
        DB  $D4
        CMP $FF,X
        DEC $FF,X
        SMB5 $FF

        CLD
        CMP $FFFF,Y
        PHX
        DB  $DB
        DB  $DC
        CMP $FFFF,X
        DEC $FFFF,X
        BBS5 $FF,*

        CPX #$FF
        SBC ($FF,X)
        DB  $E2
        DB  $E3
        CPX $FF
        SBC $FF
        INC $FF
        SMB6 $FF

        INX
        SBC #$FF
        NOP
        DB  $EB
        CPX $FFFF
        SBC $FFFF
        INC $FFFF
        BBS6 $FF,*

        BEQ *
        SBC ($FF),Y
        SBC ($FF)
        DB  $F3
        DB  $F4
        SBC $FF,X
        INC $FF,X
        SMB7 $FF

        SED
        SBC $FFFF,Y
        PLX
        DB  $FB
        DB  $FC
        SBC $FFFF,X
        INC $FFFF,X
        BBS7 $FF,*

        END
