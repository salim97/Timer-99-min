; CalcAA.INC: compute AA voltage, according to internal FVR = 2.048V
; BENABADJI Salim - ORAN - Oct. 07th, 2017
;-----------------------------------------------------------------------------
; declare in the main file:
;-----------------------------------------------------------------------------
;#include "CalcAA.INC"	; to compute AA voltage, according to FVR 2.048V
;#include "div16.INC"

;;VP2048andVddandAA
;	VP2048HI, VP2048LO	; 10-bit count of internal 2.048V measure
;	VddcntHI, VddcntLO	; 10-bit count of power supply Vdd measure
;	AAcntHI, AAcntLO 	; 10-bit count of AA voltage supply measure
;;div16
;	Counter:2		; 16-bit value for Count
;	Dividend:2		; 16-bit value for Dividend, and will store remainder
;	Divisor:2		; 16-bit value for Divisor
;	Quotient:2		; 16-bit value for Quotient
;	tmpX
;;bin2dec
;	units			 ; units digit
;	tens			 ; tens  digit
;	hunds			 ; hundreds digit
;	thous			 ; thousands digit
;	tenthous		 ; ten thousands digit
;	tmp1, tmp2, tmpX ; lo, hi bytes (16-bit value)
;	R0, R1, R2		 ; used in bin16dgt5 (R0:upper, R1:high, R2:low)
;
;/////////////////////////////////////////////////////////////////////////////
;
;-----------------------------------------------------------------------------
; 10-bit measure AA voltage supply, thru AN1
;-----------------------------------------------------------------------------
measAA
	MOVLW	b'00000100'		; <6-2>=00001, mask for channel AN1
	CALL	meas10btoFVR

	MOVLB 1					; switch to bank1
	movf	ADRESH, W		; yes, then retreive high byte
	MOVLB 0					; switch to bank0
	movwf	AAcntHI			; store it in AAcntHI
	MOVLB 1					; switch to bank1
	movf	ADRESL, W		; retreive low byte
	MOVLB 0					; switch to bank0
	movwf	AAcntLO			; store it in AAcntLO
	RETURN
;------------------------------------------------------------------------------
; 10-bit measure (subroutine duration = 48 us @ 25°C (75us, measured on EM120)
; internal 4 MHz for minidatalogger V9
; internal 31 kHz in this KL101 prototype (temporisateur 99 min.)
; tip: do 4MHz @ initialisation, meas. Vdd, AA, then, do 31 kHz, display Vdd...
;------------------------------------------------------------------------------
meas10btoFVR
	MOVLB	1				; switch to bank1
	MOVWF	ADCON0

	MOVLW	b'11000011'		;right justified, <64>=100 => Fosc/4 (if 4MHz)... 
 	MOVWF	ADCON1			;... <10>=11 => VREF+ connected to internal FVR
	;MOVLW	b'11000000'		;right justified, <64>=100 => Fosc/4 (if 4MHz)... 
 	;MOVWF	ADCON1			;... <10>=00 => VREF+ connected to Vdd

	BSF	ADCON0, ADON		; switch on ADC module

	MOVLW	.8				; 32 us delai interne (if 4 MHz)
	ADDLW	0FFh			; revient à soustraire 1 de W
	BTFSS	STATUS, Z		; bit Z=1 (i.e. W = 0 ?)
	GOTO	$-2				; boucle interne = 4 * 8 = 32 us

	BSF	 ADCON0, ADGO		; initiate one A/D conversion
	BTFSC  ADCON0, ADGO		; A/D conversion complete ?
	GOTO	$-1				; no

	NOP
	BCF	 ADCON0, ADON		; switch off ADC module
	RETURN
;-----------------------------------------------------------------------------
; compute AA according to FVR = 2.048V
;-----------------------------------------------------------------------------
calcAA
	CALL	measAA		; 10-bit meas. AA voltage supply
	;first, multiply by 2
	BCF	STATUS, C	; clear CARRY
	RLF	AAcntLO, f
	RLF	AAcntHI, f
	; then convert 16bit (tmp1,tmp2) -> 5 dgts (tenthous,...,units)
	movf	AAcntLO, w
	MOVWF	tmp1
	movf	AAcntHI, w
	MOVWF	tmp2
	CALL	bin16dgt5
	
	movf	thous, w
	movwf	AATens
	movf	hunds, w
	movwf	AAUnits
	RETURN
