#ifndef CFG2NPDA_CFG_HXX
#define CFG2NPDA_CFG_HXX

#include "utils.h"
#include<queue>
#include<stack>
#include<algorithm>
#include<map>
#include<set>
#include<string>
#include <iostream>
using namespace std;

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
                for (auto ii = i->second.begin(); ii != i->second.end();) { // 查找i的所有产生式中形如i -> j r的
                    const string &AiRight = *ii;
                    if (!AiRight.empty() && AiRight[0] == AjLeft) { //  形如i -> j r的
                        for (const string &AjRight: j->second) { // 用j的所有产生式 j → δ1|δ2|…|δk 去替换 i → j r
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
            _eliminateDirectLeftRecursion(i->first, i->second, i); // 为Ai的所有产生式消除直接左递归
        }
    }

    void _eliminateDirectLeftRecursion(char AiLeft, set<string> oldright, map<char, set<string>>::iterator &pi) {
        bool has = false;
        char z = allocateNewToken();
        for (auto it = oldright.begin(); it != oldright.end();) {
            const string &right = *it;
            if (right.size() > 1 && right[0] == AiLeft) {
                cout << "正在消除直接左递归产生式 " << AiLeft << " -> " << right << endl;
                has = true;
                string a = right.substr(1);
                prod[z].insert(a);
                cout << "  引入 " << z << " -> " << a << endl;
                prod[z].insert(a + z);
                cout << "  引入 " << z << " -> " << a + z << endl;
                pi->second.erase(pi->second.find(right));
                it = oldright.erase(it);
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
            cout << "已消除 " << AiLeft << " 的左递归 " << endl;
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
    void eliminateOnlySingle() {
        for (auto &p: prod) {
            const char &left = p.first;
            vector<char> chainSet = _getChainSet(left);
            for (auto &right: chainSet) {
                prod[left].erase(string(1, right));
                prod[left].insert(prod[right].begin(), prod[right].end());
            }
        }
    }

    // 消除无用符号
    void removeUselessSymbol() {
        // 两步顺序不能调换
        _deleteUselessSymbol1(); // 删除不能最终推导出终结符串的非终结符
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

    vector<char> _getChainSet(char left) {
        vector<char> T;
        for (const string &right: prod[left]) {
            if (right.size() == 1 && isNonTerminal(right[0])) {
                T.push_back(right[0]);
                vector<char> subT = _getChainSet(right[0]);
                T.insert(T.end(), subT.begin(), subT.end());
            }
        }
        T.erase(unique(T.begin(), T.end()), T.end()); // 去重
        return T;
    }

    void _deleteUselessSymbol1() {  // 删除不能最终推导出终结符串的非终结符
        // 先找到所有经过若干步推导最终可以推出终结符串（最终全为终结符）的非终结符   V ->* T*
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

    void _deleteUselessSymbol2() { // 删除从开始符号S不能推导出的非终结符
        // 先找到从开始符号S可以推出的全部非终结符V。 即存在S ->* AVC,  A、C可以是终结符也可以是非终结符
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

#endif //CFG2NPDA_CFG_HXX
