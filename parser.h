#ifndef _PARSER_H_
#define _PARSER_H_

#include "tokenizer.h"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <vector>

enum class NodeKind {
  UNKNOWN = 0,
  EXPR,
  ID,
  NOT,
  OPERATOR,
};

struct Node {
  Node(std::weak_ptr<Node> parent, NodeKind kind) : parent(parent), kind(kind) {}
  Node(std::weak_ptr<Node> parent, NodeKind kind, Token token) : parent(parent), kind(kind), token(token) {}
  Node(NodeKind kind) : kind(kind) {}

  void add_child(std::shared_ptr<Node> child) {
    children.push_back(child);
  }
  NodeKind kind;
  std::weak_ptr<Node> parent;
  // for non-terminal nodes
  std::vector<std::shared_ptr<Node>> children;
  // for terminal nodes
  std::optional<Token> token;
};

enum class ParseStatus {
  OK,
  INVALID_TOKEN,
  NO_CLOSE_PAREN,
  UNKNOWN,
};

class Parser {
public:
  Parser(std::string_view input);
  ParseStatus parse();
  Token get_current_token();

  void dot(std::string_view label);

private:
  ParseStatus parse_expr(std::shared_ptr<Node> node, int precedence = 0);

  void dot_recurse(std::shared_ptr<Node> node, std::string_view label);
  void dot_add_label(std::shared_ptr<Node> node);
  void dot_add_path(std::shared_ptr<Node> node);

  Tokenizer tokenizer;
  std::shared_ptr<Node> root;
};

Parser::Parser(std::string_view input) : tokenizer(input) {
  root = std::make_shared<Node>(NodeKind::EXPR);
  tokenizer.next();
}
ParseStatus Parser::parse() {
  return parse_expr(root);
}

ParseStatus Parser::parse_expr(std::shared_ptr<Node> node, int precedence) {
  std::shared_ptr<Node> created_node;

  Token current_token = tokenizer.current_token;

  if (current_token.kind == TokenKind::ID || current_token.kind == TokenKind::AND || current_token.kind == TokenKind::OR) {
    created_node = std::make_shared<Node>(node, NodeKind::ID, current_token);
  } else if (current_token.kind == TokenKind::NOT) {
    Token id_or_paren_token = tokenizer.next();
    if (id_or_paren_token.kind == TokenKind::ID || id_or_paren_token.kind == TokenKind::NOT || id_or_paren_token.kind == TokenKind::AND || id_or_paren_token.kind == TokenKind::OR || id_or_paren_token.kind == TokenKind::CLOSE_PAREN) {
      created_node = std::make_shared<Node>(node, NodeKind::EXPR);
      created_node.get()->add_child(std::make_shared<Node>(created_node, NodeKind::NOT, current_token));
      created_node.get()->add_child(std::make_shared<Node>(created_node, NodeKind::ID, id_or_paren_token));
    } else if (id_or_paren_token.kind == TokenKind::OPEN_PAREN) {
      created_node = std::make_shared<Node>(node, NodeKind::EXPR);
      created_node.get()->add_child(std::make_shared<Node>(created_node, NodeKind::NOT, current_token));

      auto expr_node = std::make_shared<Node>(created_node, NodeKind::EXPR);
      created_node.get()->add_child(expr_node);

      tokenizer.next();
      auto status = parse_expr(expr_node);
      if (status != ParseStatus::OK) {
        return status;
      }

      Token close_paren_token = tokenizer.current_token;

      if (close_paren_token.kind != TokenKind::CLOSE_PAREN) {
        return ParseStatus::NO_CLOSE_PAREN;
      }
    } else {
      return ParseStatus::INVALID_TOKEN;
    }
  } else if (current_token.kind == TokenKind::OPEN_PAREN) {
    created_node = std::make_shared<Node>(node, NodeKind::EXPR);
    tokenizer.next();
    auto status = parse_expr(created_node);
    if (status != ParseStatus::OK) {
      return status;
    }

    Token close_paren_token = tokenizer.current_token;

    if (close_paren_token.kind != TokenKind::CLOSE_PAREN) {
      return ParseStatus::NO_CLOSE_PAREN;
    }

  } else {
    return ParseStatus::INVALID_TOKEN;
  }

  tokenizer.next();

  if (tokenizer.current_token.kind == TokenKind::END_OF || tokenizer.current_token.kind == TokenKind::CLOSE_PAREN) {
    node.get()->add_child(created_node);
    created_node.get()->parent = node;
    return ParseStatus::OK;
  }

  int new_precedence = tokenizer.current_token.kind == TokenKind::OR ? 0 : tokenizer.current_token.kind == TokenKind::AND ? 1 : -1;

  if (new_precedence == -1) {
    return ParseStatus::INVALID_TOKEN;
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

      if (std::shared_ptr<Node> spt = node.get()->parent.lock()) {
        spt.get()->add_child(std::make_shared<Node>(spt, NodeKind::OPERATOR, tokenizer.current_token));
        tokenizer.next();
        return parse_expr(spt, 0);
      }
      return ParseStatus::UNKNOWN;
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

  return ParseStatus::OK;
}

Token Parser::get_current_token() {
  return tokenizer.current_token;
}

void Parser::dot(std::string_view label) {
  dot_recurse(root, label);
}

void Parser::dot_recurse(std::shared_ptr<Node> node, std::string_view label) {
  std::cout << "digraph tree {\n";

  std::cout << "\tlabel=\"" << label << "\"\n";
  std::cout << "\tlabelloc=\"t\";\n";
  std::cout << "\tfontname=\"Helvetica,Arial,sans-serif\"\n";
  dot_add_label(node);
  dot_add_path(node);
  std::cout << "}\n";
}

void Parser::dot_add_label(std::shared_ptr<Node> node) {
  if (node) {
    if (node.get()->kind == NodeKind::ID || node.get()->kind == NodeKind::NOT || node.get()->kind == NodeKind::OPERATOR) {
      std::cout << "\t" << (size_t)node.get() << " [label=\"" << node.get()->token.value_or(Token{TokenKind::UNKNOWN, "Unknown"}).text << "\" shape=\"plain\" fontsize=\"18\" fontcolor=\"orangered3\"]\n";
    } else {
      std::cout << "\t" << (size_t)node.get() << " [label=\"EXPR\" shape=\"box\"]\n";
    }
  }

  for (auto i : node.get()->children) {
    dot_add_label(i);
  }
}

void Parser::dot_add_path(std::shared_ptr<Node> node) {
  if (std::shared_ptr<Node> spt = node.get()->parent.lock()) {
    std::cout << "\t" << (size_t)spt.get() << " -> " << (size_t)node.get() << "\n";
  }
  for (auto i : node.get()->children) {
    dot_add_path(i);
  }
}

#endif