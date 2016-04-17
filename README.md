# LLVM V9Cpu Backend

### TODOs
- 近期目标
    - 编译器
        - 变长参数
        - ~~结构体作为函数参数~~
        - ~~结构体作为函数返回值~~
        - ~~函数指针~~
        - ~~取地址~~
        - ~~32位常量Load~~
        - \*\*switch/case
        - \*\*浮点数
    - 编译器已知bug
        - 调用函数没有保存所有called saved regsiters,
            而且保存的寄存器有不必要的
        - 调用非函数指针函数时, 偏移不能超过16位
    - 编译器目标代码生成模块
        - 检查每条指令的二进制代码是否正确
        - opcode > 7位的指令暂时生成错误

- 长期目标
    - 汇编器/内联汇编
    - 连接器
        - ~~全局变量~~
        - 函数指针
        - 局部跳转
        - DWARF调试信息生成(源码级调试)

