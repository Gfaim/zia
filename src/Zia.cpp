#include "Zia.hpp"

#include <iostream>
#include <ziapi/Logger.hpp>

#include "CLI.hpp"
#include "ConfigLoader.hpp"
#include "ModuleLoader.hpp"
#include "cxxopts.hpp"
#include "ziapi/Color.hpp"

namespace zia {

int Zia::Start(int argc, char const **argv)
{
    try {
        auto args = ParseCommandLineArguments(argc, argv);
        auto cfg = LoadConfig(args.config_file_path);

        CLI(args, cfg);
    } catch (const HelpException &) {
        return ExitSuccess;
    } catch (const std::exception &e) {
        std::cerr << ziapi::color::RED << "Exception occured: " << ziapi::color::DEFAULT << e.what() << '\n';
        return ExitFailure;
    }
    return ExitSuccess;
}

Params Zia::ParseCommandLineArguments(int ac, char const **av)
{
    cxxopts::Options options("Zia", "Epitech's best HTTP server.");

    options.add_options()("C,config", "Configuration path",
                          cxxopts::value<std::string>()->default_value("./zia-config.yml"))(
        "T,num-threads", "Number of threads", cxxopts::value<int>()->default_value("4"))("h,help", "This help message");

    options.allow_unrecognised_options();
    auto result = options.parse(ac, av);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        throw HelpException();
    }
    if (result["num-threads"].as<int>() <= 0)
        throw std::logic_error("num-threads argument must be a positive integer");

    return Params{.config_file_path = result["config"].as<std::string>(),
                  .num_threads = result["num-threads"].as<int>()};
}

}  // namespace zia
