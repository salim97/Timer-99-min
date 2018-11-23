; pg15.ASM: if no pushbtn pressed after 10s => dim light to save more power
; BENABADJI Salim - ORAN - Oct. 20th, 2017
; pg14.ASM: Btn2(--) to RA3. RA1/AN1 meas. batt. AA
; BENABADJI Salim - ORAN - Oct. 07th, 2017
; pg13.ASM: ultimate optimisation
; BENABADJI Salim - ORAN - Sep. 18th, 2015
; pg12.ASM: adding meas. Vdd with VP2048count; display at initialisation phase
; BENABADJI Salim - ORAN - June 20th, 2015
; pg11.ASM: accelerate Btn1(++), Btn2(--) to ease setting count down
; BENABADJI Salim - ORAN - Apr. 28th, 2015
; pg10.ASM: adding electronic ON/OFF (mosfet BS250) auto OFF to save energy
; BENABADJI Salim - ORAN - Apr. 03rd, 2015
; pg09.ASM: adding siren (hi ho)
; BENABADJI Salim - ORAN - Apr. 01st, 2015
; pg08.ASM: finition
; BENABADJI Salim - ORAN - Mar. 30th, 2015
; pg07.ASM: use TMR0
; BENABADJI Salim - ORAN - Mar. 29th, 2015
; pg06.ASM: use 31 kHz
; BENABADJI Salim - ORAN - Mar. 27th, 2015
; pg05.ASM: two complementary 7-segment LED display (AC + CC)
; BENABADJI Salim - ORAN - Mar. 26th, 2015
; pg04.ASM: 7-segment LED display (AC), Btn1(++), Btn2(--)
; BENABADJI Salim - ORAN - Mar. 25th, 2015
; pg03.ASM: pushbutton thru RA0, in toggle mode
; BENABADJI Salim - ORAN - Mar. 24th, 2015
; pg02.ASM: pushbutton thru RA0
; BENABADJI Salim - ORAN - Mar. 22nd, 2015
; pg01.ASM: blink a LED1 thru RA2
; BENABADJI Salim - ORAN - Mar. 21st, 2015
;
	List P = 16F1824   	;14p/core14/fROM4kW/sRAM256/EEP256/8ADC10b/2cmp/tmr4-1
#include <p16f1824.inc>
	Errorlevel -302		; avoid warning #302: Register in operand not in bank0. 
						; Ensure that bank bits are correct.

	__CONFIG _CONFIG1, H'3FFF' & _FCMEN_OFF & _IESO_OFF & _BOREN_OFF & _MCLRE_OFF & _PWRTE_ON & _WDTE_OFF & _FOSC_INTOSC
	__CONFIG _CONFIG2, H'3FFF' & _LVP_OFF & _PLLEN_OFF
;activate the Watchdog Timer if necessary to recover from unexpected events.

;------------------------------ define hardware -------------------------------
;internal 31 kHz
#define	 BTN1	PORTA, 0		; Btn1(++) connected to RA0
;#define	 BTN2	PORTA, 1		; Btn2(--) connected to RA1
#define	 BTN2	PORTA, 3		; Btn2(--) connected to RA3
#define	 PIEZO	PORTA, 4		; piezo (alarm) connected to RA4
#define	 OnOff	PORTA, 5		; output LOW to keep device running
	
;----------------------------- define variables ------------------------------
;SRAM=256 bytes
	;CBLOCK	0x70		; common BANK = [0x70....0x7F]  = 16 bytes
	;CBLOCK	0x20		; BANK0 = [0x20....0x6F]  = 80 bytes
	;CBLOCK	0xA0		; BANK1 = [0xA0....0xEF]  = 80 bytes
	;CBLOCK	0x120		; BANK2 = [0x120...0x170] = 80 bytes
;======================================
	CBLOCK	0x20	; BANK0 = [0x20....0x6F]  = 80 bytes
;delai
	cpt1, cpt2, cpt3 ; counters
	dly10s			 ; if no pushbtn pressed after 10s => dim light
;countdown (compte à rebours)
	minutes, seconds	; minutes <= 99 (2 digits only)
;display
	temp			; temporary variable
	VddTens			; tens digit of Vdd
	VddUnits		; units digit of Vdd
	AATens			; tens digit of AA voltage supply
	AAUnits			; units digit of AA voltage supply
;SP_DIGBYTE
	;units			; units digit
	;tens			; tens  digit
	;hunds			; hund. digit
	;tmp1, tmp2		; lo, hi bytes (16-bit value)
