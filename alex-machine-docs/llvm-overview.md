# LLVM Alex-Machine后端自上而下介绍

- 什么是自上而下的介绍
    - 从宏观上来看, 在llvm中一个编译器需要的功能是在哪一个模块, 如何实现的.
    - 再介绍这些模块依赖与那些模块, 他们如何协同工作.
    
---

1. 总体上说, 后端做了什么?
    - 解析一个LLVM IR bitcode文件, 生成汇编代码文件或者目标文件
    
1. LLVM IR是如何变成汇编文件的
    - 多趟遍历IR  
    
1. LLVM IR是如何表示的?
    - SSA: static single assignment, 静态单赋值形式, llvm中表示为以下形式
        ```
        %vreg1 = CopyFromReg %a
        %vreg2 = CopyFromReg %b
        %vreg3 = add %a, %b
        %vreg4 = CopyFromReg %c
        %vreg5 = sub %vreg3, %vreg4
        ```
    - DAG: directed acyclic graph, 有向无环图, 在llvm中表示为以下形式
        ```
        (set %vreg1, (sub (add %a, %b), %c))
        ```
    - 在clang生成的文件中是SSA
    
1. 有几趟遍历? 分别是什么?
    - llc -debug-pass=Structure可以显示所有趟
    - ![passes](assets/passes.png)
    - 1. LLVM IR SSA -> LLVM IR DAG
    - 1. LLVM IR DAG -> Alex DAG
    - 1. Alex DAG -> Alex SSA
    - 1. Alex SSA Lowering
    - 1. Alex SSA -> Asm text
    - 其实还有很多趟遍历, 包括寄存器分配, peephole optimization等等, 是llvm默认进行的, 我们没有继承, 修改它.

1. 为什么要LLVM IR SSA -> LLVM IR DAG? 都做了了什么? 
    - 原因: LLVM IR bitcode文件中是以SSA形式保存的, 转化成DAG方便优化和翻译(见tblgen部分).
    同时LLVM IR是比较高级的表示形式, 该指令集为了在实际中运行效率, 跨平台更容易等等的考虑, 做的比较冗余.
    我们需要去掉其中一些指令, 以简化后面的翻译流程.
    - 做了什么: 
        1. 将DAG中比较高层的指令展开.
            - 有在llvm内部实现的: select -> branch,
            - 还有Alex平台相关的实现: 
                - globaladdress -> load imm
                - return -> $t0 =  target; ret;
                - %return_val = call func, %a, %b -> 
                    ```
                    CALLSEQ_START
                    %vreg1 = CopyFromReg %a
                    %vreg2 = CopyFromReg %b
                    ...
                    call func
                    %return_val = CopyFrom Reg %t0
                    CALLSEQ_END
                    ```
        2. SSA -> DAG llvm内部完成
        
1. 为什么LLVM IR DAG -> Alex DAG? 具体做了什么?
    - 为什么: 显然, 后端的主要功能就是LLVM IR翻译为Alex机器码/汇编代码, 
        然而在DAG这一层面上进行转换主要是为了方便匹配(见tblgen部分)
        
    - 将LLVM IR DAG转换为Alex DAG, 注意
        这一趟过后, DAG的拓扑结构基本没变, 只是每一个node都成了平台相关的了.
        比如, (add (sub $vreg1, $vreg2), i32 1) -> (ADDi (SUB $vreg1, $vreg2), i32 1)
        
1. 为什么Alex DAG -> Alex SSA? 具体做了什么?
    - 原因:对DAG进行完优化之后, 转换成SSA再进行寄存器分配和优化, 并且SSA更容易转换成asm/机器码
    - llvm实现
    
1. Alex SSA遍历
    1. 展开CALLSEQ_START, CALLSEQ_END, 这些伪指令;
    1. 插入Prologue, Epilogue;
    1. 将FrameIndex变成具体的访存操作(load/store)
    1. 展开%a = %b这样的指令(load/store)
    1. 分配物理寄存器
    1. 插入寄存器保存/恢复指令, caller/callee saved

1. Alex SSA -> Asm text
    - 使用InstPrinter打印出SSA表示的汇编指令
    
1. llvm tblgen介绍
    参见[llvm-tblgen](llvm-tblgen.md)