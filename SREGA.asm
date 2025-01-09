SREGA    START   1000 
         LDA     VALUE             
         STA     TEMP              
         LDA     TEMP
         DIV     C2
         AND     THIRD
         DIV     C3
         STA     NUM1
         LDA     TEMP
         DIV     C1
         AND     THIRD
         DIV     C3
         STA     NUM2
         LDA     TEMP
         AND     THIRD
         DIV     C3
         STA     NUM3
         LDA     TEMP
         AND     FOURTH
         DIV     C2
         STA     NUM4
         LDA     TEMP
         AND     FIFTH
         DIV     C1
         STA     NUM5
         LDA     TEMP
         AND     SIXTH
         STA     NUM6
         J       OUTER
OLP      TD      DEV05             
         JEQ     OLP
         LDA     FINAL
         DIV     C2
         AND     OUT2
         DIV     C2
         STA     ANS1
         WD      DEV05             
OLP1     TD      DEV05
         JEQ     OLP1               
         LDA     FINAL
         AND     OUT2            
         DIV     C2
         STA     ANS2                   
         WD      DEV05             
OLP2     TD      DEV05
         JEQ     OLP2               
         LDA     FINAL
         AND     OUT1
         STA     ANS3
         WD      DEV05
         LDA     FINAL             
         RSUB
OUTER    LDA     SIX   
         STA     COUNT
BUBBLE   LDA     NUM1
         COMP    NUM2
         JLT     NOSWAP1            
         LDA     NUM2
         STA     TEMP               
         LDA     NUM1
         STA     NUM2               
         LDA     TEMP
         STA     NUM1               
NOSWAP1  LDA     NUM2
         COMP    NUM3
         JLT     NOSWAP2            
         LDA     NUM3
         STA     TEMP
         LDA     NUM2
         STA     NUM3
         LDA     TEMP
         STA     NUM2
NOSWAP2  LDA     NUM3
         COMP    NUM4
         JLT     NOSWAP3            
         LDA     NUM4
         STA     TEMP
         LDA     NUM3
         STA     NUM4
         LDA     TEMP
         STA     NUM3
NOSWAP3  LDA     NUM4
         COMP    NUM5
         JLT     NOSWAP4
         LDA     NUM5
         STA     TEMP
         LDA     NUM4
         STA     NUM5
         LDA     TEMP
         STA     NUM4
NOSWAP4  LDA     NUM5
         COMP    NUM6
         JLT     DONE
         LDA     NUM6
         STA     TEMP
         LDA     NUM5
         STA     NUM6
         LDA     TEMP
         STA     NUM5
DONE     LDA     COUNT
         SUB     ONE
         STA     COUNT
         COMP    ZERO
         JGT     BUBBLE
FINISH   LDA     NUM1            
         MUL     SIXTEEN            
         ADD     NUM2               
         MUL     SIXTEEN            
         ADD     NUM3
         MUL     SIXTEEN            
         ADD     NUM4
         MUL     SIXTEEN            
         ADD     NUM5
         MUL     SIXTEEN            
         ADD     NUM6
         STA     FINAL
         J       OLP
ANS1     RESW    1
ANS2     RESW    1
ANS3     RESW    1
NUM1     RESW    1                  
NUM2     RESW    1
NUM3     RESW    1
NUM4     RESW    1
NUM5     RESW    1
NUM6     RESW    1
TEMP     RESW    1                  
FINAL    RESW    1
COUNT    RESW    1
DEV05    BYTE    X'05'
THIRD    WORD    61440
FOURTH   WORD    3840       
FIFTH    WORD    240
SIXTH    WORD    15             
SIXTEEN  WORD    16                 
SIX      WORD    6
ONE      WORD    1
OUT1     WORD    255
OUT2     WORD    65280
ZERO     WORD    0
C3       WORD    4096
C2       WORD    256
C1       WORD    16
VALUE    WORD    -7982
         END     SREGA
