#ifndef _PARSER_H_
#define _PARSER_H_

#include "tokenizer.h"

#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <unordered_map>
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

  void add_child(std::weak_ptr<Node> parent, NodeKind kind);
  void add_child(std::weak_ptr<Node> parent, NodeKind kind, Token token);
  void add_child(NodeKind kind);
  void add_child(std::weak_ptr<Node> parent, std::shared_ptr<Node> child);

  NodeKind kind;
  std::weak_ptr<Node> parent;
  // for non-terminal nodes
  std::vector<std::shared_ptr<Node>> children;
  // for terminal nodes
  std::optional<Token> token;
};

void Node::add_child(std::weak_ptr<Node> parent, NodeKind kind) {
  children.push_back(std::make_unique<Node>(parent, kind));
}

void Node::add_child(std::weak_ptr<Node> parent, NodeKind kind, Token token) {
  children.push_back(std::make_unique<Node>(parent, kind, token));
}

void Node::add_child(NodeKind kind) {
  children.push_back(std::make_unique<Node>(kind));
}

void Node::add_child(std::weak_ptr<Node> parent, std::shared_ptr<Node> child) {
  children.push_back(child);
  child->parent = parent;
}

enum class ParseStatus {
  OK,
  INVALID_TOKEN,
  NO_CLOSE_PAREN,
  UNKNOWN,
};

enum class EvalStatus {
  OK,
  ERR,
  UNKNOWN,
};

class Parser {
public:
  Parser(std::string_view input);
  ParseStatus parse();

  EvalStatus eval(std::string_view input, bool* value) {
    for (auto& i : id_map) {
      i.second = input.find(i.first) != std::string_view::npos;
    }

    *value = false;

    return eval_tree(root, value);
  }

  Token get_current_token();
  const std::unordered_map<std::string_view, bool>& get_id_map() { return id_map; }

  std::string dot(std::string_view label);

private:
  ParseStatus parse_expr(std::shared_ptr<Node> node, int precedence = 0);

