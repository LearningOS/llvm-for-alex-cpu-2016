# LLVM tblgen简介

LLVM tblgen是LLVM定义的用于描述目标平台架构信息的DSL.
tblgen不规定语义, 只规定语法, 具体的语义由具体的tblgen后端给出.
LLVM使用几种不同的tblgen后端作为代码生成器(这里是指生成编译器的C++代码, 不是编译器生成目标平台的代码),
生成编译器代码, 从而简化domain-specific的程序设计. [LLVM tblgen Introduction](http://llvm.org/docs/TableGen/index.html) 

1. tblgen的语法简介, [LLVM tblgen Language Intro](http://llvm.org/docs/TableGen/LangIntro.html)
    - 类似C++模板
    - 类型系统, 基本类型有整数, 字符串, 列表, DAG, bit, bit序列
    - 内置宏!strcat等等.
    - 包含两部分, '类(class)'和'定义(definition)', 他们都被成为'记录(record)'
    - '定义'包含两部分, 名字, 基类. 用关键字def定义, 没有未定义量.
    - '类'包含三部分, 名字, 参数, 基类. 用关键字class定义, 除了参数外均为已定义量.
    - 例子
        ```
        class InstrRI<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern = [],
                 InstrItinClass itin = NoItinerary>: AlexInst<outs, ins, asmstr, pattern, itin>
        {
          bits<4>  ra;
          bits<16> imm16;
        
          let Opcode = op;
        
          let Inst{23-20} = ra;
          let Inst{19-16} = 0;
          let Inst{15-0}  = imm16;
        }
        
        def LI      : InstrRI<0x31, (outs Int32Regs:$ra), (ins simm16:$imm16),
                "li\t$ra, $imm16",
                [(set Int32Regs:$ra, (i32 immSExt16:$imm16))], NoItinerary>;
        ```
        定义了class InstrRI和definition LI.

2. tblgen描述寄存器
    - 例子
        ```
        def R0     : AlexAllReg<0,   "r0">,     DwarfRegNum<[0]>;
        def T0     : AlexAllReg<1,   "t0">,     DwarfRegNum<[1]>;
        def T1     : AlexAllReg<2,   "t1">,     DwarfRegNum<[2]>;
        def T2     : AlexAllReg<3,   "t2">,     DwarfRegNum<[3]>;
        def T3     : AlexAllReg<4,   "t3">,     DwarfRegNum<[4]>;
        ...
        def Int32Regs   : RegisterClass<"Alex", [i32], 32, (add R0, T0, T1, T2, T3, T4, S0, S1, S2, S3, S4, FP, SP)>;
        ```
3. tblgen描述指令集
    - 例子
        ```
        class AlexInst<dag outs, dag ins, string asmstr, list<dag> pattern,
                       InstrItinClass itin>: Instruction
        {
          // 生成的机器码即为Inst
          field bits<32> Inst;
          let Size = 4;
          bits<8> Opcode = 0;
          let Inst{31-24} = Opcode;
        
          // 输出参数和输入参数, 供寄存器分配, 优化等步骤使用
          let OutOperandList = outs;
          let InOperandList  = ins;
          
          // 给汇编器使用
          let AsmString   = asmstr;
          // 给DAGToDAG使用, 用来模式匹配翻译LLVM IR DAG
          let Pattern     = pattern;
          ...
        }
        class InstrRI<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern = [],
                 InstrItinClass itin = NoItinerary>: AlexInst<outs, ins, asmstr, pattern, itin>
        {
          bits<4>  ra;
          bits<16> imm16;
        
          let Opcode = op;
        
          let Inst{23-20} = ra;
          let Inst{19-16} = 0;
          let Inst{15-0}  = imm16;
        }
        
        // LI指令是加载16bit立即数到寄存器
        // e.g. LI $ra, 0xffff
        // 以下定义的例子是说, opcode = 0x31, 
        // (outs Int32Regs:$ra), 有一个输出寄存器$ra(未定)
        // (ins simm16:$imm16), 有一个输入参数是16bit有符号立即数
        // asmstr是"li\t$ra, $imm16", 其中$ra, $imm16是引用前面的变量
        // [(set Int32Regs:$ra, (i32 immSExt16:$imm16))], 这部分是说匹配
        // 这样的LLVM IR DAG node, 将这样的node替换成LI node
        def LI      : InstrRI<0x31, (outs Int32Regs:$ra), (ins simm16:$imm16),
                    "li\t$ra, $imm16",
                    [(set Int32Regs:$ra, (i32 immSExt16:$imm16))]>;
                    
        // 未命名definition, 仅仅用于模式匹配, 含义和上一个类似
        def : Pat<(brcc SETLT, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
                  (BNEZ (LT Int32Regs:$ra, Int32Regs:$rb), brtarget16:$imm16)>;
        ```


4. tblgen描述函数调用约定
    - 例子
        ```
        def RetCC_Alex : CallingConv<[
          CCIfType<[i32], CCAssignToReg<[T0, T1]>>
        ]>;
        ```
5. 这些tblgen代码如何参与IR pass
    - 通过llvm-tblgen编译器将其翻译成C++代码
    - 同一份tblgen代码使用不同的tblgen后端会分别生成不同部分, enum定义, 类定义, 继承, 代码片段等
    - 例如, 下面是LLVM IR DAG -> Alex DAG的片段, 其中SelectCode是通过tblgen生成的
    ```
    // AlexGenDAGISel.inc
    SDNode *SelectCode(SDNode *N) {
        // Some target values are emitted as 2 bytes, TARGET_VAL handles
        // this.
        #define TARGET_VAL(X) X & 255, unsigned(X) >> 8
        static const unsigned char MatcherTable[] = {
        /*0*/       OPC_SwitchOpcode /*36 cases */, 64|128,1/*192*/, TARGET_VAL(ISD::LOAD),// ->197
        /*5*/         OPC_RecordMemRef,
        /*6*/         OPC_RecordNode, // #0 = 'ld' chained node
        /*7*/         OPC_RecordChild1, // #1 = $addr
        /*8*/         OPC_CheckPredicate, 0, // Predicate_unindexedload
        /*10*/        OPC_CheckType, MVT::i32,
        /*12*/        OPC_Scope, 17, /*->31*/ // 9 children in Scope
        /*14*/          OPC_CheckPredicate, 1, // Predicate_load
        /*16*/          OPC_CheckPredicate, 2, // Predicate_load_a
        /*18*/          OPC_CheckComplexPat, /*CP*/0, /*#*/1, // SelectAddr:$addr #2 #3
        ...
        return SelectCodeCommon(N, MatcherTable,sizeof(MatcherTable));
    }
    
    // lib/CodeGen/SelectionDAG/SelectionDAGISel.cpp
    SDNode *SelectionDAGISel::
    SelectCodeCommon(SDNode *NodeToMatch, const unsigned char *MatcherTable,
                     unsigned TableSize) {
        ...
        // NodeToMatch是当前遍历到的LLVM IR DAG Node,
        // 这里while循环遍历MatcherTable中的每一个Pattern, switch/case尝试匹配pattern
        // 
        while (1) {
            ...
            BuiltinOpcodes Opcode = (BuiltinOpcodes)MatcherTable[MatcherIndex++];
            switch (Opcode) {
            case OPC_Scope: {
            case ...
            case OPC_SwitchOpcode: {
                  unsigned CurNodeOpcode = N.getOpcode();
                  unsigned SwitchStart = MatcherIndex-1; (void)SwitchStart;
                  unsigned CaseSize;
                  while (1) {
                    // Get the size of this case.
                    CaseSize = MatcherTable[MatcherIndex++];
                    if (CaseSize & 128)
                      CaseSize = GetVBR(CaseSize, MatcherTable, MatcherIndex);
                    if (CaseSize == 0) break;
            
                    uint16_t Opc = MatcherTable[MatcherIndex++];
                    Opc |= (unsigned short)MatcherTable[MatcherIndex++] << 8;
            
                    // If the opcode matches, then we will execute this case.
                    if (CurNodeOpcode == Opc)
                      break;
            
                    // Otherwise, skip over this case.
                    MatcherIndex += CaseSize;
                  }
            }
            ...
        }
    ```