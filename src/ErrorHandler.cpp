#include "ErrorHandler.hpp"

namespace sicxe {

void ErrorHandler::report(int line, ErrorSeverity severity, const std::string& message) {
    diagnostics.emplace_back(line, severity, message);
    if (severity == ErrorSeverity::ERROR) errorCount++;
    else if (severity == ErrorSeverity::WARNING) warningCount++;
}

void ErrorHandler::printReport() const {
    for (const auto& diag : diagnostics) {
        std::cout << diag.toString() << std::endl;
    }
    std::cout << "Assembly complete: " << errorCount << " errors, " 
              << warningCount << " warnings." << std::endl;
}

} // namespace sicxe