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
        
1. AlexFrameLowering
    - LLVM IR中有函数的概念, 
    这个类提供了一个机会可以在LLVM IR中的函数前后插入DAG node.
    
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
    
1. AlexISelLowering
    - 这个类对函数调用, 返回, 获取一个符号(全局变量, 外部符号, 跳转表等方式)地址, 等等高层概念继续细化.
    - 构造函数.
    这里可以调用setOperationAction(), 对不希望生成的LLVM IR指令进行展开, 或者自定义等.
    例如`setOperationAction(ISD::SELECT_CC, MVT::i32,   Expand);`, 对select_cc(类似与c语言中的a?b:c表达式), 进行展开,
    它会被展开成brcc(类似于if-then-else). 但是不是所有指令都可以展开, 对于load/store这种基本的指令, 无法展开.
    而且如果这里成环的话(a展成b, b展成a)会导致编译时死循环.

1. AlexISelDAGToDAG
    - 这个类负责LLVM IR DAG -> Alex machine instruction DAG
    
    - AlexDAGToDAGISel::Select()调用了AlexDAGToDAGISel::SelectCode(), 后者为tblgen根据AlexInstructionInfo.td生成
    这里进行DAG模式匹配和翻译, 这里过后, 我们得到了完全是Alex Machine Instruction(LLVM IR被"lower"掉了)的DAG.
    
    - 在AlexDAGToDAGISel::Select()中, 可以添加代码从而处理一些DAGToDAG模式匹配的特例

    - 关于tblgen的模式匹配, 翻译, 请详见[LLVM tblgen简介](llvm-tblgen.md)
    
1. AlexInstrInfo
    - 这个类


5. AlexRegisterInfo


6. DAG和MI