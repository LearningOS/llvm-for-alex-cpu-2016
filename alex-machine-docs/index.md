# LLVM Alex-Machine后端文档

## 1. Build
- 详细的build方法请见官网[Building LLVM with CMake](http://llvm.org/docs/CMake.html)
    下面提供在Ubuntu16.04-amd64上测试过的简单build方法
##### 注意在Mac OS下clang生成的LLVM IR格式和linux下不同, 故暂时不支持Mac OS    

##### 方案1
- 依赖库
    - llvm-dev 3.7+
    - cmake 2.8.12.2+
    - clang 3.7+
- LLVM不支持在源码目录下build
    ```
    $ git clone https://github.com/a1exwang/llvm.git llvm
    $ cd llvm/tools && git clone https://github.com/a1exwang/lld.git
    $ cd ../..
    $ mkdir llvm-build
    $ cd llvm-build
    $ cmake "GNU Makefiles" ../llvm
    $ make
    ```
- 之后在llvm-build/bin会得到编译器, 链接器等等工具的二进制文件

##### 方案2
- 自动build的脚本
    ```
    $ mkdir alex-machine-build
    $ git clone https://github.com/a1exwang/alex-cpu-test.git alex-machine-build/alex-cpu-test
    $ cd alex-machine-build/alex-cpu-test
    $ ./setup
    ```
- 这样会自动下载依赖库, 编译llvm并且执行测试

### 2. C/C++源码的编译
- 假设LLVM_PATH为LLVM build完成后得到的bin文件夹的路径
1. C/C++源码编译到LLVM IR
    ```
    $ clang -emit-llvm -c xxx.c -o xxx.c.bc
    ```
2. LLVM IR到Alex-Machine汇编指令或者目标文件
    ```
    $ LLVM_PATH/llc -march=alex -filetype=asm -o xxx.c.ll xxx.c.bc # 汇编
    $ LLVM_PATH/llc -march=alex -filetype=obj -o xxx.c.o xxx.c.bc  # 目标文件
    ```
3. 链接
    ```
    # 生成xxx, elf可执行文件
    $ LLVM_PATH/lld -flavor gnu -o xxx xxx1.c.o xxx2.c.o ...
    ```
    
### 3. 调试符号的生成
- 如果需要生成调试符号, 需要在clang编译选项中添加-g
- 假设a.out是步骤2中编译生成的elf文件
    ```
    # 需要安装ruby 1.9+
    # 生成Nodejs版本模拟器使用的调试符号
    $ ./alex2v9 a.out outfile
    # 生成浏览器版本模拟器使用的调试符号
    $ ./dwarf2v9 a.out outfile
    ```
    
### 4. 其实还有一个自动编译当前目录下所有c/cpp文件的脚本, 是alex-cpu-test项目的build文件, 仅供测试, 使用方法参见./build --help

## 2. 设计文档
- 1. [LLVM总体介绍](llvm-intro.md)
- 2. [自上而下的介绍](llvm-overview.md)
- 3. [自下而上的介绍](llvm-details.md)
- 4. [杂项](llvm-misc.md)
- 5. [LLVM tblgen简介](llvm-tblgen.md)
- 6. [LLVM IR简介](llvm-ir.md)
- 7. [FAQ](llvm-faq.md)
- 参考[Cpu0 LLVM后端](http://jonathan2251.github.io/lbd/index.html)