
def CC_Alex : CallingConv<[
  // 每个参数4bytes, 按8bytes对齐, 用堆栈传递参数
  CCAssignToStack<8, 8>
]>;

def RetCC_Alex : CallingConv<[
  //CCIfType<[i32], CCAssignToReg<[T0]>>
  CCIfType<[i32], CCAssignToReg<[S0]>>
]>;

def CSR_CalleeSavedRegs : CalleeSavedRegs<(add S1, T0)>;
//def CSR_CallerSavedRegs : CallerSavedRegs<(add T0, T1, T2)>;
