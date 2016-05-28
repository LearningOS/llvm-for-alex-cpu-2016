# LLVM tblgen简介

LLVM tblgen是LLVM定义的用于描述目标平台架构信息的DSL.
tblgen不规定语义, 只规定语法, 具体的语义由具体的tblgen后端给出.
LLVM使用几种不同的tblgen后端作为代码生成器(这里是指生成编译器的C++代码, 不是编译器生成目标平台的代码),
生成编译器代码, 从而简化domain-specific的程序设计.

1. tblgen的语法

2. tblgen描述寄存器

3. tblgen描述指令集

4. tblgen描述调用约定
