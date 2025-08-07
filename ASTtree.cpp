#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <stdexcept>
#include <fstream>
#include <sstream>

using namespace std;


struct Token {
    std::string type;  // 比如 IDENTIFIER, INTEGER, SYMBOL
    std::string value; // 原始文本
};


bool is_keyword(const std::string& word) {
    return word == "let";
}

vector<Token> tokens;
Token next_token(const std::string &input, int &pos){
    while (isspace(input[pos])) pos++;
    Token token;
    if(isalpha(input[pos])){
        while (isalpha(input[pos])) {
            token.value += input[pos];
            pos++;
        }
        token.type = (is_keyword(token.value)) ? "KEYWORD" : "IDENTIFIER";
    }else if (isdigit(input[pos])) {
        while (isdigit(input[pos])) {
            token.value += input[pos++];
        }
        token.type = "INTEGER";
    }else if (ispunct(input[pos])) {
        char c = input[pos++];
        token.value = std::string(1, c);
        switch (c) {
            case '=': token.type = "ASSIGN"; break;
            case '+': token.type = "PLUS"; break;
            case '-': token.type = "MINUS"; break;
            case '*': token.type = "STAR"; break;
            case '(': token.type = "LPAREN"; break;
            case ')': token.type = "RPAREN"; break;
            case ';': token.type = "SEMICOLON"; break;
            default: token.type = "UNKNOWN";
        }
    }
    return token;
}


/**
 * 将表达式化为一个AST抽象树
 * 对于““let x = 3 + 5 * (2 - 1);”这一行语句，
 * 其AST树的结构如下：
 *    +
     / \
    3   *
       / \
      5   -
         / \
        2   1

 */

 enum class ExprType { Integer, Identifier, BinaryOp };

struct Expr {
    ExprType type;

    // 只有 Integer 或 Identifier 用这个字段
    std::string value;

    // 只有 BinaryOp 用这个字段
    std::string op;
    std::shared_ptr<Expr> left;
    std::shared_ptr<Expr> right;

    Expr(ExprType t, const std::string& val) : type(t), value(val), left(nullptr), right(nullptr) {}
    Expr(const std::string& oper, std::shared_ptr<Expr> l, std::shared_ptr<Expr> r)
        : type(ExprType::BinaryOp), op(oper), left(l), right(r) {}
};


/** 
1 KEYWORD let
2 IDENTIFIER x
3 ASSIGN =
4 INTEGER 3
5 PLUS +
6 INTEGER 5
7 STAR *
8 LPAREN (
9 INTEGER 2
10 MINUS -
11 INTEGER 1
12 RPAREN )
13 SEMICOLON ;
 */

std::shared_ptr<Expr> parse_expression(const std::vector<Token>& tokens, int& pos);
std::shared_ptr<Expr> parse_term(const std::vector<Token>& tokens, int& pos);
std::shared_ptr<Expr> parse_factor(const std::vector<Token>& tokens, int& pos);



// 处理数字、括号表达式
std::shared_ptr<Expr> parse_factor(const std::vector<Token>& tokens, int& pos) {

    if (tokens[pos].type == "MINUS") {
    pos++; // 吃掉 -
    auto inner = parse_factor(tokens, pos);
    // 构造一个 Unary Minus 操作，作为 BinaryOp("-", 0, inner)
    auto zero = std::make_shared<Expr>(ExprType::Integer, "0");
    return std::make_shared<Expr>("-", zero, inner);
    }else if (tokens[pos].type == "INTEGER") {
        return make_shared<Expr>(ExprType::Integer, tokens[pos++].value);
    } else if (tokens[pos].type == "LPAREN") {
        pos++; // skip '('
        auto node = parse_expression(tokens, pos);
        if (tokens[pos].type != "RPAREN") {
            throw runtime_error("Expected ')'");
        }
        pos++; // skip ')'
        return node;
    } else if (tokens[pos].type == "IDENTIFIER") {
        return make_shared<Expr>(ExprType::Identifier, tokens[pos++].value);
    } else {
        throw runtime_error("Unexpected token in factor");
    }
}

