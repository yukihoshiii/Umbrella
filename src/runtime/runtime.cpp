#include "runtime.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <regex>
#include <unistd.h>
namespace umbrella {
namespace runtime {
void print(const std::string& message) {
    std::cout << message;
}
void println(const std::string& message) {
    std::cout << message << std::endl;
}
std::string toString(double value) {
    // If value is effectively an integer, print as integer
    double intPart;
    if (std::modf(value, &intPart) == 0.0) {
        return std::to_string((long long)value);
    }
    std::ostringstream ss;
    ss << value;
    return ss.str();
}
std::string toString(int value) {
    return std::to_string(value);
}
std::string toString(long long value) {
    return std::to_string(value);
}
std::string toString(size_t value) {
    return std::to_string(value);
}
std::string toString(bool value) {
    return value ? "true" : "false";
}
double toNumber(const std::string& str) {
    try {
        return std::stod(str);
    } catch (...) {
        return 0.0;
    }
}
namespace Math {
    double sqrt(double x) {
        return std::sqrt(x);
    }
    double pow(double base, double exponent) {
        return std::pow(base, exponent);
    }
    double abs(double x) {
        return std::abs(x);
    }
    double floor(double x) {
        return std::floor(x);
    }
    double ceil(double x) {
        return std::ceil(x);
    }
    double round(double x) {
        return std::round(x);
    }
    double max(double a, double b) {
        return a > b ? a : b;
    }
    double min(double a, double b) {
        return a < b ? a : b;
    }
    double random() {
        static bool initialized = false;
        if (!initialized) {
            std::srand(std::time(nullptr));
            initialized = true;
        }
        return static_cast<double>(std::rand()) / RAND_MAX;
    }
}
int String::length(const std::string& str) {
    return static_cast<int>(str.length());
}
std::string String::toUpperCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}
std::string String::toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}
std::string String::substring(const std::string& str, int start, int end) {
    if (start < 0) start = 0;
    if (end > static_cast<int>(str.length())) end = str.length();
    if (start >= end) return "";
    return str.substr(start, end - start);
}
int String::indexOf(const std::string& str, const std::string& search) {
    size_t pos = str.find(search);
    return pos == std::string::npos ? -1 : static_cast<int>(pos);
}
std::string String::replace(const std::string& str, const std::string& from, const std::string& to) {
    std::string result = str;
    size_t pos = result.find(from);
    if (pos != std::string::npos) {
        result.replace(pos, from.length(), to);
    }
    return result;
}
Array<std::string> String::split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do {
        pos = str.find(delimiter, prev);
        if (pos == std::string::npos) pos = str.length();
        tokens.push_back(str.substr(prev, pos - prev));
        prev = pos + delimiter.length();
    } while (pos < str.length() && prev < str.length());
    return Array<std::string>(tokens);
}
std::string String::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}
bool String::startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && 
           str.compare(0, prefix.size(), prefix) == 0;
}
bool String::endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}
std::string String::repeat(const std::string& str, int count) {
    std::string result;
    for (int i = 0; i < count; i++) {
        result += str;
    }
    return result;
}
std::string String::padStart(const std::string& str, int length, const std::string& pad) {
    if (static_cast<int>(str.length()) >= length) return str;
    int padLength = length - str.length();
    std::string padding;
    while (static_cast<int>(padding.length()) < padLength) {
        padding += pad;
    }
    return padding.substr(0, padLength) + str;
}
std::string String::padEnd(const std::string& str, int length, const std::string& pad) {
    if (static_cast<int>(str.length()) >= length) return str;
    int padLength = length - str.length();
    std::string padding;
    while (static_cast<int>(padding.length()) < padLength) {
        padding += pad;
    }
    return str + padding.substr(0, padLength);
}
long long Date::now() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}
std::string Date::toISOString(long long timestamp) {
    std::time_t time = timestamp / 1000;
    std::tm* tm = std::gmtime(&time);
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", tm);
    return std::string(buffer);
}
std::string Date::toDateString(long long timestamp) {
    std::time_t time = timestamp / 1000;
    std::tm* tm = std::localtime(&time);
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", tm);
    return std::string(buffer);
}
std::string Date::toTimeString(long long timestamp) {
    std::time_t time = timestamp / 1000;
    std::tm* tm = std::localtime(&time);
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", tm);
    return std::string(buffer);
}
std::string JSON::stringify(const std::string& value) {
    return "\"" + value + "\"";
}
std::string JSON::parse(const std::string& json) {
    if (json.size() >= 2 && json.front() == '"' && json.back() == '"') {
        return json.substr(1, json.size() - 2);
    }
    return json;
}
std::string File::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
void File::writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not write to file: " + path);
    }
    file << content;
}
bool File::exists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}
void File::deleteFile(const std::string& path) {
    std::remove(path.c_str());
}
void Console::log(const std::string& message) {
    std::cout << message << std::endl;
}
void Console::error(const std::string& message) {
    std::cerr << "[ERROR] " << message << std::endl;
}
void Console::warn(const std::string& message) {
    std::cout << "[WARN] " << message << std::endl;
}
void Console::info(const std::string& message) {
    std::cout << "[INFO] " << message << std::endl;
}
void Console::clear() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}
HTTPResponse HTTP::get(const std::string& url) {
    return request("GET", url);
}
HTTPResponse HTTP::post(const std::string& url, const std::string& body) {
    return request("POST", url, body);
}
HTTPResponse HTTP::put(const std::string& url, const std::string& body) {
    return request("PUT", url, body);
}
HTTPResponse HTTP::del(const std::string& url) {
    return request("DELETE", url);
}
HTTPResponse HTTP::request(const std::string& method, const std::string& url,
                           const std::string& body,
                           const std::map<std::string, std::string>& headers) {
    HTTPResponse response;
    std::string cmd = "curl -s -w '\\n%{http_code}' ";
    cmd += "-X " + method + " ";
    if (!body.empty()) {
        cmd += "-d '" + body + "' ";
    }
    for (const auto& header : headers) {
        cmd += "-H '" + header.first + ": " + header.second + "' ";
    }
    cmd += "'" + url + "'";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        response.statusCode = 0;
        response.body = "Failed to execute request";
        return response;
    }
    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    size_t lastNewline = result.find_last_of('\n');
    if (lastNewline != std::string::npos && lastNewline > 0) {
        std::string statusStr = result.substr(lastNewline + 1);
        response.statusCode = std::stoi(statusStr);
        response.body = result.substr(0, lastNewline);
    } else {
        response.statusCode = 200;
        response.body = result;
    }
    return response;
}
Regex::Regex(const std::string& pat) : pattern(pat) {}
bool Regex::test(const std::string& str) const {
    std::regex re(pattern);
    return std::regex_search(str, re);
}
Array<std::string> Regex::match(const std::string& str) const {
    std::vector<std::string> result;
    std::regex re(pattern);
    std::smatch matches;
    if (std::regex_search(str, matches, re)) {
        for (const auto& match : matches) {
            result.push_back(match.str());
        }
    }
    return Array<std::string>(result);
}
Array<std::string> Regex::findAll(const std::string& str) const {
    std::vector<std::string> result;
    std::regex re(pattern);
    std::sregex_iterator it(str.begin(), str.end(), re);
    std::sregex_iterator end;
    while (it != end) {
        result.push_back(it->str());
        ++it;
    }
    return Array<std::string>(result);
}
std::string Regex::replace(const std::string& str, const std::string& replacement) const {
    std::regex re(pattern);
    return std::regex_replace(str, re, replacement);
}
std::string Env::get(const std::string& name, const std::string& defaultValue) {
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : defaultValue;
}
void Env::set(const std::string& name, const std::string& value) {
    #ifdef _WIN32
        _putenv_s(name.c_str(), value.c_str());
    #else
        setenv(name.c_str(), value.c_str(), 1);
    #endif
}
bool Env::has(const std::string& name) {
    return std::getenv(name.c_str()) != nullptr;
}
std::string Env::home() {
    #ifdef _WIN32
        return get("USERPROFILE", "");
    #else
        return get("HOME", "");
    #endif
}
std::string Env::cwd() {
    char buffer[1024];
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    }
    return "";
}
}  
}  
