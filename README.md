

## 概述

- 使用 线程池 + 非阻塞socket + epoll + 事件处理的并发模型  
- 支持解析GET HTTP请求报文
- 访问服务器redis数据库实现web端用户查询、抽奖、结束抽奖、下载中奖名单功能

![raffle_webserver/structure.png at main · Hazel001313/raffle_webserver (github.com)](https://github.com/Hazel001313/raffle_webserver/blob/main/image/structure.png)



## 运行

./raffle_server 7777（端口号） 40000（门票数） 2（server编号）  



## 接口

抽奖：curl -G -d 'id=11111' http://127.0.0.1:7777/  

结束抽奖: curl -G -d 'passwd=123' http://127.0.0.1:7777/fin  

下载文件 http://127.0.0.1:7777/winnerlist.html  

### demo  

![raffle_webserver/function.png at main · Hazel001313/raffle_webserver (github.com)](https://github.com/Hazel001313/raffle_webserver/blob/main/image/function.png)



## 压力测试

![raffle_webserver/wrk.png at main · Hazel001313/raffle_webserver (github.com)](https://github.com/Hazel001313/raffle_webserver/blob/main/image/wrk.png)









