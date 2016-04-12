
def CC_Alex : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R1, R2, R3]>>,
  // 每个参数4bytes, 按8bytes对齐
  CCAssignToStack<4, 8>
]>;

def RetCC_Alex : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R1, R2, R3]>>
]>;

def CSR_CalleeSavedRegs : CalleeSavedRegs<(add R1, R2, R3, LR, FP)>;