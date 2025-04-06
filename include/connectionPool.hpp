#ifndef CONNECTION_POOL
#define CONNECTION_POOL
#include "connection.hpp"
#include <string>
#include <mutex>
#include <queue>
#include <atomic>
#include <thread>
#include <memory>
#include <functional>
#include <condition_variable>
#include <chrono>
using namespace std;

class ConnectionPool
{
public:
    static ConnectionPool *getConnectionPool();
    // 给外部提供接口,从连接池中获取一个可用的空闲链接
    shared_ptr<Connection> getConnection();

private:
    void createNewConnection();

    ConnectionPool();

    void scannerConnectionTask();

    // 从配置文件中加载配置项
    bool loadConfigFile();

    // 运行在独立线程中,专门用来生产新链接
    void produceConnectionTask();

    // Mysql 信息
    string _ip;
    unsigned short _port;
    string _username;
    string _password;
    string _dbname;
    int _initSize;          // 初始连接量
    int _maxSize;           // 最大连接量
    int _maxIdleTime;       // 最大空闲时间
    int _connectionTimeout; // 获取链接的超时时间

    queue<Connection *> _connectionQueue; // 存储MySQL链接的队列
    mutex _queueMutex;                    // 维护链接队列的线程安全互斥锁
    condition_variable cv;                // 设置条件变量,用于连接的生产消费线程通信

    atomic_int _connectionCnt; // 记录链接所创建的connection链接总数量
};

#endif