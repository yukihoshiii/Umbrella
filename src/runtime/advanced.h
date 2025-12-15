#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "runtime.h"
namespace umbrella {
namespace runtime {
class Database {
public:
    std::shared_ptr<void> db;   
    std::shared_ptr<void> stmtHandle; // prepared statement (sqlite3_stmt*)
    std::string dbPath;
    Database(const std::string& path);
    // Destructor removed as shared_ptr handles cleanup
    void exec(const std::string& sql);
    Array<Row> query(const std::string& sql);

    // Optional prepared-statement API (not heavily used yet, but fully implemented)
    void prepare(const std::string& sql);
    void bind(int index, const std::string& value);
    void bind(int index, double value);
    void bind(int index, int value);
    bool step();     // returns true if a row is available
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
    static Thread spawn(std::function<void()> func);
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
    static Process spawn(const std::string& cmd, const Array<std::string>& args); // Added overload
    std::string stdout();
    std::string stderr();
    int wait();
    void kill();
    bool isRunning();
};
class Timer {
public:
    static void sleep(int milliseconds);
    static void setTimeout(std::function<void()> callback, int milliseconds);
    static void setInterval(std::function<void()> callback, int milliseconds);
};
}  
}  
