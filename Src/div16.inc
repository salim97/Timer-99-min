; DIV16.INC: (Dividend:2) div16 (Divisor:2) => (Quotient:2) and Dividend
; BENABADJI Salim - ORAN - Aug. 12th, 2010
;
;------------------------------------------------------------------------------
; declare in the main file:
;------------------------------------------------------------------------------
;div16
;	Counter:2	; 16-bit value for Counter
;	Dividend:2	; 16-bit value for Dividend, and will store remainder
;	Divisor:2	; 16-bit value for Divisor
;	Quotient:2	; 16-bit value for Quotient
;	tmpX
;------------------------------------------------------------------------------
div16
	clrf	Quotient
	clrf	Quotient + 1

	movlw	1		;  Initialize Counter
	movwf	Counter
	clrf	Counter + 1

StartLoop			; Find How Large "Divisor" can be 
	btfsc	Divisor + 1, 7	; If at the "top", then do 
	goto	Loopdiv16	; the Division

	bcf	STATUS, C	; Shift Counter and Divisor Up
	rlf	Counter, f
	rlf	Counter + 1, f
  
	rlf	Divisor, f
	rlf	Divisor + 1, f
	goto	StartLoop

Loopdiv16			; Now, Take Away "Divisor" from "Dividend"
	movf	Divisor, w	; If Dividend => Divisor then
	subwf	Dividend, w	; Take Away
	movwf	tmpX
	movlw	0
	btfss	STATUS, C
	movlw	1
	addwf	Divisor + 1, w
	subwf	Dividend + 1, w
	btfss	STATUS, C
	goto	Skipdiv16	; Divisor < Dividend

	movwf	Dividend + 1	; Save the New Dividend
	movf	tmpX, w
	movwf	Dividend

	movf	Counter, w	; Add Counter to the Quotient
	addwf	Quotient, f
	movf	Counter + 1, w
	addwf	Quotient + 1, f	; No Opportunity for Carry

Skipdiv16			; Shift Divisor/Counter Down
	bcf	STATUS, C
	rrf	Divisor + 1, f
	rrf	Divisor, f

	rrf	Counter + 1, f	; If Carry Set after Counter
	rrf	Counter, f	; Shift, Finished

	btfss	STATUS, C	; If Carry NOT Set, then 
	goto	Loopdiv16	; Process next Bit
	RETURN
