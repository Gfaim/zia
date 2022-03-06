#include "FileWatcher.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <iostream>

namespace zia {

void FileWatcher::AddFile(const std::filesystem::path &fileToWatch)
{
    struct stat sb;

    stat(fileToWatch.c_str(), &sb);
    filesToWatch_.push_back(std::make_tuple(fileToWatch, sb.st_mtim.tv_nsec));
}

FileWatcher::FileWatcher(const std::filesystem::path &fileToWatch) : filesToWatch_() { AddFile(fileToWatch); }

FileWatcher::FileWatcher(const std::vector<std::filesystem::path> &filesToWatch) : filesToWatch_()
{
    for (const auto &fileToWatch : filesToWatch) {
        AddFile(fileToWatch);
    }
}

bool FileWatcher::HasChanged() const
{
    for (auto &file : filesToWatch_) {
        struct stat sb;

        stat(std::get<0>(file).c_str(), &sb);
        if (sb.st_mtim.tv_nsec != std::get<1>(file)) {
            return true;
        }
    }
    return false;
}

void FileWatcher::Update()
{
    for (auto &file : filesToWatch_) {
        struct stat sb;

        stat(std::get<0>(file).c_str(), &sb);
        if (sb.st_mtim.tv_nsec != std::get<1>(file)) {
            std::get<1>(file) = sb.st_mtim.tv_nsec;
        }
    }
}

std::vector<std::filesystem::path> FileWatcher::GetModifications() const
{
    std::vector<std::filesystem::path> modifications;

    for (const auto &file : filesToWatch_) {
        struct stat sb;

        stat(std::get<0>(file).c_str(), &sb);
        if (sb.st_mtim.tv_nsec != std::get<1>(file)) {
            modifications.push_back(std::get<0>(file));
        }
    }
    return modifications;
}

}  // namespace zia
