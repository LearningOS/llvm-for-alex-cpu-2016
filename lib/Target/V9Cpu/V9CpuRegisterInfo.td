class V9CpuReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "V9Cpu";
}

class V9GPReg<bits<16> Enc, string n> : V9CpuReg<Enc, n>;

//let Namespace = "V9Cpu" in {
  //@ General Purpose Registers
  def A     : V9GPReg<0,  "a">,     DwarfRegNum<[0]>;
//  def B     : V9GPReg<1,  "b">,     DwarfRegNum<[1]>;
//  def C     : V9GPReg<2,  "c">,     DwarfRegNum<[2]>;
//  def F     : V9GPReg<8,  "f">,     DwarfRegNum<[8]>;
//  def G     : V9GPReg<9,  "g">,     DwarfRegNum<[9]>;
//  def FP    : V9GPReg<10, "fp">,    DwarfRegNum<[10]>;
//  def TSP   : V9GPReg<12,  "tpc">,   DwarfRegNum<[12]>;
//  def PC    : V9GPReg<13,  "pc">,    DwarfRegNum<[13]>;
//  def SP    : V9GPReg<14,  "sp">,    DwarfRegNum<[14]>;
//  def FLAGS : V9GPReg<15,  "flags">,  DwarfRegNum<[15]>;
//}
//def Int32Regs   : RegisterClass<"V9Cpu", [i32], 32, (add A, B, C, PC, SP, TSP, FLAGS, FP)>;
//def IntRegs     : RegisterClass<"V9Cpu", [i32], 32, (add A, B, C)>;
def ARegs       : RegisterClass<"V9Cpu", [i32], 32, (add A)>;
//def FloatRegs   : RegisterClass<"V9Cpu", [f64], 64, (add F, G)>;
//def ControlRegs : RegisterClass<"V9Cpu", [i32], 32, (add PC, SP, TSP, FLAGS)>;