
// ez-search "not (cats or dogs)"
// ez-search "not cats and not dogs"

#include "parser.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Expected 1 arguement, got " << argc - 1 << '\n';
    std::cerr << "Usage: " << argv[0] << "INPUT\n";
    return 1;
  }

  std::string_view input(argv[1]);
  Parser p(input);
  if (!p.parse()) {
    return 1;
  }

  p.dot(input);
  return 1;
}
