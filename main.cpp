#include "RegexParser.h"

int main() {
    RegexParser parser;
    const RegexNode* root = parser.parse("((a|b)*c|d+)?e*");
    if (root == nullptr) return -1;

    RegexParser::print_ast(root);

    delete root;
    return 0;
}
