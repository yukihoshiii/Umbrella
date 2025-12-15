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
Database::Database(const std::string& path) : dbPath(path) {
    sqlite3* rawDb = nullptr;
    if (sqlite3_open(path.c_str(), &rawDb) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(rawDb) << std::endl;
        // Even if open fails, we might need to close if handle was allocated
        if (rawDb) sqlite3_close(rawDb);
        db = nullptr;
    } else {
        // Use shared_ptr with custom deleter to close sqlite3 handle
        db = std::shared_ptr<void>(rawDb, [](void* ptr) {
            if (ptr) {
                sqlite3_close((sqlite3*)ptr);
            }
        });
    }
}

// Destructor removed as shared_ptr handles cleanup

void Database::exec(const std::string& sql) {
    if (!db) return;
    char* errMsg = 0;
    if (sqlite3_exec((sqlite3*)db.get(), sql.c_str(), 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

Array<Row> Database::query(const std::string& sql) {
    std::vector<Row> results;
    if (!db) return Array<Row>(results);
    
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2((sqlite3*)db.get(), sql.c_str(), -1, &stmt, 0) == SQLITE_OK) {
        int cols = sqlite3_column_count(stmt);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<std::string, std::string> rowData;
            for (int i = 0; i < cols; i++) {
                const char* name = sqlite3_column_name(stmt, i);
                const char* val = (const char*)sqlite3_column_text(stmt, i);
                rowData[name ? name : ""] = val ? val : "";
            }
            Row row;
            row.data = Map<std::string, std::string>(rowData);
            results.push_back(row);
        }
    }
    sqlite3_finalize(stmt);
    return Array<Row>(results);
}

void Database::prepare(const std::string& sql) {
    if (!db) return;

    // Finalize any existing statement first
    finalize();

    sqlite3_stmt* rawStmt = nullptr;
    if (sqlite3_prepare_v2((sqlite3*)db.get(), sql.c_str(), -1, &rawStmt, nullptr) != SQLITE_OK) {
        std::cerr << "SQL prepare error: " << sqlite3_errmsg((sqlite3*)db.get()) << std::endl;
        if (rawStmt) {
            sqlite3_finalize(rawStmt);
        }
        return;
    }

    // Manage statement lifetime with shared_ptr deleter
    stmtHandle = std::shared_ptr<void>(rawStmt, [](void* ptr) {
        if (ptr) {
            sqlite3_finalize((sqlite3_stmt*)ptr);
        }
    });
}

void Database::bind(int index, const std::string& value) {
    if (!stmtHandle) return;
    sqlite3_stmt* stmt = (sqlite3_stmt*)stmtHandle.get();
    sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT);
}

void Database::bind(int index, double value) {
    if (!stmtHandle) return;
    sqlite3_stmt* stmt = (sqlite3_stmt*)stmtHandle.get();
    sqlite3_bind_double(stmt, index, value);
}

void Database::bind(int index, int value) {
    if (!stmtHandle) return;
    sqlite3_stmt* stmt = (sqlite3_stmt*)stmtHandle.get();
    sqlite3_bind_int(stmt, index, value);
}

bool Database::step() {
    if (!stmtHandle) return false;
    sqlite3_stmt* stmt = (sqlite3_stmt*)stmtHandle.get();
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        return true; // row available
    }
    if (rc != SQLITE_DONE) {
        std::cerr << "SQL step error: " << sqlite3_errmsg((sqlite3*)db.get()) << std::endl;
    }
    return false;
}

void Database::reset() {
    if (!stmtHandle) return;
    sqlite3_stmt* stmt = (sqlite3_stmt*)stmtHandle.get();
    sqlite3_reset(stmt);
}

void Database::finalize() {
    // Resetting shared_ptr will call sqlite3_finalize via deleter
    stmtHandle.reset();
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
    if (!db) return 0;
    return (int)sqlite3_last_insert_rowid((sqlite3*)db.get());
}
int Database::changes() {
    if (!db) return 0;
    return sqlite3_changes((sqlite3*)db.get());
}
void Database::close() {
    // Resetting the shared_ptr will trigger the deleter if this is the last reference
    db.reset();
}
Thread Thread::spawn(std::function<void()> func) {
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
Process Process::spawn(const std::string& cmd, const Array<std::string>& args) {
    return spawn(cmd, args.data);
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
void Timer::setTimeout(std::function<void()> callback, int milliseconds) {
    std::thread([callback, milliseconds]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        callback();
    }).detach();
}
void Timer::setInterval(std::function<void()> callback, int milliseconds) {
    std::thread([callback, milliseconds]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
            callback();
        }
    }).detach();
}
}  
}  
