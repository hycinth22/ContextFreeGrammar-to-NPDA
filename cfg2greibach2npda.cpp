//
// 需C++11标准编译。
// 规定A-Z为非终结符，右侧为0代表空产生式。
// 右侧可以用|分割表示多个产生式，但不要有多余空格
//
//S->aSA|bB
//A->cA|c|0
//B->bB|cA|0
//- -
#include<iostream>
#include<string>
#include<queue>
#include<stack>
#include<vector>
#include<set>
#include<map>
#include<algorithm>
#include<cstdlib>
#include<cassert>
using namespace std;

inline bool isNonTerminal(const char x) { return x >= 'A' && x <= 'Z'; }

inline bool isTerminal(const char x) { return !isNonTerminal(x); }

// 在代入产生式时，需要拼接右部。concatRight方便解决可能其中一个是空的问题，防止出现εA、Aε的输出（应是A）
string concatRight(const string &a, const string &b) {
    if (a == "0") return b; else if (b == "0") return a;
    return a + b;
}

string concatRight(const string &a, char b) {
    if (a == "0") return string(1, b); else if (b == '0') return a;
    return a + b;
}

class ContextFreeGrammar {
public:
    map<char, set<string> > prod; // char → string

    // 消除左递归
    void eliminateLeftRecursion() {
        for (auto i = prod.begin(); i != prod.end(); ++i) {
            const char &AiLeft = i->first;
            cout << "正在寻找 " << AiLeft << " 的间接左递归" << endl;
            for (auto j = prod.begin(); j != i; ++j) {
                const char &AjLeft = j->first;
                for (auto ii = i->second.begin(); ii != i->second.end();) {
                    const string &AiRight = *ii;
                    if (!AiRight.empty() && AiRight[0] == AjLeft) {
                        for (const string &AjRight: j->second) {
                            string newAiRight = concatRight(AjRight, AiRight.substr(1));
                            cout << "  把 " << AjLeft << " -> " << AjRight << " 代入 " << AiLeft << " -> " << AiRight;
                            cout << "   得到 " << AiLeft << " -> " << newAiRight << endl;
                            prod[AiLeft].insert(newAiRight);
                        }
                        ii = i->second.erase(ii);
                    } else {
                        ++ii;
                    }
                }
            }
            // 为Ai的所有产生式消除直接左递归
            set<string> oldright = i->second;
            bool has = false;
            char z = allocateNewToken();
            for (auto it = oldright.begin(); it != oldright.end();) {
                const string &right = *it;
                if (right.size() > 1 && right[0] == AiLeft) {
                    cout << "正在消除直接左递归产生式 " << AiLeft << " -> " << right << endl;
                    has = true;
                    string a = right.substr(1);
                    prod[z].insert(a);
                    prod[z].insert(a + z);
                    i->second.erase(i->second.find(right));
                    it = oldright.erase(it);
                    cout << "  引入 " << z << " -> " << a << endl;
                    cout << "  引入 " << z << " -> " << a + z << endl;
                } else {
                    ++it;
                }
            }
            if (has) {
                for (auto &right: oldright) {
                    if ((!right.empty()) && right[0] != AiLeft) {
                        prod[AiLeft].insert(concatRight(right, z));
                        cout << "  引入 " << AiLeft << " -> " << concatRight(right, z) << endl;
                    }
                }
                cout << "已消除 " << AiLeft << " 的所有左递归 " << endl;
            }
        }
    }

