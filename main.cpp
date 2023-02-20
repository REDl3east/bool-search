
// ez-search "not (cats or dogs)"
// ez-search "not cats and not dogs"

#include "parser.h"

int main(int argc, char** argv) {
  std::string_view input = "not cats and not dogs or pizza";
  Parser p(input);
  p.parse();

  p.dot(std::string(input).c_str());
}
