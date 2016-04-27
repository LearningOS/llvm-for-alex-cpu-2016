
/* V9Cpu的指令 */

/* 访存指令 */
def V9LL      : InstrI<0xFF, (outs), (ins simm16:$imm16), "v9ll\t$imm16", [], NoItinerary>;
def V9SL      : InstrI<0xFF, (outs), (ins simm16:$imm16), "v9sw\t$imm16", [], NoItinerary>;

def PUSHr     : InstrR<0xFF, (outs), (ins Int32Regs:$ra), "pushr\t$ra", [], NoItinerary>;
def POPr      : InstrR<0xFF, (outs Int32Regs:$ra), (ins), "popr\t$ra", [], NoItinerary>;
def MOVrr     : InstrRR<0xFF, (outs Int32Regs:$ra), (ins Int32Regs:$rb), "movrr\t$ra, $rb", [], NoItinerary>;

def LEAg      : InstrI<0xFF, (outs), (ins simm16:$imm16), "leag\tRA, PC, $imm16", [], NoItinerary>;

/* 分支,函数调用,返回指令 */
def V9ENT     : InstrI<0xFF, (outs), (ins simm16:$imm16), "v9ent\t$imm16", [], NoItinerary>;
def V9LEV     : InstrPlain<0xFF, "v9lev", [], NoItinerary>;

let isBranch = 1, isTerminator = 1, isBarrier = 1, hasDelaySlot = 1 in {
    def V9JSR : InstrI<0xFF, (outs), (ins jmptarget:$imm16),
                "v9jsr\t$imm16", [], NoItinerary>;
    def V9JMP : InstrI<0xFF, (outs), (ins jmptarget:$imm16),
                "v9jmp\t$imm16", [(br bb:$imm16)], NoItinerary>;
}

let isBranch = 1, isTerminator = 1, isBarrier = 1, hasDelaySlot = 1 in {
    def V9BNEZ    : InstrI<0xFF, (outs), (ins brtarget16:$imm16), "v9bnz\tRA, $imm16", [], NoItinerary>;
    def V9JSRA    : InstrPlain<0xFF, "v9jsra", [], NoItinerary>;
}


/* 算数逻辑指令 */
def V9ADD      : InstrPlain<0xFF, "v9add", [], NoItinerary>;
def V9SUB      : InstrPlain<0xFF, "v9sub", [], NoItinerary>;
def V9MUL      : InstrPlain<0xFF, "v9mul", [], NoItinerary>;
def V9DIV      : InstrPlain<0xFF, "v9div", [], NoItinerary>;

def V9LT      : InstrPlain<0xFF, "v9lt\tRA, RB", [], NoItinerary>;
def V9GT      : InstrPlain<0xFF, "v9gt\tRA, RB", [], NoItinerary>;
def V9LE      : InstrPlain<0xFF, "v9le\tRA, RB", [], NoItinerary>;
def V9GE      : InstrPlain<0xFF, "v9ge\tRA, RB", [], NoItinerary>;
def V9EQ      : InstrPlain<0xFF, "v9eq\tRA, RB", [], NoItinerary>;
def V9NE      : InstrPlain<0xFF, "v9ne\tRA, RB", [], NoItinerary>;

/* 伪指令 */
defm V9LW_PS   : LoadM32<0xFF,  "v9lw_ps",  load_a, 1>;
defm V9SW_PS   : StoreM32<0xFF,  "v9sw_ps",  store_a, 1>;

def V9LT_PS   : AlexPseudo<(outs Int32Regs:$ra), (ins Int32Regs:$rb, Int32Regs:$rc),
                "v9lt_ps\t$ra, $rb, $rc",
                [(set Int32Regs:$ra, (setlt Int32Regs:$rb, Int32Regs:$rc))]>;
def V9GT_PS   : AlexPseudo<(outs Int32Regs:$ra), (ins Int32Regs:$rb, Int32Regs:$rc),
                "v9gt_ps\t$ra, $rb, $rc",
                [(set Int32Regs:$ra, (setgt Int32Regs:$rb, Int32Regs:$rc))]>;
def V9LE_PS   : AlexPseudo<(outs Int32Regs:$ra), (ins Int32Regs:$rb, Int32Regs:$rc),
                "v9le_ps\t$ra, $rb, $rc",
                [(set Int32Regs:$ra, (setle Int32Regs:$rb, Int32Regs:$rc))]>;
def V9GE_PS   : AlexPseudo<(outs Int32Regs:$ra), (ins Int32Regs:$rb, Int32Regs:$rc),
                "v9ge_ps\t$ra, $rb, $rc",
                [(set Int32Regs:$ra, (setge Int32Regs:$rb, Int32Regs:$rc))]>;
def V9EQ_PS   : AlexPseudo<(outs Int32Regs:$ra), (ins Int32Regs:$rb, Int32Regs:$rc),
                "v9eq_ps\t$ra, $rb, $rc",
                [(set Int32Regs:$ra, (seteq Int32Regs:$rb, Int32Regs:$rc))]>;
def V9NE_PS   : AlexPseudo<(outs Int32Regs:$ra), (ins Int32Regs:$rb, Int32Regs:$rc),
                "v9ne_ps\t$ra, $rb, $rc",
                [(set Int32Regs:$ra, (setne Int32Regs:$rb, Int32Regs:$rc))]>;

let isBranch = 1, isTerminator = 1, isBarrier = 1, hasDelaySlot = 1 in {
    def V9BLT_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9blt-ps",
                    [(brcc SETLT, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16)]>;
    def V9BGT_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9bgt-ps",
                    [(brcc SETGT, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16)]>;
    def V9BLE_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9ble-ps",
                    [(brcc SETLE, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16)]>;
    def V9BGE_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9bge-ps",
                    [(brcc SETGE, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16)]>;
    def V9BEQ_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9beq-ps",
                    [(brcc SETEQ, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16)]>;
    def V9BNE_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9bne-ps",
                    [(brcc SETNE, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16)]>;
}

/* Alex-machine的几条指令, 以后会删除 */
defm LW     : LoadM32<0x2d,  "lw",  load_a>;
defm SW     : StoreM32<0x34, "sw",  store_a>;
def : Pat<(i32 addr:$in),
          (LW mem:$in)>;