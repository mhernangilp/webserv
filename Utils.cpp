// Utils.cpp
#include "Utils.hpp"

std::string removeDuplicateSlashes(const std::string& path) {
    std::string result;
    bool lastWasSlash = false;

    for (size_t i = 0; i < path.size(); ++i) {
        if (path[i] == '/') {
            if (!lastWasSlash) {
                result += path[i];
            }
            lastWasSlash = true;
        } else {
            result += path[i];
            lastWasSlash = false;
        }
    }
    return result;
}

std::string normalizeUrl(const std::string& url) {
    std::string normalized = removeDuplicateSlashes(url);

    // Eliminar barras al final
    size_t end = normalized.length();
    while (end > 0 && normalized[end - 1] == '/') {
        --end;
    }

    return normalized.substr(0, end);
}
