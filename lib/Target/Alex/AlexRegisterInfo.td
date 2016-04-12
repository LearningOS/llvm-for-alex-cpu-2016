class AlexReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "Alex";
}

class AlexGPReg<bits<16> Enc, string n> : AlexReg<Enc, n>;

let Namespace = "Alex" in {
  //@ General Purpose Registers
  def R0     : AlexGPReg<0,  "r0">,   DwarfRegNum<[0]>;
  def R1     : AlexGPReg<1,  "r1">,     DwarfRegNum<[1]>;
  def R2     : AlexGPReg<2,  "r2">,     DwarfRegNum<[2]>;
  def R3     : AlexGPReg<3,  "r3">,     DwarfRegNum<[3]>;
  def RA     : AlexGPReg<4,  "ra">,     DwarfRegNum<[4]>;
  def FP     : AlexGPReg<5,  "fp">,     DwarfRegNum<[5]>;
  def SP     : AlexGPReg<6,  "sp">,     DwarfRegNum<[6]>;
  def PC     : AlexGPReg<7,  "pc">,     DwarfRegNum<[7]>;
}
def Int32Regs   : RegisterClass<"Alex", [i32], 32, (add R0, R1, R2, R3, RA, FP, SP, PC)>;