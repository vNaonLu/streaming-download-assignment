// Copyright 2022, naon

#ifndef INCLUDE_JIGENTEC_COMPILE_H_
#define INCLUDE_JIGENTEC_COMPILE_H_

#ifdef __GNUC__
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x)   x
#define UNLIKELY(x) x
#endif

#endif  // INCLUDE_JIGENTEC_COMPILE_H_
