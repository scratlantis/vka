#pragma once
#include <vector>
template <class T>
inline bool cmpPtr(const T *a, const T *b)
{
	if (a == nullptr && b == nullptr)
	{
		return true;
	}
	if (a == nullptr || b == nullptr)
	{
		return false;
	}
	return *a == *b;
}

template <class T>
inline bool cmpArray(const T *a, const T *b, uint32_t count)
{
	if (count == 0)
	{
		return true;
	}
	bool isEqual = true;
	for (size_t i = 0; i < count; i++)
	{
		isEqual &= a[i] == b[i];
	}
	return isEqual;
}

template <class T>
inline bool cmpVector(const std::vector<T> &a, const std::vector<T> &b)
{
	if (a.size() != b.size())
	{
		return false;
	}
	bool isEqual = true;
	for (size_t i = 0; i < a.size(); i++)
	{
		isEqual &= a[i] == b[i];
	}
	return isEqual;
}
