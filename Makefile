raffle_server: main.cpp http_conn.cpp redis.cpp
	g++ main.cpp http_conn.cpp redis.cpp raffle.cpp server.cpp /usr/local/lib/libhiredis.a -o raffle_server -pthread -std=c++11 