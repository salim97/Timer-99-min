; SIREN.INC: siren (hi ho) @ 31 kHz (1 instr. = 129 µs)
; BENABADJI Salim- ORAN - Apr. 01st, 2015
; macros: 
; routines: Siren2
;-----------------------------------------------------------------------------
; declare in the main file:
;-----------------------------------------------------------------------------
;#define	PIEZO	PORTA, 4		; piezo for alarm
;;siren
;	;frequency
;	;counter
;	cycles
;	;steps
;	;cycles2

;-----------------------------------------------------------------------------
; Siren2 sound (hi ho)
;-----------------------------------------------------------------------------
Siren2
	banksel	cycles
	CLRF  cycles
Ho
	BCF   PIEZO		; PIEZO LOW...
	NOP				;...during 2 x 0.129 = 0.258 ms
	BSF   PIEZO		; PIEZO HIGH...
	NOP				;...during 2 x 0.129 = 0.258 ms
	DECFSZ cycles, f
	GOTO   Ho		; repeat Ho tone during 0.903 * 256 = 231.168 ms
Hi
	BCF   PIEZO		; PIEZO LOW during 0.129 ms
	BSF   PIEZO		; PIEZO HIGH during 0.129 ms
	BCF   PIEZO		; PIEZO LOW during 0.129 ms
	BSF   PIEZO		; PIEZO HIGH during 0.129 ms
	DECFSZ cycles, f
	GOTO   Hi		; repeat Hi tone during 0.903 * 256 = 231.168 ms

	BCF   PIEZO		; to turn off transistor T1 (driver of piezo)
	RETURN
