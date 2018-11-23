; CalcVdd2.INC: compute Vdd using 10-bit meas. of internal 2.048V ref (AN1072)
; BENABADJI Salim - ORAN - June 20th, 2015
;-----------------------------------------------------------------------------
; declare in the main file:
;-----------------------------------------------------------------------------
;#include "CalcVdd2.INC"	; to compute Vdd from FVR 2.048V
;#include "div16.INC"

;;VP2048andVdd
;	VP2048HI, VP2048LO ; 10-bit count of internal 2.048V measure
;	VddcntHI, VddcntLO ; 10-bit count of power supply Vdd measure
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
; 10-bit measure from internal FVR 2.048V
;-----------------------------------------------------------------------------
measFVR2048
	MOVLB 2					; switch to bank2
	movlw	b'10000010'		; <7>=1: FVR on, <10>=10: 2.048V reference  
	iorwf FVRCON, F
	;wait until reference stable
	btfss FVRCON, FVRRDY	; FVRRDY=1 => reference is ready
	goto $-1

	MOVLW	b'01111100'		; <62>=11111, mask for internal FVR reference
	CALL	meas10b

	MOVLB 1					; switch to bank1
	movf	ADRESH, W		; yes, then retreive high byte
	MOVLB 0					; switch to bank0
	movwf	VP2048HI		; store it in VP2048HI
	MOVLB 1					; switch to bank1
	movf	ADRESL, W		; retreive low byte
	MOVLB 0					; switch to bank0
	movwf	VP2048LO		; store it in VP2048LO
	RETURN
;-----------------------------------------------------------------------------
; 10-bit measure (subroutine duration = 48 us @ 25°C (75us, measured on EM120)
; internal 4 MHz for minidatalogger V9
; internal 31 kHz in this KL101 prototype (temporisateur 99 min.)
; tip: do 4MHz @ initialisation, meas. Vdd, then, do 31 kHz, display Vdd, ...
;-----------------------------------------------------------------------------
meas10b
	MOVLB	1				; switch to bank1
	MOVWF	ADCON0

	;MOVLW	b'11000011'		;right justified, <64>=100 => Fosc/4 (if 4MHz)... 
 	;MOVWF	ADCON1			;... <10>=11 => VREF+ connected to internal FVR
	MOVLW	b'11000000'		;right justified, <64>=100 => Fosc/4 (if 4MHz)... 
 	MOVWF	ADCON1			;... <10>=00 => VREF+ connected to Vdd

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
; compute Vdd from equation 2 (AN1072)
;-----------------------------------------------------------------------------
calcVdd
;prepare divide
	;MOVLW	0xEF		; Dividend = numerator = EFC4 = 61380 (0.60V @Vdd=2.50)
	;MOVWF	Dividend + 1
	;MOVLW	0xC4
	;MOVWF	Dividend
 ;MOVLW	0xEB		; Dividend = numerator = EBC5 = 60357 (0.59V @Vdd=4.75)
 ;MOVWF	Dividend + 1
 ;MOVLW	0xC5
 ;MOVWF	Dividend
	;MOVLW	0xE7		; Dividend = numerator = E7C6 = 59334 (0.58V @Vdd=4.5)
	;MOVWF	Dividend + 1
	;MOVLW	0xC6
	;MOVWF	Dividend
;MOVLW	0x08		; Dividend = numerator = 082F = 2095 (2.048V @Vdd=5.0)
;MOVWF	Dividend + 1
;MOVLW	0x2F
;MOVWF	Dividend
	MOVLW	0x51		; Dividend = numerator = 51D7 = 20951 (2.048V @Vdd=5.0)
	MOVWF	Dividend + 1
	MOVLW	0xD7
	MOVWF	Dividend

	MOVF	VP2048HI, W ; Divisor = denominator = VP2048
	MOVWF	Divisor + 1
	MOVF	VP2048LO, W
	MOVWF	Divisor

	CALL	div16	;(Dividend:2) / (Divisor:2) => (Quotient:2) and Dividend

	MOVF	Quotient, w
	MOVWF	VddcntLO
	MOVF	Quotient + 1, w
	MOVWF	VddcntHI
	RETURN
;-----------------------------------------------------------------------------
; meas. fixed internal ref. 2.048V then compute Vdd from equation 2 (AN1072)
;-----------------------------------------------------------------------------
measVdd
	CALL	measFVR2048	; meas. internal 2.048V ref
	CALL	calcVdd		; compute Vdd from equation 2 (AN1072)

	MOVF	VddcntLO, w
	MOVWF	tmp1		; temp. file LOBYTE
	MOVF	VddcntHI, w
	MOVWF	tmp2		; temp. file HIBYTE

 ;MOVF	VP2048LO, W
 ;MOVWF	tmp1		; temp. file LOBYTE
 ;MOVF	VP2048HI, w
 ;MOVWF	tmp2		; temp. file HIBYTE

	CALL	bin16dgt5	; 16bit (tmp1,tmp2) -> 5 dgts (tenthous,...,units)
	movf	tens, w
	movwf	VddTens
	movf	units, w
	movwf	VddUnits
	RETURN
