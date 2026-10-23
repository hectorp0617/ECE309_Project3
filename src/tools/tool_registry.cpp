// src/tools/tool_registry.cpp — YOU IMPLEMENT (spec §4.3).
//
// A hand-written hash table. No std::unordered_map / std::map / std::set.
// Every function below is a stub so the starter compiles. Replace each body.

#include "tools/tool_registry.h"
#include <cstdint>
#include <utility>



// ToolRegistry::ToolRegistry() = default;    // TODO: create your initial buckets
//LINE ABOVE REPLACED WITH BELOW LINE
ToolRegistry::ToolRegistry() : buckets(16) {}       //Initializes the buckets vector with 16 empty buckets (vectors of unique_ptr<Tool>)


ToolRegistry::~ToolRegistry() = default;   // unique_ptrs in your buckets free the tools


//rehash
void ToolRegistry::rehash(std::size_t new_bucket_count) {
    std::vector<std::vector<std::unique_ptr<Tool>>> new_buckets(new_bucket_count); // Create a new vector of buckets with the specified size

    for (auto& bucket : buckets) { // Iterate through each existing bucket
        for (auto& tool_ptr : bucket) { // Iterate through each tool in the bucket
            if (tool_ptr) { // Check if the tool pointer is valid
                std::size_t new_index = hash(tool_ptr->name()) % new_bucket_count; // Calculate the new index for the tool in the new buckets
                new_buckets[new_index].push_back(std::move(tool_ptr)); // Move the tool to the new bucket
            }
        }
        buckets = std::move(new_buckets); // Replace the old buckets with the new buckets
    }
}
//end rehash


//hash
std::size_t ToolRegistry::hash(std::string_view key) noexcept {
    std::uint64_t h = 14695981039346656037ULL;

    for (unsigned char c : key) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ULL;
    }

    return static_cast<std::size_t>(h);
}
//end hash


// Takes ownership of `tool` and indexes it under tool->name().
// Returns false (and destroys `tool`) if `tool` is null or the name is
// already registered; the existing tool is left untouched.
bool ToolRegistry::add(std::unique_ptr<Tool> tool) {
    if (!tool) {
        return false;
    }

    if (find(tool->name()) != nullptr) {
        return false;
    }

    if (element_count + 1 > buckets.size()) {
        rehash(buckets.size() * 2); // Double the number of buckets if the load factor exceeds 1.0
    }

    std::size_t index = hash(tool->name()) % buckets.size();
    buckets[index].push_back(std::move(tool));
    ++element_count;

    return true;
}
//end add





// Find the tool registered under `name`, or nullptr if not found.
Tool* ToolRegistry::find(std::string_view name) const {
    std::size_t index = hash(name) % buckets.size(); // Calculate the bucket index using the hash function
    const auto& bucket = buckets[index]; // Get the corresponding bucket

    for (const auto& tool_ptr : bucket) { // Iterate through the tools in the bucket
        if (tool_ptr && tool_ptr->name() == name) { // Check if the tool's name matches
            return tool_ptr.get(); // Return a raw pointer to the found tool
        }
    }

    return nullptr; // Return nullptr if the tool is not found
}
//End find




std::size_t ToolRegistry::size() const noexcept {
    return element_count;   // TODO
}

// Calls fn once per registered tool. Order is unspecified (callers that
// need a stable order, like --tools, sort the names themselves).
void ToolRegistry::for_each(const std::function<void(const Tool&)>& fn) const {
    
    for (const auto& bucket : buckets) { // Iterate through each bucket
        for (const auto& tool_ptr : bucket) { // Iterate through each tool in the bucket
            if (tool_ptr) { // Check if the tool pointer is valid
                fn(*tool_ptr); // Call the provided function with the dereferenced tool
            }
        }
    }
}
//end for_each

std::size_t ToolRegistry::bucket_count() const noexcept {
    return buckets.size();   
}

double ToolRegistry::load_factor() const noexcept {
    // Calculate the load factor as the ratio of the number of registered tools to the number of buckets
    return static_cast<double>(element_count) / static_cast<double>(buckets.size());
}

std::size_t ToolRegistry::max_probe_length() const {
    return 0;   // TODO
}
