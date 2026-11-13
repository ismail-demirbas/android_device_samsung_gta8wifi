#include <android-base/file.h>
#include <android-base/strings.h>
#include <android-base/unique_fd.h>
#include <string>
#include <string_view>
namespace android {
namespace base {
std::string Basename(const std::string& path) { return Basename(std::string_view(path)); }
std::string Trim(const std::string& s) { return Trim(std::string_view(s)); }
bool WriteStringToFd(const std::string& content, borrowed_fd fd) { return WriteStringToFd(std::string_view(content), fd); }
}
}
