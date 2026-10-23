#include "core/sentinel_scanner.h"
#include <algorithm>
#include <stdexcept>


SentinelScanner::Out SentinelScanner::feed(std::string_view chunk) {
    std::string combined = pending_;
    combined.append(chunk);

    //Handle the search result
    std::size_t pos = combined.find(sentinel_);
    if (pos != std::string::npos) {
        // Sentinel found, return the safe text and set sentinel_found to true
        std::string safe_text = combined.substr(0, pos);
        pending_.clear();
        return {safe_text, true};
    }

    std::size_t keep = std::min(combined.size(), sentinel_.size() - 1);
    std::size_t start = combined.size() - keep;

    std::string safe_text = combined.substr(0, start);
    pending_ = combined.substr(start);

    return {safe_text, false};
}       //End of feed

SentinelScanner::Out SentinelScanner::flush() {
    std::string safe_text = pending_;
    pending_.clear();
    return {safe_text, false};
}       //End of flush



