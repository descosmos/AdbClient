#include "utils.h"

int add_t(int a, int b) {
    return a + b;
}

std::vector<std::string> string_split(const std::string &str, char delimiter) {
  std::vector<std::string> res;
  int startIndex = 0, endIndex = 0;
  for (int i = 0; i <= str.size(); i++) {
    if (str[i] == delimiter || i == str.size()) {
      endIndex = i;
      std::string temp;
      temp.append(str, startIndex, endIndex - startIndex);
      res.push_back(temp);
      startIndex = endIndex + 1;
    }
  }

  if (!res.empty()) {
    res.pop_back();
  }
  return res;
}