    // 消除空产生式
    void eliminateEpsilon() {
        // 找到可空集合（所有经过若干步推导最终可以推出空的非终结符）
        set<char> T;
        bool changed;
        do {
            changed = false; // 发生至少一次插入会被置为true
            for (auto &p: prod) {
                for (auto &right: p.second) {
                    if (right == "0" || all_of(right.begin(), right.end(),
                                               [&T](char ch) { return isNonTerminal(ch) && T.count(ch); })) {
                        auto ret = T.insert(p.first);
                        changed = changed || ret.second;
                    }
                }
            }
        } while (changed);
        cout << "可空集合（经过若干步可以推出空的非终结符）:";
        for (char ch: T) {
            cout << ch;
        }
        cout << endl;
        // 插入新产生式
        for (auto it = prod.begin(); it != prod.end(); ++it) {
            set<string> &allright = it->second;
            for (auto ix = allright.begin(); ix != allright.end(); ++ix) {
                const string &right = *ix;
                //cout << "检查产生式是否含可空变量" << it->first << " -> " << right << endl;
                vector<size_t> PosOfNullableVar;
                auto iy = find_if(right.begin(), right.end(), [&T](char ch) { return T.count(ch); });
                while (iy != right.end()) {
                    //cout << "含有可空变量" << right[iy - right.begin()] << endl;
                    PosOfNullableVar.push_back(iy - right.begin());
                    iy = find_if(iy + 1, right.end(), [&T](char ch) { return T.count(ch); });
                }
                if (PosOfNullableVar.empty()) continue;
                vector<string> prodRight, nextRight;
                prodRight.push_back(right);
                const char DEL_POS = '\1';
                for (size_t i = 0; i < PosOfNullableVar.size(); i++) {
                    nextRight.clear();
                    for (string newRight: prodRight) {
                        nextRight.push_back(newRight);
                        newRight[PosOfNullableVar[i]] = DEL_POS;
                        nextRight.push_back(newRight);
                    }
                    prodRight = nextRight;
                }
                for (size_t i = 0; i < prodRight.size(); i++) {
                    auto end = remove(prodRight[i].begin(), prodRight[i].end(), DEL_POS);
                    prodRight[i].resize(end - prodRight[i].begin());
                    if (!prodRight[i].empty()) prod[it->first].insert(prodRight[i]);
                }
            }
        }
        // 删除所有空产生式
        for (auto it = prod.begin(); it != prod.end(); ++it) {
            set<string> &allright = it->second;
            for (auto ix = allright.begin(); ix != allright.end();) {
                if (*ix == "0") {
                    ix = allright.erase(ix);
                } else {
                    ++ix;
                }
            }
        }
        if (T.count('S')) {
            prod['S'].insert("0");
        }
    }

    // 消除单一产生式
    void eliminateSingle() {
        for (auto &p: prod) {
            const char &left = p.first;
            set<char> chainSet = _getChainSet(left);
            for (auto &right: chainSet) {
                prod[left].erase(string(1, right));
                prod[left].insert(prod[right].begin(), prod[right].end());
            }
        }
    }

    // 消除无用符号
    void removeUselessSymbol() {
        // 两步顺序不能调换
        _deleteUselessSymbol1(); // 删除不能经过若干步最终推导出句子（最终全为终结符）的非终结符
        _deleteUselessSymbol2(); // 删除从开始符号S不能推导出的非终结符
    }

    void transformFirstSymbolToTerminal() {
        // 所有开头位置的非终结符，使用其产生式代入改为终结符
        // 将所有不以终结符号开头的产生式A->Bxxx 用B->yyy代入
        for (auto &p: prod) {
            const char &ALeft = p.first;
            for (auto pr = p.second.begin(); pr != p.second.end();) {
                const string &ARight = *pr;
                if (!ARight.empty() && isNonTerminal(ARight[0])) {
                    stack<string> sta;
                    sta.push(ARight);
                    while (!sta.empty()) {
                        string newRight = sta.top();
                        sta.pop();
                        if (!newRight.empty() && isNonTerminal(newRight[0])) {
                            char B = newRight[0];
                            for (const string &BRight: prod[B]) {
                                sta.push(concatRight(BRight, ARight.substr(1)));
                            }
                        } else {
                            prod[ALeft].insert(newRight);
                        }
                    }
                    pr = p.second.erase(pr);
                } else {
                    ++pr;
                }
            }
        }
    }

