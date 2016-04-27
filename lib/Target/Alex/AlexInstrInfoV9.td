
/* V9Cpu的指令 */

/* 访存指令 */
def V9LL      : InstrI<0x0E, (outs), (ins simm16:$imm16), "v9ll\t$imm16", [], NoItinerary>;
def V9LBL     : InstrI<0x26, (outs), (ins simm16:$imm16), "v9lbl\t$imm16", [], NoItinerary>;
def V9SL      : InstrI<0x40, (outs), (ins simm16:$imm16), "v9sw\t$imm16", [], NoItinerary>;

def V9LI      : InstrI<0x23, (outs), (ins simm16:$imm16), "v9li\t$imm16", [], NoItinerary>;
def V9LIH     : InstrI<0x24, (outs), (ins simm16:$imm16), "v9lih\t$imm16", [], NoItinerary>;

def V9PUSHA   : InstrPlain<0x9d, "v9pusha", [], NoItinerary>;
def V9PUSHB   : InstrPlain<0xa0, "v9pushb", [], NoItinerary>;
def V9PUSHC   : InstrPlain<0xae, "v9pushc", [], NoItinerary>;
def V9PUSHD   : InstrPlain<0xd2, "v9pushd", [], NoItinerary>;
def V9POPA    : InstrPlain<0xa3, "v9popa", [], NoItinerary>;
def V9POPB    : InstrPlain<0xa1, "v9popb", [], NoItinerary>;
def V9POPC    : InstrPlain<0xaf, "v9popc", [], NoItinerary>;
def V9POPD    : InstrPlain<0xd3, "v9popd", [], NoItinerary>;

//def PUSHr     : InstrR<0xFF, (outs), (ins Int32Regs:$ra), "pushr\t$ra", [], NoItinerary>;
//def POPr      : InstrR<0xFF, (outs Int32Regs:$ra), (ins), "popr\t$ra", [], NoItinerary>;
def PUSHr     : AlexPseudo<(outs), (ins Int32Regs:$ra), "pushr\t$ra", []>;
def POPr      : AlexPseudo<(outs Int32Regs:$ra), (ins), "popr\t$ra", []>;

def LEAg      : InstrI<0x08, (outs), (ins simm16:$imm16), "leag\tRA, PC, $imm16", [], NoItinerary>;

/* 分支,函数调用,返回指令 */
def V9ENT     : InstrI<0x01, (outs), (ins simm16:$imm16), "v9ent\t$imm16", [], NoItinerary>;
def V9LEV     : InstrPlain<0x02, "v9lev", [], NoItinerary>;

let isBranch = 1, isTerminator = 1, isBarrier = 1, hasDelaySlot = 1 in {
    def V9JSR : InstrI<0x05, (outs), (ins jmptarget:$imm16),
                "v9jsr\t$imm16", [], NoItinerary>;
    def V9JMP : InstrI<0x03, (outs), (ins jmptarget:$imm16),
                "v9jmp\t$imm16", [(br bb:$imm16)], NoItinerary>;
}

let isBranch = 1, isTerminator = 1, isBarrier = 1, hasDelaySlot = 1 in {
    def V9BNEZ    : InstrI<0x86, (outs), (ins brtarget16:$imm16), "v9bnz\tRA, $imm16", [], NoItinerary>;
    def V9JSRA    : InstrPlain<0x06, "v9jsra", [], NoItinerary>;
}

/* 算数逻辑指令 */
def V9ADD      : InstrPlain<0x53, "v9add", [], NoItinerary>;
def V9SUB      : InstrPlain<0x56, "v9sub", [], NoItinerary>;
def V9MUL      : InstrPlain<0x59, "v9mul", [], NoItinerary>;
def V9DIV      : InstrPlain<0x5c, "v9div", [], NoItinerary>;

def V9ADDi     : InstrI<0x54, (outs), (ins simm16:$imm16), "v9addi", [], NoItinerary>;

def V9LT      : InstrPlain<0x7e, "v9lt\tRA, RB", [], NoItinerary>;
def V9GT      : InstrPlain<0x81, "v9gt\tRA, RB", [], NoItinerary>;
def V9EQ      : InstrPlain<0x7a, "v9eq\tRA, RB", [], NoItinerary>;
def V9NE      : InstrPlain<0x7c, "v9ne\tRA, RB", [], NoItinerary>;
def V9LE      : InstrPlain<0xFF, "v9le\tRA, RB", [], NoItinerary>;
def V9GE      : InstrPlain<0xFF, "v9ge\tRA, RB", [], NoItinerary>;

/* 系统指令 */
def HALT      : InstrPlain<0x00, "halt", [], NoItinerary>;
def BIN       : InstrPlain<0x99, "bin", [], NoItinerary>;
def BOUT      : InstrPlain<0x9a, "bout", [], NoItinerary>;

/* 伪指令 */

def V9LI16_PS : AlexPseudo<(outs Int32Regs:$ra), (ins simm16:$imm16),
                "v9li\t$ra, $imm16",
                [(set Int32Regs:$ra, (i32 immSExt16:$imm16))]>;
def V9LI32_PS : AlexPseudo<(outs Int32Regs:$ra), (ins simm16:$imm),
                "v9li32\t$ra, $imm",
                [(set Int32Regs:$ra, (AlexLI32 immSExt32:$imm))]>;
def V9ADDi_PS : AlexPseudo<(outs Int32Regs:$ra), (ins Int32Regs:$rb, simm16:$imm),
                "v9addi_ps\t$ra, $rb, $imm",
                []>;

defm V9LW_PS   : LoadM32<0,  "v9lw_ps",  load_a, 1>;
defm V9SW_PS   : StoreM32<0,  "v9sw_ps",  store_a, 1>;

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

def : Pat<(i32 immSExt16:$in),
          (V9LI16_PS imm:$in)>;

def : Pat<(add Int32Regs:$hi, (AlexLo tjumptable:$lo)),
          (V9ADDi_PS Int32Regs:$hi, tjumptable:$lo)>;
def : Pat<(add Int32Regs:$hi, (AlexLo tglobaladdr:$lo)),
          (V9ADDi_PS Int32Regs:$hi, tglobaladdr:$lo)>;

/* Alex-machine的几条指令, 以后会删除 */
defm LW     : LoadM32<0x2d,  "lw",  load_a>;
defm SW     : StoreM32<0x34, "sw",  store_a>;
def : Pat<(i32 addr:$in),
          (LW mem:$in)>;