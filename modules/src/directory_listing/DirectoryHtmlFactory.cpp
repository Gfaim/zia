#include "DirectoryHtmlFactory.hpp"

#include <map>

DirectoryHtmlFactory::DirectoryHtmlFactory(const std::filesystem::path &path) : m_path(path)
{
    CreateHeader();
    CreateBodyOpening();
    for (const auto &entry : std::filesystem::directory_iterator(path)) AddElement(entry);
    CreateBodyEnding();
}

void DirectoryHtmlFactory::CreateHeader()
{
    ss << "<!doctype html>\n";
    ss << "<html>\n\n";
    ss << "<head>\n";
    ss << "    <meta charset=\"UTF-8\">\n";
    ss << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    ss << "    <script src=\"https://cdn.tailwindcss.com\"></script>\n";
    ss << "    <link rel=\"shortcut icon\" href=\"/favicon.ico\">";
    ss << "</head>\n\n";
    ss << "<style>\n";
    ss << "    * {\n";
    ss << "        font-family: 'SF Pro Text', -apple-system, BlinkMacSystemFont, Roboto, 'Segoe UI', Helvetica, "
          "Arial, sans-serif, 'Apple Color Emoji', 'Segoe UI Emoji', 'Segoe UI Symbol';\n";
    ss << "overflow: scroll;\n";
    ss << "    }\n";
    ss << "</style>\n\n";
}

void DirectoryHtmlFactory::CreateBodyOpening()
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

void DirectoryHtmlFactory::CreateBodyEnding()
{
    ss << "        </div>\n";
    ss << "    </div>\n";
    ss << "</body>\n\n";
    ss << "</html>\n";
}

void DirectoryHtmlFactory::AddElement(const std::filesystem::path &element)
{
    auto path = element.string();
    if (!path.empty() and path.front() == '.')
        path.erase(0, 1);

    ss << "            <a href=\"" << path << "\"\n";
    ss << "                class=\"h-6 even:bg-gray-50 flex items-center text-sm cursor-pointer hover:bg-[#0156dd] "
          "hover:text-white\">\n";
    ss << "                <img class=\"h-5 w-5 mx-2\"\n";
    if (std::filesystem::is_directory(element)) {
        ss << "                    src=\"" << GetFolderIcon(element.stem().string()) << "\">\n";
    } else {
        ss << "                    src=\"" << GetFileIcon(element.stem().string(), element.extension().string())
           << "\">\n";
    }
    ss << "                " << element.filename().string() << '\n';
    ss << "            </a>\n";
}

std::string DirectoryHtmlFactory::GetFileIcon(const std::string &name, const std::string &ext)
{
    const std::string macosicons_path = "https://media.macosicons.com/parse/files/macOSicons/";
    static const std::map<std::string, std::string> file_icon_ext_map{
        {".sh", macosicons_path + "27245bb3e486a32b72b019e48a877753_low_res_Terminal.png"},
        {".bat", macosicons_path + "27245bb3e486a32b72b019e48a877753_low_res_Terminal.png"},
        {".py", macosicons_path + "d5883eb8dabd513e9420f6393933a4e0_low_res_Python.png"},
        {".png", macosicons_path + "faed70674a981a736803370d05622d9b_low_res_Imagine.png"},
        {".jpg", macosicons_path + "faed70674a981a736803370d05622d9b_low_res_Imagine.png"},
        {".jpeg", macosicons_path + "faed70674a981a736803370d05622d9b_low_res_Imagine.png"},
        {".md", "https://cdn-icons-png.flaticon.com/512/2991/2991108.png"},
        {".json", "https://icons-for-free.com/iconfiles/png/128/vscode+icons+type+light+json-1324451356742329907.png"},
        {".yaml", "https://icons-for-free.com/iconfiles/png/128/vscode+icons+type+light+json-1324451356742329907.png"},
        {".yml", "https://icons-for-free.com/iconfiles/png/128/vscode+icons+type+light+json-1324451356742329907.png"},
        {".cmake", "https://symbols.getvecta.com/stencil_77/1_cmake-icon.1db4b46889.png"},
        {".dll", "https://cdn-icons-png.flaticon.com/512/167/167186.png"},
        {".so", "https://cdn-icons-png.flaticon.com/512/167/167186.png"},
        {".dylib", "https://cdn-icons-png.flaticon.com/512/167/167186.png"},
        {".rb", "https://www.codementor.io/assets/topic/category_header/ruby-on-rails.png"},
        {".html", "https://cdn-icons-png.flaticon.com/512/732/732212.png"},
        {".c", "https://anthony-thillerot.fr/static/img/c-programming.png"},
        {".h", "https://anthony-thillerot.fr/static/img/c-programming.png"},
        {".cpp", "https://themanis.fr/wp-content/uploads/2021/01/52.png"},
        {".hpp", "https://themanis.fr/wp-content/uploads/2021/01/52.png"}};

    if (file_icon_ext_map.contains(ext))
        return file_icon_ext_map.at(ext);

    static const std::map<std::string, std::string> file_icon_name_map{
        {".gitignore", macosicons_path + "46d4dd5198af49d43e1d6a17e2216268_Git.png"},
        {".gitkeep", macosicons_path + "46d4dd5198af49d43e1d6a17e2216268_Git.png"},
        {".dockerignore", "https://cdn-icons-png.flaticon.com/512/5969/5969059.png"},
        {"Dockerfile", "https://cdn-icons-png.flaticon.com/512/5969/5969059.png"},
        {"Makefile", "https://cdn-icons-png.flaticon.com/512/999/999292.png"},
        {"CMakeLists", "https://symbols.getvecta.com/stencil_77/1_cmake-icon.1db4b46889.png"}};

    if (file_icon_name_map.contains(name))
        return file_icon_name_map.at(name);
    return macosicons_path + "eb678a62479f3a23eaa4c1e6149ade15_low_res_File_TXT.png";
}

std::string DirectoryHtmlFactory::GetFolderIcon(const std::string &name)
{
    const std::string macosicons_path = "https://media.macosicons.com/parse/files/macOSicons/";

    static const std::map<std::string, std::string> folder_icon_name_map{
        {"build", macosicons_path + "e56c36ad6af3fb967cdf1b5fe70c87c4_low_res_Folder___Build.png"},
        {"docs", macosicons_path + "8f216ac9b80618bbb916c41dbe941bac_low_res_Books_Folder.png"},
        {"data", macosicons_path + "b7f3eba9869b35b421a4ea17afa17b99_low_res_Database_Folder.png"},
        {".git", macosicons_path + "ab1e083d44e61bc104f978d6802a05b9_low_res_Git___Folder.png"},
        {".github", macosicons_path + "4ed6898c6ad4d5a6ebed7864c0be6314_low_res_Github___Folder.png"},
        {".vscode", macosicons_path + "bc2f8c72431f0c35cb4da9911a292779_low_res_Visual_Studio_Code_Folder.png"}};

    if (folder_icon_name_map.contains(name))
        return folder_icon_name_map.at(name);
    return macosicons_path + "af21153d07a2e92bde7b2ad155055489_low_res_1619092574091.png";
}

std::string DirectoryHtmlFactory::GetHtml() { return ss.str(); }
