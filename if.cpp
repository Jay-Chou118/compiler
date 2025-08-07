// LLVM IR if-else 结构生成器（逐步构建版）

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <sstream>

using namespace std;

int reg_id = 1;
int label_id = 0;
vector<string> llvm_ir;
unordered_map<string, string> llvm_vars;

string new_reg() {
    return "%" + to_string(reg_id++);
}

string new_label(const string& base) {
    return base + to_string(label_id++);
}

int main() {
    // 假设源代码: 
    // let x = 3;
    // let y = 5;
    // if (x < y) {
    //   let z = 1;
    // } else {
    //   let z = 2;
    // }

    // 基本结构
    llvm_ir.push_back("define i32 @main() {");
    llvm_ir.push_back("entry:");

    // x = 3
    string x_ptr = "%x";
    llvm_vars["x"] = x_ptr;
    llvm_ir.push_back("  " + x_ptr + " = alloca i32");
    llvm_ir.push_back("  store i32 3, i32* " + x_ptr);

    // y = 5
    string y_ptr = "%y";
    llvm_vars["y"] = y_ptr;
    llvm_ir.push_back("  " + y_ptr + " = alloca i32");
    llvm_ir.push_back("  store i32 5, i32* " + y_ptr);

    // 条件比较 x < y
    string xval = new_reg();
    llvm_ir.push_back("  " + xval + " = load i32, i32* " + x_ptr);
    string yval = new_reg();
    llvm_ir.push_back("  " + yval + " = load i32, i32* " + y_ptr);
    string cond = new_reg();
    llvm_ir.push_back("  " + cond + " = icmp slt i32 " + xval + ", " + yval);

    // 条件跳转
    string then_label = new_label("then");
    string else_label = new_label("else");
    string end_label  = new_label("end");
    llvm_ir.push_back("  br i1 " + cond + ", label %" + then_label + ", label %" + else_label);

    // then:
    llvm_ir.push_back(then_label + ":");
    string z_then = new_reg();
    llvm_ir.push_back("  " + z_then + " = alloca i32");
    llvm_ir.push_back("  store i32 1, i32* " + z_then);
    llvm_ir.push_back("  br label %" + end_label);

    // else:
    llvm_ir.push_back(else_label + ":");
    string z_else = new_reg();
    llvm_ir.push_back("  " + z_else + " = alloca i32");
    llvm_ir.push_back("  store i32 2, i32* " + z_else);
    llvm_ir.push_back("  br label %" + end_label);

    // end:
    llvm_ir.push_back(end_label + ":");
    llvm_ir.push_back("  ret i32 0");
    llvm_ir.push_back("}");

    // 输出 LLVM IR
    for (const auto& line : llvm_ir) {
        cout << line << endl;
    }

    return 0;
}
