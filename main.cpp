#include <iostream>
#include <time.h>
#include <chrono>
#include <queue>
#include <functional>
#include "connection.hpp"
#include "connectionPool.hpp"
using namespace std;

int times;
int thread_num = 10;
char sql[1024] = "insert into users(name, age) values('zhang san', 20);";

void test_without_pool()
{

    for (int i = 0; i < times / thread_num; ++i)
    {
        Connection conn;
        conn.connect("127.0.0.1", 3306, "root", "123456", "project_mysql_connection_pool");
        conn.update(sql);
    }
}

void test_with_pool()
{

    for (int i = 0; i < times / thread_num; ++i)
    {
        ConnectionPool *cp = ConnectionPool::getConnectionPool();
        auto sp = cp->getConnection();
        sp->update(sql);
    }
}

void clear_mysql()
{
    Connection conn;
    conn.connect("127.0.0.1", 3306, "root", "123456", "project_mysql_connection_pool");
    conn.update("DROP TABLE users;");
    conn.update("create table users(id INT AUTO_INCREMENT PRIMARY KEY, name varchar(50) NOT NULL, age int NOT NULL);");
}

void run_test(bool use_pool)
{

    vector<thread> vec;

    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < thread_num; ++i)
    {
        if (use_pool)
            vec.emplace_back(test_with_pool);
        else
            vec.emplace_back(test_without_pool);
    }

    for (int i = 0; i < thread_num; ++i)
    {
        vec[i].join();
    }

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration = end - start;

    if (use_pool)
        cout << "用连接池运行" << times << "次" << endl;
    else
        cout << "不用连接池运行" << times << "次" << endl;

    cout << "耗时: " << duration.count() << " 秒" << endl
         << endl;
}

int main()
{
    while (true)
    {
        cin >> times;
        clear_mysql();
        run_test(true);

        clear_mysql();
        run_test(false);
    }

    return 0;
}