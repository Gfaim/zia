#include "Perl.hpp"
#include "dylib/dylib.hpp"

DYLIB_API ziapi::IModule *LoadZiaModule() { return new PerlCGI(); }