;siren
	;frequency
	;counter
	cycles
	;steps
	;cycles2
;VP2048andVddandAA
	VP2048HI, VP2048LO	; 10-bit count of internal 2.048V measure
	VddcntHI, VddcntLO	; 10-bit count of power supply Vdd measure
	AAcntHI, AAcntLO 	; 10-bit count of AA voltage supply measure
;div16
	Counter:2	; 16-bit value for Counter
	Dividend:2	; 16-bit value for Dividend, and will store remainder
	Divisor:2	; 16-bit value for Divisor
	Quotient:2	; 16-bit value for Quotient
	;tmpX
;bin2dec
	units			 ; units digit
	tens			 ; tens  digit
	hunds			 ; hundreds digit
	thous			 ; thousands digit
	tenthous		 ; ten thousands digit
	tmp1, tmp2, tmpX ; lo, hi bytes (16-bit value)
	R0, R1, R2		 ; used in bin16dgt5 (R0:upper, R1:high, R2:low)
;others
	beep
;flags
	bnFlags			;<0>=1, any btn pushed
					;<1>=1, overflow odd
					;<2>=1, do timing
	ENDC
;TOTAL = 38 bytes used !
;--------------------------------- define bits -------------------------------

#define	btnPushed	bnFlags, 0 ;<0>=1 => any btn pushed
#define	pairImpair	bnFlags, 1 ;<1>=1 => overflow even
#define	doTiming	bnFlags, 2 ;<2>=1 => do timing

;-----------------------------------------------------------------------------
; move a literal value (constant) to a register
;-----------------------------------------------------------------------------
MOVLF	macro	literal, file	; literal->file
	BANKSEL	  file
	MOVLW	  literal
	MOVWF	  file
	endm
;-----------------------------------------------------------------------------
; increment one digit from 0 to maxi-1 ; if maxi, then wrap to 0 
;-----------------------------------------------------------------------------
INC1	macro	digitX, maxi, jump 
	banksel	digitX
	INCF	digitX, f	; increment digitX 
	MOVLW	maxi
	XORWF	digitX, w
	BTFSS	STATUS, Z	; digitX = maxi ?
	GOTO	jump		; no
	CLRF	digitX		; yes, then clear digitX
	endm

;-----------------------------------------------------------------------------
; decrement one digit from maxi to 0 ; if -1, then wrap to maxi
;-----------------------------------------------------------------------------
DEC1	macro	digitX, maxi, jump
	banksel	digitX
	DECF	digitX, f	; decrement digitX 
	MOVLW	.255
	XORWF	digitX, w	; digitX = -1 ?
	BTFSS	STATUS, Z
	GOTO	jump		; no
	MOVLW	maxi		; yes, then update digitX to highest value
	MOVWF	digitX
	endm
	
;/////////////////////////////////////////////////////////////////////////////
;	MAIN PROGRAM
;/////////////////////////////////////////////////////////////////////////////
	ORG		0x00			; address reset
	GOTO	Main
	ORG		0x04			; address interrupt
	GOTO	intTMR0

;/////////////////////////////////////////////////////////////////////////////
;
;-----------------------------------------------------------------------------
; L.U.T. to convert a decimal value into the corresponding 7 segment code
;-----------------------------------------------------------------------------
Tab7seg	ADDWF PCL,f		; display 7 segments GFEDCBA
	DT 3Fh, 06h, 5Bh, 4Fh, 66h, 6Dh, 7Dh, 07h, 7Fh, 6Fh ; for CC

;-----------------------------------------------------------------------------
#include "BIN8DGT3.INC"	; convert 8-bit into 3 digits (hunds, tens, units)
#include "IF8b16b3.INC"	; all the tests IF
#include "SIREN.INC"	; siren (hi ho) for alarm
#include "CalcVdd2.INC"	; compute Vdd from FVR 2.048V
#include "CalcAA.INC"	; compute AA voltage, according to FVR 2.048V
#include "div16.INC"
#include "BIN16DGT5.INC"

;-----------------------------------------------------------------------------
; delay 1 ms knowing that 1 instruction = 129 µs
;-----------------------------------------------------------------------------
;1032µs
Delai_1ms			; appel CALL : 2 x 129 µs = 258 µs
	GOTO	$+1		; 2 x 129 = 258 µs
	GOTO	$+1		; 2 x 129 = 258 µs
	RETURN			; retour  : 2 x 129 µs = 258 µs
