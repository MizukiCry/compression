#include <cassert>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

#include "src/lz77.h"

std::mt19937 Rs(std::chrono::system_clock::now().time_since_epoch().count());

inline size_t Rand(size_t l, size_t r) { return Rs() % (r - l + 1) + l; }

void TestLz77Compress() {}

void TestLz77Decompress() {}

void PrintBytes(const std::string& str) {
  std::cout << "[PrintBytes] " << str << std::endl;
  for (auto c : str) {
    std::cout << static_cast<int>(c) << ' ';
  }
  std::cout << std::endl;
}

int main() {
  std::string s1;
  // const int OneGigaBytes = 1 * 1024 * 1024 * 1024;
  // const size_t kMegaBytes = 1 * 1024 * 1024;
  // const size_t kStringSize = 10 * kMegaBytes;
  // s1.reserve(kStringSize);
  // for (size_t i = 1; i <= kStringSize; ++i) {
  //   s1.push_back(static_cast<char>(Rand(0, 127)));
  // }

  std::fstream fs("./dic.txt");

  for (std::string tmp; fs >> tmp; s1 += tmp)
    ;

  // std::cout << s1.length() << std::endl;
  std::cout << "Source size: " << std::fixed << std::setprecision(2)
            << s1.length() / 1024.0 / 1024.0 << "MB" << std::endl;

  // PrintBytes(s1);

  auto clock1 = std::chrono::steady_clock::now();
  std::string s2 = compression::lz77::Lz77CompressToString(s1);
  auto clock2 = std::chrono::steady_clock::now();

  int64_t t1 =
      std::chrono::duration_cast<std::chrono::milliseconds>(clock2 - clock1)
          .count();

  // PrintBytes(s2);

  std::cout << "Compression time: " << std::fixed << std::setprecision(3)
            << t1 / 1000.0 << "s\n";

  std::cout << "Compression speed: " << std::fixed << std::setprecision(2)
            << s1.length() / 1024.0 / t1 << "MB / s\n";

  std::cout << "Compression ratio: " << std::fixed << std::setprecision(2)
            << static_cast<double>(s2.size()) / s1.size() << "%\n";

  auto clock3 = std::chrono::steady_clock::now();
  std::string s3 = compression::lz77::Lz77DecompressToString(s2);
  auto clock4 = std::chrono::steady_clock::now();

  int64_t t2 =
      std::chrono::duration_cast<std::chrono::milliseconds>(clock4 - clock3)
          .count();

  // PrintBytes(s3);

  std::cout << "Decompression time: " << std::fixed << std::setprecision(3)
            << t2 / 1000.0 << "s\n";

  std::cout << "Decompression speed: " << std::fixed << std::setprecision(2)
            << s2.length() / 1024.0 / t2 << "MB / s\n";

  assert(s1 == s3);

  std::cout << "Result correct. Congratulations!" << std::endl;

  return 0;
}

/*
Source size: 4.19MB
Compression time: 8.534s
Compression speed: 0.50MB / s
Compression ratio: 0.69%
Decompression time: 0.013s
Decompression speed: 226.56MB / s
Result correct. Congratulations!
*/