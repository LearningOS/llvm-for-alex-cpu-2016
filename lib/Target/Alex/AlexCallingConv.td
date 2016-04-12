
def CC_Alex : CallingConv<[
  // 每个参数4bytes, 按4bytes对齐, 用堆栈传递参数
  CCAssignToStack<4, 4>
]>;

def RetCC_Alex : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R1, R2, R3]>>
]>;

def CSR_CalleeSavedRegs : CalleeSavedRegs<(add R1, R2, R3)>;