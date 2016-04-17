
def CC_Alex : CallingConv<[
  // 每个参数4bytes, 按4bytes对齐, 用堆栈传递参数
  CCAssignToStack<4, 8>
]>;

def RetCC_Alex : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[T0, T1]>>
]>;

def CSR_CalleeSavedRegs : CalleeSavedRegs<(add S0, S1)>;