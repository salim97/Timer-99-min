; BIN8DGT3.INC: convert 8-bit binary value to 3-digit decimal value
; BENABADJI Salim - ORAN - Monday, Feb. 16th, 2015
; macros: 
; routines: SP_DIGBYTE, (for bin24asc7 & bin24asc8, see bin2dgts.inc ...
; ... in d:\bena16Go\BACKUP_PII_NIB\Eln_bn\miniSolarDataLogger\V4\
;
;-----------------------------------------------------------------------------
; declare in the main file:
;-----------------------------------------------------------------------------
;;bin2dec
;	units			; units digit
;	tens			; tens  digit
;	hunds			; hund. digit
;	thous			; thous. digit
;	tmp1, tmp2		; lo, hi bytes (16-bit value)

;-----------------------------------------------------------------------------
; convert 16 bit value (tmp1,tmp2) to 3 separate bytes (hunds,tens,units) 
;-----------------------------------------------------------------------------
SP_DIGBYTE
	CLRF	hunds		; hundreds
	CLRF	tens		; tens
	CLRF	units		; units
	MOVLW	.100		; computing hundreds
Hundrds
	INCF	hunds, f
	SUBWF	tmp1, f		; tmp1 -= 100
	BTFSC	STATUS, C	; tmp1 < 0 ?
	GOTO	Hundrds		; no

	DECF	tmp2, f
	BTFSS	tmp2, 7		; temp. file HIBYTE < 0 ?
	GOTO	Hundrds		; no

	DECF	hunds, f
	ADDWF	tmp1, f
TTens
	MOVLW	.10		; computing tens
	INCF	tens, f
	SUBWF	tmp1, f		; tmp1 -= 10
	BTFSC	STATUS, C	; tmp1 < 0 ?
	GOTO	TTens		; no
	DECF	tens, f
	ADDWF	tmp1, f
	MOVF	tmp1, w		; computing units
	MOVWF	units
	RETURN