    void transformIntoNonTerminalExceptFirst() {
        // 将后面的终结符改为非终结符
        map<char, set<string> > newProd;
        map<char, char> reverseMap; // 反向映射终结符对应的非终结符
        for (auto &p: prod) {
            const char &left = p.first;
            for (const string &right: p.second) {
                string newRight = right;
                for (int i = 1; i < newRight.size(); ++i) {
                    if (isTerminal(newRight[i])) {
                        if (reverseMap.count(newRight[i])) {
                            newRight[i] = reverseMap[newRight[i]];
                        } else {
                            char t = allocateNewToken();
                            reverseMap[newRight[i]] = t;
                            prod[t].insert(string(1, newRight[i]));
                            newProd[t].insert(string(1, newRight[i]));
                            newRight[i] = t;
                        }
                    }
                }
                newProd[left].insert(newRight);
            }
        }
        prod = newProd;
    }

    void output() {
        // 先输出S，再输出其他
        output('S');
        for (auto &p: prod) {
            if (p.first == 'S') continue;
            output(p.first);
        }
        cout << endl;
    }

    void output(char left) {
        bool first = true;
        cout << left << " -> ";
        for (auto &right: prod[left]) {
            if (first) first = false;
            else cout << " | ";
            cout << right;
        }
        cout << endl;
    }

private:
    char allocateNewToken() {
        char ch = 'A';
        for (auto &p: prod) {
            if (ch > 'Z') {
                cerr << "错误：！非终结符符号已用尽！" << endl;
                return ' ';
            }
            if (ch < p.first) return ch;
            else if (ch == p.first) ++ch;
        }
        return ch;
    }

    set<char> _getChainSet(char left) {
        set<char> T;
        for (const string &right: prod[left]) {
            if (right.size() == 1 && isNonTerminal(right[0])) {
                T.insert(right[0]);
                set<char> subT = _getChainSet(right[0]);
                T.insert(subT.begin(), subT.end());
            }
        }
        return T;
    }

    void _deleteUselessSymbol1() {
        // 找到所有经过若干步推导最终可以推出句子（最终全为终结符）的非终结符   V ->* T*
        set<char> S1;
        bool changed;
        do {
            changed = false; // 发生至少一次插入会被置为true
            for (auto &p: prod) {
                for (auto &right: p.second) {
                    if (all_of(right.begin(), right.end(),
                               [&S1](char ch) { return isTerminal(ch) || S1.count(ch); })) {
                        auto ret = S1.insert(p.first);
                        changed = changed || ret.second;
                    }
                }
            }
        } while (changed);
        cout << "可以推出句子的非终结符集合: ";
        for (char ch: S1) {
            cout << ch;
        }
        cout << "(不在此集合中的一定是无用符号)" << endl;
        // 删除_不_可以推出句子的非终结符（不含在集合S1中的非终结符）的产生式，包括其在左边和右边两种情况都需要删除
        for (auto it = prod.begin(); it != prod.end();) {
            if (!S1.count(it->first)) {
                cout << "发现无用符号 " << it->first << "，删除出现在左端的所有产生式" << endl;
                it = prod.erase(it);
            } else {
                set<string> &allright = it->second;
                for (auto ix = allright.begin(); ix != allright.end();) {
                    if (any_of(ix->begin(), ix->end(),
                               [&S1](char ch) { return isNonTerminal(ch) && !S1.count(ch); })) {
                        cout << "无用符号出现在右端，产生式 " << it->first << " -> " << *ix << " 被删除" << endl;
                        ix = allright.erase(ix);
                    } else {
                        ++ix;
                    }
                }
                ++it;
            }
        }
    }

