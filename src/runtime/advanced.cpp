#include "advanced.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include <sqlite3.h>
namespace umbrella {
namespace runtime {
Database::Database(const std::string& path) : dbPath(path), db(nullptr) {
    if (sqlite3_open(path.c_str(), (sqlite3**)&db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg((sqlite3*)db) << std::endl;
        db = nullptr;
    }
}
Database::~Database() {
    close();
}
void Database::exec(const std::string& sql) {
    char* errMsg = 0;
    if (sqlite3_exec((sqlite3*)db, sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

Array<Map<std::string, std::string>> Database::query(const std::string& sql) {
    std::vector<Map<std::string, std::string>> results;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2((sqlite3*)db, sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
        int cols = sqlite3_column_count(stmt);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<std::string, std::string> rowData;
            for (int i = 0; i < cols; i++) {
                const char* name = sqlite3_column_name(stmt, i);
                const char* val = (const char*)sqlite3_column_text(stmt, i);
                rowData[name ? name : ""] = val ? val : "";
            }
            results.push_back(Map<std::string, std::string>(rowData));
        }
    }
    sqlite3_finalize(stmt);
    return Array<Map<std::string, std::string>>(results);
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
    return (int)sqlite3_last_insert_rowid((sqlite3*)db);
}
int Database::changes() {
    return sqlite3_changes((sqlite3*)db);
}
void Database::close() {
    if (db) {
        sqlite3_close((sqlite3*)db);
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
