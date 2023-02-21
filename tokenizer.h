#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <string_view>

enum class TokenKind {
  UNKNOWN = 0,
  ID,
  AND,
  OR,
  NOT,
  OPEN_PAREN,
  CLOSE_PAREN,
  END_OF,
  TOKEN_KIND_COUNT,
};

struct Token {
  TokenKind kind;
  std::string_view text;
  const char* to_str();
};

class Tokenizer {
public:
  Token current_token = {TokenKind::UNKNOWN};
  Tokenizer(std::string_view input) : input(input) {}
  Token next();

private:
  std::string_view input;
};

Token Tokenizer::next() {
  auto space_pos = input.find_first_not_of(" ");
  if (space_pos == std::string_view::npos) {
    input.remove_prefix(input.size());
    current_token.kind = TokenKind::END_OF;
    return current_token;
  }

  input.remove_prefix(space_pos);

  auto id_pos = input.find_first_of(" ");
  if (id_pos == std::string_view::npos) {
    if (input.substr(0, input.size()) == "and") {
      current_token = {TokenKind::AND, "and"};
    } else if (input.substr(0, input.size()) == "or") {
      current_token = {TokenKind::OR, "or"};
    } else if (input.substr(0, input.size()) == "not") {
      current_token = {TokenKind::NOT, "not"};
    } else if (input.substr(0, input.size()) == "\\(") {
      current_token = {TokenKind::ID, "("};
    } else if (input.substr(0, input.size()) == "(") {
      current_token = {TokenKind::OPEN_PAREN, "("};
    } else if (input.substr(0, input.size()) == ")") {
      current_token = {TokenKind::CLOSE_PAREN, ")"};
    } else {
      current_token = {TokenKind::ID, input.substr(0, input.size())};
    }
    input.remove_prefix(input.size());
    return current_token;
  }

  if (input.substr(0, id_pos) == "and") {
    current_token = {TokenKind::AND, "and"};
  } else if (input.substr(0, id_pos) == "or") {
    current_token = {TokenKind::OR, "or"};
  } else if (input.substr(0, id_pos) == "not") {
    current_token = {TokenKind::NOT, "not"};
  } else if (input.substr(0, id_pos) == "\\(") {
    current_token = {TokenKind::ID, "("};
  } else if (input.substr(0, id_pos) == "(") {
    current_token = {TokenKind::OPEN_PAREN, "("};
  } else if (input.substr(0, id_pos) == ")") {
    current_token = {TokenKind::CLOSE_PAREN, ")"};
  } else {
    current_token = {TokenKind::ID, input.substr(0, id_pos)};
  }

  input.remove_prefix(id_pos);

  return current_token;
}

const char* Token::to_str() {
  switch (kind) {
    case TokenKind::UNKNOWN:
      return "UNKNOWN";
    case TokenKind::ID:
      return "ID";
    case TokenKind::AND:
      return "AND";
    case TokenKind::OR:
      return "OR";
    case TokenKind::NOT:
      return "NOT";
    case TokenKind::OPEN_PAREN:
      return "OPEN_PAREN";
    case TokenKind::CLOSE_PAREN:
      return "CLOSE_PAREN";
    case TokenKind::END_OF:
      return "END_OF";
    case TokenKind::TOKEN_KIND_COUNT:
      return "TOKEN_KIND_COUNT";
    default:
      return "???";
  }
}

#endif