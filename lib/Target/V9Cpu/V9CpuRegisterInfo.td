class V9CpuReg<bits<16> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "V9Cpu";
}

class V9GPReg<bits<16> Enc, string n> : V9CpuReg<Enc, n>;

let Namespace = "V9Cpu" in {
  //@ General Purpose Registers
  def R0     : V9GPReg<0,  "r0">,     DwarfRegNum<[0]>;
  def R1     : V9GPReg<1,  "r1">,     DwarfRegNum<[1]>;
  def R2     : V9GPReg<2,  "r2">,     DwarfRegNum<[2]>;
  def R3     : V9GPReg<3,  "r3">,     DwarfRegNum<[3]>;
  def R4     : V9GPReg<4,  "r4">,     DwarfRegNum<[4]>;
  def R5     : V9GPReg<5,  "r5">,     DwarfRegNum<[5]>;
  def R6     : V9GPReg<6,  "r6">,     DwarfRegNum<[6]>;
  def R7     : V9GPReg<7,  "r7">,     DwarfRegNum<[7]>;
  def F     : V9GPReg<8,  "f">,     DwarfRegNum<[8]>;
  def G     : V9GPReg<9,  "g">,     DwarfRegNum<[9]>;
  def FP    : V9GPReg<10, "fp">,    DwarfRegNum<[10]>;
  def TSP   : V9GPReg<12,  "tpc">,   DwarfRegNum<[12]>;
  def PC    : V9GPReg<13,  "pc">,    DwarfRegNum<[13]>;
  def SP    : V9GPReg<14,  "sp">,    DwarfRegNum<[14]>;
  def FLAGS : V9GPReg<15,  "flags">,  DwarfRegNum<[15]>;
}
def Int32Regs   : RegisterClass<"V9Cpu", [i32], 32, (add R0, R1, R2, R3, R4, R5, R6, R7)>;
def IntRegs     : RegisterClass<"V9Cpu", [i32], 32, (add R0, R1, R2, R3)>;
def ARegs       : RegisterClass<"V9Cpu", [i32], 32, (add R0, R1)>;
def FloatRegs   : RegisterClass<"V9Cpu", [f64], 64, (add F, G)>;
def ControlRegs : RegisterClass<"V9Cpu", [i32], 32, (add PC, SP, TSP, FLAGS)>;
