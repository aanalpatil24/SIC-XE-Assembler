#ifndef ERRORHANDLER_HPP
#define ERRORHANDLER_HPP

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include "Types.hpp" // Required so the compiler knows what AssemblyError is

namespace sicxe {

enum class ErrorSeverity {
    WARNING,
    ERROR,
    FATAL
};

// Stores error context for the final listing file generation
struct Diagnostic {
    int line;
    ErrorSeverity severity;
    std::string message;
    
    Diagnostic(int ln, ErrorSeverity sev, const std::string& msg)
        : line(ln), severity(sev), message(msg) {}
    
    std::string toString() const {
        std::stringstream ss;
        ss << "Line " << line << " [";
        switch(severity) {
            case ErrorSeverity::WARNING: ss << "WARNING"; break;
            case ErrorSeverity::ERROR: ss << "ERROR"; break;
            case ErrorSeverity::FATAL: ss << "FATAL"; break;
        }
        ss << "]: " << message;
        return ss.str();
    }
};

class ErrorHandler {
private:
    std::vector<Diagnostic> diagnostics;
    int errorCount;
    int warningCount;
    
public:
    ErrorHandler() : errorCount(0), warningCount(0) {}
    
    void report(int line, ErrorSeverity severity, const std::string& message);
    void error(int line, const std::string& msg) { 
        report(line, ErrorSeverity::ERROR, msg); 
        errorCount++;
    }
    void warning(int line, const std::string& msg) { 
        report(line, ErrorSeverity::WARNING, msg); 
        warningCount++;
    }
    void fatal(int line, const std::string& msg) { 
        report(line, ErrorSeverity::FATAL, msg); 
        errorCount++;
        throw AssemblyError(msg, line); // Halts execution entirely
    }
    
    bool hasErrors() const { return errorCount > 0; }
    int getErrorCount() const { return errorCount; }
    int getWarningCount() const { return warningCount; }
    
    void printReport() const;
    const auto& getDiagnostics() const { return diagnostics; }
    void clear() { diagnostics.clear(); errorCount = 0; warningCount = 0; }
};

} // namespace sicxe
#endif