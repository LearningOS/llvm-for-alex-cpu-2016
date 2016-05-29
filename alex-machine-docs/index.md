# LLVM Alex-Machine后端文档

## 目录

---

### 1. 使用说明
1. [如何构建编译器](build.md)
1. [如何使用编译器](usage.md)

### 2. 设计文档
1. [LLVM总体介绍](llvm-intro.md)
1. [LLVM IR简介](llvm-ir.md)
1. [自上而下的介绍](llvm-overview.md)
1. [自下而上的介绍](llvm-details.md)
1. [LLVM tblgen简介](llvm-tblgen.md)
1. [LLD连接器介绍](lld.md)
1. [FAQs](faqs.md)
1. [一些坑](yixiekeng.md)

- 参考
    - [Cpu0 LLVM后端](http://jonathan2251.github.io/lbd/index.html)
    - [LLVM 指令集文档](http://llvm.org/docs/LangRef.html)
    - [LLVM tblgen文档](http://llvm.org/docs/TableGen/index.html)
    - [LLVM codegen](http://llvm.org/docs/CodeGenerator.html)
    - [LLVM MC Layer](http://llvm.org/docs/CodeGenerator.html#mc-layer)
    
### 3. 相关项目
1. [alex-machine模拟器 - 用nodejs实现版本](https://github.com/a1exwang/alex-machine)
1. [alex-cpu-test - 该编译器的测试用例/自动化测试脚本](https://github.com/a1exwang/alex-cpu-test)
1. [alex2v9 - 将该编译器生成的elf文件转化为v9可执行文件和调试文件生成](https://github.com/a1exwang/alex2v9)