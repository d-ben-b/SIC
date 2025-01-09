COPY    START   1000
FIRST   LDA     LENGTH
        ADD     #5
        STA     BUFFER
        CLEAR   A
        COMP    ZERO
        JEQ     EXIT
EXIT    RSUB
LENGTH  WORD    5
BUFFER  RESW    1
ZERO    WORD    0
        END     COPY
