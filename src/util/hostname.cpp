#include <string>

#include <util/optional.hpp>

#include "hostname.hpp"

#ifdef __linux__
extern "C" {
    #include <unistd.h>
}
#endif

namespace nest {
namespace mc {
namespace util {

#ifdef __linux__
util::optional<std::string> hostname() {
    // Hostnames can be up to 256 characters in length, however on many systems
    // it is limitted to 64.
    char name[256];
    auto result = gethostname(name, sizeof(name));
    if (result) {
        return util::nothing;
    }
    return std::string(name);
}
#else
util::optional<std::string> hostname() {
    return util::nothing;
}
#endif

} // namespace util
} // namespace mc
} // namespace nest

