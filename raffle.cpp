#include "raffle.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdio.h>

//constructor of master server
raffle::raffle(conn_pool<Redis> &dbpool, const std::string filename, int ticket_number, int serverid) : db_pool_(dbpool), ticket_num_(ticket_number), staff_num_(0), department_num_(0), server_id_(serverid)
{
    std::shared_ptr<Redis> redis_ptr = db_pool_.get();
    conn_guard<Redis> guard(redis_ptr);
    if (server_id_ == 1)
    {
        redis_ptr->flush();
    }
    init_database(filename);
}

//read stafflist file, generate ticket according to staff number of each department
void raffle::init_database(const std::string filename)
{
    std::ifstream file(filename);
    std::cout << filename << std::endl;
    if (!file)
    {
        throw("Failed to open file.");
    }
    std::shared_ptr<Redis> redis_ptr = db_pool_.get();
    conn_guard<Redis> guard(redis_ptr);

    //register stafflist into redis
    std::string line;
    std::string id, depart;
    std::stringstream strstream;
    department_staffnum_.resize(1000);
    while (getline(file, line))
    {
        //std::cout<<line<<std::endl;
        strstream.str(line);
        getline(strstream, id, ',');
        getline(strstream, depart);
        strstream.clear();
        if (server_id_ == 1)
        {
            redis_ptr->hset(id, "department", depart);
            redis_ptr->sadd("stafflist", id);
        }

        ++staff_num_;
        int depart_id = std::stoi(depart);
        department_num_ = std::max(department_num_, depart_id);
        department_staffnum_[depart_id - 1]++;
        staff_info[id] = {id, depart_id, -1};
        //std::cout<<"get: id="<<id<<"department="<<depart<<std::endl;
    }
    file.close();

    if (server_id_ == 1)
    {
        redis_ptr->set("ticket_num", std::to_string(ticket_num_));
        redis_ptr->set("staff_num", std::to_string(staff_num_));
        redis_ptr->set("department_num", std::to_string(department_num_));

        //generate and distribute valid tickets according to staff number of each department
        std::vector<std::string> ticket_id;
        std::vector<int> ticket_num(department_num_, 0);
        for (int i = 0; i < staff_num_; i++)
        {
            ticket_id.push_back(std::to_string(i + 1));
        }

        int distributed_num = 0;
        for (int department_i = 0; department_i < department_num_; department_i++)
        {
            int distributed_i = department_staffnum_[department_i] * ticket_num_ / staff_num_;
            for (int i = distributed_num; i < distributed_num + distributed_i; i++)
            {
                redis_ptr->sadd("d" + std::to_string(department_i + 1) + "_ticketpool", ticket_id[i]); //eg: SADD d1_ticketpool 1
            }
            distributed_num += distributed_i;
            ticket_num[department_i] += distributed_i;
        }
        //distribute remaining valid tickets
        srand(time(0));
        for (int i = distributed_num; i < ticket_num_; i++)
        {
            int dptm = rand() % department_num_;
            ++ticket_num[dptm];
            redis_ptr->sadd("d" + std::to_string(dptm + 1) + "_ticketpool", ticket_id[i]);
            ++distributed_num;
        }
        //distribute invalid tickets
        for (int department_i = 0; department_i < department_num_; department_i++)
        {
            int distributed_i = department_staffnum_[department_i] - ticket_num[department_i];
            for (int i = distributed_num; i < distributed_num + distributed_i; i++)
            {
                redis_ptr->sadd("d" + std::to_string(department_i + 1) + "_ticketpool", ticket_id[i]);
            }
            distributed_num += distributed_i;
        }
    }
    printf("Initialization finished.\n");
}

//whether id is legal(in staff-list)
bool raffle::ismember(const std::string id)
{
    return (staff_info.find(id) != staff_info.end());
}

//staff-id draw a ticket, return whether win or not
int raffle::draw_ticket(const std::string id)
{
    std::shared_ptr<Redis> redis_ptr = db_pool_.get();
    conn_guard<Redis> guard(redis_ptr);

    if (staff_info[id].ticketnum != -1)
        return staff_info[id].ticketnum;
    int ticket_id = 0;
    std::string ticketpool = "d" + std::to_string(staff_info[id].department) + "_ticketpool";
    while (ticket_id == 0)
    {
        ticket_id = std::stoi(redis_ptr->hget(id, "ticket"));
        if (ticket_id != 0)
        {
            return ticket_id;
        }
        ticket_id = std::stoi(redis_ptr->spop(ticketpool));
    }

    bool ret = redis_ptr->hsetnx(id, "ticket", std::to_string(ticket_id));
    if (ret == 0)
    {
        redis_ptr->sadd(ticketpool, std::to_string(ticket_id));
        ticket_id = std::stoi(redis_ptr->hget(id, "ticket"));
    }
    else
    {
        staff_info[id].ticketnum = ticket_id;
    }
    return ticket_id;
}

int raffle::draw_ticket_(const std::string id)
{
    int ticket_n = draw_ticket(id);
    return ticket_n <= ticket_num_ ? ticket_n : 0;
}

void raffle::finish()
{
    std::fstream file;
    std::vector<int> department_winnum(department_num_, 0);
    file.open("./winlist.csv", std::ofstream::out | std::ofstream::trunc);
    file << "id,department,ticket number" << std::endl;
    for (auto it = staff_info.begin(); it != staff_info.end(); it++)
    {
        if (it->second.ticketnum == -1)
        {
            it->second.ticketnum = draw_ticket(it->first);
        }
        if (it->second.ticketnum <= ticket_num_)
        {
            file << it->first << "," << it->second.department << "," << it->second.ticketnum << std::endl;
            department_winnum[it->second.department - 1]++;
        }
    }
    file.close();

    for (int i = 0; i < department_num_; i++)
    {
        std::cout << "department " << i + 1 << ": " << department_winnum[i] << "/" << department_staffnum_[i] << "staff got the ticket" << std::endl;
    }

    file.open("./winnerlist.html", std::ofstream::out | std::ofstream::trunc);
    file << "<!DOCTYPE html><html><head><title>raffle winner list</title></head><body><h1 align=\"center\">Winner List</h1><table align=\"center\" style=\"width:371px;text-align:center;\">";
    file << "<thead><tr><th>ID</th><th>department</th><th>ticket number</th></tr></thead><tbody>";

    for (auto it = staff_info.begin(); it != staff_info.end(); it++)
    {
        if (it->second.ticketnum == -1)
        {
            it->second.ticketnum = draw_ticket(it->first);
        }
        if (it->second.ticketnum <= ticket_num_)
        {
            file << "<tr>";
            file << "<td>" << it->first << "</td>";
            file << "<td>" << it->second.department << "</td>";
            file << "<td>" << it->second.ticketnum << "</td>";
            //department_winnum[it->second.department-1]++;
            file << "</tr>";
        }
    }
    file << "</tbody></table></body></html>" << std::endl;
    file.close();
}

void raffle::webbench_test()
{
    std::shared_ptr<Redis> redis_ptr = db_pool_.get();
    conn_guard<Redis> guard(redis_ptr);
    redis_ptr->set("key", "value");
    //std::cout<<"test"<<std::endl;
}