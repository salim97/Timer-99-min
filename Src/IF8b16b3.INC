; IF8b16b3.INC: useful macros for SI instructions + rajout SIregGEval16b
; BENABADJI Salim - ORAN - July 10th, 2011
; macros: SIregNUL, SIregNOTNUL, SIregEQval8b, SIregGEval8b, SIregLTval8b, 
;	  SIregaLTregb8b, SIregaEQregb8b
;	  SIregEQval16b, SIregaGEregb16b
; routines:
;------------------------------------------------------------------------
; if (file == 0) goto jmpOk ;
;------------------------------------------------------------------------
SIregNUL  macro file, jmpOk
	MOVF	file, w
	BTFSC	STATUS, Z	; file <> 0 ?
	GOTO    jmpOk		; no, i.e. file = 0
	endm
;------------------------------------------------------------------------
; if (file <> 0) goto jmpOk ;
;------------------------------------------------------------------------
SIregNOTNUL  macro file, jmpOk
	MOVF	file, w
	BTFSS	STATUS, Z	; file == 0 ?
	GOTO    jmpOk		; no, i.e. file <> 0
	endm
;------------------------------------------------------------------------
; if (file == val) goto jmpOk ;
;------------------------------------------------------------------------
SIregEQval8b	macro	file, cst, jmpOk
	MOVLW	cst
	XORWF	file, w		; file = cst
	BTFSC	STATUS, Z
	GOTO	jmpOk		; yes
	endm
;------------------------------------------------------------------------
; if (file <> val) goto jmpOk ;
;------------------------------------------------------------------------
SIregNEval8b macro file, val, jmpOk
	MOVLW	val
	XORWF	file, w
	BTFSS	STATUS, Z	; file == val ?
	GOTO	jmpOk		; no
	endm
;------------------------------------------------------------------------
; if (file >= val) goto jmpOk ;
;------------------------------------------------------------------------
SIregGEval8b	macro	file, val, jmpOk
	MOVLW	val
	SUBWF	file, w		; w = file - val
	BTFSC	STATUS, C
	GOTO	jmpOk		; yes
	endm
;------------------------------------------------------------------------
; if (file < val) goto jmpOk ;
;------------------------------------------------------------------------
SIregLTval8b	macro	file, val, jmpOk
	MOVLW	val
	SUBWF	file, w		; w = file - val
	BTFSS	STATUS, C
	GOTO	jmpOk		; yes
	endm
;------------------------------------------------------------------------
; if (regA < regB) goto jmpOk ;
;------------------------------------------------------------------------
SIregaLTregb8b	macro	regA, regB, jmpOk
	MOVF	regB, w
	SUBWF	regA, w		; w = regA - regB
	BTFSS	STATUS, C	; w >= 0
	GOTO	jmpOk		; no
	endm
;------------------------------------------------------------------------
; if (regA == regB) goto jmpOk ;
;------------------------------------------------------------------------
SIregaEQregb8b	macro	regA, regB, jmpOk
	MOVF	regB, w
	XORWF	regA, w		; regA = regB
	BTFSC	STATUS, Z	
	GOTO	jmpOk		; yes
	endm
;------------------------------------------------------------------------
; if (regA <> regB) goto jmpOk ;
;------------------------------------------------------------------------
SIregaNEregb8b	macro	regA, regB, jmpOk
	MOVF	regB, w
	XORWF	regA, w		; regA <> regB
	BTFSS	STATUS, Z	
	GOTO	jmpOk		; yes
	endm
;------------------------------------------------------------------------
; if ((file1 == val1) && (file2 == val2)) goto jmpOk ; else goto jmpNo ;
;------------------------------------------------------------------------
SIregEQval16b	macro	file1, val1, file2, val2, jmpOk, jmpNo
	MOVLW	val1
	XORWF	file1, w	; file1 = val1 ?
	BTFSS	STATUS, Z
	GOTO	jmpNo		; no
	MOVLW	val2
	XORWF	file2, w	; file2 = val2 ?
	BTFSS	STATUS, Z
	GOTO	jmpNo		; no
	GOTO	jmpOk
	endm
