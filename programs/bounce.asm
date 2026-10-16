; Шість кадрів: x=61,62,63,62,61,60; y=16.
.define X 0x800
.define DIRECTION 0x801
.define REMAINING 0x802
.define FRAMES 6
LOADI A, 61
STORE [X], A
LOADI A, 1
STORE [DIRECTION], A
LOADI A, FRAMES
STORE [REMAINING], A
frame:
    CLS
    LOAD A, [X]
    LOADI B, 16
    PLOT
    SHOW
    LOAD A, [X]
    LOAD B, [DIRECTION]
    ADD A, B
    STORE [X], A
    LOADI B, 63
    CMP A, B
    JZ turn_left
    LOADI B, 0
    CMP A, B
    JZ turn_right
next:
    LOAD A, [REMAINING]
    DEC A
    STORE [REMAINING], A
    JNZ frame
    HALT
turn_left:
    LOADI A, 255
    STORE [DIRECTION], A
    JMP next
turn_right:
    LOADI A, 1
    STORE [DIRECTION], A
    JMP next
