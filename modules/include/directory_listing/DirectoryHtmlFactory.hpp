#pragma once

#include <filesystem>
#include <sstream>

class DirectoryHtmlFactory {
private:
    std::ostringstream ss{};
    const std::filesystem::path &m_path;

    void CreateHeader();
    void CreateBodyOpening();
    void CreateBodyEnding();
    void AddElement(const std::filesystem::path &element);

    static std::string GetFileIcon(const std::string &name, const std::string &ext);
    static std::string GetFolderIcon(const std::string &name);

public:
    DirectoryHtmlFactory(const std::filesystem::path &path);
    ~DirectoryHtmlFactory() = default;
    std::string GetHtml();
};