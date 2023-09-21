// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"

#include <assert.h>
#include <stdint.h>
#include <iostream>
#include <cstddef>
#include <cstdlib>      // std::calloc, etc..
#include <algorithm>    // std::min/max, etc..
#include <functional>
#include <mutex>
#include <memory>
#include <sstream>
#include <string>
#include <shared_mutex>
#include <typeinfo>
#include <unordered_map>
#include <atomic>
#include <utility>
#include <vector>
#include <list>
#include <stack>
#include <map>
#include <deque>
#include <queue>
#include <set>
#include <unordered_set>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <cstring>
#include <array>
#include <random>
#include <ctime>
#include <iosfwd>

#endif //PCH_H
