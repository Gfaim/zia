#include "cgi/PHP.hpp"
#include "dylib/dylib.hpp"

DYLIB_API ziapi::IModule *LoadZiaModule() { return new PHPCGI(); }