//
// 需C++11标准编译。
// 规定A-Z为非终结符，右侧为0代表空产生式。
// 右侧可以用|分割表示多个产生式，但不要有多余空格
//
//S->aSA|bB
//A->cA|c|0
//B->bB|cA|0
//- -
#include "npda.hxx"
#include "cfg.hxx"
#include "utils.h"
#include <iostream>
#include <string>
#include <set>
using namespace std;

int main() {
    ContextFreeGrammar g;
    cout << "说明：S为开始符号， 要按照A->B的格式输入，不能有多余空格" << endl
         << "右侧可以用|分割表示多个产生式，也不能有多余空格" << endl
         << "以空行或一行-结束输入。" << endl;
    cout << "输入文法：" << endl;
    string line;
    char input_left;
    string input_right;
    while (getline(cin, line) && !line.empty()) {
        if(line[0]=='-') break;
        if(! (line[1]=='-' && line[2]=='>') ) {
            cerr << "输入包含一行错误格式的文法" << endl;
            break;
        }
        input_left=line[0];
        input_right = line.substr(3);
        set<string> x = splitStringToSet(input_right, '|');
        for (auto right: x) {
            if (right.size()==1 && input_left == right[0]) {
                cerr << "文法错误，左右不能完全相同";
                return 0;
            }
            if (right.empty()) {
                cerr << "文法错误，右侧不能为空，空产生式用0表示";
                return 0;
            }
            if (right.size()>1 && right.find('0')!=string::npos) {
                cerr << "文法错误，右侧的0只能用于表示空产生式";
                return 0;
            }
            if (right[0]==' ') {
                cerr << "文法错误，右侧不能含有空格";
                return 0;
            }
            g.prod[input_left].insert(right);
        }
        //g.prod[input_left].insert(x.begin(), x.end());
    }
    if (g.prod['S'].empty()) {
        cerr << "文法错误，必须包含开始符号S";
        return 0;
    }
    cout << "初始文法:" << endl;
    g.output();

    g.removeUselessSymbol();
    cout << "消除无用符号后:" << endl;
    g.output();

    g.eliminateEpsilon();
    cout << "消除空产生式后:" << endl;
    g.output();

    g.eliminateOnlySingle();
    cout << "消除单一产生式后:" << endl;
    g.output();

    g.eliminateLeftRecursion();
    cout << "消除左递归后:" << endl;
    g.output();

    g.transformFirstSymbolToTerminal();
    cout << "消除非终结符开头后:" << endl;
    g.output();

    g.transformIntoNonTerminalExceptFirst();
    g.removeUselessSymbol();
    cout << "Greibach范式:" << endl;
    g.output();

    NPDA npda(g);
    cout << "NPDA转移规则:" << endl;
    npda.showRules();

    string text;
    cout << "输入一行句子判断是否属于此文法描述的语言（直接回车可以判断空串是否属于）：" << endl;
    while (getline(cin, text)) {
        if (npda.canAccepted(text)) {
            cout << "√ " << (text.empty() ? "ε" : text) << "属于该语言" << endl;
        } else {
            cout << "× " << (text.empty() ? "ε" : text) << "不属于该语言" << endl;
        }
        cout << endl;
    }
    return 0;
}

