# C/C++源码的编译

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
