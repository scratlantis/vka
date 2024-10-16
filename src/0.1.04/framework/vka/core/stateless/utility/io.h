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

void printVka(const char *path, glm::mat4 mat);

void printVka(const char *path, glm::vec4 v);

std::vector<std::string> split(const std::string &s, char delimiter);