##### 概述

redis的AOF文件本质是1个按照RESP协议组织的文本文件，直接打开有一定的辩认度，但是其充斥着RESP协议的符号以及大量的CRLF字符，影响其可读性。
注意，redis从4.0开始引入了aof和rdb混合持久化，即aof文件中存放rdb的内容，由于rdb信息使用二进制编码，没有解析处理的必要，这里如果检测到是
aof和rdb混合版的aof，则无法解析而提示报错，即本程序只能解析单纯的aof。混合持久化是可以被关闭的，具体可以针对redis.conf进行设置。

##### 上手

1. git clone下载本仓库到本地

   ```
   git clone https://github.com/guoxiangCN/AofParser.git
   ```

   

2. 直接shell中执行命令'make'

   ```shell
   make
   ```

   

3. 运行

   ```shell
   ./aofparser.app your/path/to/aoffile.aof
   ```



##### 截图
![image](https://github.com/guoxiangCN/AofParser/blob/main/example.jpg)
