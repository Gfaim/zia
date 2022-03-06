#include "FileHtmlFactory.hpp"

#include <map>

FileHtmlFactory::FileHtmlFactory(const std::filesystem::path &file_path) : m_path(file_path)
{
    CreateHeader();
    CreateBodyOpening();
    CreateBodyEnding();
}

void FileHtmlFactory::CreateHeader()
{
    ss << "<!doctype html>\n";
    ss << "<html>\n\n";
    ss << "<head>\n";
    ss << "    <meta charset=\"UTF-8\">\n";
    ss << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    ss << "    <script src=\"https://cdn.tailwindcss.com\"></script>\n";
    ss << "</head>\n\n";
    ss << "<style>\n";
    ss << "    * {\n";
    ss << "        font-family: 'SF Pro Text', -apple-system, BlinkMacSystemFont, Roboto, 'Segoe UI', Helvetica, "
          "Arial, sans-serif, 'Apple Color Emoji', 'Segoe UI Emoji', 'Segoe UI Symbol';\n";
    ss << "overflow: scroll;\n";
    ss << "    }\n";
    ss << "</style>\n\n";
}

void FileHtmlFactory::CreateBodyOpening()
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
    ss << "        <div class=\"flex justify-center p-1 h-8 font-semibold border-b border-gray-200 items-center\">\n";
    ss << "            <img class=\"h-5 w-5 mx-2\"\n";
    ss << "                "
          "src=\"https://media.macosicons.com/parse/files/macOSicons/"
          "af21153d07a2e92bde7b2ad155055489_low_res_1619092574091.png\">\n";
    ss << "            " << m_path.filename().string() << '\n';
    ss << "        </div>\n";
    ss << "        <div class=\"bg-white h-full\">\n";
    if (m_path != "" and m_path != "/" and m_path != "./") {
        auto parent_path = m_path.parent_path().string();
        if (!parent_path.empty() and parent_path.front() == '.')
            parent_path.erase(0, 1);
        if (parent_path.empty())
            parent_path.push_back('/');
        ss << "            <a href=\"" << parent_path << "\"\n";
        ss << "                class=\"h-6 even:bg-gray-50 flex items-center text-sm cursor-pointer hover:bg-[#0156dd] "
              "hover:text-white\">\n";
        ss << "                <img class=\"h-5 w-5 mx-2\"\n";
        ss << "                    "
              "src=\"https://media.macosicons.com/parse/files/macOSicons/"
              "af21153d07a2e92bde7b2ad155055489_low_res_1619092574091.png\">\n";
        ss << "                ..\n";
        ss << "            </a>\n";
    }
}

void FileHtmlFactory::CreateBodyEnding()
{
    ss << "        </div>\n";
    ss << "    </div>\n";
    ss << "</body>\n\n";
    ss << "</html>\n";
}

std::string FileHtmlFactory::GetLanguageName(const std::string &ext) { return ""; }

std::string FileHtmlFactory::GetHtml() { return ss.str(); }