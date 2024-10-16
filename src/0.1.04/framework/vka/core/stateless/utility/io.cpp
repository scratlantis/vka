#include "io.h"


uint32_t longestCommonPrefix(const std::string &s1, const std::string &s2)
{
	uint32_t i = 0;
	while (i < s1.size() && i < s2.size() && s1[i] == s2[i])
		i++;
	return i;
}

std::string getRelativePath(const std::string &dir, const std::string &path)
{
	uint32_t    lcp      = longestCommonPrefix(dir, path);
	std::string rel_path = path.substr(lcp);
	std::string rel_dir  = dir.substr(lcp);

	uint32_t dir_depth = std::count(rel_dir.begin(), rel_dir.end(), '/') + 1;

	for (uint32_t i = 0; i < dir_depth; i++)
	{
		rel_path = "../" + rel_path;
	}
	return rel_path;
}

std::vector<char> readFile(const std::string &filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cout << "Failed to open a file: " << filename << std::endl;
		throw std::runtime_error("Failed to open a file!");
		return {};
	}
	size_t            file_size = (size_t) file.tellg();
	std::vector<char> file_buffer(file_size);
	file.seekg(0);
	file.read(file_buffer.data(), file_size);
	file.close();
	return file_buffer;
}

std::vector<char> readTextFile(const std::string &filename)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
	{
		std::cout << "Failed to open a file: " << filename << std::endl;
		throw std::runtime_error("Failed to open a file!");
		return {};
	}
	size_t            file_size = (size_t) file.tellg();
	std::vector<char> file_buffer(file_size);
	file.seekg(0);
	file.read(file_buffer.data(), file_size);
	file.close();
	file_buffer.push_back('\0');
	return file_buffer;
}

void writeFile(const std::string &filename, const std::string &data)
{
	std::ofstream f(filename);
	f <<  data;
	f.close();
}

//https://stackoverflow.com/questions/51431425/how-to-recursively-copy-files-and-directories
namespace fs = std::filesystem;
// Recursively copies those files and folders from src to target which matches
// predicate, and overwrites existing files in target.
void copyRecursive(const fs::path &src, const fs::path &target,
                   const std::function<bool(fs::path)> &predicate /* or use template */) noexcept
{
	try
	{
		for (const auto &dirEntry : fs::recursive_directory_iterator(src))
		{
			const auto &p = dirEntry.path();
			if (predicate(p))
			{
				// Create path in target, if not existing.
				const auto relativeSrc      = fs::relative(p, src);
				const auto targetParentPath = target / relativeSrc.parent_path();
				fs::create_directories(targetParentPath);

				// Copy to the targetParentPath which we just created.
				fs::copy(p, targetParentPath, fs::copy_options::overwrite_existing);
			}
		}
	}
	catch (std::exception &e)
	{
		std::cout << e.what();
	}
}

void printVka(const char *path, glm::mat4 mat);

void printVka(const char *path, glm::vec4 v);

std::vector<std::string> split(const std::string &s, char delimiter);