##### 概述

redis的AOF文件本质是1个按照RESP协议组织的文本文件，直接打开有一定的辩认度，但是其充斥着RESP协议的符号以及大量的CRLF字符，影响人的可读性。

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
![https://github.com/guoxiangCN/AofParser/blob/main/example.PNG]
