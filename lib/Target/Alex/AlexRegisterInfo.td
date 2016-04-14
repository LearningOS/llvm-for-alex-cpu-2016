class AlexReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "Alex";
}

class AlexGPReg<bits<16> Enc, string n> : AlexReg<Enc, n>;

let Namespace = "Alex" in {
  //@ General Purpose Registers
  def T0     : AlexGPReg<0,  "t0">,     DwarfRegNum<[0]>;
  def T1     : AlexGPReg<1,  "t1">,     DwarfRegNum<[1]>;
  def T2     : AlexGPReg<2,  "t2">,     DwarfRegNum<[2]>;
  def S0     : AlexGPReg<3,  "s0">,     DwarfRegNum<[3]>;
  def S1     : AlexGPReg<4,  "s1">,     DwarfRegNum<[4]>;
  def FP     : AlexGPReg<5,  "fp">,     DwarfRegNum<[5]>;
  def SP     : AlexGPReg<6,  "sp">,     DwarfRegNum<[6]>;
  def PC     : AlexGPReg<7,  "pc">,     DwarfRegNum<[7]>;
}
def Int32Regs   : RegisterClass<"Alex", [i32], 32, (add T0, T1, T2, S0, S1, FP, SP, PC)>;
def NonSpecialRegs: RegisterClass<"Alex", [i32], 32, (add T1, T2, S0, S1)>;