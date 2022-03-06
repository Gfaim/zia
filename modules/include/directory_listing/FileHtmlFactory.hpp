#pragma once

#include <filesystem>
#include <sstream>

class FileHtmlFactory {
private:
    std::ostringstream ss;
    const std::filesystem::path &m_path;

    void CreateHeader();
    void CreateBodyOpening();
    void CreateBodyEnding();

    static std::string GetLanguageName(const std::string &ext);

public:
    FileHtmlFactory(const std::filesystem::path &file_path);
    ~FileHtmlFactory() = default;
    std::string GetHtml();
};