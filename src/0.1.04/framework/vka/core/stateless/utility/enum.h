#include <string>

template <typename e>
struct eString_val;
template <typename e>
struct eIterator_begin;
template <typename e>
struct eIterator_next;
template <typename e>
struct eIterator_end;

#define ENUM_ADD_STRING_VALUES(TYPE, VALUES) \
template <>\
struct eString_val<TYPE>\
{\
	std::string operator()(TYPE val)\
	{\
		return VALUES[val];\
	}\
};

#define ENUM_ADD_ITERATOR(TYPE, FIRST, LAST)\
template <>\
struct eIterator_begin<TYPE>\
{\
	TYPE operator()()\
	{\
		return FIRST;\
	}\
};\
template <>\
struct eIterator_end<TYPE>\
{\
	TYPE operator()()\
	{\
		return static_cast<TYPE>(LAST + 1);\
	}\
};\
template <>\
struct eIterator_next<TYPE>\
{\
	TYPE operator()(TYPE val)\
	{\
		return static_cast<TYPE>(val + 1);\
	}\
};

