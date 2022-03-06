#include "FileHtmlFactory.hpp"

#include <fstream>
#include <map>

FileHtmlFactory::FileHtmlFactory(const std::filesystem::path& file_path) : m_path(file_path)
{
    CreateHeader();
    CreateBody();
}

void FileHtmlFactory::CreateHeader()
{
    ss << "<!doctype html>\n";
    ss << "<html>\n\n";
    ss << "<head>\n";
    ss << "    <meta charset=\"UTF-8\">\n";
    ss << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    ss << "    <link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">\n";
    ss << "    <link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>\n";
    ss << "    <link href=\"https://fonts.googleapis.com/css2?family=Roboto+Mono&display=swap\" rel=\"stylesheet\">\n";
    ss << "    <script src=\"https://cdn.tailwindcss.com\"></script>\n";
    ss << "    <link rel=\"stylesheet\" "
          "href=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.4.0/styles/github.min.css\">\n";
    ss << "    <script src=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.4.0/highlight.min.js\"></script>\n";
    ss << "</head>\n\n";
    ss << "<style>\n";
    ss << "    * {\n";
    ss << "        font-family: 'Roboto Mono' !important;\n";
    ss << "    }\n\n";
    ss << "    .file-name {\n";
    ss << "        font-family: 'SF Pro Text', -apple-system, BlinkMacSystemFont, Roboto, 'Segoe UI', Helvetica, "
          "Arial, sans-serif, 'Apple Color Emoji', 'Segoe UI Emoji', 'Segoe UI Symbol' !important;\n";
    ss << "    }\n";
    ss << "</style>\n\n";
}

void FileHtmlFactory::CreateBody()
{
    ss << "<body class=\"flex items-center justify-center w-screen h-screen\">\n";
    ss << "    <div class=\"w-[720px] h-[480px] bg-gray-100 rounded-xl shadow-xl border border-gray-200 relative "
          "overflow-hidden\">\n";
    ss << "        <div class=\"flex p-2 h-8 items-center absolute\">\n";
    ss << "            <div class=\"h-[14px] w-[14px] rounded-full bg-[#fe6055] border-[1px] border-gray-200 "
          "mr-1\"></div>\n";
    ss << "            <div class=\"h-[14px] w-[14px] rounded-full bg-[#ffbc2d] border-[1px] border-gray-200 "
          "mr-1\"></div>\n";
    ss << "            <div class=\"h-[14px] w-[14px] rounded-full bg-[#25c73e] border-[1px] "
          "border-gray-200\"></div>\n";
    ss << "        </div>\n";
    ss << "        <div class=\"file-name flex justify-center p-1 h-8 font-semibold border-b border-gray-200 "
          "items-center\">\n";
    ss << "            " << m_path.filename().string() << '\n';
    ss << "        </div>\n";
    ss << "        <div class=\"bg-white h-full overflow-scroll p-2 text-sm\">\n";
    ss << "            <pre>\n";
    ss << "<code lang=\"\" class=\"!bg-white\">";
    ss << GetFileContent(m_path.string()) << "</code>\n";
    ss << "            </pre>\n";
    ss << "        </div>\n";
    ss << "    </div>\n";
    ss << "    <script>\n";
    ss << "        window.onload = function () {\n";
    ss << "            hljs.highlightAll();\n";
    ss << "        }\n";
    ss << "    </script>\n";
    ss << "</body>\n\n";
    ss << "</html>\n";
}

void FileHtmlFactory::ReplaceAllOccurences(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

std::string FileHtmlFactory::GetFileContent(const std::string& path)
{
    std::string file_content{};
    std::ifstream file_stream(path);
    if (!file_stream.is_open())
        return "";
    std::getline(file_stream, file_content, '\0');
    ReplaceAllOccurences(file_content, "<", "&lt;");
    ReplaceAllOccurences(file_content, ">", "&gt;");
    return file_content;
}

std::string FileHtmlFactory::GetHtml() { return ss.str(); }