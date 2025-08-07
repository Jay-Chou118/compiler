想学习一下编译器底层相关的设计和思维，便创建了这个仓库。用于记录简单编译器的实现。

**part1**         -2025.8.7

    1. 实现简单的编译器基本的解析方法：详细见complier.cpp
    通过使用token分解，将“let x = 3 + 5 * (2 - 1);”这一行语句分解为多个token。

**part2**


    2.AST树的构建
    采用递归的方法
    先处理整个表达式
    然后将表达式细分为更细的结构——乘除段和更细的整数段。
    再细分到最终只有整数。

```cpp
std::shared_ptr<Expr> parse_factor(const std::vector<Token>& tokens, int& pos) {
    if (tokens[pos].type == "INTEGER") {
        return make_shared<Expr>(ExprType::Integer, tokens[pos++].value);
    } else if (tokens[pos].type == "LPAREN") {
        pos++; // skip '('
        auto node = parse_expression(tokens, pos);
        if (tokens[pos].type != "RPAREN") {
            throw runtime_error("Expected ')'");
        }
        pos++; // skip ')'
        return node;
    } else {
        throw runtime_error("Unexpected token in factor");
    }
}
```
    然后需要另外一个结构体来保存整棵AST树
    



