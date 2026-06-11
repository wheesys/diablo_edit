// QuickLZ implementation - compiled once to avoid multiple definition errors.
// Kept separate from compress_quicklz.h (which only needs declarations).

#ifndef QLZ_COMPRESSION_LEVEL
#   define QLZ_COMPRESSION_LEVEL 3
#endif

#ifndef QLZ_STREAMING_BUFFER
#   define QLZ_STREAMING_BUFFER 0
#endif

#include "quicklz.c.h"