;total=258 x 4 = 1032 µs
;-----------------------------------------------------------------------------
; delay 256 ms
;-----------------------------------------------------------------------------
Delai_256ms			  ; appel CALL  : 2 x 129 µs = 258 µs
	MOVLF	.180, cpt3	  ; macro MOVLF : 3 x 129 µs = 387 µs
	call	Delai_1ms ; 1032 µs
	decfsz   cpt3, F  ; 1 instr = 129 µs
	GOTO	$-2		  ; saut = 2 x 129 µs = 258 µs
	RETURN			  ; retour RETURN: 2 x 129 µs = 258 µs
;total=180 x 1.419 = 255.42 ms + 0.903 ms
;-----------------------------------------------------------------------------
; display 1 digit on AC 7 seg. disp.(sa valeur est dans la variable cnt)
;-----------------------------------------------------------------------------
affdgtAC
	banksel	tens
	MOVLW high Tab7seg
	MOVWF PCLATH
	MOVF  tens, w		; copy counter variable into W
	CALL  Tab7seg		; convert to 7 segment code
	movwf	temp
	comf	temp, F		; complement temp as we use AC display
;config output "1" as input	
	movf	temp, W
	banksel	TRISC
	movwf	TRISC
;output only "0"	
	banksel	PORTC
	movf	temp, W
	movwf	PORTC
;deal with segment G
	btfss	temp, 6		; temp<6> == 1 ?
	goto	noAC		; no
	;bsf	PORTA, 2	; yes => RA2 = 1
	banksel	TRISA
	bsf		TRISA, 2
	goto	rtnAC
noAC:
	banksel	TRISA
	bcf		TRISA, 2
	banksel	PORTA
	bcf		PORTA, 2	; no => RA2 = 0
rtnAC:
	RETURN
;-----------------------------------------------------------------------------
; display 1 digit on CC 7 seg. disp.(sa valeur est dans la variable cnt)
;-----------------------------------------------------------------------------
affdgtCC
	banksel	units
	MOVLW high Tab7seg
	MOVWF PCLATH
	MOVF  units, w		; copy counter variable into W
	CALL  Tab7seg		; convert to 7 segment code
	movwf	temp
	;comf	temp, F		; complement temp as we use AC display
;config output "0" as input	
	comf	temp, W
	banksel	TRISC
	movwf	TRISC
;output only "1"	
	banksel	PORTC
	movf	temp, W
	movwf	PORTC
;deal with segment G
	btfss	temp, 6		; temp<6> == 1 ?
	goto	noCC		; no
	;bsf	PORTA, 2	; yes => RA2 = 1
	banksel	TRISA
	bcf		TRISA, 2
	banksel	PORTA
	bsf		PORTA, 2	; RA2 = 1
	goto	rtnCC
noCC:
	banksel	TRISA
	bsf		TRISA, 2
rtnCC:
	RETURN
;-----------------------------------------------------------------------------
; test pushbutton BTN1 & BTN2
;-----------------------------------------------------------------------------
testBtn:
	banksel	PORTA
	bcf		btnPushed
	btfss	BTN2		; BTN2 relaché ?
	goto	btn2appuye	; no, il est appuyé
	btfsc	BTN1		; BTN1 appuyé ?
	RETURN				; no
;----------
btn1appuye:
	clrf	dly10s
	;clrf	seconds
	MOVLF	.59, seconds	; valeur initiale: seconds = 59
	bsf		doTiming		;we are setting the count down!
	bsf		btnPushed
	;banksel	minutes
	incf	minutes, f		; minutes++
	SIregLTval8b	minutes, .100, rtn ; if (minutes < 100) goto rtn
	clrf	minutes
	incf	minutes, F		; minutes = 1
	RETURN
;----------
btn2appuye:
	clrf	dly10s
	SIregNOTNUL  minutes, jmpOk2
	incf	minutes, F
jmpOk2	
	;clrf	seconds
	MOVLF	.59, seconds	; valeur initiale: seconds = 59
	bsf		doTiming		;we are setting the count down!
	bsf		btnPushed
	;banksel	minutes
	decfsz	minutes, f		; minutes--
	;SIregGEval8b	minutes, .1, rtn ; if (minutes >= 1) goto rtn
	;SIregNUL	minutes, setMinutes ; if (minutes == 0) goto setMinutes
	;decf	minutes, f		; minutes--
	goto	rtn
;setMinutes
	movlw	.99
	movwf	minutes
rtn:
	RETURN
