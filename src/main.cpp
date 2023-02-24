
#include <filesystem>
#include <fstream>

#include "argtable3.h"
#include "parser.h"
#include "termcolor.hpp"

bool handle_file(const std::filesystem::path& path, Parser& p);
bool handle_directory(const std::filesystem::path& directory, Parser& p);
void handle_file_println(const std::filesystem::path& path, const size_t line_num, const std::string& line);
void handle_stdin_println(const size_t line_num, const std::string& line);

int main(int argc, char** argv) {
  struct arg_lit* recursive_arg = arg_lit0("r", "recursive", "recusivly search given directories");
  struct arg_lit* help_arg      = arg_lit0("h", "help", "display this help and exit");
  struct arg_str* expr_arg      = arg_str1(NULL, NULL, "EXPR", "The expression that is used to search");
  struct arg_file* file_arg     = arg_filen(NULL, NULL, "FILE", 0, argc + 2, "The file or directory (if has -r option) to search from");
  struct arg_end* end           = arg_end(20);

  void* argtable[] = {recursive_arg, help_arg, expr_arg, file_arg, end};

  if (arg_nullcheck(argtable) != 0) {
    std::cerr << argv[0] << ": insufficient memory\n";
    return 1;
  }

  int nerr = arg_parse(argc, argv, argtable);

  if (help_arg->count > 0) {
    std::cout << argv[0] << " - A command line tool that searches things with boolean expressions.\n\n";
    std::cerr << "Usage: " << argv[0] << ' ';
    arg_print_syntax(stdout, argtable, "\n");
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");

    std::cout << "\n" << "When FILE is absent, read in input from standard input. Read from \".\", if -r option is specified.\n";

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
    if (recursive_arg->count > 0) {
      handle_directory(".", p);
    } else {
      std::string line;
      int line_num = 1;
      while (std::getline(std::cin, line)) {
        bool result;
        EvalStatus eval_status = p.eval(line, &result);

        if (eval_status != EvalStatus::OK) {
          line_num++;
          continue;
        }

        if (result) {
          handle_stdin_println(line_num, line);
        }
        line_num++;
      }
    }
  } else {
    for (int i = 0; i < file_arg->count; i++) {
      const char* filename = file_arg->filename[i];
      if (std::filesystem::is_directory(filename)) {
        if (recursive_arg->count > 0) {
          handle_directory(filename, p);
        } else {
          std::cout << argv[0] << ": " << filename << ": Is a directory\n";
        }
      } else if (std::filesystem::is_regular_file(filename)) {
        handle_file(filename, p);
      } else {
        std::cout << "'" << filename << "' is not a valid file\n";
      }
    }
  }

  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

  return 0;
}

bool handle_file(const std::filesystem::path& path, Parser& p) {
  std::ifstream file(path);
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

    if (result) {
      handle_file_println(path, line_num, line);
    }
    line_num++;
  }

  return true;
}

bool handle_directory(const std::filesystem::path& directory, Parser& p) {
  for (const std::filesystem::directory_entry& entry : std::filesystem::recursive_directory_iterator(directory)) {
    if (entry.is_regular_file()) {
      handle_file(entry.path(), p);
    }
  }
  return true;
}

void handle_file_println(const std::filesystem::path& path, const size_t line_num, const std::string& line) {
  std::cout << termcolor::magenta << path.string() << termcolor::blue << ":" << termcolor::green << line_num << termcolor::reset << ": " << termcolor::bold << line << termcolor::reset << '\n';
}

void handle_stdin_println(const size_t line_num, const std::string& line){
  std::cout << termcolor::green << line_num << termcolor::reset << ": " << termcolor::bold << line << termcolor::reset << '\n';
}
