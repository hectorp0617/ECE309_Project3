// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <cstdio>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>        

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}

// ------------------------------------------------------------- your tests
//
// Still to write (spec §6 has the full list). Suggested names:
//   reg_null_rejected, reg_for_each_visits_each_once
//   reg_10000_tools               REQUIRED: print bucket_count / load_factor /
//                                 max_probe_length, check the bound
//   reg_alternative_hash          REQUIRED: longest chain of a weak hash
//   ws_<case>                     REQUIRED: every row of the table in spec §4.4
//   calc_<case>                   left-to-right, parentheses, negatives

// Tests added below

// Test that adding a null tool is rejected and does not change the size of the registry
TEST(reg_null_rejected){
    ToolRegistry r;

    CHECK(r.size() == 0);   // Initial size should be 0
    CHECK(!r.add(nullptr)); // Adding a null tool should return false
    CHECK(r.size() == 0);   // Size should remain 0
    CHECK(r.find("missing") == nullptr); // Finding any tool should return nullptr
}

// Test that for_each visits each registered tool exactly once
TEST(reg_for_each_visits_each_once){
    ToolRegistry r;
    auto t1 = std::make_unique<NamedTool>("alpha");
    auto t2 = std::make_unique<NamedTool>("beta");
    auto t3 = std::make_unique<NamedTool>("gamma");

    REQUIRE(r.add(std::move(t1)));
    REQUIRE(r.add(std::move(t2)));
    REQUIRE(r.add(std::move(t3)));

    std::vector<std::string> visited;
    r.for_each([&visited](const Tool& tool) {
        visited.push_back(tool.name());
    });

    // Check that all tools were visited exactly once
    CHECK(visited.size() == 3);
    CHECK(std::find(visited.begin(), visited.end(), "alpha") != visited.end());
    CHECK(std::find(visited.begin(), visited.end(), "beta") != visited.end());
    CHECK(std::find(visited.begin(), visited.end(), "gamma") != visited.end());
}

// Test that adding 10,000 tools maintains the load factor <= 1.0 and prints bucket_count, load_factor, and max_probe_length
TEST(reg_10000_tools){
    ToolRegistry r;
    Tool* first = nullptr;

    // Add 10,000 tools to the registry
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        REQUIRE(r.add(std::make_unique<NamedTool>(name)));

    
        if (i == 0) {
            first = r.find(name); // Store the first tool for later verification
            REQUIRE(first != nullptr); // Ensure the first tool was added successfully
        }
        CHECK(r.load_factor() <= 1.0); // Ensure load factor does not exceed 1.0
    }



    CHECK(r.size() == 10000); // Ensure the size is 10,000

    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        Tool* tool = r.find(name);
        REQUIRE(tool != nullptr);
        CHECK(tool->name() == name);
    }

    CHECK(r.find("tool_0") == first); // Ensure the first tool is still the same object
    CHECK(r.max_probe_length() <= 64); // Ensure the max probe length does not exceed 64


    // Print bucket_count, load_factor, and max_probe_length for instrumentation
    std::printf("Bucket count: %zu\n", r.bucket_count());
    std::printf("Load factor: %.6f\n", r.load_factor());
    std::printf("Max probe length: %zu\n", r.max_probe_length());

}       //END of 10000 tools test

TEST(reg_alt_hash) {
    ToolRegistry r;

    // Add 10,000 tools with names that are likely to collide under a weak hash function
    for (int i = 0; i < 10000; ++i) {
        REQUIRE(r.add(std::make_unique<NamedTool>("tool_" + std::to_string(i))));
    }

    std::vector<int> counts(r.bucket_count(), 0);

    // Count the number of tools in each bucket
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        std::size_t weak_hash = static_cast<unsigned char>(name[0]) + name.size(); // Weak hash function

        ++counts[weak_hash % r.bucket_count()];     // Increment the count for the corresponding bucket
    }

    int longest = 0;

    // Find the length of the longest chain (bucket) under the weak hash function
    for (int count : counts) {
        if (count > longest) {
            longest = count; // Update the longest chain length if the current bucket's count is greater
        }
    }

    // Print the instrumentation results
    std::printf("Bucket count: %zu\n", r.bucket_count());
    std::printf("Load factor: %.6f\n", r.load_factor());
    std::printf("Max probe length: %zu\n", r.max_probe_length());
    std::printf("Longest chain under weak hash: %d\n", longest);
    CHECK(longest > 1); // Ensure that there is at least one collision under the

}


// Test that the workspace accepts valid paths and normalizes them correctly
TEST(ws_accepts_valid_paths) {
    TempDir d;
    Workspace ws(d.path());

    // Test cases for valid paths and their expected normalized forms
    struct Case {
        std::string user_path;
        fs::path expected;
    };

    // Define a vector of test cases with user paths and their expected normalized forms
    std::vector<Case> cases = {
        {"notes/todo.txt", ws.root() / "notes/todo.txt"},
        {"./notes//todo.txt", ws.root() / "notes/todo.txt"},
        {".", ws.root()},
        {"a/../b", ws.root() / "b"},
        {"a/..", ws.root()},
        {"new/dir/file.txt", ws.root() / "new/dir/file.txt"},
        {"my..notes.txt", ws.root() / "my..notes.txt"}
    };

    for (const auto& c : cases) {
        fs::path out;
        std::string reason;
        REQUIRE(ws.resolve(c.user_path, &out, &reason)); // Ensure the path resolves successfully
        CHECK(out == c.expected); // Check that the resolved path matches the expected normalized form
        
    }
}       //END of ws_accepts_valid_paths test

TEST(ws_rejects_invalid_paths) {
    TempDir d;
    Workspace ws(d.path());

    // Test cases for invalid paths and their expected rejection reasons
    struct Case {
        std::string user_path;
        std::string expected_reason;
    };

    // Define a vector of test cases with user paths and their expected rejection reasons
    std::vector<Case> cases = {
        {"", "empty path"},
        {std::string("notes") + '\0' + "/x",
        "Path contains null character"},
        {"/etc/passwd", "absolute path not allowed"},
        {"..", "path escapes workspace"},
        {"../etc/passwd", "path escapes workspace"},
        {"a/../../b", "path escapes workspace"},
        {"notes/../../ws/notes/todo.txt", "path escapes workspace"}
    };

    for (const auto& c : cases) {
        fs::path out;
        std::string reason;
        CHECK(!ws.resolve(c.user_path, &out, &reason)); // Ensure the path is rejected
        CHECK(reason == c.expected_reason); // Check that the rejection reason matches the expected reason
        CHECK(!reason.empty()); // Ensure that the reason is not empty
    }
}       //END of ws_rejects_invalid_paths test

int main(int argc, char** argv) { return mt::run_all(argc, argv); }
