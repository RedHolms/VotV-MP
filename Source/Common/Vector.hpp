#pragma once

template <typename T>
struct BaseVector2 {
  T x, y;

  constexpr BaseVector2() : x(0), y(0) {}
  constexpr explicit BaseVector2(T v) : x(v), y(v) {}
  constexpr BaseVector2(T x, T y) : x(x), y(y) {}
};

using Vector2   = BaseVector2<float>;
using Vector2u  = BaseVector2<uint>;
using Vector2i  = BaseVector2<int>;
using Vector2us = BaseVector2<unsigned short>;
using Vector2s  = BaseVector2<short>;
