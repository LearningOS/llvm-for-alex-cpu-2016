
/* V9Cpu的一些指令 */
def PUSHr     : InstrR<0xFF, (outs), (ins Int32Regs:$ra), "pushr\t$ra", [], NoItinerary>;
def POPr      : InstrR<0xFF, (outs Int32Regs:$ra), (ins), "popr\t$ra", [], NoItinerary>;
def MOVrr     : InstrRR<0xFF, (outs Int32Regs:$ra), (ins Int32Regs:$rb), "movrr\t$ra, $rb", [], NoItinerary>;
def ADDa      : InstrPlain<0xFF, "adda", [], NoItinerary>;
def SUBa      : InstrPlain<0xFF, "suba", [], NoItinerary>;
def MULa      : InstrPlain<0xFF, "mula", [], NoItinerary>;
def DIVa      : InstrPlain<0xFF, "diva", [], NoItinerary>;

def V9BNEZ    : InstrI<0xFF, (outs), (ins brtarget16:$imm16), "v9bnz\tRA, $imm16", [], NoItinerary>;

def V9LT      : InstrPlain<0xFF, "v9lt\tRA, RB", [], NoItinerary>;
def V9GT      : InstrPlain<0xFF, "v9gt\tRA, RB", [], NoItinerary>;
def V9LE      : InstrPlain<0xFF, "v9le\tRA, RB", [], NoItinerary>;
def V9GE      : InstrPlain<0xFF, "v9ge\tRA, RB", [], NoItinerary>;
def V9EQ      : InstrPlain<0xFF, "v9eq\tRA, RB", [], NoItinerary>;
def V9NE      : InstrPlain<0xFF, "v9ne\tRA, RB", [], NoItinerary>;

/* Pseudo instructions */
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
def V9BLT_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9blt-ps", []>;
def V9BGT_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9bgt-ps", []>;
def V9BLE_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9ble-ps", []>;
def V9BGE_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9bge-ps", []>;
def V9BEQ_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9beq-ps", []>;
def V9BNE_PS  : AlexPseudo<(outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16), "v9bne-ps", []>;
}
def : Pat<(brcc SETLT, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (V9BLT_PS Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16)>;
def : Pat<(brcc SETGT, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (V9BGT_PS Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16)>;
def : Pat<(brcc SETLE, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (V9BLE_PS Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16)>;
def : Pat<(brcc SETGE, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (V9BGE_PS Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16)>;
def : Pat<(brcc SETEQ, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (V9BEQ_PS Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16)>;
def : Pat<(brcc SETNE, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (V9BNE_PS Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16)>;