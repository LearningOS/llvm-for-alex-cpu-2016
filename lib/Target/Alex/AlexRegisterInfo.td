class AlexReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "Alex";
}

class AlexAllReg<bits<16> Enc, string n> : AlexReg<Enc, n>;

let Namespace = "Alex" in {
  //@ General Purpose Registers
  def R0     : AlexAllReg<0,   "r0">,     DwarfRegNum<[0]>;
  def T0     : AlexAllReg<0,   "t0">,     DwarfRegNum<[1]>;
  def T1     : AlexAllReg<1,   "t1">,     DwarfRegNum<[2]>;
  def T2     : AlexAllReg<2,   "t2">,     DwarfRegNum<[3]>;
  def T3     : AlexAllReg<3,   "t3">,     DwarfRegNum<[4]>;
  def T4     : AlexAllReg<4,   "t4">,     DwarfRegNum<[5]>;
  def S0     : AlexAllReg<5,   "s0">,     DwarfRegNum<[6]>;
  def S1     : AlexAllReg<6,   "s1">,     DwarfRegNum<[7]>;
  def S2     : AlexAllReg<7,   "s2">,     DwarfRegNum<[8]>;
  def S3     : AlexAllReg<8,   "s3">,     DwarfRegNum<[9]>;
  def S4     : AlexAllReg<9,   "s4">,     DwarfRegNum<[10]>;
  def FP     : AlexAllReg<10,  "fp">,     DwarfRegNum<[11]>;
  def SP     : AlexAllReg<11,  "sp">,     DwarfRegNum<[12]>;
  def GP     : AlexAllReg<12,  "gp">,     DwarfRegNum<[13]>;
  def AT     : AlexAllReg<13,  "fp">,     DwarfRegNum<[14]>;
  def LR     : AlexAllReg<14,  "sp">,     DwarfRegNum<[15]>;
}

def Int32Regs   : RegisterClass<"Alex", [i32], 32, (add T0, T1, T2, T3, T4, S0, S1, FP, SP)>;
def NonSpecialRegs: RegisterClass<"Alex", [i32], 32, (add T1, T2, S0, S1)>;