;------------------------------------------------------------------------
; if (fileHI < valHI) goto jmpOk ;
; if (fileHI == valHI)
; {if (fileLO < valLO) goto jmpOk ; else goto jmpNo ;
; }else goto jmpNo ;
;------------------------------------------------------------------------
SIregLTval16b	macro	fileHI, valHI, fileLO, valLO, jmpOk, jmpNo
	MOVLW	valHI
	SUBWF	fileHI, W	; W = fileHI - valHI
	BTFSS	STATUS, C	; C=1, if W >= 0
	GOTO	jmpOk		; yes (i.e. fileHI < valHI)

	MOVLW	valHI
	XORWF	fileHI, W	; fileHI = valHI
	BTFSS	STATUS, Z
	GOTO	jmpNo		; no (i.e. fileHI <> valHI)

	MOVLW	valLO
	SUBWF	fileLO, W	; W = fileLO - valLO
	BTFSS	STATUS, C	; C=1, if W >= 0
	GOTO	jmpOk		; yes (i.e. fileLO < valLO)
	GOTO	jmpNo
	endm
;------------------------------------------------------------------------
; if (fileHI < valHI) goto jmpNo ;
; if (fileHI == valHI)
; {if (fileLO < valLO) goto jmpNo ; else goto jmpOk ;
; }else goto jmpOk ;
;------------------------------------------------------------------------
SIregGEval16b	macro	fileHI, valHI, fileLO, valLO, jmpOk, jmpNo
	MOVLW	valHI
	SUBWF	fileHI, W	; W = fileHI - valHI
	BTFSS	STATUS, C	; C=1, if W >= 0
	GOTO	jmpNo		; yes (i.e. fileHI < valHI)

	MOVLW	valHI
	XORWF	fileHI, W	; fileHI = valHI
	BTFSS	STATUS, Z
	GOTO	jmpOk		; no (i.e. fileHI <> valHI)

	MOVLW	valLO
	SUBWF	fileLO, W	; W = fileLO - valLO
	BTFSS	STATUS, C	; C=1, if W >= 0
	GOTO	jmpNo		; yes (i.e. fileLO < valLO)
	GOTO	jmpOk
	endm
;------------------------------------------------------------------------
; if (fileHI < Nhi) goto jmpOk ;
; if (Nhi < fileHI) goto jmpNo ;
; if (Nlo >= fileLO) goto jmpOk ; else goto jmpNo ; 
;------------------------------------------------------------------------
SIregaGEregb16b	macro	Nhi, fileHI, Nlo, fileLO, jmpOk, jmpNo
	MOVF	Nhi, W
	SUBWF	fileHI, W	; w = fileHI - Nhi
	BTFSS	STATUS, C	; C=1, if W >= 0
	GOTO	jmpOk		; no (i.e. fileHI < Nhi)

	MOVF	fileHI, W
	SUBWF	Nhi, W		; W = Nhi - fileHI
	BTFSS	STATUS, C	; C=1, if W >= 0
	GOTO	jmpNo		; no (i.e. Nhi < fileHI)

	MOVF	fileLO, W
	SUBWF	Nlo, W		; W = Nlo - fileLO
	BTFSS	STATUS, C	; C=1, if W >= 0
	GOTO	jmpNo		; no (i.e. Nlo < fileLO)
	GOTO	jmpOk
	endm
;------------------------------------------------------------------------
; if ((Nhi == fileHI) && (Nlo == fileLO)) goto jmpOk ; else goto jmpNo ;
;------------------------------------------------------------------------
SIregaEQregb16b	macro	Nhi, fileHI, Nlo, fileLO, jmpOk, jmpNo
	MOVF	fileHI, w
	SUBWF	Nhi, w		; w = Nhi - fileHI
	BTFSS	STATUS, Z	; w == 0 ?
	GOTO	jmpNo		; no

	MOVF	fileLO, w
	SUBWF	Nlo, w		; w = Nlo - fileLO
	BTFSS	STATUS, Z	; w == 0 ?
	GOTO	jmpNo		; no
	GOTO	jmpOk
	endm
;
;////////////////////////////////////////////////////////////////////////
;
