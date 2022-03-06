#pragma once

#include <filesystem>
#include <sstream>

class FileHtmlFactory {
private:
    std::ostringstream ss{};
    const std::filesystem::path &m_path;

    void CreateHeader();
    void CreateBody();

    static void ReplaceAllOccurences(std::string &str, const std::string &from, const std::string &to);

public:
    FileHtmlFactory(const std::filesystem::path &file_path);
    ~FileHtmlFactory() = default;
    std::string GetHtml();
    static std::string GetFileContent(const std::string &path);
};