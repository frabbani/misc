#pragma once
#include <cinttypes>
#include <variant>

#define SWAP( a, b ) \
{ \
  __typeof__(a) t = a; \
  a = b; \
  b = t; \
} \

#define LERP ( a, b, s ) ( (1-(s))*(a) + (s)*(b) )

#define CLAMP( a, l, h ) \
{ \
  a = a < (l) ? (l) : a > (h) ? (h) : a;  \
} \

#define SORT2( a, b ) \
{ \
    if( (b) < (a) )\
        SWAP( a, b );\
} \

#define SORT3( a, b, c )  \
{\
    if( (c) < (b) )\
        SWAP( b, c );\
    if( (c) < (a) )\
        SWAP( a, c );\
    if( (b) < (a) )\
        SWAP( a, b );\
}\


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

template<class T, class TypeList>
struct IsContainedIn;

template<class T, class... Ts>
struct IsContainedIn<T, std::variant<Ts...>>
: std::bool_constant<(... || std::is_same<T, Ts> {})>
{};