    void _deleteUselessSymbol2() {
        // 找到从开始符号S可以推出的全部非终结符V。 即存在S ->* AVC,  A、C可以是终结符也可以是非终结符
        set<char> S2;
        queue<char> q;
        q.push('S');
        while (!q.empty()) {
            char left = q.front();
            S2.insert(left);
            for (auto &right: prod[left]) {
                for (char ch: right) {
                    if (isNonTerminal(ch) && S2.count(ch) == 0) {
                        q.push(ch);
                    }
                }
            }
            q.pop();
        }
        cout << "从开始符号S可以到达的非终结符集合: ";
        for (char ch: S2) {
            cout << ch;
        }
        cout << "(不在此集合中的一定是无用符号)" << endl;
        // 删除从开始符号S不可以推出的非终结符（不含在集合S2中的非终结符）的产生式
        for (auto it = prod.begin(); it != prod.end();) {
            if (!S2.count(it->first)) {
                cout << "发现无用符号 " << it->first << "，删除出现在左端的所有产生式" << endl;
                it = prod.erase(it);
            } else {
                ++it;
            }
        }
    }

};


typedef int NPDAState;
typedef pair<NPDAState, string> NPDANext; // 两个元素分别代表下一个状态，入栈符号
const NPDAState FINAL = 0x7f7f7f7f;
const char EMPTY = 'z';
const char STACK_BOTTOM = 'z';
const string EMPTY_STACK = string(1, STACK_BOTTOM);
class NPDA {
    map<tuple<NPDAState, char, char>, set<NPDANext> > rules; // tuple三个元素分别代表当前状态，当前读入，栈顶符号。

public:
    NPDA(const ContextFreeGrammar &g) {
        rules[make_tuple(0, EMPTY, STACK_BOTTOM)].insert(make_pair(1, string("S")+STACK_BOTTOM ));
        rules[make_tuple(1, EMPTY, STACK_BOTTOM)].insert(make_pair(FINAL, EMPTY_STACK));
        for (auto &p: g.prod) {
            const char &left = p.first;
            for (auto pr = p.second.begin(); pr != p.second.end(); ++pr) {
                const string &right = *pr;
                char ch = right[0];
                if (right.empty() || isNonTerminal(ch)) {
                    cerr << "输入文法错误，不是Greibach范式" << endl;
                    return;
                }
                if (ch == '0') {
                    if (left != 'S') {
                        cerr << "Greibach范式的空产生式左侧只能为开始符号S" << endl;
                        return;
                    }
                    rules[make_tuple(1, EMPTY, 'S')].insert(make_pair(FINAL, ""));
                } else {
                    rules[make_tuple(1, ch, left)].insert(make_pair(1, right.substr(1)));
                }
            }
        }
    }

    bool canAccepted(const string &text) {
        vector<pair<NPDAState, string> > current, next; //  由于是非确定型，需要存储一组当前信息
        current.emplace_back(0, EMPTY_STACK); // 起始状态
        bool matched = false;
        for (int i = 0; i < text.size(); ++i) {
            next.clear();
            for (int k = 0; k < current.size(); ++k) {
                pair<NPDAState, string> cur = current[k];
                NPDAState state = cur.first;
                const string &stack = cur.second;

                auto epsilonRule = rules.find(make_tuple(state, EMPTY, stack[0]));
                if (epsilonRule != rules.end()) {
                    for (const pair<NPDAState, string> &transfer: epsilonRule->second) {
                        pair<NPDAState, string> o = make_pair(transfer.first,nextStack(stack, transfer.second));
                        if (std::find(current.begin(), current.end(), o) == current.end()) {
                            // cout << o.first << "," << o.second << endl;
                            current.push_back(o);
                        }
                    }
                }

                auto r = rules.find(make_tuple(state, text[i], stack[0]));
                if (r != rules.end()) {
                    for (const pair<NPDAState, string> &transfer: r->second) {
                        pair<NPDAState, string> o = make_pair(transfer.first,
                                                              nextStack(std::get<1>(cur), transfer.second));
                        if (std::find(next.begin(), next.end(), o) == next.end()) {
                            next.push_back(o);
                       }
                    }
                    matched = true;
                }
            }
            if (!matched) {
                cout<<"当前状态没有任何规则可以匹配输入符号"<<text[i]<<endl;
                return false;
            }
            cout << "读取到符号" << text[i] << endl;
            for (pair<NPDAState, string> &s: next) {
                cout << "{状态" << s.first << ", " << s.second.substr(0, s.second.size()) << "}" << endl;
            }
            current = next;
            matched = false;
        }
        for (int k = 0; k < current.size(); ++k) {
            pair<NPDAState, string> cur = current[k];
            NPDAState state = cur.first;
            string &stack = cur.second;
            auto epsilonRule = rules.find(make_tuple(state, EMPTY, stack[0]));
            if (epsilonRule != rules.end()) {
                for (const pair<NPDAState, string> &transfer: epsilonRule->second) {
                    current.emplace_back(transfer.first, nextStack(cur.second, transfer.second));
                    if (current.back().first == FINAL && current.back().second==EMPTY_STACK ) return true;
                }
            }
        }
        return false;
    }

