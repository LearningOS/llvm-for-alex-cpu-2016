# LLVM IR介绍

LLVM IR是LLVM定义的平台无关的指令中间表示.
有如下特点:
- 基于寄存器(简称vreg), 默认有无穷多个寄存器
- 有类型系统, 每条指令都可以有各种类型的操作数
- 有函数调用, 符号, 编译单元等概念
- 基本单元为BasicBlock, 每个BasicBlock是最多的能顺序执行的指令列表

基于以上几点, Alex Machine后端需要:
- 将vreg的访存翻译成局部变量(对堆栈上地址的load/store)
- 需要计算的时候将局部变量加载到寄存器
- 不同类型之间的转换使用load ext/store指令, cpu不提供的, 用移位指令软件实现.
- 需要将一条函数调用指令展开成参数入栈, 跳转, 恢复堆栈指令, 也需要在被调用者处插入Prologue/Epilogue
- 需要对GlobalAddress, Jumptable, BlockAddress生成符号和标志给链接器回填地址用
