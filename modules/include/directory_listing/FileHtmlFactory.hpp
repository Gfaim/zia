#pragma once

#include <filesystem>
#include <sstream>

class FileHtmlFactory {
private:
    std::ostringstream ss{};
    const std::filesystem::path &m_path;

    void CreateHeader();
    void CreateBody();

    static std::string GetFileContent(const std::string &path);

public:
    FileHtmlFactory(const std::filesystem::path &file_path);
    ~FileHtmlFactory() = default;
    std::string GetHtml();
};