    static string nextStack(const string &oldStack, const string &x) {
        return x + oldStack.substr(1);
    }

    void showRules() {
        for (const auto &p: rules) {
            int state;
            char inputChar;
            char stackTop;
            tie(state, inputChar, stackTop) = p.first;

            cout << "δ(" << state << ",";
            if (inputChar == EMPTY) {
                cout << "ε";
            } else {
                cout << inputChar;
            }
            cout << ",";
            if (stackTop == EMPTY) {
                cout << "z";
            } else {
                cout << stackTop;
            }
            cout << ")= { ";
            bool first = true;
            for (const auto &next: p.second) {
                if (first) first = false;
                else cout << ", ";
                cout << "(";
                if (next.first == FINAL) {
                    cout << "F";
                } else {
                    cout << next.first;
                }
                string pushStack = next.second;
                if (pushStack.empty()) pushStack = "ε";
                else if (pushStack.back() == STACK_BOTTOM) pushStack.back()='z';
                cout << ", " << pushStack << ")";
            }
            cout << " }" << endl;
        }
        cout << endl;
    }
};

set<string> splitStringToSet(const string &str, char delim) {
    set<string> s;
    size_t kb = 0, ke = str.find(delim);
    while (ke != string::npos) {
        s.insert(str.substr(kb, ke - kb));
        kb = ke + 1;
        ke = str.find(delim, kb);
    }
    s.insert(str.substr(kb));
    return s;
}

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
                cerr << "文法错误，右侧除了表示空产生式不能含有0";
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

    g.eliminateLeftRecursion();
    cout << "消除左递归后:" << endl;
    g.output();

    g.eliminateEpsilon();
    cout << "消除空产生式后:" << endl;
    g.output();

    g.eliminateSingle();
    cout << "消除单一产生式后:" << endl;
    g.output();

    g.transformFirstSymbolToTerminal();
    cout << "消除非终结符开头后:" << endl;
    g.output();

    g.removeUselessSymbol();
    cout << "消除无用符号后:" << endl;
    g.output();

    g.transformIntoNonTerminalExceptFirst();
    cout << "Greibach范式:" << endl;
    g.output();

    NPDA npda(g);
    cout << "NPDA转移规则:" << endl;
    npda.showRules();

    string text;
    cout << "输入一行句子判断是否属于此文法描述的语言（直接回车可以判断空串是否属于）：" << endl;
    while (getline(cin, text)) {
        if (npda.canAccepted(text)) {
            cout << (text.empty()?"ε":text) << "属于该语言" << endl;
        } else {
            cout << (text.empty()?"ε":text) << "不属于该语言" << endl;
        }
        cout << endl;
    }
    return 0;
}

