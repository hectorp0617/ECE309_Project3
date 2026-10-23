#ifndef SENTINEL_SCANNER_H
#define SENTINEL_SCANNER_H

#include <string>
#include <string_view>
#include <stdexcept>

class SentinelScanner {
public:
    // Constructor that initializes the scanner with a string view
    explicit SentinelScanner(std::string sentinel) 
    : sentinel_(sentinel){
    if (sentinel_.empty()) {
            throw std::invalid_argument("Sentinel cannot be empty");
    }
    }

        struct Out {
            std::string safe_text;
            bool sentinel_found;        // Indicates if the sentinel was found in the chunk
        };
            
        // Feed a chunk of text to the scanner and check for the sentinel
        Out feed(std::string_view chunk);
        Out flush();

private: 
    std::string sentinel_;
    std::string pending_;       

    friend struct SentinelScannerTest; // Grant access to the test struct for testing purposes

};

#endif // SENTINEL_SCANNER_H