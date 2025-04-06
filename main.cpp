#include <iostream>
#include <time.h>
#include <chrono>
#include "connection.hpp"
#include "connectionPool.hpp"
using namespace std;

char sql[1024] = "insert into users(name, age) values('zhang san', 20);";

void test_without_pool(int times)
{
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < times; ++i)
    {
        Connection conn;
        conn.connect("127.0.0.1", 3306, "root", "123456", "project_mysql_connection_pool");
        conn.update(sql);
    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;

    std::cout << "不用连接池运行" << times << "次" << endl
              << "耗时: " << duration.count() << " 秒" << endl
              << endl;
}

void test_with_pool(int times)
{
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < times; ++i)
    {
        ConnectionPool *cp = ConnectionPool::getConnectionPool();
        auto sp = cp->getConnection();
        sp->update(sql);
    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;

    std::cout << "用连接池运行" << times << "次" << endl
              << "耗时: " << duration.count() << " 秒" << endl
              << endl;
}

void clear_mysql()
{
    Connection conn;
    conn.connect("127.0.0.1", 3306, "root", "123456", "project_mysql_connection_pool");
    conn.update("DROP TABLE users;");
    conn.update("create table users(id INT AUTO_INCREMENT PRIMARY KEY, name varchar(50) NOT NULL, age int NOT NULL);");
}

int main()
{
    int times;
    while (true)
    {
        cin >> times;
        clear_mysql();
        test_with_pool(times);
        clear_mysql();
        test_without_pool(times);
    }

    return 0;
}