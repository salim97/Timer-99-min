; BIN16DGT5.INC: convert 16-bit binary value to 5-digit decimal value
; BENABADJI Salim - ORAN - Monday, Mar. 02nd, 2015
;-----------------------------------------------------------------------------
; declare in the main file:
;-----------------------------------------------------------------------------
;;bin2dec
;	units			 ; units digit
;	tens			 ; tens  digit
;	hunds			 ; hundreds digit
;	thous			 ; thousands digit
;	tenthous		 ; ten thousands digit
;	tmp1, tmp2, tmpX ; lo, hi bytes (16-bit value)
;	R0, R1, R2		 ; used in bin16dgt5 (R0:upper, R1:high, R2:low)
;
;////////////////////////////////////////////////////////////////////////
;
;-----------------------------------------------------------------------------
; convert 16 bit value (tmp1,tmp2) to 5 digits in BCD format (in R0, R1, R2)
; 											where: R0:upper, R1:high, R2:low
; then to 5 bytes (units, tens, hunds, thous, tenthous) 
;-----------------------------------------------------------------------------
bin16dgt5
	BCF	STATUS, C
	MOVLW	.16
	MOVWF	cpt1
	CLRF	R0
	CLRF	R1
	CLRF	R2
loop16
	RLF	tmp1, F		; low byte
	RLF	tmp2, F		; high byte
	RLF	R2, F
	RLF	R1, F
	RLF	R0, F
	DECFSZ	cpt1, F
	GOTO	adjDEC
	;RETURN
	GOTO	bcd5dgt
adjDEC
	MOVLW	R2
	MOVWF	FSR0L
	CLRF	FSR0H
	CALL	adjBCD

	MOVLW	R1
	MOVWF	FSR0L
	CLRF	FSR0H
	CALL	adjBCD

	MOVLW	R0
	MOVWF	FSR0L
	CLRF	FSR0H
	CALL	adjBCD
	GOTO	loop16
adjBCD
	MOVLW	3
	ADDWF	0, W
	MOVWF	tmpX
	BTFSC	tmpX, 3		; test if result > 7
	MOVWF	0

	MOVLW	30
	ADDWF	0, W
	MOVWF	tmpX
	BTFSC	tmpX, 7		; test if result > 7
	MOVWF	0

bcd5dgt
;(R0, R1, R2)->(units, tens, hunds, thous, tenthous)
	MOVF	R2, w
	ANDLW	0x0F
	MOVWF	units

	SWAPF	R2, w
	ANDLW	0x0F
	MOVWF	tens

	MOVF	R1, w
	ANDLW	0x0F
	MOVWF	hunds

	SWAPF	R1, w
	ANDLW	0x0F
	MOVWF	thous

	MOVF	R0, w
	ANDLW	0x0F
	MOVWF	tenthous
	RETURN