  EvalStatus eval_tree(std::shared_ptr<Node> node, bool* value) {
    if (!node) {
      std::cerr << "node is null value\n";
      return EvalStatus::ERR;
    }
    Node* node_ptr = node.get();
    if (node_ptr->children.size() == 1) {
      if (eval_tree(node_ptr->children[0], value) != EvalStatus::OK) return EvalStatus::ERR;
    } else if (node_ptr->kind == NodeKind::ID) {
      if (!node_ptr->token.has_value()) {
        std::cerr << "token has no value\n";
        return EvalStatus::ERR;
      }
      *value = id_map.at(node_ptr->token.value().text);
    } else if (node_ptr->children.size() == 2 && node_ptr->children[0] && node_ptr->children[1]) {
      if (node_ptr->children[0].get()->kind == NodeKind::NOT && (node_ptr->children[1].get()->kind == NodeKind::EXPR || node_ptr->children[1].get()->kind == NodeKind::ID)) {
        if (eval_tree(node_ptr->children[1], value) != EvalStatus::OK) return EvalStatus::ERR;
        *value = !(*value);
      } else {
        std::cerr << "Don't know how to handle node\n";
        return EvalStatus::ERR;
      }
    } else if (node_ptr->kind == NodeKind::EXPR && node_ptr->children.size() % 2 != 0 && node_ptr->children.size() >= 3) {
      bool result;
      bool first_value;
      bool second_value;

      std::shared_ptr<Node> first_node    = node_ptr->children[0];
      std::shared_ptr<Node> operator_node = node_ptr->children[1];
      std::shared_ptr<Node> second_node   = node_ptr->children[2];

      if (!first_node || !operator_node || !second_node) {
        std::cerr << "Don't know how to handle node\n";
        return EvalStatus::ERR;
      }

      if (eval_tree(first_node, &first_value) != EvalStatus::OK) return EvalStatus::ERR;
      if (eval_tree(second_node, &second_value) != EvalStatus::OK) return EvalStatus::ERR;

      if (!operator_node.get()->token.has_value()) {
        std::cerr << "Don't know how to handle node\n";
        return EvalStatus::ERR;
      }

      Token operator_token = operator_node.get()->token.value();

      if (operator_token.kind == TokenKind::AND) {
        result = first_value && second_value;
      } else if (operator_token.kind == TokenKind::OR) {
        result = first_value || second_value;
      } else {
        std::cerr << "Don't know how to handle node\n";
        return EvalStatus::ERR;
      }

      if (node_ptr->children.size() == 3) {
        *value = result;
        return EvalStatus::OK;
      }

      int index = 3;
      // no bound checking needed as it is done above
      operator_node = node_ptr->children[index++];
      first_node    = node_ptr->children[index++];

      if (!first_node || !operator_node) {
        std::cerr << "Don't know how to handle node\n";
        return EvalStatus::ERR;
      }

      while (1) {
        if (eval_tree(first_node, &first_value) != EvalStatus::OK) return EvalStatus::ERR;

        if (!operator_node.get()->token.has_value()) {
          std::cerr << "Don't know how to handle node\n";
          return EvalStatus::ERR;
        }

        operator_token = operator_node.get()->token.value();

        if (operator_token.kind == TokenKind::AND) {
          result = result && first_value;
        } else if (operator_token.kind == TokenKind::OR) {
          result = result || first_value;
        } else {
          std::cerr << "Don't know how to handle node\n";
          return EvalStatus::ERR;
        }

        if (node_ptr->children.size() <= index) break;

        operator_node = node_ptr->children[index++];
        first_node    = node_ptr->children[index++];

        if (!first_node || !operator_node) {
          std::cerr << "Don't know how to handle node\n";
          return EvalStatus::ERR;
        }
      }
      *value = result;
    } else {
      std::cerr << "Don't know how to handle node\n";
      return EvalStatus::ERR;
    }
    return EvalStatus::OK;
  }

  void dot_recurse(std::shared_ptr<Node> node, std::string_view label, std::stringstream& ss);
  void dot_add_label(std::shared_ptr<Node> node, std::stringstream& ss);
  void dot_add_path(std::shared_ptr<Node> node, std::stringstream& ss);

  Tokenizer tokenizer;
  std::shared_ptr<Node> root;
  std::unordered_map<std::string_view, bool> id_map;
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

