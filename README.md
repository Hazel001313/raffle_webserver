

## 概述

- 使用 线程池 + 非阻塞socket + epoll + 事件处理的并发模型  
- 支持解析GET HTTP请求报文
- 访问服务器redis数据库实现web端用户查询、抽奖、结束抽奖、下载中奖名单功能

![raffle_webserver/structure.png at main · Hazel001313/raffle_webserver (github.com)](https://github.com/Hazel001313/raffle_webserver/blob/main/image/structure.png)



## 运行说明

| 配置项 | 说明                                          |
| ------ | --------------------------------------------- |
| p      | 监听端口，默认为7777                          |
| r      | redis IP地址，默认为127.0.0.1                 |
| t      | redis 端口地址，默认为6379                    |
| i      | server id，默认为2（id为1的server预注册信息） |
| n      | 奖票数量，默认为40000                         |

eg ./raffle_server -p7777 -r127.0.0.1 -t6379 -i1 -n40000

## 接口

抽奖：curl -G -d 'id=11111' http://127.0.0.1:7777/  

结束抽奖: curl -G -d 'passwd=123' http://127.0.0.1:7777/fin  

下载文件 http://127.0.0.1:7777/winnerlist.html  

### demo  

抽中：

![](https://github.com/Hazel001313/raffle_webserver/blob/main/image/id11111.png)

未抽中：

![](https://github.com/Hazel001313/raffle_webserver/blob/main/image/id11112.png)

无效ID：

![](https://github.com/Hazel001313/raffle_webserver/blob/main/image/id111.png)

结束抽奖-正确密码：

![](https://github.com/Hazel001313/raffle_webserver/blob/main/image/fin123.png)

结束抽奖-错误密码：

![](https://github.com/Hazel001313/raffle_webserver/blob/main/image/fin12345.png)

获取结束抽奖后生成的中奖名单：

![](https://github.com/Hazel001313/raffle_webserver/blob/main/image/getwinnerlist.png)



![raffle_webserver/function.png at main · Hazel001313/raffle_webserver (github.com)](https://github.com/Hazel001313/raffle_webserver/blob/main/image/function.png)



## 压力测试

![raffle_webserver/wrk.png at main · Hazel001313/raffle_webserver (github.com)](https://github.com/Hazel001313/raffle_webserver/blob/main/image/wrk.png)



## 更新日志

实现线程池，redis连接与工作线程解耦  

抽奖模块实现“部门间平均、部门内随机”  

实现管理redis连接的RAII类conn_guard<REDIS>;  

实现router类，管理url路径与处理函数handler之间的路由；  

将添加route的代码与server框架代码分离，业务逻辑与框架分离；  

增加获取文件的功能（需多次等待EPOLLOUT事件和write）；









