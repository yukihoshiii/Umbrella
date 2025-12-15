#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "runtime.h"
namespace umbrella {
namespace runtime {
class Database {
public:
    void* db;   
    std::string dbPath;
    Database(const std::string& path);
    ~Database();
    void exec(const std::string& sql);
    Array<Map<std::string, std::string>> query(const std::string& sql);
    void prepare(const std::string& sql);
    void bind(int index, const std::string& value);
    void bind(int index, double value);
    void bind(int index, int value);
    void step();
    void reset();
    void finalize();
    void beginTransaction();
    void commit();
    void rollback();
    int lastInsertId();
    int changes();
    void close();
};
class Thread {
public:
    void* handle;   
    static Thread spawn(void (*func)());
    void join();
    void detach();
    bool joinable();
};
class Mutex {
public:
    void* handle;   
    Mutex();
    ~Mutex();
    void lock();
    void unlock();
    bool tryLock();
};
class Process {
public:
    int pid;
    std::string command;
    static Process spawn(const std::string& cmd, const std::vector<std::string>& args);
    std::string stdout();
    std::string stderr();
    int wait();
    void kill();
    bool isRunning();
};
class Timer {
public:
    static void sleep(int milliseconds);
    static void setTimeout(void (*callback)(), int milliseconds);
    static void setInterval(void (*callback)(), int milliseconds);
};
}  
}  
