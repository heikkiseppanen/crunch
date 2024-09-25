#pragma once

#include "Crunch.hpp"

#include <filesystem>

namespace Cr
{

std::vector<U8> read_binary_file(const std::filesystem::path& path);

} // namespace Cr
