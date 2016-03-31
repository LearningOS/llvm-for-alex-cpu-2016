Low Level Virtual Machine (LLVM)
================================

This directory and its subdirectories contain source code for LLVM,
a toolkit for the construction of highly optimized compilers,
optimizers, and runtime environments.

LLVM is open source software. You may freely distribute it under the terms of
the license agreement found in LICENSE.txt.

Please see the documentation provided in docs/ for further
assistance with LLVM, and in particular docs/GettingStarted.rst for getting
started with LLVM and docs/README.txt for an overview of LLVM's
documentation setup.

If you are writing a package for LLVM, see docs/Packaging.rst for our
suggestions.

## V9CPU的Toolchain on LLVM


- 整体流程
    1. clang编译C到LLVM IR
    2. 翻译LLVM IR到V9汇编码
    3. V9汇编器生成目标代码
    4. 链接
- 工作量
    - MIPS(包括16,32,64位一共16个子架构): 35K行
    - Sparc(包括3个子架构): 10K行
    - Sparc主要代码量:
        - AsmParser(汇编码到机器码的内部表示/支持内联汇编): 1.5K
        - ISelLowering(IR DAG到机器码DAG转换): 3K
        - ELFObjectWriter: 1.5K
        - *.td(llvm-tblgen用的td文件, 是对目标机器的描述): 3.8K

- LLVM IR到V9汇编码的翻译(AsmPrinter)
- assembler/inline assembly(AsmParser)
- 编译成elf文件, 并且可以使用llvm-ld链接, (MCELFObjectWriter)
- \*源码级调试(elf文件中的符号信息?)
- \*链接脚本(暂时不知道llvm-ld是否可以)

参考资料:
    - [LLVM backend官方教程](http://llvm.org/docs/WritingAnLLVMBackend.html)
    - [LLVM IR文档](http://llvm.org/docs/LangRef.html)
    - [V9CPU文档](https://github.com/chyyuu/v9-cpu)
    - [用LLVM给自定义CPU写全套工具链的教程, 包括IR转汇编, 汇编器, 反汇编器, elf目标文件生成, 链接器](http://jonathan2251.github.io/lbd/index.html)
    - [用LLVM给OpenRISC 1000写后端教程](http://www.embecosm.com/appnotes/ean10/ean10-howto-llvmas-1.0.html)
    - [fork了一个llvm](https://github.com/a1exwang/llvm)
    - [LLVM Assembler教程](http://www.embecosm.com/appnotes/ean10/ean10-howto-llvmas-1.0.html#idp112800)