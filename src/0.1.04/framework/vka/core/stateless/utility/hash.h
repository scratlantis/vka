#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace vka
{

typedef uint64_t hash_t;

template <class T>
inline void hashCombineLocal(hash_t &s, const T &v)
{
	std::hash<T> h;
	s ^= h(v) + 0x9e3779b9 + (s << 6) + (s >> 2);
}

template <class A, class B>
inline hash_t hashCombine(const A &a, const B &b)
{
	std::hash<A> h_a;
	std::hash<B> h_b;
	hash_t       out = 0;
	out ^= h_a(a);
	out ^= h_b(b) + 0x9e3779b9 + (out << 6) + (out >> 2);
	return out;
}

template <class T>
inline hash_t hashVector(const T arr[], std::size_t count)
{
	hash_t value = 0;
	for (std::size_t i = 0; i < count; ++i)
	{
		hashCombineLocal(value, &arr[i]);
	}
	return value;
}

template <class T>
inline hash_t hashVector(const T *p, uint32_t count)
{
	hash_t value = 0;
	for (std::size_t i = 0; i < count; ++i)
	{
		hashCombineLocal(value, &(p[i]));
	}
	return value;
}

template <class T>
inline hash_t byteHashPtr(const T *p)
{
	if (p == nullptr)
	{
		return 0;
	}
	hash_t   value     = 0;
	uint32_t byteCount = sizeof(T);
	for (std::size_t i = 0; i < byteCount; ++i)
	{
		hashCombineLocal(value, ((uint8_t *) p)[byteCount]);
	}
	return value;
}

template <class T>
inline hash_t hashVector(const std::vector<T> &arr)
{
	hash_t value = 0;
	for (std::size_t i = 0; i < arr.size(); ++i)
	{
		hashCombineLocal(value, arr[i]);
	}
	return value;
}
}        // namespace vka

// custom operator
// https://stackoverflow.com/questions/8425077/can-i-create-a-new-operator-in-c-and-how

// generic LHSlt holder
template <typename LHS, typename OP>
struct LHSlt
{
	LHS lhs_;
};

// declare myop as an operator-like construct
enum
{
	hashCombine_OP
};

// parse 'lhs <myop' into LHSlt
template <typename LHS>
LHSlt<LHS, decltype(hashCombine_OP)> operator<(const LHS &lhs, decltype(hashCombine_OP))
{
	return {lhs};
}

template <class A, class B>
int operator>(LHSlt<A, decltype(hashCombine_OP)> lhsof, B rhs)
{
	A &lhs = lhsof.lhs_;
	// actual implementation
	return vka::hashCombine(lhs, rhs);
}

#define HASHC < hashCombine_OP >

namespace std
{

template <>
struct hash<glm::vec3>
{
	size_t operator()(glm::vec3 const &type) const
	{
		return type.x HASHC type.y HASHC type.z;
	}
};

template <>
struct hash<glm::vec2>
{
	size_t operator()(glm::vec2 const &type) const
	{
		return type.x HASHC type.y;
	};
};

}        // namespace std
