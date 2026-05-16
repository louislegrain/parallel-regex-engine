#include <iostream>
#include <string>
#include "RegexParser.h"

RegexNode* RegexParser::parse_expr() {
    RegexNode* node = nullptr;

    while (true) {
        const char c = curr();
        RegexNode* right = c == '|' || c == ')' || c == '\0'
            ? new RegexNode{RegexNodeType::EPSILON, 0, nullptr, nullptr}
            : parse_concat();
        if (right == nullptr) {
            delete node;
            return nullptr;
        }

        node = node == nullptr
            ? right
            : new RegexNode{RegexNodeType::ALTER, 0, node, right};

        if (curr() == '|') {
            ++pos;
        } else {
            break;
        }
    }

    return node;
}

RegexNode* RegexParser::parse_concat() {
    RegexNode* node = parse_unary();
    if (node == nullptr) return nullptr;

    char c;
    while ((c = curr()) != '\0' && c != '|' && c != ')') {
        RegexNode* right = parse_unary();
        if (right == nullptr) {
            delete node;
            return nullptr;
        }
        node = new RegexNode{RegexNodeType::CONCAT, 0, node, right};
    }

    return node;
}

RegexNode* RegexParser::parse_unary() {
    RegexNode* node = parse_atom();
    if (node == nullptr) return nullptr;

    const char c = curr();
    if (c != '*' && c != '+' && c != '?') return node;

    ++pos;
    const char next = curr();
    if (next == '*' || next == '+' || next == '?') {
        delete node;
        return nullptr;
    }

    if (c == '*') {
        return new RegexNode{RegexNodeType::STAR, 0, node, nullptr};
    } else if (c == '+') {
        const auto right = new RegexNode{RegexNodeType::STAR, 0, node->clone(), nullptr};
        return new RegexNode{RegexNodeType::CONCAT, 0, node, right};
    } else {
        const auto left = new RegexNode{RegexNodeType::EPSILON, 0, nullptr, nullptr};
        return new RegexNode{RegexNodeType::ALTER, 0, left, node};
    }
}

RegexNode* RegexParser::parse_atom() {
    const char c = curr();
    if (c == '(') {
        ++pos;
        RegexNode* node = parse_expr();
        if (node == nullptr || curr() != ')') {
            delete node;
            return nullptr;
        }
        ++pos;
        return node;
    } else if (c == '|' || c == ')' || c == '*' || c == '+' || c == '?' || c == '\0') {
        return nullptr;
    } else {
        ++pos;
        return new RegexNode{RegexNodeType::CHAR, c, nullptr, nullptr};
    }
}

RegexNode* RegexParser::parse(const std::string& regex) {
    input = regex;
    pos = 0;
    RegexNode* node = parse_expr();

    if (node == nullptr || pos != input.size()) {
        std::cerr << "Invalid syntax" << std::endl;
        delete node;
        return nullptr;
    }

    return node;
}

void RegexParser::print_ast_helper(const RegexNode* root, const int indent) {
    std::cout << regex_type_debug[static_cast<int>(root->type)];
    if (root->type == RegexNodeType::CHAR) std::cout << " " << root->value;
    std::cout << std::endl;

    RegexNode* arr[] = { root->op1, root->op2 };
    for (const auto node : arr) {
        if (node == nullptr) break;
        std::cout << std::string(2 * indent, ' ') << "|_ ";
        print_ast_helper(node, indent+1);
    }
}

void RegexParser::print_ast(const RegexNode* root) {
    print_ast_helper(root, 0);
}
