class AlexReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "Alex";
}

class AlexAllReg<bits<16> Enc, string n> : AlexReg<Enc, n>;

let Namespace = "Alex" in {
  //@ General Purpose Registers
  def T0     : AlexAllReg<0,   "t0">,     DwarfRegNum<[0]>;
  def T1     : AlexAllReg<1,   "t1">,     DwarfRegNum<[1]>;
  def T2     : AlexAllReg<2,   "t2">,     DwarfRegNum<[2]>;
  def T3     : AlexAllReg<3,   "t3">,     DwarfRegNum<[3]>;
  def T4     : AlexAllReg<4,   "t4">,     DwarfRegNum<[4]>;
  def S0     : AlexAllReg<5,   "s0">,     DwarfRegNum<[5]>;
  def S1     : AlexAllReg<6,   "s1">,     DwarfRegNum<[6]>;
  def S2     : AlexAllReg<7,   "s2">,     DwarfRegNum<[7]>;
  def S3     : AlexAllReg<8,   "s3">,     DwarfRegNum<[8]>;
  def AT     : AlexAllReg<9,   "at">,     DwarfRegNum<[9]>;
  def LR     : AlexAllReg<10,  "lr">,     DwarfRegNum<[10]>;
  def ZERO   : AlexAllReg<11,  "zero">,   DwarfRegNum<[11]>;
  def GP     : AlexAllReg<12,  "gp">,     DwarfRegNum<[12]>;
  def FP     : AlexAllReg<13,  "fp">,     DwarfRegNum<[13]>;
  def SP     : AlexAllReg<14,  "sp">,     DwarfRegNum<[14]>;
  def PC     : AlexAllReg<15,  "pc">,     DwarfRegNum<[15]>;
}
def Int32Regs   : RegisterClass<"Alex", [i32], 32, (add T0, T1, T2, S0, S1, FP, SP, PC)>;
def NonSpecialRegs: RegisterClass<"Alex", [i32], 32, (add T1, T2, S0, S1)>;