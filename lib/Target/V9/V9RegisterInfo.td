
//===----------------------------------------------------------------------===//
//  Declarations that describe the V9 register file
//===----------------------------------------------------------------------===//

class V9Reg<bits<32> Enc, string n> : Register<n> {
  let HWEncoding = Enc;
  let Namespace = "V9";
}

// Cpu0 CPU Registers
class V9GPRReg<bits<32> Enc, string n> : V9Reg<Enc, n>;

//===----------------------------------------------------------------------===//
//@Registers
//===----------------------------------------------------------------------===//
// The register string, such as "a" or "b" will show on "llvm-objdump -d"
//@ All registers definition
let Namespace = "V9" in {
  //@ General Purpose Registers
  def A     : V9GPRReg<0,  "a">,    DwarfRegNum<[0]>;
  def B     : V9GPRReg<1,  "b">,    DwarfRegNum<[1]>;
  def C     : V9GPRReg<2,  "c">,    DwarfRegNum<[2]>;
  def SP    : V9GPRReg<3, "sp">,    DwarfRegNum<[13]>;

  def PC    : V9C0Reg<0, "pc">,     DwarfRegNum<[20]>;
  def SP    : V9C0Reg<1, "sp">,     DwarfRegNum<[21]>;
}

//===----------------------------------------------------------------------===//
//@Register Classes
//===----------------------------------------------------------------------===//

def CPURegs : RegisterClass<"V9", [i32], 32, (add
  // Reserved
  // Return Values and Arguments
  B, C,
  // Not preserved across procedure calls
  A,
  // Callee save
  // Reserved
  SP)>;

//@Status Registers class
def SR     : RegisterClass<"V9", [i32], 32, (add FLAGS)>;

//@Co-processor 0 Registers class
def C0Regs : RegisterClass<"V9", [i32], 32, (add PC)>;