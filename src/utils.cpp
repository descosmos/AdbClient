#include "utils.h"
#include <sstream>
#include <string>
#include <algorithm>

int add_t(int a, int b) {
    return a + b;
}

std::vector<std::string> string_split(const std::string &str, char delimiter) {
  std::vector<std::string> res;
  std::stringstream ss(str);
  std::string word;
  while (!ss.eof()) {
    std::getline(ss, word, delimiter);
    res.emplace_back(word);
  }

  return res;
}

std::string unique_character(std::string &str) {
  auto res_it = std::unique(str.begin(), str.end(), [](char lhs, char rhs) {
     return lhs == rhs && lhs == ' '; 
  });

  str.erase(res_it, str.end());
  return str;
}