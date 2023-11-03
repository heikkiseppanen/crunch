#include "Shared/Filesystem.hpp"

#include <fstream>
#include <iterator>

namespace Cr
{

std::vector<u8> read_binary_file(const std::string& path)
{
	std::ifstream file;

	file.exceptions(std::ifstream::badbit);
	file.open(path, std::ifstream::binary);

	return {std::istreambuf_iterator<char>(file), {}};
}

} // namespace Cr
