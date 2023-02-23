
#include <filesystem>
#include <fstream>

#include "argtable3.h"
#include "parser.h"

bool handle_file(const char* filename, Parser& p) {
  std::ifstream file(filename);
  if (!file) return false;

  std::string line;
  int line_num = 1;
  while (std::getline(file, line)) {
    bool result;
    EvalStatus eval_status = p.eval(line, &result);

    if (eval_status != EvalStatus::OK) {
      line_num++;
      continue;
    }

    if(result){
      std::cout << filename << ":" << line_num << ": " << line << '\n';
    }
      line_num++;
  }

  return true;
}

bool handle_directory(const char* directory, Parser& p) {
  return true;
}

int main(int argc, char** argv) {
  struct arg_lit* recursive_arg = arg_lit0("r", "recursive", "recusivly search a directory");
  struct arg_lit* help_arg      = arg_lit0("h", "help", "display this help and exit");
  struct arg_str* expr_arg      = arg_str1(NULL, NULL, "EXPR", "The expression that is used to search");
  struct arg_file* file_arg     = arg_filen(NULL, NULL, "FILE", 1, argc + 2, "The file or directory to search from");
  struct arg_end* end           = arg_end(20);

  void* argtable[] = {recursive_arg, help_arg, expr_arg, file_arg, end};

  if (arg_nullcheck(argtable) != 0) {
    std::cerr << argv[0] << ": insufficient memory\n";
    return 1;
  }

  int nerr = arg_parse(argc, argv, argtable);

  if (help_arg->count > 0) {
    std::cout << argv[0] << " - A command line tool that searches things.\n\n";
    std::cerr << "Usage: " << argv[0] << ' ';
    arg_print_syntax(stdout, argtable, "\n");
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
  }

  if (nerr > 0) {
    arg_print_errors(stdout, end, argv[0]);
    std::cerr << "Try '" << argv[0] << " --help' for more information.\n";
    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  std::string_view input(expr_arg->sval[0]);
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

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 1;
  }

  if (file_arg->count == 0) {
  } else {
    for (int i = 0; i < file_arg->count; i++) {
      const char* filename = file_arg->filename[i];
      if (std::filesystem::is_directory(filename)) {
        handle_directory(filename, p);
      } else if (std::filesystem::is_regular_file(filename)) {
        handle_file(filename, p);
      } else {
        std::cout << "'" << filename << "' is not a valid file\n";
      }
    }
  }

  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

  // if (argc != 3) {
  //   std::cerr << "Expected 2 arguement, got " << argc - 1 << '\n';
  //   std::cerr << "Usage: " << argv[0] << " INPUT SEARCH\n";
  //   return 1;
  // }

  // std::string_view input(argv[1]);
  // Parser p(input);
  // auto status = p.parse();
  // if (status != ParseStatus::OK) {
  //   if (status == ParseStatus::INVALID_TOKEN) {
  //     auto token = p.get_current_token();
  //     std::cerr << "Invalid Token: " << token.text << '\n';
  //   } else if (status == ParseStatus::NO_CLOSE_PAREN) {
  //     std::cerr << "Missing a closing parenthasis\n";
  //   } else if (status == ParseStatus::UNKNOWN) {
  //     std::cerr << "Encountered an unknown error\n";
  //   }

  //   return 1;
  // }

  // bool result;
  // EvalStatus eval_status = p.eval(argv[2], &result);

  // if (eval_status != EvalStatus::OK) {
  //   std::cerr << "There was an error after eval\n";
  //   return 1;
  // }

  // auto& id_map = p.get_id_map();

  // std::cout << "Input:  " << input << '\n';
  // std::cout << "Search: " << argv[2] << '\n';
  // std::cout << '\n';

  // for (auto& i : id_map) {
  //   std::cout << i.first << ": " << (i.second ? "true" : "false") << '\n';
  // }

  // std::cout << "\nresult: " << (result ? "true" : "false") << '\n';

  // std::string dot_graph = p.dot(input);
  // std::cout << dot_graph;

  return 0;
}
