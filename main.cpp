#include <cassert>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>
// ez-search "not (cats or dogs)"
// ez-search "not cats and not dogs"

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
  const char* to_str() {
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
};

class Tokenizer {
public:
  Token current_token = {TokenKind::UNKNOWN};
  Tokenizer(std::string_view input) : input(input) {}
  Token next() {
    auto space_pos = input.find_first_not_of(" ");
    if (space_pos == std::string_view::npos) {
      input.remove_prefix(input.size());
      current_token.kind = TokenKind::END_OF;
      return current_token;
    }

    input.remove_prefix(space_pos);

    if (input.front() == '(') {
      input.remove_prefix(1);
      current_token = {TokenKind::OPEN_PAREN, "("};
      return current_token;
    } else if (input.front() == ')') {
      input.remove_prefix(1);
      current_token = {TokenKind::CLOSE_PAREN, "("};
      return current_token;
    }

    auto id_pos = input.find_first_of("() ");
    if (id_pos == std::string_view::npos) {
      if (input.substr(0, id_pos) == "and") {
        current_token = {TokenKind::AND, input.substr(0, input.size())};
      } else if (input.substr(0, input.size()) == "or") {
        current_token = {TokenKind::OR, input.substr(0, input.size())};
      } else if (input.substr(0, input.size()) == "not") {
        current_token = {TokenKind::NOT, input.substr(0, input.size())};
      } else {
        current_token = {TokenKind::ID, input.substr(0, input.size())};
      }
      input.remove_prefix(input.size());
      return current_token;
    }

    if (input.substr(0, id_pos) == "and") {
      current_token = {TokenKind::AND, input.substr(0, id_pos)};
    } else if (input.substr(0, id_pos) == "or") {
      current_token = {TokenKind::OR, input.substr(0, id_pos)};
    } else if (input.substr(0, id_pos) == "not") {
      current_token = {TokenKind::NOT, input.substr(0, id_pos)};
    } else {
      current_token = {TokenKind::ID, input.substr(0, id_pos)};
    }

    input.remove_prefix(id_pos);

    return current_token;
  }

private:
  std::string_view input;
};

enum class NodeKind {
  UNKNOWN = 0,
  EXPR,
  ID,
  NOT,
  OPERATOR,
};

struct Node {
  Node(std::shared_ptr<Node> parent, NodeKind kind) : parent(parent), kind(kind) {}
  Node(std::shared_ptr<Node> parent, NodeKind kind, Token token) : parent(parent), kind(kind), token(token) {}
  Node(NodeKind kind) : parent(nullptr), kind(kind) {}

  void add_child(std::shared_ptr<Node> child) {
    children.push_back(child);
  }
  NodeKind kind;
  std::shared_ptr<Node> parent;
  // for non-terminal nodes
  std::vector<std::shared_ptr<Node>> children;
  // for terminal nodes
  std::optional<Token> token;
};

class Parser {
public:
  Parser(std::string_view input) : tokenizer(input) {
    root          = std::make_shared<Node>(NodeKind::EXPR);
    current_token = tokenizer.next();
  }
  bool parse() {
    return parse_expr(root);
  }

  void dot(const char* label) {
    dot_recurse(root, label);
  }

  void dot_recurse(std::shared_ptr<Node> node, const char* label) {
    fprintf(stdout, "digraph tree {\n");

    fprintf(stdout, "\tlabel=\"%s\"\n", label);
    fprintf(stdout, "\tlabelloc=\"t\";\n");
    fprintf(stdout, "\tfontname=\"Helvetica,Arial,sans-serif\"\n");
    dot_add_label(node);
    dot_add_path(node);
    fprintf(stdout, "}\n");
  }

  void dot_add_label(std::shared_ptr<Node> node) {
    if (node.get() != NULL) {
      if (node.get()->kind == NodeKind::ID || node.get()->kind == NodeKind::NOT || node.get()->kind == NodeKind::OPERATOR) {
        fprintf(stdout, "\t%ld [label=\"%s\" shape=\"%s\" fontsize=\"%s\" fontcolor=\"%s\"]\n", (size_t)node.get(), std::string(node.get()->token.value().text).c_str(), "plain", "18", "orangered3");
      } else {
        fprintf(stdout, "\t%ld [label=\"%s\" shape=\"%s\"]\n", (size_t)node.get(), "EXPR", "box");
      }
    }

    for (auto&& i : node.get()->children) {
      dot_add_label(i);
    }
  }