;=============================================================================
Main
;----------	
;choice 4MHz freq. @ initialisation only (just to meas. FVR, then compute Vdd)
	MOVLF	b'01101010', OSCCON	; OSCCON<6:3> = 1101 (4 MHz)
	;MOVLF	b'00000010', OSCCON	; OSCCON<6:3> = 000X (31 kHz)
;wait until clock ready and clock stable
	btfss  OSCSTAT, HFIOFR	; HFINTOSC is ready ?
	goto   $-1
	btfss  OSCSTAT, HFIOFS	; HFINTOSC is at least 0.5% accurate ?
	goto   $-1
;----------	
;choix I/O
	MOVLF	b'00001111', TRISA	;RA0, RA1, RA2, RA3 inputs
	MOVLF	b'00111111', TRISC	;PORTC all inputs
;----------	
;	MOVLF	b'01010100', OPTION_REG	; <7>=0:pullups ON; <5>=0: RA2, not T0CKI
;									; <3>=0:enable prescale to TMR0
;									; <2:0>=100: prescale = 1:32
	MOVLF	b'01010011', OPTION_REG	; <7>=0:pullups ON; <5>=0: RA2, not T0CKI
									; <3>=0:enable prescale to TMR0
									; <2:0>=011: prescale = 1:16

	MOVLF	b'00001001', WPUA	;pullup (RA0, RA3) enabled
	;MOVLF	b'00111111', WPUC	;all pullup enabled for PORTC
	CLRF	WPUC		; no pullup for PORTC
;mode I/O
	banksel	ANSELA
	;clrf	ANSELA 		; PORTA digital I/O, not analog
	MOVLF	b'00000010', ANSELA	; use RA1 as AN1 analog input
	banksel	ANSELC
	clrf	ANSELC 		; PORTC digital I/O, not analog
;==========
	call	measVdd		; meas. Vdd and compute
	call	calcAA		; meas. AA and compute

;----------	
;choice 31KHz freq. definitively
	;MOVLF	b'01101010', OSCCON	; OSCCON<6:3> = 1101 (4 MHz)
	MOVLF	b'00000010', OSCCON	; OSCCON<6:3> = 000X (31 kHz)
;wait until clock ready
	btfss  OSCSTAT, LFIOFR	; LFINTOSC is ready ?
	goto   $-1
;----------	

;PROVISOIRE
 ;call	Siren2

;==========
;display Vdd(tens,units) for 1s
	MOVLF	.45, cpt3		; display for 1s
	;MOVLF	.180, cpt3		; display for 4s
	MOVF	VddTens, w
	MOVWF	tens
	MOVF	VddUnits, w
	MOVWF	units
visu2dgt
	call	affdgtAC
	call	affdgtCC
	
	banksel	cpt3	
	DECFSZ cpt3, f		; cpt3-- and test if nul
	GOTO	visu2dgt	; no
;==========
;display AA(thous,hunds) for 1s
	MOVLF	.45, cpt3		; display for 1s
	;MOVLF	.180, cpt3		; display for 4s
	MOVF	AATens, w
	MOVWF	tens
	MOVF	AAUnits, w
	MOVWF	units
visu2dgtAA
	call	affdgtAC
	call	affdgtCC
	
	banksel	cpt3	
	DECFSZ cpt3, f		; cpt3-- and test if nul
	GOTO	visu2dgtAA	; no

;==========
;display OFF
	MOVLF	b'00111111', TRISC
	banksel	TRISA
	bsf		TRISA, 2	
;==========
;aff.OFF	
	MOVLF	b'00000100', LATA ; LATA = b'0000 0100'
	MOVLF	b'11111111', LATC ; LATC = b'1111 1111'
duree:
	MOVLF	.10, seconds		; valeurs initiales : seconds = 10
	clrf	minutes
	;incf	minutes, F			; minutes = 1
	clrf	beep
	clrf	bnFlags
	clrf	dly10s

;;;;;----------
;;veilleuse:
;	bcf	  INTCON, TMR0IE	; <5>=0 do not use TMR0 int
;	clrf	TMR0
;	bcf	  INTCON, TMR0IF
;	
;	banksel	IOCAF
;	clrf	IOCAF
;	bsf	INTCON, IOCIE			; bit IOCIE = 1 pour autoriser W.O.C.
;	MOVLF	b'00000011', IOCAN	; detect falling edge on RA0 or RA1 
;	sleep						; mode veilleuse
;;----------
;	;banksel	IOCAN
;	;clrf	IOCAN			; do not detect any more falling edge on RA0, RA1
;	bcf	INTCON, IOCIE		; bit IOCIE = 0 pour inhiber W.O.C.
;	banksel	IOCAF
;	clrf	IOCAF			; clear flags
;;;;;

