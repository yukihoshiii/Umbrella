#include "advanced.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
namespace umbrella {
namespace runtime {
Database::Database(const std::string& path) : dbPath(path), db(nullptr) {
    std::cout << "Database opened: " << path << std::endl;
}
Database::~Database() {
    close();
}
void Database::exec(const std::string& sql) {
    std::cout << "Executing SQL: " << sql << std::endl;
}
std::vector<std::map<std::string, std::string>> Database::query(const std::string& sql) {
    std::vector<std::map<std::string, std::string>> results;
    std::cout << "Querying: " << sql << std::endl;
    return results;
}
void Database::beginTransaction() {
    exec("BEGIN TRANSACTION");
}
void Database::commit() {
    exec("COMMIT");
}
void Database::rollback() {
    exec("ROLLBACK");
}
int Database::lastInsertId() {
    return 0;
}
int Database::changes() {
    return 0;
}
void Database::close() {
    if (db) {
        std::cout << "Database closed" << std::endl;
        db = nullptr;
    }
}
Thread Thread::spawn(void (*func)()) {
    Thread t;
    t.handle = new std::thread(func);
    return t;
}
void Thread::join() {
    if (handle) {
        ((std::thread*)handle)->join();
    }
}
void Thread::detach() {
    if (handle) {
        ((std::thread*)handle)->detach();
    }
}
bool Thread::joinable() {
    if (handle) {
        return ((std::thread*)handle)->joinable();
    }
    return false;
}
Mutex::Mutex() {
    handle = new std::mutex();
}
Mutex::~Mutex() {
    if (handle) {
        delete (std::mutex*)handle;
    }
}
void Mutex::lock() {
    if (handle) {
        ((std::mutex*)handle)->lock();
    }
}
void Mutex::unlock() {
    if (handle) {
        ((std::mutex*)handle)->unlock();
    }
}
bool Mutex::tryLock() {
    if (handle) {
        return ((std::mutex*)handle)->try_lock();
    }
    return false;
}
Process Process::spawn(const std::string& cmd, const std::vector<std::string>& args) {
    Process p;
    pid_t pid = fork();
    if (pid == 0) {
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(cmd.c_str()));
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);
        execvp(cmd.c_str(), argv.data());
        exit(1);
    } else {
        p.pid = pid;
        p.command = cmd;
    }
    return p;
}
std::string Process::stdout() {
    return "";
}
std::string Process::stderr() {
    return "";
}
int Process::wait() {
    int status;
    waitpid(pid, &status, 0);
    return WEXITSTATUS(status);
}
void Process::kill() {
    ::kill(pid, SIGTERM);
}
bool Process::isRunning() {
    int status;
    pid_t result = waitpid(pid, &status, WNOHANG);
    return result == 0;
}
void Timer::sleep(int milliseconds) {
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}
void Timer::setTimeout(void (*callback)(), int milliseconds) {
    std::thread([callback, milliseconds]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        callback();
    }).detach();
}
void Timer::setInterval(void (*callback)(), int milliseconds) {
    std::thread([callback, milliseconds]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
            callback();
        }
    }).detach();
}
}  
}  
