```mermaid
graph TD;
    START["🟢 START_LEX"]
    
    %% ===== IDENTIFIKÁTORY =====
    ID["📝 ID"]
    GLOBAL_ID["📝 GLOBAL_ID<br/>(__ prefix)"]
    
    %% ===== ČÍSLA =====
    ZERO["🔢 ZERO"]
    NUMBER["🔢 NUMBER"]
    HEXA["🔢 HEXA<br/>(0x...)"]
    FLOAT["🔢 FLOAT"]
    EXP_INT["🔢 INT_EXP<br/>(e/E)"]
    EXP_FLOAT["🔢 FLOAT_EXP<br/>(e/E)"]
    
    %% ===== ŘETĚZCE =====
    STRING["📄 STRING"]
    STRING_BLK["📄 STRING_BLOCK<br/>(triple quotes)"]
    HEXA_CHR["📄 HEXA_CHAR<br/>(\\xHH)"]
    
    %% ===== KOMENTÁŘE =====
    SLASH["/ "]
    LINE_CMT["// Řádkový"]
    BLOCK_CMT["/* */ Blokový<br/>(nested)"]
    
    %% ===== OPERÁTORY =====
    PLUS["➕ +"]
    MINUS["➖ -"]
    MULT["✖️ *"]
    ASSIGN["= "]
    EQ["== "]
    LT["< "]
    LTE["<= "]
    GT["> "]
    GTE[">= "]
    
    %% ===== LOGICKÉ =====
    AND["&& "]
    OR["|| "]
    NOT["! "]
    NOTEQ["!= "]
    
    %% ===== ZÁVORKY =====
    LBRACE["{"]
    RBRACE["}"]
    LPAREN["("]
    RPAREN[")"]
    
    %% ===== OSTATNÍ =====
    DOT["· "]
    DDOT["·· "]
    TDOT["··· "]
    COLON[": "]
    COMMA[", "]
    QUESTION["? "]
    
    %% ===== TERMINÁLY =====
    TOKEN["✓ TOKEN"]
    EOF["🏁 EOF"]
    ERROR["❌ ERROR"]
    
    %% ===== PŘECHODY Z START =====
    START -->|isalpha| ID
    START -->|_| GLOBAL_ID
    START -->|0| ZERO
    START -->|digit| NUMBER
    START -->|"| STRING
    START -->|/| SLASH
    START -->|{| LBRACE
    START -->|}| RBRACE
    START -->|(| LPAREN
    START -->|)| RPAREN
    START -->|+| PLUS
    START -->|-| MINUS
    START -->|*| MULT
    START -->|=| ASSIGN
    START -->|<| LT
    START -->|>| GT
    START -->|.| DOT
    START -->|:| COLON
    START -->|?| QUESTION
    START -->|&| AND
    START -->||| OR
    START -->|!| NOT
    START -->|,| COMMA
    START -->|\0| EOF
    START -->|ostatní| ERROR
    
    %% ===== IDENTIFIKÁTORY =====
    ID -->|alnum/underscore| ID
    ID -->|jinak| TOKEN
    
    GLOBAL_ID -->|alnum/underscore| GLOBAL_ID
    GLOBAL_ID -->|jinak| TOKEN
    
    %% ===== ČÍSLA =====
    ZERO -->|x| HEXA
    ZERO -->|.| FLOAT
    ZERO -->|e/E| EXP_INT
    ZERO -->|jinak| TOKEN
    ZERO -->|digit| ERROR
    
    NUMBER -->|digit| NUMBER
    NUMBER -->|.| FLOAT
    NUMBER -->|e/E| EXP_INT
    NUMBER -->|jinak| TOKEN
    
    HEXA -->|xdigit| HEXA
    HEXA -->|jinak| TOKEN
    
    FLOAT -->|digit| FLOAT
    FLOAT -->|e/E| EXP_FLOAT
    FLOAT -->|jinak| TOKEN
    
    EXP_INT -->|digit| EXP_INT
    EXP_INT -->|+/-| EXP_INT
    EXP_INT -->|jinak| TOKEN
    
    EXP_FLOAT -->|digit| EXP_FLOAT
    EXP_FLOAT -->|+/-| EXP_FLOAT
    EXP_FLOAT -->|jinak| TOKEN
    
    %% ===== ŘETĚZCE =====
    STRING -->|"\n| ERROR
    STRING -->|\\x| HEXA_CHR
    STRING -->|"| STRING_BLK
    STRING -->|ostatní| STRING
    
    HEXA_CHR -->|xdigit x2| STRING
    HEXA_CHR -->|špatně| ERROR
    
    STRING_BLK -->|"| STRING_BLK
    STRING_BLK -->|"""| TOKEN
    
    %% ===== KOMENTÁŘE =====
    SLASH -->|/| LINE_CMT
    SLASH -->|*| BLOCK_CMT
    SLASH -->|jinak| MULT
    
    LINE_CMT -->|\n| TOKEN
    LINE_CMT -->|ostatní| LINE_CMT
    
    BLOCK_CMT -->|*/| BLOCK_CMT
    BLOCK_CMT -->|/*| BLOCK_CMT
    BLOCK_CMT -->|EOF| ERROR
    
    %% ===== OPERÁTORY =====
    ASSIGN -->|=| EQ
    ASSIGN -->|jinak| TOKEN
    EQ --> TOKEN
    
    LT -->|=| LTE
    LT -->|jinak| TOKEN
    LTE --> TOKEN
    
    GT -->|=| GTE
    GT -->|jinak| TOKEN
    GTE --> TOKEN
    
    DOT -->|.| DDOT
    DDOT -->|.| TDOT
    
    %% ===== LOGICKÉ =====
    AND --> TOKEN
    OR --> TOKEN
    NOT -->|=| NOTEQ
    NOT -->|jinak| TOKEN
    NOTEQ --> TOKEN
    
    %% ===== JEDNODUCHÉ SYMBOLY =====
    PLUS --> TOKEN
    MINUS --> TOKEN
    MULT --> TOKEN
    LBRACE --> TOKEN
    RBRACE --> TOKEN
    LPAREN --> TOKEN
    RPAREN --> TOKEN
    COLON --> TOKEN
    COMMA --> TOKEN
    QUESTION --> TOKEN
    TDOT --> TOKEN

    %% Styling
    classDef start fill:#FFE6E6
    classDef number fill:#E6F3FF
    classDef string fill:#E6FFE6
    classDef comment fill:#FFF9E6
    classDef op fill:#F0E6FF
    classDef terminal fill:#FFE6CC
    classDef err fill:#FF6B6B
    
    class START start
    class ZERO,NUMBER,HEXA,FLOAT,EXP_INT,EXP_FLOAT number
    class STRING,STRING_BLK,HEXA_CHR string
    class SLASH,LINE_CMT,BLOCK_CMT comment
    class PLUS,MINUS,MULT,ASSIGN,EQ,LT,LTE,GT,GTE,AND,OR,NOT,NOTEQ,DOT,DDOT,TDOT op
    class TOKEN,EOF terminal
    class ERROR err
```