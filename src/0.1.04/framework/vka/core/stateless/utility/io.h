#pragma once
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <functional>
#include <filesystem>

std::vector<char> readFile(const std::string &filename);

std::vector<char> readTextFile(const std::string &filename);

void writeFile(const std::string &filename, const std::string &data);
void copyRecursive(const std::filesystem::path &src, const std::filesystem::path &target,
                   const std::function<bool(std::filesystem::path)> &predicate /* or use template */) noexcept;


    uint32_t longestCommonPrefix(const std::string &s1, const std::string &s2);
std::string getRelativePath(const std::string &dir, const std::string &path);


inline void printVka(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	printf("\n");
}

inline void printVka(const char *path, glm::mat4 mat)
{
	std::string format(path);
	format.append(": \n %.3f %.3f %.3f %.3f \n %.3f %.3f %.3f %.3f \n %.3f %.3f %.3f %.3f \n %.3f %.3f %.3f %.3f \n");
	printVka(format.c_str(),
	         mat[0].x, mat[1].x, mat[2].x, mat[3].x,
	         mat[0].y, mat[1].y, mat[2].y, mat[3].y,
	         mat[0].z, mat[1].z, mat[2].z, mat[3].z,
	         mat[0].w, mat[1].w, mat[2].w, mat[3].w);
}

inline void printVka(const char *path, glm::vec4 v)
{
	std::string format(path);
	format.append(": \n %.3f %.3f %.3f %.3f \n");
	printVka(format.c_str(), v.x, v.y, v.z, v.w);
}

inline std::vector<std::string> split(const std::string &s, char delimiter)
{
	std::vector<std::string> tokens;
	std::string              token;
	std::istringstream       tokenStream(s);
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
	}
	return tokens;
}