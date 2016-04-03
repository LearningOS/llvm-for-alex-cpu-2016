def CC_V9Cpu : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R0, R1, R2, R3, R4, R5]>>,
  //CCIfType<[f32, f64], CCCustom<"CC_V9_Assign_Split_64">>,
  // 每个参数4bytes, 按8bytes对齐
  CCAssignToStack<4, 8>
]>;

def RetCC_V9Cpu : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R0]>>,
  CCIfType<[f32, f64], CCAssignToReg<[F]>>
]>;

def CSR_CalleeSavedRegs : CalleeSavedRegs<(add R0, R1, R2, R3, FP)>;