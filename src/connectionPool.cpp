#include "connectionPool.hpp"
#include "public.hpp"

ConnectionPool *ConnectionPool::getConnectionPool()
{
    static ConnectionPool pool;
    return &pool;
}

// 从配置文件中加载配置项
bool ConnectionPool::loadConfigFile()
{
    FILE *pf = fopen("mysql_connection.conf", "r");
    if (pf == nullptr)
    {
        LOG("mysql_connection.conf file is not exist!");
        return false;
    }

    while (!feof(pf))
    {
        char line[1024] = {0};
        fgets(line, 1024, pf);
        string str = line;
        int idx = str.find('=', 0);
        if (idx == -1)
        {
            // 无效配置项或注释
            continue;
        }

        int endidx = str.find('\n', idx);
        string key = str.substr(0, idx);
        string value = str.substr(idx + 1, endidx - idx - 1);

        // cout << key << " " << value << endl;

        if (key == "ip")
        {
            _ip = value;
        }
        else if (key == "port")
        {
            _port = atoi(value.c_str());
        }
        else if (key == "username")
        {
            _username = value;
        }
        else if (key == "password")
        {
            _password = value;
        }
        else if (key == "dbname")
        {
            _dbname = value;
        }
        else if (key == "initSize")
        {
            _initSize = atoi(value.c_str());
        }
        else if (key == "maxSize")
        {
            _maxSize = atoi(value.c_str());
        }
        else if (key == "maxIdleTime")
        {
            _maxIdleTime = atoi(value.c_str());
        }
        else if (key == "connectionTimeOut")
        {
            _connectionTimeout = atoi(value.c_str());
        }
    }

    return true;
}

ConnectionPool::ConnectionPool()
{
    if (!loadConfigFile())
    {
        return;
    }

    // 创建初始数量的连接
    for (int i = 0; i < _initSize; ++i)
    {
        createNewConnection();
    }

    // 启动一个新的线程,作为连接的生产者
    thread produce(std::bind(&ConnectionPool::produceConnectionTask, this));
    produce.detach();

    // 启动一个新的定时线程,扫描超过maxIdleTime时间的空闲链接,将其回收
    thread scanner(std::bind(&ConnectionPool::scannerConnectionTask, this));
    scanner.detach();
}

void ConnectionPool::createNewConnection()
{
    Connection *p = new Connection();
    p->connect(_ip, _port, _username, _password, _dbname);
    p->refreshAliveTime();
    _connectionQueue.push(p);
    _connectionCnt++;
}

// 消费者线程
shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(_queueMutex);
    while (_connectionQueue.empty())
    {
        // 需要等待空闲线程,比如新创建的或者归还的
        if (cv_status::timeout == cv.wait_for(lock, chrono::milliseconds(_connectionTimeout)))
        {
            if (_connectionQueue.empty())
            {
                // 说明是超时了还没有线程可以用
                LOG("获取空闲链接超时了...获取链接失败");
                return nullptr;
            }
        }
    }

    shared_ptr<Connection> sp(_connectionQueue.front(), [&](Connection *pcon) { // 删除器,用来智能指针时把线程换回来
        unique_lock<mutex> lock(_queueMutex);
        pcon->refreshAliveTime();
        _connectionQueue.push(pcon);
    });
    _connectionQueue.pop();
    cv.notify_all();
    return sp;
}

// 生产者线程
void ConnectionPool::produceConnectionTask()
{
    for (;;)
    {
        unique_lock<mutex> lock(_queueMutex);
        while (!_connectionQueue.empty())
        {
            cv.wait(lock);
        }

        // 连接数量没有到达上限,继续创建新的连接
        if (_connectionCnt < _maxSize)
        {
            createNewConnection();
        }

        cv.notify_all();
    }
}

void ConnectionPool::scannerConnectionTask()
{
    for (;;)
    {
        // 通过sleep模拟定时效果
        this_thread::sleep_for(chrono::seconds(_maxIdleTime));

        // 扫描整个队列,释放多余的连接
        unique_lock<mutex> lock(_queueMutex);
        while (_connectionCnt > _initSize)
        {
            Connection *p = _connectionQueue.front();
            if (p->getAliveTime() >= _maxIdleTime)
            {
                _connectionQueue.pop();
                _connectionCnt--;
                delete p;
            }
            else
            {
                break;
            }
        }
    }
}