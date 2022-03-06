#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace zia {

class FileWatcher {
public:
    FileWatcher(const std::filesystem::path &fileToWatch);
    FileWatcher(const std::vector<std::filesystem::path> &filesToWatch);

    void AddFile(const std::filesystem::path &fileToWatch);
    bool HasChanged() const;
    void Update();
    std::vector<std::filesystem::path> GetModifications() const;

private:
    std::vector<std::tuple<std::filesystem::path, long>> filesToWatch_;
};

}  // namespace zia
