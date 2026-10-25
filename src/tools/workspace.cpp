// src/tools/workspace.cpp — YOU IMPLEMENT (spec §4.4).
//
// Workspace::resolve: a purely lexical check, steps 1–6 of the spec.
// Do not touch the filesystem here (no exists(), canonical(), ...).
// The constructor, root() and check_symlinks() are provided in
// workspace_provided.cpp.

#include "tools/workspace.h"

namespace fs = std::filesystem;

bool Workspace::resolve(std::string_view user_path, fs::path* out,
                        std::string* reason) const {
    // *reason = "Workspace::resolve not implemented yet";   // TODO

    //ADDED PORTION BELOW

    // Check if the user_path is empty or contains null characters
    if (user_path.empty()) {
        *reason = "empty path";
        return false;
    }

    // Check for null characters in the user_path
    if (user_path.find('\0') != std::string_view::npos) {
        *reason = "Path contains null character";
        return false;
    }

    fs::path relative_path(user_path); // Convert user_path to a filesystem path

    // Check if the path is absolute or has a root path (e.g., starts with '/')
    if (relative_path.is_absolute() || relative_path.has_root_path()) {
        *reason = "absolute path not allowed";
        return false;
    }

    fs::path n = relative_path.lexically_normal(); // Normalize the path to remove redundant elements

    // Check if the normalized path is empty or starts with ".." (parent directory)
    if (!n.empty() && *n.begin() == "..") {
        *reason = "path escapes workspace";
        return false;
    }

    if (n == ".") {
        *out = root(); // If the normalized path is ".", set out to the root of the workspace
    } else {
        *out = root() / n; // Otherwise, append the normalized path to the root
    }

    return true; // Resolution successful
}
