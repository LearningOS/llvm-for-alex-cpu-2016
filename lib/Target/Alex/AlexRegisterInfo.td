class AlexReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "Alex";
}

class AlexGPReg<bits<16> Enc, string n> : AlexReg<Enc, n>;

let Namespace = "Alex" in {
  //@ General Purpose Registers
  def ZERO   : AlexGPReg<0,  "zero">,   DwarfRegNum<[0]>;
  def R1     : AlexGPReg<1,  "r1">,     DwarfRegNum<[1]>;
  def R2     : AlexGPReg<2,  "r2">,     DwarfRegNum<[2]>;
  def R3     : AlexGPReg<3,  "r3">,     DwarfRegNum<[3]>;
  def R4     : AlexGPReg<4,  "r4">,     DwarfRegNum<[4]>;
  def FP     : AlexGPReg<5,  "fp">,     DwarfRegNum<[5]>;
  def SP     : AlexGPReg<6,  "sp">,     DwarfRegNum<[6]>;
  def PC     : AlexGPReg<7,  "pc">,     DwarfRegNum<[7]>;
}
def Int32Regs   : RegisterClass<"Alex", [i32], 32, (add ZERO, R1, R2, R3, R4, FP, SP, PC)>;