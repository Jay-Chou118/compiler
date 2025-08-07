#include <iostream>
#include <vector>


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



string ourtest = "let x = 3 + 5 * (2 - 1);";

int main(){
    vector<Token> ans;
    int pot = 0;
    while(pot < ourtest.size()) {
        Token token;
        token = next_token(ourtest,pot);
        ans.push_back(token);
    }

    for(int i = 0;i < ans.size();++i){
        cout << i+1 << " " << ans[i].type << " " << ans[i].value << endl;
    }


    return 0;
}