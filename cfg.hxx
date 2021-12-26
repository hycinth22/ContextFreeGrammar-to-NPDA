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
    map<char, set<string> > prod; // char �� string
    // ������ݹ�
    void eliminateLeftRecursion() {
        for (auto i = prod.begin(); i != prod.end(); ++i) {
            const char &AiLeft = i->first;
            cout << "����Ѱ�� " << AiLeft << " �ļ����ݹ�" << endl;
            for (auto j = prod.begin(); j != i; ++j) {
                const char &AjLeft = j->first;
                for (auto ii = i->second.begin(); ii != i->second.end();) { // ����i�����в���ʽ������i -> j r��
                    const string &AiRight = *ii;
                    if (!AiRight.empty() && AiRight[0] == AjLeft) { //  ����i -> j r��
                        for (const string &AjRight: j->second) { // ��j�����в���ʽ j �� ��1|��2|��|��k ȥ�滻 i �� j r
                            string newAiRight = concatRight(AjRight, AiRight.substr(1));
                            cout << "  �� " << AjLeft << " -> " << AjRight << " ���� " << AiLeft << " -> " << AiRight;
                            cout << "   �õ� " << AiLeft << " -> " << newAiRight << endl;
                            prod[AiLeft].insert(newAiRight);
                        }
                        ii = i->second.erase(ii);
                    } else {
                        ++ii;
                    }
                }
            }
            _eliminateDirectLeftRecursion(i->first, i->second, i); // ΪAi�����в���ʽ����ֱ����ݹ�
        }
    }

    void _eliminateDirectLeftRecursion(char AiLeft, set<string> oldright, map<char, set<string>>::iterator &pi) {
        bool has = false;
        char z = allocateNewToken();
        for (auto it = oldright.begin(); it != oldright.end();) {
            const string &right = *it;
            if (right.size() > 1 && right[0] == AiLeft) {
                cout << "��������ֱ����ݹ����ʽ " << AiLeft << " -> " << right << endl;
                has = true;
                string a = right.substr(1);
                prod[z].insert(a);
                cout << "  ���� " << z << " -> " << a << endl;
                prod[z].insert(a + z);
                cout << "  ���� " << z << " -> " << a + z << endl;
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
                    cout << "  ���� " << AiLeft << " -> " << concatRight(right, z) << endl;
                }
            }
            cout << "������ " << AiLeft << " ����ݹ� " << endl;
        }
    }

    // �����ղ���ʽ
    void eliminateEpsilon() {
        // �ҵ��ɿռ��ϣ����о������ɲ��Ƶ����տ����Ƴ��յķ��ս����
        set<char> T;
        bool changed;
        do {
            changed = false; // ��������һ�β���ᱻ��Ϊtrue
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
        cout << "�ɿռ��ϣ��������ɲ������Ƴ��յķ��ս����:";
        for (char ch: T) {
            cout << ch;
        }
        cout << endl;
        // �����²���ʽ
        for (auto it = prod.begin(); it != prod.end(); ++it) {
            set<string> &allright = it->second;
            for (auto ix = allright.begin(); ix != allright.end(); ++ix) {
                const string &right = *ix;
                //cout << "������ʽ�Ƿ񺬿ɿձ���" << it->first << " -> " << right << endl;
                vector<size_t> PosOfNullableVar;
                auto iy = find_if(right.begin(), right.end(), [&T](char ch) { return T.count(ch); });
                while (iy != right.end()) {
                    //cout << "���пɿձ���" << right[iy - right.begin()] << endl;
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
        // ɾ�����пղ���ʽ
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

    // ������һ����ʽ
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

    // �������÷���
    void removeUselessSymbol() {
        // ����˳���ܵ���
        _deleteUselessSymbol1(); // ɾ�����������Ƶ����ս�����ķ��ս��
        _deleteUselessSymbol2(); // ɾ���ӿ�ʼ����S�����Ƶ����ķ��ս��
    }

    void transformFirstSymbolToTerminal() {
        // ���п�ͷλ�õķ��ս����ʹ�������ʽ�����Ϊ�ս��
        // �����в����ս���ſ�ͷ�Ĳ���ʽA->Bxxx ��B->yyy����
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
        // ��������ս����Ϊ���ս��
        map<char, set<string> > newProd;
        map<char, char> reverseMap; // ����ӳ���ս����Ӧ�ķ��ս��
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
        // �����S�����������
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
                cerr << "���󣺣����ս���������þ���" << endl;
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
        T.erase(unique(T.begin(), T.end()), T.end()); // ȥ��
        return T;
    }

    void _deleteUselessSymbol1() {  // ɾ�����������Ƶ����ս�����ķ��ս��
        // ���ҵ����о������ɲ��Ƶ����տ����Ƴ��ս����������ȫΪ�ս�����ķ��ս��   V ->* T*
        set<char> S1;
        bool changed;
        do {
            changed = false; // ��������һ�β���ᱻ��Ϊtrue
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
        cout << "�����Ƴ����ӵķ��ս������: ";
        for (char ch: S1) {
            cout << ch;
        }
        cout << "(���ڴ˼����е�һ�������÷���)" << endl;
        // ɾ��_��_�����Ƴ����ӵķ��ս���������ڼ���S1�еķ��ս�����Ĳ���ʽ������������ߺ��ұ������������Ҫɾ��
        for (auto it = prod.begin(); it != prod.end();) {
            if (!S1.count(it->first)) {
                cout << "�������÷��� " << it->first << "��ɾ����������˵����в���ʽ" << endl;
                it = prod.erase(it);
            } else {
                set<string> &allright = it->second;
                for (auto ix = allright.begin(); ix != allright.end();) {
                    if (any_of(ix->begin(), ix->end(),
                               [&S1](char ch) { return isNonTerminal(ch) && !S1.count(ch); })) {
                        cout << "���÷��ų������Ҷˣ�����ʽ " << it->first << " -> " << *ix << " ��ɾ��" << endl;
                        ix = allright.erase(ix);
                    } else {
                        ++ix;
                    }
                }
                ++it;
            }
        }
    }

    void _deleteUselessSymbol2() { // ɾ���ӿ�ʼ����S�����Ƶ����ķ��ս��
        // ���ҵ��ӿ�ʼ����S�����Ƴ���ȫ�����ս��V�� ������S ->* AVC,  A��C�������ս��Ҳ�����Ƿ��ս��
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
        cout << "�ӿ�ʼ����S���Ե���ķ��ս������: ";
        for (char ch: S2) {
            cout << ch;
        }
        cout << "(���ڴ˼����е�һ�������÷���)" << endl;
        // ɾ���ӿ�ʼ����S�������Ƴ��ķ��ս���������ڼ���S2�еķ��ս�����Ĳ���ʽ
        for (auto it = prod.begin(); it != prod.end();) {
            if (!S2.count(it->first)) {
                cout << "�������÷��� " << it->first << "��ɾ����������˵����в���ʽ" << endl;
                it = prod.erase(it);
            } else {
                ++it;
            }
        }
    }

};

#endif //CFG2NPDA_CFG_HXX
