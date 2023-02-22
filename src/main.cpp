
#include "parser.h"

int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "Expected 2 arguement, got " << argc - 1 << '\n';
    std::cerr << "Usage: " << argv[0] << " INPUT SEARCH\n";
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
      std::cerr << "Missing a closing parenthasis\n";
    } else if (status == ParseStatus::UNKNOWN) {
      std::cerr << "Encountered an unknown error\n";
    }

    return 1;
  }

  bool result;
  EvalStatus eval_status = p.eval(argv[2], &result);

  if(eval_status != EvalStatus::OK){
    std::cerr << "There was an error after eval\n";
    return 1;
  }

  auto& id_map = p.get_id_map();

  std::cout << "Input:  " << input << '\n';
  std::cout << "Search: " << argv[2] << '\n';
  std::cout << '\n';

  for(auto& i : id_map){
    std::cout << i.first << ": " << (i.second ? "true" : "false") << '\n';
  }

  std::cout << "\nresult: " << (result ? "true" : "false") << '\n';

  // std::string dot_graph = p.dot(input);
  // std::cout << dot_graph;

  return 0;
}
