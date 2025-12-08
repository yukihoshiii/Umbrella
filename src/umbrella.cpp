#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif
#include <limits.h>
#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "compiler/codegen.h"
using namespace umbrella;
std::string getExecutablePath() {
    char path[PATH_MAX];
#ifdef __APPLE__
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) == 0) {
        return std::string(path);
    }
#else
    ssize_t count = readlink("/proc/self/exe", path, PATH_MAX);
    if (count != -1) {
        path[count] = '\0';
        return std::string(path);
    }
#endif
    return "";
}
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
void writeFile(const std::string& filename, const std::string& content) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not write to file: " + filename);
    }
    file << content;
}
void printVersion() {
    std::cout << "Umbrella Programming Language Compiler v1.0.0" << std::endl;
    std::cout << "Copyright (c) 2025 Umbrella Programming Language" << std::endl;
    std::cout << "MIT License - https://github.com/umbrella-lang/umbrella" << std::endl;
}
void printHelp() {
    std::cout << "Umbrella Programming Language Compiler" << std::endl;
    std::cout << "Usage: ./umbrella <input.umb> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -o <output>     Specify output executable name (default: a.out)" << std::endl;
    std::cout << "  --emit-cpp      Only generate C++ code without compiling" << std::endl;
    std::cout << "  --verbose       Show detailed compilation steps" << std::endl;
    std::cout << "  --version       Show version information" << std::endl;
    std::cout << "  --help          Show this help message" << std::endl;
}
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Error: No input file specified" << std::endl;
        printHelp();
        return 1;
    }
    if (std::string(argv[1]) == "--version" || std::string(argv[1]) == "-v") {
        printVersion();
        return 0;
    }
    std::string inputFile;
    std::string outputFile = "a.out";
    bool emitCppOnly = false;
    bool verbose = true;
    bool run = true;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--help") {
            printHelp();
            return 0;
        } else if (arg == "--emit-cpp") {
            emitCppOnly = true;
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg == "--run") {
            run = true;
        } else if (arg == "--no-run") {
            run = false;
        } else if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        } else if (arg[0] != '-') {
            inputFile = arg;
        }
    }
    if (inputFile.empty()) {
        std::cerr << "Error: No input file specified\n";
        printHelp();
        return 1;
    }
    try {
        if (verbose) {
            std::cout << "Reading source file: " << inputFile << std::endl;
            std::cout.flush();
        }
        std::string source = readFile(inputFile);
        if (verbose) {
            std::cout << "Lexical analysis..." << std::endl;
            std::cout.flush();
        }
        Lexer lexer(source);
        std::vector<Token> tokens = lexer.tokenize();
        if (verbose) {
            std::cout << "Generated " << tokens.size() << " tokens" << std::endl;
            std::cout.flush();
        }
        if (verbose) {
            std::cout << "Parsing..." << std::endl;
            std::cout.flush();
        }
        Parser parser(tokens);
        auto program = parser.parse();
        if (verbose) {
            std::cout << "AST generated successfully" << std::endl;
            std::cout.flush();
        }
        if (verbose) {
            std::cout << "Generating C++ code..." << std::endl;
            std::cout.flush();
        }
        CodeGenerator codegen;
        std::string cppCode = codegen.generate(*program);
        if (cppCode.find("int main(") == std::string::npos) {
            cppCode += "\nint main() {\n    return 0;\n}\n";
        }
        std::string cppFile = "/tmp/umbrella_temp.cpp";
        writeFile(cppFile, cppCode);
        if (verbose || emitCppOnly) {
            std::cout << "Generated C++ code:\n";
            std::cout << "-------------------\n";
            std::cout << cppCode;
            std::cout << "-------------------\n";
        }
        if (emitCppOnly) {
            std::cout << "C++ code written to: " << cppFile << "\n";
            return 0;
        }
        if (verbose) {
            std::cout << "Compiling to native code..." << std::endl;
            std::cout.flush();
        }
        std::string compilerPath = getExecutablePath();
        if (compilerPath.empty()) {
            compilerPath = argv[0];
        }
        size_t lastSlash = compilerPath.find_last_of("/\\");
        std::string compilerDir = (lastSlash != std::string::npos) 
            ? compilerPath.substr(0, lastSlash) 
            : ".";
        std::stringstream compileCmd;
        compileCmd << "g++ -std=c++17 ";
        compileCmd << "-I" << compilerDir << "/../src ";
        compileCmd << cppFile << " ";
        compileCmd << compilerDir << "/../src/runtime/runtime.cpp ";
        compileCmd << "-o " << outputFile;
        if (verbose) {
            std::cout << "Compile command: " << compileCmd.str() << "\n";
        }
        int result = system(compileCmd.str().c_str());
        if (result != 0) {
            std::cerr << "Error: Compilation failed\n";
            return 1;
        }
        if (verbose) {
            std::cout << "Compilation successful!" << std::endl;
            std::cout.flush();
        }
        std::cout << "Output written to: " << outputFile << "\n";
        if (run) {
            if (verbose) {
                std::cout << "-------------------" << std::endl;
                std::cout << "Running program..." << std::endl;
                std::cout.flush();
            }
            std::string runCmd = "./" + outputFile;
            if (outputFile.find("/") != std::string::npos) {
                runCmd = outputFile;
            }
            int ret = system(runCmd.c_str());
            (void)ret;  
        }
        remove(cppFile.c_str());
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
