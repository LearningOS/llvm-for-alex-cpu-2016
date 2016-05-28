# LLVM Alex-Machine后端自下而上的分析

- 什么叫自下而上的分析?
    - 详细介绍各个类的功能, 以及他们如何相互依赖, 如何进行一起完成功能.
    - 下面按照执行顺序介绍各个类
    
---
1. AlexTargetMachine/AlexSubtarget
    - 我们的LLVM后端主要是实现一些callbacks,
        在LLVM对DAG进行一趟遍历的时候, 我们的callbacks在某一趟扫描被调用, 
        对DAG进行某些修改, 就对LLVM IR进行了翻译.
    - AlexTargetMachine构造函数是整个Alex-Machine后端的入口点, 
        这个类负责创建AlexSubtarget. 
    - AlexSubtarget创建了下面几个类. XXXSubtarget的用处在于, 
        如果一个平台有多种类似的cpu, 比如80386, 80486等等, 
        可以继承XXXSubtarget这个类, 从而共用cpu之间相同的部分.
    - 在AlexTargetMachine.cpp中
        ```
        extern "C" void LLVMInitializeAlexTarget() {
                 RegisterTargetMachine<AlexTargetMachine> x(TheAlexTarget);
        }
        ```
        这个函数注册AlexTargetMachine
    
1. AlexISelLowering
    - 这个类对函数调用, 返回, 获取一个符号(全局变量, 外部符号, 跳转表等方式)地址, 等等高层概念继续细化.
    - 构造函数.
    这里可以调用setOperationAction(), 对不希望生成的LLVM IR指令进行展开, 或者自定义等.
    例如`setOperationAction(ISD::SELECT_CC, MVT::i32,   Expand);`, 对select_cc(类似与c语言中的a?b:c表达式), 进行展开,
    它会被展开成brcc(类似于if-then-else). 但是不是所有指令都可以展开, 对于load/store这种基本的指令, 无法展开.
    而且如果这里成环的话(a展成b, b展成a)会导致编译时死循环.
    
    - 还有一种setOperationAction的使用方法`setOperationAction(ISD::GlobalAddress, MVT::i32,   Custom);`
    这样该趟遍历的时候会调用AlexTargetLowering::LowerOperation(), 用户可以override该函数, 进行自定义操作.
    例如, 我们利用该函数, 把所有的GlobalAddress, BlockAddress, Jumptable node展开成了Load Low和Load High两个Node, 
    这是因为Alex-Machine是RISC, 对32bit的操作数的加载必须分成两次.
    
    - AlexTargetLowering::LowerFormalArguments(), 我们将函数形参拷贝到当前栈帧, 这样访问参数和局部变量就都一样了.
    - AlexTargetLowering::LowerReturn()翻译return指令, 这里我们将返回值保存到T0寄存器, 生成Ret node, 之后会被lower成ret MI
    - AlexTargetLowering::LowerCall()
    该函数完成实参压栈, 跳转到指定地址, 恢复堆栈.
    具体来说, 根据c调用约定, 调用者恢复堆栈. 
    CALLSEQ_START, CALLSEQ_END这两种node用于构造参数栈, 恢复参数栈.
    getOpndList()函数会生成一系列load/loadext指令加载参数到堆栈的node
    LowerCallResult()函数生成从S0取回返回值的node
    
1. AlexISelDAGToDAG
    - 这个类负责LLVM IR DAG -> Alex machine instruction DAG
    
    - AlexDAGToDAGISel::Select()调用了AlexDAGToDAGISel::SelectCode(), 后者为tblgen根据AlexInstructionInfo.td生成
    这里进行DAG模式匹配和翻译, 这里过后, 我们得到了完全是Alex Machine Instruction(LLVM IR被"lower"掉了)的DAG.
    
    - 在AlexDAGToDAGISel::Select()中, 可以添加代码从而处理一些DAGToDAG模式匹配的特例

    - 关于tblgen的模式匹配, 翻译, 请详见[LLVM tblgen简介](llvm-tblgen.md)
    
1. AlexFrameLowering
    - 这个类提供了一个机会可以在LLVM IR中的函数前后插入代码.
    
    - AlexFrameLowering::emitPrologue(), AlexFrameLowering::emitEpilogue(),
    这两个函数负责生成函数被调用函数的栈帧创建, 销毁. 举个例子, 在x86下, 
    emitPrologue()就是生成设置ebp=esp, 然后sub esp, xxx, xxx为局部变量的总大小,
    emitEpilogue()就是生成add esp, xxx, xxx和上面一样.
    要注意, 我们此时还是在操作LLVM IR DAG, 还没到Alex Machine Code.
    
    - AlexFrameLowering::determineCalleeSaves()
    该函数计算被调用者需要保存的寄存器
    
    - AlexFrameLowering::spillCalleeSavedRegisters()
    这里生成具体的Store/Load node(我们称LLVM IR DAG的节点为node, Alex machine instruction为MI), 
    保存被调用者需要保存的寄存器.
    
1. AlexRegisterInfo
    - 这个类提供寄存器相关的信息
    - eliminateFrameIndex()
    将MI中的frameindex dag node变成寄存器+偏移量这样的操作数
    由于我们有FP寄存器, 而且恰好load/save指令都是第二参数是寄存器, 第三个参数是立即数,
    我们把MI对象中的fi operand删掉, 添加两个operand: $fp, -fi*4
    
1. AlexInstrInfo
    - 这个类用于展开Alex MI伪指令, 生成符号信息给链接器用于地址回填
    - expandPostRAPseudo()
    这里展开了内部的一些伪指令, 比如符号扩展load, li32等, cpu没有提供这方面的指令, 
    所以符号扩展是用左移右移实现的(可能有非atomic指令的问题).
    之前所有的BlockAddress, GlobalAddress等都会生成LI32伪指令, 
    LI32会生成li , lih, 操作数为0, 并且对这两条指令标记为ALEX_LO, ALEX_HI, 供链接器回填地址.
    - storeRegToStackSlot/loadRegFromStackSlot
    这两个函数展开对堆栈上地址访存的指令, 在alex machine下, 则是简单的load/store
    - copyPhysReg
    展开$a=$b这样的伪指令, 我们实现为addi $a, $b, 0
