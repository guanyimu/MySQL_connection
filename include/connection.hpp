#ifndef CONNECTION_HPP
#define CONNECTION_HPP
#include <string>
#include <mysql/mysql.h>
#include <iostream>
#include <ctime>
#include <chrono>
using namespace std;
// 实现MySQL增删改查的操作

class Connection
{
public:
    // 创建链接库
    Connection();

    // 释放链接库
    ~Connection();

    // 链接数据库
    bool connect(string ip, unsigned short port, string user, string password, string dbname);

    // 更新操作 insert delete update
    bool update(string sql);

    // 查询操作 select
    MYSQL_RES *query(string sql);

    // 刷新一下链接的其实空闲时间
    void refreshAliveTime() { _alivetime = std::chrono::high_resolution_clock::now(); }

    // 返回存货的时间
    double getAliveTime() const { return (std::chrono::high_resolution_clock::now() - _alivetime).count(); }

private:
    MYSQL *_conn;                                              // 表示和MySQL Server的一条连接
    std::chrono::high_resolution_clock::time_point _alivetime; // 进入空闲状态后的时间
};

#endif
