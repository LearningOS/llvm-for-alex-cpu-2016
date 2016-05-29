# 一些坑

1. 在AlexTargetLowering构造函数中可以用setOperationAction从而展开某条指令
    - 如果循环展开, 编译时会死循环
    - 例如`setOperationAction(ISD::SELECT_CC, MVT::i32,   Expand);`, 
    对select_cc(类似与c语言中的a?b:c表达式), 进行展开,
    它会被展开成brcc(类似于if-then-else). 但是不是所有指令都可以展开, 对于load/store这种基本的指令, 无法展开.
    而且如果这里成环的话(a展成b, b展成a)会导致编译时死循环.
    
2. setOperationAction中如果要展开跳转指令, 指令类型一定要设置为MVT::iOther, 整数类型是不行的.