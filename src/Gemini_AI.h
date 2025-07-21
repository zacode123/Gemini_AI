/*
 * Gemini_AI.h - C++ wrapper guard for Gemini AI core
 * 
 * @brief Prevents usage in C and ensures compatibility with C++ compilers only. Includes the Gemini_AI.hpp interface when used in a C++ environment.
 * 
 * @author  zacode123
 * @date    16 July 2025
 * @license MIT License
 * 
 * MIT License
 * 
 * Copyright (c) 2025 zacode123
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 */

#pragma once

#ifdef __cplusplus

// #define USE_TTS  
<<<<<<< HEAD
=======
// #define CUSTOM_TOKEN_COUNTS
>>>>>>> 36ef09d (Last updated library to v6.5.0)

#define GEMINI_AI_VERSION "6.5.0"
#define PAYLOAD_BUFFER_SIZE 1500

#ifdef USE_TTS  
<<<<<<< HEAD
  #define MAX_TOKENS 400  
#else  
  #define MAX_TOKENS 5000  
=======
  #define MAX_TOKENS 400
#ifdef CUSTOM_TOKEN_COUNTS
  #define DEFAULT_TOKENS 300 
#endif
#else  
  #define MAX_TOKENS 5000
#ifdef CUSTOM_TOKEN_COUNTS
  #define DEFAULT_TOKENS 1000 
#endif
>>>>>>> 36ef09d (Last updated library to v6.5.0)
#endif  

#include "Gemini_AI.hpp"

#else

#error "Gemini_AI requires a C++ compiler. Please rename your file to .cpp or .cc"

#endif