  if (current_token.kind == TokenKind::ID || current_token.kind == TokenKind::AND || current_token.kind == TokenKind::OR || current_token.kind == TokenKind::CLOSE_PAREN) {
    created_node = std::make_shared<Node>(node, NodeKind::ID, current_token);
    id_map.insert({current_token.text, false});
  } else if (current_token.kind == TokenKind::NOT) {
    Token id_or_paren_token = tokenizer.next();
    if (id_or_paren_token.kind == TokenKind::ID || id_or_paren_token.kind == TokenKind::NOT || id_or_paren_token.kind == TokenKind::AND || id_or_paren_token.kind == TokenKind::OR || id_or_paren_token.kind == TokenKind::CLOSE_PAREN) {
      created_node = std::make_shared<Node>(node, NodeKind::EXPR);
      created_node.get()->add_child(created_node, NodeKind::NOT, current_token);
      created_node.get()->add_child(created_node, NodeKind::ID, id_or_paren_token);
      id_map.insert({id_or_paren_token.text, false});
    } else if (id_or_paren_token.kind == TokenKind::OPEN_PAREN) {
      created_node = std::make_shared<Node>(node, NodeKind::EXPR);
      created_node.get()->add_child(created_node, NodeKind::NOT, current_token);

      auto expr_node = std::make_shared<Node>(created_node, NodeKind::EXPR);
      created_node.get()->add_child(created_node, expr_node);

      tokenizer.next();

      if (tokenizer.current_token.kind == TokenKind::END_OF) {
        created_node = std::make_shared<Node>(node, NodeKind::ID, tokenizer.current_token);
        id_map.insert({tokenizer.current_token.text, false});
      } else {
        auto status = parse_expr(expr_node);
        if (status != ParseStatus::OK) {
          return status;
        }

        Token close_paren_token = tokenizer.current_token;

        if (close_paren_token.kind != TokenKind::CLOSE_PAREN) {
          return ParseStatus::NO_CLOSE_PAREN;
        }
      }

    } else if (id_or_paren_token.kind == TokenKind::END_OF) {
      created_node = std::make_shared<Node>(node, NodeKind::ID, id_or_paren_token);
      id_map.insert({id_or_paren_token.text, false});
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
    node.get()->add_child(node, created_node);
    return ParseStatus::OK;
  }

  int new_precedence = tokenizer.current_token.kind == TokenKind::OR ? 0 : tokenizer.current_token.kind == TokenKind::AND ? 1
                                                                                                                          : -1;

  if (new_precedence == -1) {
    return ParseStatus::INVALID_TOKEN;
  }

  if (precedence == new_precedence) {
    node.get()->add_child(node, created_node);
    node.get()->add_child(node, NodeKind::OPERATOR, tokenizer.current_token);
    tokenizer.next();
    return parse_expr(node, precedence);
  } else {
    if (precedence == 1) {
      node.get()->add_child(node, created_node);

      if (std::shared_ptr<Node> spt = node.get()->parent.lock()) {
        spt.get()->add_child(spt, NodeKind::OPERATOR, tokenizer.current_token);
        tokenizer.next();
        return parse_expr(spt, 0);
      }
      return ParseStatus::UNKNOWN;
    } else {
      std::shared_ptr<Node> expr_node = std::make_shared<Node>(node, NodeKind::EXPR);
      node.get()->add_child(node, expr_node);
      expr_node.get()->add_child(expr_node, created_node);
      expr_node.get()->add_child(expr_node, NodeKind::OPERATOR, tokenizer.current_token);
      tokenizer.next();
      return parse_expr(expr_node, 1);
    }
  }

  return ParseStatus::OK;
}

Token Parser::get_current_token() {
  return tokenizer.current_token;
}

std::string Parser::dot(std::string_view label) {
  std::stringstream ss;
  dot_recurse(root, label, ss);
  return ss.str();
}

void Parser::dot_recurse(std::shared_ptr<Node> node, std::string_view label, std::stringstream& ss) {
  ss << "digraph tree {\n";

  ss << "\tlabel=\"" << label << "\"\n";
  ss << "\tlabelloc=\"t\";\n";
  ss << "\tfontname=\"Helvetica,Arial,sans-serif\"\n";
  dot_add_label(node, ss);
  dot_add_path(node, ss);
  ss << "}\n";
}

void Parser::dot_add_label(std::shared_ptr<Node> node, std::stringstream& ss) {
  if (node) {
    if (node.get()->kind == NodeKind::ID || node.get()->kind == NodeKind::NOT || node.get()->kind == NodeKind::OPERATOR) {
      ss << "\t" << (size_t)node.get() << " [label=\"" << node.get()->token.value_or(Token{TokenKind::UNKNOWN, "Unknown"}).text << "\" shape=\"plain\" fontsize=\"18\" fontcolor=\"orangered3\"]\n";
    } else {
      ss << "\t" << (size_t)node.get() << " [label=\"EXPR\" shape=\"box\"]\n";
    }
  }

  for (auto i : node.get()->children) {
    dot_add_label(i, ss);
  }
}

void Parser::dot_add_path(std::shared_ptr<Node> node, std::stringstream& ss) {
  if (std::shared_ptr<Node> spt = node.get()->parent.lock()) {
    ss << "\t" << (size_t)spt.get() << " -> " << (size_t)node.get() << "\n";
  }
  for (auto i : node.get()->children) {
    dot_add_path(i, ss);
  }
}

#endif