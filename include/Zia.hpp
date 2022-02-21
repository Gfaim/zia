#pragma once

#include <filesystem>
#include <vector>

#include "Params.hpp"
#include "ziapi/Module.hpp"

namespace zia {

class Zia {
public:
    /// Starts the Zia program and returns the program's exit code.
    static int Start(int argc, char const **argv);

private:
    class HelpException : public std::exception {
    };

    static Params ParseCommandLineArguments(int argc, char const **argv);

    static constexpr int ExitSuccess = 0;

    static constexpr int ExitFailure = 1;
};

}  // namespace zia