;----------	
	MOVLF	b'00001111', TMR0	; TMR0=15, pour compter seulement 241
	bcf	INTCON, TMR0IF
	
	bsf	INTCON, TMR0IE	; <5>=1 use TMR0 int
	bsf	INTCON, GIE 	; <7>=1 enable general interrupt
;====================
;infinite loop (waiting for TMR0 interrupt)
loop
	SIregNUL   beep, loop	; if (beep == 0) goto loop
;====================
;count down finished, 
;do Beep only if timing has been set, then stop !
	btfss	doTiming	; if bit doTiming == 1, then skip goto theEnd
	goto	theEnd

;doBeep:
	bcf	INTCON, GIE 	; <7>=0 disable general interrupt
	MOVLF	.10, seconds ; 10 s alarm
HiHo:
	call	Siren2
	call	Delai_256ms
	call	Delai_256ms
	decfsz  seconds, F
	goto	HiHo
	;reset
	;goto	duree
theEnd
	banksel	TRISA
	bsf		TRISA, 5	; RA5 input	=> power OFF ...
	goto	$
	;;goto	Main

;=============================================================================
intTMR0
;each 0.5s
	bcf	INTCON, TMR0IF

	; reload init value to count 256 - 14 = 242 ticks (*129µs = 31.225ms)
	; => 31.225 * 16 = 499.6 ms, period of interrupt

	; reload init value to count 256 - 13 = 243 ticks (*129µs = 31.354ms)
	; => 31.354 * 16 = 501.664 ms, period of interrupt

	; reload init value to count 256 - 12 = 244 ticks (*129µs = 31.483ms)
	; => 31.483 * 16 = 503.741 ms, period of interrupt
	MOVLF	.12, TMR0		; TMR0=12, pour compter seulement 244

	btfss	pairImpair
	goto	pairimpair0		; no, i.e. <pairImpair> = 0

;==========
pairimpair1
	bcf		pairImpair
	call	testBtn
	btfss	btnPushed
	goto	endInt
	goto	conv
;==========
pairimpair0
	bsf		pairImpair
	call	testBtn
	SIregGEval8b dly10s, .10, sec ; if (dly10s >= 10) goto sec
	INCF	dly10s, F 
sec
	DEC1   seconds, .59, conv  
	SIregNUL   minutes, cntEnd	; if (minutes == 0) goto cntEnd

	decfsz	minutes, F
	GOTO	conv		; no
cntEnd	
	MOVLF	.1, beep	; countdown finished, do beep,beep,beep,...
	goto	endInt
conv:
	;banksel	minutes
	movf	minutes, W
	movwf	tmp1		; temp. file LOBYTE = minutes
	clrf	tmp2		; temp. file HIBYTE = 0
	call	SP_DIGBYTE	; 16bit (tmp1, tmp2) -> 3 dgts (hunds,tens,units)
;----------
;allumer aff. 33% de 1s
	MOVLF	.20, cpt3		; cpt3 = 20
visu:
	call	affdgtAC
	call	affdgtCC
	
	;banksel	cpt3	
	;DECFSZ cpt3, f		; cpt3-- and test if nul
	;GOTO	visu		; no
;----------
;éteindre aff. 66% de 1s
	MOVLF	b'00111111', TRISC
	banksel	TRISA
	bsf		TRISA, 2

	banksel	dly10s
	movf	dly10s, W
	btfsc	STATUS, Z	; dly10s != 0 ?
	goto	nxtcpt3		; no, i.e. dly10s = 0
	movwf	cpt2
dimLight	
	;call	Delai_1ms 	; 1032 µs
	decf	cpt2, F
	btfsc	STATUS, Z	; cpt2 != 0 ?
	goto	nxtcpt3		; no, i.e. cpt2 = 0
	decf	cpt3, F
	btfsc	STATUS, Z	; cpt3 != 0 ?
	goto	endInt		; no, i.e. cpt3 = 0
	goto	dimLight

nxtcpt3
	banksel	cpt3	
	DECFSZ cpt3, f		; cpt3-- and test if nul
	GOTO	visu		; no
;----------
endInt:
	banksel	   beep		; back to BANK0
	RETFIE
;----------------------------------------------------------------------------
	END
