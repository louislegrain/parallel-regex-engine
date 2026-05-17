#ifndef PARALLEL_REGEX_ENGINE_REGEXPARSER_H
#define PARALLEL_REGEX_ENGINE_REGEXPARSER_H
#include <string>

enum class RegexNodeType {
    EPSILON,
    CHAR,
    STAR,
    ALTER,
    CONCAT,
};

inline std::string regex_type_debug[] = {
    "EPSILON",
    "CHAR",
    "STAR",
    "ALTER",
    "CONCAT",
};

struct RegexNode {
    RegexNodeType type;
    char value;
    RegexNode *op1, *op2;

    [[nodiscard]] RegexNode* clone() const {
        return new RegexNode {
            type,
            value,
            op1 == nullptr ? nullptr : op1->clone(),
            op2 == nullptr ? nullptr : op2->clone(),
        };
    }
    ~RegexNode() {
        delete op1;
        delete op2;
    }
};

class RegexParser {
    // supports grouping, alternation operator, Kleene star, plus operator and the question mark operator
private:
    size_t pos = 0;
    std::string input;

    [[nodiscard]] char curr() const {
        return pos < input.size() ? input[pos] : '\0';
    }
    RegexNode* parse_expr();
    RegexNode* parse_concat();
    RegexNode* parse_unary();
    RegexNode* parse_atom();
    static void print_ast_helper(const RegexNode* root, int indent);
public:
    RegexNode* parse(const std::string& regex);
    static void print_ast(const RegexNode* root);
};

#endif //PARALLEL_REGEX_ENGINE_REGEXPARSER_H