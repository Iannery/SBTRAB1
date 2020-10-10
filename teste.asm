;SECTION	TEXT
;MUL_N:	MACRO 
;MULT	N
;STORE	N
;ENDMACRO
;INPUT	N
;LOAD	N
;FAT:	SUB	ONE
;JMPZ	FIM
;STORE	AUX
;MUL_N
;LOAD	AUX
;JMP	FAT
;FIM: OUTPUT N
;STOP
;SECTION DATA
;AUX: SPACE
;N: SPACE
;ONE: CONST 1

;SECTION TEXT
;INPUT OLD_DATA
;LOAD OLD_DATA
;L1: DIV DOIS
;STORE  NEW_DATA
;MULT DOIS
;STORE TMP_DATA
;LOAD OLD_DATA
;SUB TMP_DATA
;STORE TMP_DATA
;OUTPUT TMP_DATA
;COPY NEW_DATA, OLD_DATA
;LOAD OLD_DATA
;JMPP L1
;STOP
;SECTION DATA
;DOIS: CONST 2
;OLD_DATA: SPACE
;NEW_DATA: SPACE
;TMP_DATA: SPACE

SECTION TEXT
COPY ZERO, OLDER
COPY ONE, OLD
INPUT LIMIT
OUTPUT OLD
FRONT: LOAD OLDER
ADD OLD
STORE NEW
SUB LIMIT
JMPP FINAL
OUTPUT NEW
COPY OLD, OLDER
COPY NEW, OLD
JMP FRONT
FINAL: OUTPUT LIMIT
STOP
SECTION DATA
ZERO: CONST 0
ONE: CONST 1
OLDER: SPACE
OLD: SPACE
NEW: SPACE
LIMIT: SPACE