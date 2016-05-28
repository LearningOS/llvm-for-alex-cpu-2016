# 构建说明

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

