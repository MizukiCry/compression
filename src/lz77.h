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

const size_t kMinCompressionLength = 5;
const size_t kDefaultWindowSize = 4096;

void AppendVlqNumber(std::string& str, size_t value) {
  if (value <= 0x7full) {
    str.push_back(0x80 | value);
    str.push_back(0);
    return;
  }
  do {
    const char c = value & 0x7f;
    value >>= 7;
    str.push_back(value > 0 ? c | 0x80 : c);
  } while (value > 0);
}

bool AppendVlqNumber(uint8_t*& destination,
                     const uint8_t* const destination_end, size_t value) {
  if (value <= 0x7full) {
    if (destination_end - destination < 2) {
      return false;
    }
    *destination++ = 0x80 | value;
    *destination++ = 0;
    return true;
  }
  do {
    if (destination == destination_end) {
      return false;
    }
    const char c = value & 0x7f;
    value >>= 7;
    *destination++ = value > 0 ? c | 0x80 : c;
  } while (value > 0);
  return true;
}

size_t LongestCommonPrefix(const uint8_t* a, const uint8_t* b,
                           size_t max_length) {
  size_t result = 0;
  while (result < max_length && *(a + result) == *(b + result)) {
    ++result;
  }
  return result;
}

std::pair<bool, size_t> Lz77CompressToDestination(
    const uint8_t* source, const uint8_t* const source_end,
    uint8_t* destination, const uint8_t* const destination_end,
    const size_t window_size = kDefaultWindowSize) {
  const uint8_t* const source_begin = source;
  const uint8_t* const destination_begin = destination;

  while (source != source_end) {
    if (destination == destination_end) {
      return {false, 0};
    }
    size_t max_lcp = 0;
    size_t max_lcp_offset = 0;
    for (size_t offset = 1; offset <= window_size && static_cast<size_t>(source - source_begin) >= offset; ++offset) {
      size_t lcp =
          LongestCommonPrefix(source - offset, source, source_end - source);
      if (lcp >= kMinCompressionLength && lcp > max_lcp) {
        max_lcp = lcp;
        max_lcp_offset = offset;
        // if (lcp >= window_size) {
        //   break;
        // }
      }
    }
    if (max_lcp < kMinCompressionLength) {
      *destination++ = *source++;
    } else {
      if (!AppendVlqNumber(destination, destination_end, max_lcp_offset)) {
        return {false, 0};
      }
      if (!AppendVlqNumber(destination, destination_end, max_lcp)) {
        return {false, 0};
      }
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
  const uint8_t* const source_begin = source;
  std::string result;
  result.reserve(source_end - source);

  while (source != source_end) {
    size_t max_lcp = 0;
    size_t max_lcp_offset = 0;
    for (size_t offset = 1; offset <= window_size && static_cast<size_t>(source - source_begin) >= offset; ++offset) {
      size_t lcp =
          LongestCommonPrefix(source - offset, source, source_end - source);
      if (lcp >= kMinCompressionLength && lcp > max_lcp) {
        max_lcp = lcp;
        max_lcp_offset = offset;
        // if (lcp >= window_size) {
        //   break;
        // }
      }
    }
    if (max_lcp < kMinCompressionLength) {
      result.push_back(*source++);
    } else {
      AppendVlqNumber(result, max_lcp_offset);
      AppendVlqNumber(result, max_lcp);
      source += max_lcp;
    }
  }

  result.shrink_to_fit();
  return result;
}

size_t ReadVlqNumber(const uint8_t*& source) {
  size_t result = 0, offset = 0;
  do {
    result |= (*source & 0x7f) << offset;
    offset += 7;
  } while (*source++ > 0x7f);
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
  const uint8_t* const destination_begin = destination;
  while (source != source_end) {
    if (*source >> 7) {
      if (destination == destination_end) {
        return {false, 0};
      }
      *destination++ = *source++;
    } else {
      size_t offset = ReadVlqNumber(source);
      size_t length = ReadVlqNumber(source);
      if (length > static_cast<size_t>(destination_end - destination)) {
        return {false, 0};
      }
      while (length--) {
        *destination = *(destination - offset);
        ++destination;
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
  std::string result;
  result.reserve(source_end - source);
  while (source != source_end) {
    if (*source >> 7) {
      size_t offset = ReadVlqNumber(source);
      size_t length = ReadVlqNumber(source);
      while (length--) {
        result.push_back(*(result.end() - offset));
      }
    } else {
      result.push_back(*source++);
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