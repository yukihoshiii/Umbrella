#pragma once
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <chrono>
#include <memory>
#include <algorithm>
namespace umbrella {
namespace runtime {
void print(const std::string& message);
void println(const std::string& message);
std::string toString(double value);
std::string toString(int value);
std::string toString(long long value);
std::string toString(size_t value);
std::string toString(bool value);
double toNumber(const std::string& str);

// Forward declaration so Math helpers can accept Array<T>
template<typename T>
class Array;

namespace Math {
    double sqrt(double x);
    double pow(double base, double exponent);
    double abs(double x);
    double floor(double x);
    double ceil(double x);
    double round(double x);
    double max(double a, double b);
    double min(double a, double b);
    double random();  
    const double PI = 3.14159265358979323846;
    const double E = 2.71828182845904523536;

    // Aggregate helpers for numeric arrays (e.g. Math::max(nums))
    template<typename T>
    T max(const Array<T>& arr) {
        if (arr.data.empty()) {
            throw std::runtime_error("Math::max() called on empty array");
        }
        T current = arr.data[0];
        for (size_t i = 1; i < arr.data.size(); ++i) {
            if (arr.data[i] > current) {
                current = arr.data[i];
            }
        }
        return current;
    }

    template<typename T>
    T min(const Array<T>& arr) {
        if (arr.data.empty()) {
            throw std::runtime_error("Math::min() called on empty array");
        }
        T current = arr.data[0];
        for (size_t i = 1; i < arr.data.size(); ++i) {
            if (arr.data[i] < current) {
                current = arr.data[i];
            }
        }
        return current;
    }
}

template<typename T>
class Array {
public:
    std::vector<T> data;
    Array() = default;
    Array(const std::vector<T>& d) : data(d) {}
    Array(std::initializer_list<T> init) : data(init) {}
    size_t length() const { return data.size(); }
    void push(const T& value) {
        data.push_back(value);
    }
    T pop() {
        if (data.empty()) throw std::runtime_error("Array is empty");
        T value = data.back();
        data.pop_back();
        return value;
    }
    T shift() {
        if (data.empty()) throw std::runtime_error("Array is empty");
        T value = data.front();
        data.erase(data.begin());
        return value;
    }
    void unshift(const T& value) {
        data.insert(data.begin(), value);
    }
    void reverse() {
        std::reverse(data.begin(), data.end());
    }
    void sort() {
        std::sort(data.begin(), data.end());
    }
    void splice(size_t start, size_t deleteCount) {
        if (start >= data.size()) return;
        size_t end = std::min(start + deleteCount, data.size());
        data.erase(data.begin() + start, data.begin() + end);
    }
    void fill(const T& value, size_t start = 0, size_t end = SIZE_MAX) {
        if (end == SIZE_MAX) end = data.size();
        for (size_t i = start; i < end && i < data.size(); i++) {
            data[i] = value;
        }
    }
    Array<T> slice(size_t start = 0, size_t end = SIZE_MAX) const {
        if (end == SIZE_MAX) end = data.size();
        std::vector<T> result;
        for (size_t i = start; i < end && i < data.size(); i++) {
            result.push_back(data[i]);
        }
        return Array<T>(result);
    }
    Array<T> concat(const Array<T>& other) const {
        std::vector<T> result = data;
        result.insert(result.end(), other.data.begin(), other.data.end());
        return Array<T>(result);
    }
    std::string join(const std::string& separator = ",") const {
        std::stringstream ss;
        for (size_t i = 0; i < data.size(); i++) {
            if (i > 0) ss << separator;
            ss << data[i];
        }
        return ss.str();
    }
    int indexOf(const T& value, size_t fromIndex = 0) const {
        for (size_t i = fromIndex; i < data.size(); i++) {
            if (data[i] == value) return static_cast<int>(i);
        }
        return -1;
    }
    int lastIndexOf(const T& value, size_t fromIndex = SIZE_MAX) const {
        if (fromIndex == SIZE_MAX) fromIndex = data.size() - 1;
        for (int i = static_cast<int>(fromIndex); i >= 0; i--) {
            if (data[i] == value) return i;
        }
        return -1;
    }
    bool includes(const T& value, size_t fromIndex = 0) const {
        return indexOf(value, fromIndex) != -1;
    }
    template<typename Func>
    Array<T> filter(Func predicate) const {
        std::vector<T> result;
        for (const auto& item : data) {
            if (predicate(item)) {
                result.push_back(item);
            }
        }
        return Array<T>(result);
    }
    template<typename Func, typename R>
    Array<R> map(Func transform) const {
        std::vector<R> result;
        for (const auto& item : data) {
            result.push_back(transform(item));
        }
        return Array<R>(result);
    }
    template<typename Func>
    void forEach(Func callback) const {
        for (size_t i = 0; i < data.size(); i++) {
            callback(data[i], i);
        }
    }
    template<typename Func>
    bool some(Func predicate) const {
        for (const auto& item : data) {
            if (predicate(item)) return true;
        }
        return false;
    }
    template<typename Func>
    bool every(Func predicate) const {
        for (const auto& item : data) {
            if (!predicate(item)) return false;
        }
        return true;
    }
    template<typename Func, typename R>
    R reduce(Func reducer, R initialValue) const {
        R accumulator = initialValue;
        for (const auto& item : data) {
            accumulator = reducer(accumulator, item);
        }
        return accumulator;
    }
    T& operator[](size_t index) {
        if (index >= data.size()) throw std::out_of_range("Array index out of bounds");
        return data[index];
    }
    const T& operator[](size_t index) const {
        if (index >= data.size()) throw std::out_of_range("Array index out of bounds");
        return data[index];
    }
    T at(int index) const {
        if (index < 0) index += static_cast<int>(data.size());
        if (index < 0 || index >= static_cast<int>(data.size())) throw std::out_of_range("Array index out of bounds");
        return data[index];
    }
    template<typename Func>
    T find(Func predicate) const {
        auto it = std::find_if(data.begin(), data.end(), predicate);
        if (it != data.end()) return *it;
        throw std::runtime_error("Element not found in Array.find()");
    }
    template<typename Func>
    int findIndex(Func predicate) const {
        auto it = std::find_if(data.begin(), data.end(), predicate);
        if (it != data.end()) return static_cast<int>(std::distance(data.begin(), it));
        return -1;
    }
};

class String {
public:
    static int length(const std::string& str);
    static std::string toUpperCase(const std::string& str);
    static std::string toLowerCase(const std::string& str);
    static std::string substring(const std::string& str, int start, int end);
    static int indexOf(const std::string& str, const std::string& search);
    static std::string replace(const std::string& str, const std::string& from, const std::string& to);
    static Array<std::string> split(const std::string& str, const std::string& delimiter);
    static std::string trim(const std::string& str);
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
    static std::string repeat(const std::string& str, int count);
    static std::string padStart(const std::string& str, int length, const std::string& pad = " ");
    static std::string padEnd(const std::string& str, int length, const std::string& pad = " ");
};
template<typename K, typename V>
class Map {
public:
    std::map<K, V> data;
    Map() = default;
    Map(const std::map<K, V>& d) : data(d) {}
    void set(const K& key, const V& value) {
        data[key] = value;
    }
    V get(const K& key) const {
        auto it = data.find(key);
        if (it == data.end()) throw std::runtime_error("Key not found");
        return it->second;
    }
    bool has(const K& key) const {
        return data.find(key) != data.end();
    }
    void remove(const K& key) {
        data.erase(key);
    }
    size_t size() const {
        return data.size();
    }
    void clear() {
        data.clear();
    }
    Array<K> keys() const {
        std::vector<K> result;
        for (const auto& pair : data) {
            result.push_back(pair.first);
        }
        return Array<K>(result);
    }
    Array<V> values() const {
        std::vector<V> result;
        for (const auto& pair : data) {
            result.push_back(pair.second);
        }
        return Array<V>(result);
    }
};

// Simple row type used by Database::query
struct Row {
    Map<std::string, std::string> data;

