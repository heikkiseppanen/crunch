#include "Crunch/Filesystem.hpp"

#include <fstream>
#include <iterator>
#include <cstring>

namespace Cr
{

std::vector<U8> read_binary_file(const std::filesystem::path& path)
{
    std::ifstream file;

    file.exceptions(std::ifstream::badbit);
    file.open(path, std::ifstream::binary);

    CR_ASSERT_THROW(file.is_open(), "Failed to open {}: {}", path.string(), std::strerror(errno));

    return {std::istreambuf_iterator<char>(file), {}};
}

} // namespace Cr
