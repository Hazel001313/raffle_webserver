## 运行
./lottery_server 7777（端口号） 40000（门票数） 2（server编号）

## 接口
抽奖：curl -G -d 'id=11111' http://127.0.0.1:7777/
结束抽奖: curl -G -d 'passwd=123' http://127.0.0.1:7777/fin
下载文件 http://127.0.0.1:7777/winnerlist.html