    std::string get(const std::string& column) const {
        return data.get(column);
    }
};
class Date {
public:
    static long long now();   
    static std::string toISOString(long long timestamp);
    static std::string toDateString(long long timestamp);
    static std::string toTimeString(long long timestamp);
};
class JSON {
public:
    static std::string stringify(const std::string& value);
    static std::string parse(const std::string& json);
};
class File {
public:
    static std::string readFile(const std::string& path);
    static void writeFile(const std::string& path, const std::string& content);
    static bool exists(const std::string& path);
    static void deleteFile(const std::string& path);
};
class Console {
public:
    static void log(const std::string& message);
    static void error(const std::string& message);
    static void warn(const std::string& message);
    static void info(const std::string& message);
    static void clear();
};
struct HTTPResponse {
    int statusCode;
    std::string body;
    std::map<std::string, std::string> headers;
};
class HTTP {
public:
    static HTTPResponse get(const std::string& url);
    static HTTPResponse post(const std::string& url, const std::string& body);
    static HTTPResponse put(const std::string& url, const std::string& body);
    static HTTPResponse del(const std::string& url);
    static HTTPResponse request(const std::string& method, const std::string& url, 
                               const std::string& body = "",
                               const std::map<std::string, std::string>& headers = {});
};
class Regex {
public:
    std::string pattern;
    Regex(const std::string& pat);
    bool test(const std::string& str) const;
    Array<std::string> match(const std::string& str) const;
    Array<std::string> findAll(const std::string& str) const;
    std::string replace(const std::string& str, const std::string& replacement) const;
};
class Env {
public:
    static std::string get(const std::string& name, const std::string& defaultValue = "");
    static void set(const std::string& name, const std::string& value);
    static bool has(const std::string& name);
    static std::string home();
    static std::string cwd();
};
}  
}  
