#pragma once

#include <string>

namespace zia {

/// Main program command line arguments.
/// --config <string>
/// --num-threads <int>
struct Params {
    std::string config_file_path{"zia-config.yaml"};

    int num_threads{0};
};

}  // namespace zia
