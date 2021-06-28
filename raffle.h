#ifndef SERVER_raffle_H_
#define SERVER_raffle_H_

#include <iostream>
#include <string>
#include <set>
#include <unordered_map>
#include <vector>

#include "conn_pool.h"
#include "redis.h"

struct info_mem
{
    std::string id;
    int department;
    int ticketnum;
};

class raffle
{
public:
    raffle(conn_pool<Redis> &dbpool, const std::string filename, int ticket_number, int serverid);
    ~raffle(){};

    bool ismember(const std::string id);
    int draw_ticket(const std::string id);
    int draw_ticket_(const std::string id);
    void finish();
    void webbench_test();
    int server_id_;

private:
    void init_database(const std::string filename);
    conn_pool<Redis> &db_pool_;
    std::vector<int> department_staffnum_;
    std::unordered_map<std::string, info_mem> staff_info; //id-info
    int staff_num_;
    int department_num_;
    int ticket_num_;
};

#endif