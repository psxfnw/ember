; Ті самі вісім елементів і обмежений цикл, що в Lab 04.
.define TARGET 65
LOADI A, 1
STORE [0x800], A
STORE [0x801], A
STORE [0x802], A
LOADI A, 65
STORE [0x803], A
LOADI A, 1
STORE [0x804], A
STORE [0x805], A
STORE [0x806], A
STORE [0x807], A
LOADH H, 0x800
loop:
    LOADI B, TARGET
    LOAD A, [H]
    CMP A, B
    JZ found
    INCH
    HLOW
    LOADI B, 8
    CMP A, B
    JZ missing
    JMP loop
found:
    HLOW
    OUTN
    HALT
missing:
    LOADI A, 255
    OUTN
    HALT
