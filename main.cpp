
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
  auto status = p.parse();
  if (status != ParseStatus::OK) {
    if (status == ParseStatus::INVALID_TOKEN) {
      auto token = p.get_current_token();
      std::cerr << "Invalid Token: " << token.text << '\n';
    } else if (status == ParseStatus::NO_CLOSE_PAREN) {
      std::cerr << "Missing a closing parenthases" << '\n';
    } else if (status == ParseStatus::UNKNOWN) {
      std::cerr << "Encountered an unknown error" << '\n';
    }

    return 1;
  }

  p.dot(input);
  return 0;
}