std::shared_ptr<Expr> parse_term(const std::vector<Token>& tokens, int& pos) {
    auto left = parse_factor(tokens, pos);
    while (pos < tokens.size() && (tokens[pos].type == "STAR" || tokens[pos].type == "SLASH")) {
        std::string op = tokens[pos++].value;
        auto right = parse_factor(tokens, pos);
        left = std::make_shared<Expr>(op, left, right);
    }
    return left;
}
std::shared_ptr<Expr> parse_expression(const std::vector<Token>& tokens, int& pos) {
    auto left = parse_term(tokens, pos);
    while (pos < tokens.size() && (tokens[pos].type == "PLUS" || tokens[pos].type == "MINUS")) {
        std::string op = tokens[pos++].value;
        auto right = parse_term(tokens, pos);
        left = std::make_shared<Expr>(op, left, right);
    }
    return left;
}

void print_ast(std::shared_ptr<Expr> node, int indent = 0) {
    if (!node) return;

    std::string space(indent * 2, ' '); // 每层缩进2个空格
    //
    if (node->type == ExprType::Integer || node->type == ExprType::Identifier) {
        std::cout << space << node->value << std::endl;
    } else if (node->type == ExprType::BinaryOp) {
        std::cout << space << node->op << std::endl;
        print_ast(node->left, indent + 1);
        print_ast(node->right, indent + 1);
    }
}


enum class StmtType { Assignment };

struct Stmt {
    StmtType type;

    // for Assignment
    std::string var_name;
    std::shared_ptr<Expr> value;

    Stmt(const std::string& name, std::shared_ptr<Expr> val)
        : type(StmtType::Assignment), var_name(name), value(val) {}
};


Stmt parse_statement(const std::vector<Token>& tokens, int& pos) {
    // expect: let IDENT = expression ;

    if (tokens[pos].type != "KEYWORD" || tokens[pos].value != "let") {
        throw std::runtime_error("Expected 'let'");
    }
    pos++; // skip 'let'

    if (tokens[pos].type != "IDENTIFIER") {
        throw std::runtime_error("Expected identifier after 'let'");
    }
    std::string var_name = tokens[pos++].value;

    if (tokens[pos].type != "ASSIGN") {
        throw std::runtime_error("Expected '='");
    }
    pos++; // skip '='

    auto expr = parse_expression(tokens, pos);

    if (tokens[pos].type != "SEMICOLON") {
        throw std::runtime_error("Expected ';'");
    }
    pos++; // skip ';'

    return Stmt(var_name, expr);
}

// int eval_expr(std::shared_ptr<Expr> node) {
//     if (node->type == ExprType::Integer) {
//         return std::stoi(node->value); // 把字符串变成数字
//     } else if (node->type == ExprType::BinaryOp) {
//         int left = eval_expr(node->left);
//         int right = eval_expr(node->right);
//         if (node->op == "+") return left + right;
//         else if (node->op == "-") return left - right;
//         else if (node->op == "*") return left * right;
//         else if (node->op == "/") return left / right; // 简化：不考虑除0
//         else throw std::runtime_error("Unknown operator: " + node->op);
//     } else {
//         throw std::runtime_error("Unsupported ExprType in eval");
//     }
// }



std::unordered_map<std::string, int> env;

int eval_expr(std::shared_ptr<Expr> node) {
    if (node->type == ExprType::Integer) {
        return std::stoi(node->value);
    } else if (node->type == ExprType::Identifier) {
        if (env.count(node->value)) {
            return env[node->value];
        } else {
            throw std::runtime_error("Undefined variable: " + node->value);
        }
    } else if (node->type == ExprType::BinaryOp) {
        int left = eval_expr(node->left);
        int right = eval_expr(node->right);
        if (node->op == "+") return left + right;
        if (node->op == "-") return left - right;
        if (node->op == "*") return left * right;
        if (node->op == "/") return left / right;
    }
    throw std::runtime_error("Unsupported expression type");
}

void exec_stmt(const Stmt& stmt) {
    if (stmt.type == StmtType::Assignment) {
        int value = eval_expr(stmt.value);
        env[stmt.var_name] = value;
        std::cout << stmt.var_name << " = " << value << std::endl;
    }
}


