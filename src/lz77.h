#ifndef COMPRESSION_LZ77_H_
#define COMPRESSION_LZ77_H_

#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

#include "lib.h"

namespace compression {
namespace lz77 {

const size_t kDefaultWindowSize = 1000;
const size_t kMaxWindowSize = 0x2000;  // 13 bits
const size_t kMinCompressionLength = 5;
const size_t kMaxCompressionLength = 0x3ff;  // 10 bits

size_t LongestCommonPrefix(const uint8_t* a, const uint8_t* b,
                           size_t max_length) {
  size_t result = 0;
  while (result < max_length && result < kMaxCompressionLength &&
         *(a + result) == *(b + result)) {
    ++result;
  }
  return result;
}

std::pair<bool, size_t> Lz77CompressToDestination(
    const uint8_t* source, const uint8_t* const source_end,
    uint8_t* destination, const uint8_t* const destination_end,
    const size_t window_size = kDefaultWindowSize) {
  assert(window_size <= kMaxWindowSize);
  const uint8_t* const source_begin = source;
  const uint8_t* const destination_begin = destination;

  while (source != source_end) {
    if (destination == destination_end) {
      return {false, 0};
    }
    size_t max_lcp = 0;
    size_t max_lcp_offset = 0;
    for (size_t offset = 1;
         offset <= window_size &&
         static_cast<size_t>(source - source_begin) >= offset;
         ++offset) {
      size_t lcp =
          LongestCommonPrefix(source - offset, source, source_end - source);
      if (lcp >= kMinCompressionLength && lcp > max_lcp) {
        max_lcp = lcp;
        max_lcp_offset = offset;
      }
    }
    if (max_lcp < kMinCompressionLength) {
      if (*source == 0xff) {
        *destination++ = 0xff;
      }
      if (destination == destination_end) {
        return {false, 0};
      }
      *destination++ = *source++;
    } else {
      if (destination_end - destination < 4) {
        return {false, 0};
      }
      max_lcp_offset -= 1;
      *destination++ = 0xff;
      *destination++ = max_lcp_offset >> 6;
      *destination++ = (max_lcp_offset & 0x3f) << 2 | (max_lcp >> 8);
      *destination++ = max_lcp & 0xff;
      source += max_lcp;
    }
  }

  return {true, destination - destination_begin};
}

std::pair<bool, size_t> Lz77CompressToDestination(
    const std::string& source, uint8_t* destination,
    const uint8_t* const destination_end,
    const size_t window_size = kDefaultWindowSize) {
  const uint8_t* source_begin =
      reinterpret_cast<const uint8_t*>(source.c_str());
  return Lz77CompressToDestination(source_begin, source_begin + source.length(),
                                   destination, destination_end, window_size);
}

std::string Lz77CompressToString(
    const uint8_t* source, const uint8_t* const source_end,
    const size_t window_size = kDefaultWindowSize) {
  assert(window_size <= kMaxWindowSize);
  const uint8_t* const source_begin = source;
  std::string result;
  result.reserve(source_end - source);

  while (source != source_end) {
    size_t max_lcp = 0;
    size_t max_lcp_offset = 0;
    for (size_t offset = 1;
         offset <= window_size &&
         static_cast<size_t>(source - source_begin) >= offset;
         ++offset) {
      size_t lcp =
          LongestCommonPrefix(source - offset, source, source_end - source);
      if (lcp >= kMinCompressionLength && lcp > max_lcp) {
        max_lcp = lcp;
        max_lcp_offset = offset;
      }
    }
    if (max_lcp < kMinCompressionLength) {
      if (*source == 0xff) {
        result.push_back(0xff);
      }
      result.push_back(*source++);
    } else {
      max_lcp_offset -= 1;
      result.push_back(0xff);
      result.push_back(max_lcp_offset >> 6);
      result.push_back((max_lcp_offset & 0x3f) << 2 | (max_lcp >> 8));
      result.push_back(max_lcp & 0xff);
      source += max_lcp;
    }
  }

  result.shrink_to_fit();
  return result;
}

std::string Lz77CompressToString(
    const std::string& source, const size_t window_size = kDefaultWindowSize) {
  const uint8_t* source_begin =
      reinterpret_cast<const uint8_t*>(source.c_str());
  return Lz77CompressToString(source_begin, source_begin + source.length(),
                              window_size);
}

std::pair<bool, size_t> Lz77DecompressToDestination(
    const uint8_t* source, const uint8_t* const source_end,
    uint8_t* destination, const uint8_t* const destination_end) {
  assert(source <= source_end);
  assert(destination <= destination_end);
  const uint8_t* const destination_begin = destination;
  while (source != source_end) {
    if (destination == destination_end) {
      return {false, 0};
    }
    if (*source != 0xff) {
      *destination++ = *source++;
    } else {
      if (*++source == 0xff) {
        *destination++ = 0xff;
        ++source;
      } else {
        size_t offset = static_cast<size_t>(*source) << 6 | (*(source + 1) >> 2);
        size_t lcp = static_cast<size_t>(*(source + 1) & 0x3) << 8 | *(source + 2);
        source += 3;
        if (static_cast<size_t>(destination_end - destination) < lcp) {
          return {false, 0};
        }
        while (lcp--) {
          *destination = *(destination - offset - 1);
          ++destination;
        }
      }
    }
  }
  return {true, destination - destination_begin};
}

std::pair<bool, size_t> Lz77DecompressToDestination(
    const std::string& source, uint8_t* destination,
    const uint8_t* const destination_end) {
  const uint8_t* source_begin =
      reinterpret_cast<const uint8_t*>(source.c_str());
  return Lz77DecompressToDestination(source_begin,
                                     source_begin + source.length(),
                                     destination, destination_end);
}

std::string Lz77DecompressToString(const uint8_t* source,
                                   const uint8_t* const source_end) {
  assert(source <= source_end);
  std::string result;
  result.reserve(source_end - source);
  while (source != source_end) {
    if (*source != 0xff) {
      result.push_back(*source++);
    } else {
      if (*++source == 0xff) {
        result.push_back(0xff);
        ++source;
      } else {
        size_t offset = static_cast<size_t>(*source) << 6 | (*(source + 1) >> 2);
        size_t lcp = static_cast<size_t>(*(source + 1) & 0x3) << 8 | *(source + 2);
        source += 3;
        while (lcp--) {
          result.push_back(*(result.end() - offset - 1));
        }
      }
    }
  }
  return result;
}

std::string Lz77DecompressToString(const std::string& source) {
  const uint8_t* source_begin =
      reinterpret_cast<const uint8_t*>(source.c_str());
  return Lz77DecompressToString(source_begin, source_begin + source.length());
}

}  // namespace lz77
}  // namespace compression

#endif