  void dot_add_path(std::shared_ptr<Node> node) {
    if (node.get()->parent != NULL) {
      fprintf(stdout, "\t%ld -> %ld;\n", (size_t)node.get()->parent.get(), (size_t)node.get());
    }
    for (auto&& i : node.get()->children) {
      dot_add_path(i);
    }
  }

private:
  bool parse_expr(std::shared_ptr<Node> node, int precedence = 0) {
    std::shared_ptr<Node> created_node;

    Token current_token = tokenizer.current_token;

    if (current_token.kind == TokenKind::ID || current_token.kind == TokenKind::AND || current_token.kind == TokenKind::OR) {
      created_node = std::make_shared<Node>(node, NodeKind::ID, current_token);
    } else if (current_token.kind == TokenKind::NOT) {
      Token id_or_paren_token = tokenizer.next();
      if (id_or_paren_token.kind == TokenKind::ID || id_or_paren_token.kind == TokenKind::NOT || id_or_paren_token.kind == TokenKind::AND || id_or_paren_token.kind == TokenKind::OR) {
        created_node = std::make_shared<Node>(node, NodeKind::EXPR);
        created_node.get()->add_child(std::make_shared<Node>(created_node, NodeKind::NOT, current_token));
        created_node.get()->add_child(std::make_shared<Node>(created_node, NodeKind::ID, id_or_paren_token));
      } else if (id_or_paren_token.kind == TokenKind::OPEN_PAREN) {
        created_node = std::make_shared<Node>(node, NodeKind::EXPR);
        created_node.get()->add_child(std::make_shared<Node>(created_node, NodeKind::NOT, current_token));
        
        auto expr_node = std::make_shared<Node>(created_node, NodeKind::EXPR);
        created_node.get()->add_child(expr_node);
      

        tokenizer.next();
        if (!parse_expr(expr_node)) {
          assert(false);
          return false;
        }

        Token close_paren_token = tokenizer.current_token;

        if (close_paren_token.kind != TokenKind::CLOSE_PAREN) {
          assert(false);
          return false;
        }
      } else {
        assert(false && "wtf");
        return false;
      }
    } else if (current_token.kind == TokenKind::OPEN_PAREN) {
      created_node = std::make_shared<Node>(node, NodeKind::EXPR);
      tokenizer.next();
      if (!parse_expr(created_node)) {
        assert(false);
        return false;
      }

      Token close_paren_token = tokenizer.current_token;

      if (close_paren_token.kind != TokenKind::CLOSE_PAREN) {
        assert(false);
        return false;
      }

    } else {
      assert(false && "wtf");
      return false;
    }

    tokenizer.next();

    if (tokenizer.current_token.kind == TokenKind::END_OF || tokenizer.current_token.kind == TokenKind::CLOSE_PAREN) {
      node.get()->add_child(created_node);
      created_node.get()->parent = node;
      return true;
    }

    int new_precedence = tokenizer.current_token.kind == TokenKind::OR ? 0 : tokenizer.current_token.kind == TokenKind::AND ? 1
                                                                                                                            : -1;

    if (new_precedence == -1) {
      assert(false && "wtf");
      return false;
    }

    if (precedence == new_precedence) {
      node.get()->add_child(created_node);
      created_node.get()->parent = node;
      node.get()->add_child(std::make_shared<Node>(node, NodeKind::OPERATOR, tokenizer.current_token));
      tokenizer.next();
      return parse_expr(node, precedence);
    } else {
      if (precedence == 1) {
        node.get()->add_child(created_node);
        created_node.get()->parent = node;
        node.get()->parent.get()->add_child(std::make_shared<Node>(node.get()->parent, NodeKind::OPERATOR, tokenizer.current_token));
        tokenizer.next();
        return parse_expr(node.get()->parent, 0);
      } else {
        std::shared_ptr<Node> expr_node = std::make_shared<Node>(node, NodeKind::EXPR);
        node.get()->add_child(expr_node);
        expr_node.get()->add_child(created_node);
        created_node.get()->parent = expr_node;
        expr_node.get()->add_child(std::make_shared<Node>(expr_node, NodeKind::OPERATOR, tokenizer.current_token));
        tokenizer.next();
        return parse_expr(expr_node, 1);
      }
    }

    return true;
  }
  Tokenizer tokenizer;
  Token current_token;
  std::shared_ptr<Node> root;
};

int main(int argc, char** argv) {
  std::string_view input = "(not cats) and (not dogs)";
  Parser p(input);
  p.parse();

  p.dot(std::string(input).c_str());
}