std::vector<Stmt> parse_program(const std::vector<Token>& tokens) {
    std::vector<Stmt> stmts;
    int pos = 0;
    while (pos < tokens.size()) {
        stmts.push_back(parse_statement(tokens, pos));
    }
    return stmts;
}

void exec_program(const std::vector<Stmt>& stmts) {
    for (const auto& stmt : stmts) {
        exec_stmt(stmt);
    }
}


std::unordered_map<std::string, std::string> llvm_vars;
std::vector<std::string> llvm_ir;
int reg_id = 0;

std::string new_reg() {
    return "%" + std::to_string(reg_id++);
}


std::string generate_llvm_ir(std::shared_ptr<Expr> node) {
    if (node->type == ExprType::Integer) {
        return node->value;
    } else if (node->type == ExprType::Identifier) {
        std::string var_ptr = llvm_vars[node->value];
        std::string result = new_reg();
        llvm_ir.push_back("  " + result + " = load i32, i32* " + var_ptr);
        return result;
    } else if (node->type == ExprType::BinaryOp) {
        std::string left = generate_llvm_ir(node->left);
        std::string right = generate_llvm_ir(node->right);
        std::string result = new_reg();
        std::string op;
        if (node->op == "+") op = "add";
        else if (node->op == "-") op = "sub";
        else if (node->op == "*") op = "mul";
        else if (node->op == "/") op = "sdiv";
        else throw std::runtime_error("Unsupported operator");
        llvm_ir.push_back("  " + result + " = " + op + " i32 " + left + ", " + right);
        return result;
    }
    throw std::runtime_error("Unsupported ExprType for LLVM IR");
}


void generate_llvm_stmt(const Stmt& stmt) {
    if (stmt.type == StmtType::Assignment) {
        std::string result = generate_llvm_ir(stmt.value); // 表达式部分
        std::string var_ptr = "%" + stmt.var_name;
        llvm_vars[stmt.var_name] = var_ptr;

        llvm_ir.push_back("  " + var_ptr + " = alloca i32");
        llvm_ir.push_back("  store i32 " + result + ", i32* " + var_ptr);
    }
}




// string ourtest = "let x = 3 + 5 * (2 - 1);";
string ourtest = "let x =-3+5*(2-1);";
 int main(){
    vector<Token> ans;
    int pos = 0;
    while(pos < ourtest.size()) {
        Token token;
        token = next_token(ourtest,pos);
        ans.push_back(token);
    }

    for(int i = 0;i < ans.size();++i){
        cout << i+1 << " " << ans[i].type << " " << ans[i].value << endl;
    }
    // pos = 0;

    // shared_ptr<Expr> ast = parse_expression(ans, pos);
    // cout << "AST:" << endl;
    // print_ast(ast);
    // auto stmt = parse_statement(ans, pos);
    // cout << "Parsed Statement:" << endl;
    // cout << "Variable: " << stmt.var_name << std::endl;
    // cout << "Value: " << std::endl;
    // print_ast(stmt.value);

    // int result = eval_expr(stmt.value);
    // cout << "Computed result: " << result << endl;  
    // exec_stmt(stmt);

    std::string input = "let x = 1 + 2; let y = x * 3;";
    std::vector<Token> tokens;
    pos = 0;
    while (pos < input.size()) {
        tokens.push_back(next_token(input, pos));
    }

    auto stmts = parse_program(tokens);
    exec_program(stmts);

    reg_id = 1;
    llvm_ir.clear();
    llvm_vars.clear();
    ;
    for (const auto& stmt : stmts) {
        generate_llvm_stmt(stmt);
    }

    cout << "define i32 @main() {\nentry:\n";
    for (const auto& line : llvm_ir) {
        cout << line << "\n";
    }
    cout << "  ret i32 0\n}\n";

    ofstream fout("output.ll");
    fout << "define i32 @main() {\n";
    fout << "entry:\n";
    for (const auto& line : llvm_ir) {
        fout << line << "\n";
    }
    fout << "  ret i32 0\n";
    fout << "}\n";
    fout.close();


    return 0;
}