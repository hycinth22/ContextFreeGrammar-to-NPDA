//
// ��C++11��׼���롣
// �涨A-ZΪ���ս�����Ҳ�Ϊ0����ղ���ʽ��
// �Ҳ������|�ָ��ʾ�������ʽ������Ҫ�ж���ո�
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

// �ڴ������ʽʱ����Ҫƴ���Ҳ���concatRight��������������һ���ǿյ����⣬��ֹ���֦�A��A�ŵ������Ӧ��A��
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
    map<char, set<string> > prod; // char �� string

    // ������ݹ�
    void eliminateLeftRecursion() {
        for (auto i = prod.begin(); i != prod.end(); ++i) {
            const char &AiLeft = i->first;
            cout << "����Ѱ�� " << AiLeft << " �ļ����ݹ�" << endl;
            for (auto j = prod.begin(); j != i; ++j) {
                const char &AjLeft = j->first;
                for (auto ii = i->second.begin(); ii != i->second.end();) {
                    const string &AiRight = *ii;
                    if (!AiRight.empty() && AiRight[0] == AjLeft) {
                        for (const string &AjRight: j->second) {
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
            // ΪAi�����в���ʽ����ֱ����ݹ�
            set<string> oldright = i->second;
            bool has = false;
            char z = allocateNewToken();
            for (auto it = oldright.begin(); it != oldright.end();) {
                const string &right = *it;
                if (right.size() > 1 && right[0] == AiLeft) {
                    cout << "��������ֱ����ݹ����ʽ " << AiLeft << " -> " << right << endl;
                    has = true;
                    string a = right.substr(1);
                    prod[z].insert(a);
                    prod[z].insert(a + z);
                    i->second.erase(i->second.find(right));
                    it = oldright.erase(it);
                    cout << "  ���� " << z << " -> " << a << endl;
                    cout << "  ���� " << z << " -> " << a + z << endl;
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
                cout << "������ " << AiLeft << " ��������ݹ� " << endl;
            }
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

    // �������÷���
    void removeUselessSymbol() {
        // ����˳���ܵ���
        _deleteUselessSymbol1(); // ɾ�����ܾ������ɲ������Ƶ������ӣ�����ȫΪ�ս�����ķ��ս��
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
        // �ҵ����о������ɲ��Ƶ����տ����Ƴ����ӣ�����ȫΪ�ս�����ķ��ս��   V ->* T*
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

    void _deleteUselessSymbol2() {
        // �ҵ��ӿ�ʼ����S�����Ƴ���ȫ�����ս��V�� ������S ->* AVC,  A��C�������ս��Ҳ�����Ƿ��ս��
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


typedef int NPDAState;
typedef pair<NPDAState, string> NPDANext; // ����Ԫ�طֱ������һ��״̬����ջ����
const NPDAState FINAL = 0x7f7f7f7f;
const char EMPTY = 'z';
const char STACK_BOTTOM = 'z';
const string EMPTY_STACK = string(1, STACK_BOTTOM);
class NPDA {
    map<tuple<NPDAState, char, char>, set<NPDANext> > rules; // tuple����Ԫ�طֱ����ǰ״̬����ǰ���룬ջ�����š�

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
                    cerr << "�����ķ����󣬲���Greibach��ʽ" << endl;
                    return;
                }
                if (ch == '0') {
                    if (left != 'S') {
                        cerr << "Greibach��ʽ�Ŀղ���ʽ���ֻ��Ϊ��ʼ����S" << endl;
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
        vector<pair<NPDAState, string> > current, next; //  �����Ƿ�ȷ���ͣ���Ҫ�洢һ�鵱ǰ��Ϣ
        current.emplace_back(0, EMPTY_STACK); // ��ʼ״̬
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
                cout<<"��ǰ״̬û���κι������ƥ���������"<<text[i]<<endl;
                return false;
            }
            cout << "��ȡ������" << text[i] << endl;
            for (pair<NPDAState, string> &s: next) {
                cout << "{״̬" << s.first << ", " << s.second.substr(0, s.second.size()) << "}" << endl;
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

            cout << "��(" << state << ",";
            if (inputChar == EMPTY) {
                cout << "��";
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
                if (pushStack.empty()) pushStack = "��";
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
    cout << "˵����SΪ��ʼ���ţ� Ҫ����A->B�ĸ�ʽ���룬�����ж���ո�" << endl
         << "�Ҳ������|�ָ��ʾ�������ʽ��Ҳ�����ж���ո�" << endl
         << "�Կ��л�һ��-�������롣" << endl;
    cout << "�����ķ���" << endl;
    string line;
    char input_left;
    string input_right;
    while (getline(cin, line) && !line.empty()) {
        if(line[0]=='-') break;
        if(! (line[1]=='-' && line[2]=='>') ) {
            cerr << "�������һ�д����ʽ���ķ�" << endl;
            break;
        }
        input_left=line[0];
        input_right = line.substr(3);
        set<string> x = splitStringToSet(input_right, '|');
        for (auto right: x) {
            if (right.size()==1 && input_left == right[0]) {
                cerr << "�ķ��������Ҳ�����ȫ��ͬ";
                return 0;
            }
            if (right.empty()) {
                cerr << "�ķ������Ҳ಻��Ϊ�գ��ղ���ʽ��0��ʾ";
                return 0;
            }
            if (right.size()>1 && right.find('0')!=string::npos) {
                cerr << "�ķ������Ҳ���˱�ʾ�ղ���ʽ���ܺ���0";
                return 0;
            }
            if (right[0]==' ') {
                cerr << "�ķ������Ҳ಻�ܺ��пո�";
                return 0;
            }
            g.prod[input_left].insert(right);
        }
        //g.prod[input_left].insert(x.begin(), x.end());
    }
    if (g.prod['S'].empty()) {
        cerr << "�ķ����󣬱��������ʼ����S";
        return 0;
    }
    cout << "��ʼ�ķ�:" << endl;
    g.output();

    g.eliminateLeftRecursion();
    cout << "������ݹ��:" << endl;
    g.output();

    g.eliminateEpsilon();
    cout << "�����ղ���ʽ��:" << endl;
    g.output();

    g.eliminateSingle();
    cout << "������һ����ʽ��:" << endl;
    g.output();

    g.transformFirstSymbolToTerminal();
    cout << "�������ս����ͷ��:" << endl;
    g.output();

    g.removeUselessSymbol();
    cout << "�������÷��ź�:" << endl;
    g.output();

    g.transformIntoNonTerminalExceptFirst();
    cout << "Greibach��ʽ:" << endl;
    g.output();

    NPDA npda(g);
    cout << "NPDAת�ƹ���:" << endl;
    npda.showRules();

    string text;
    cout << "����һ�о����ж��Ƿ����ڴ��ķ����������ԣ�ֱ�ӻس������жϿմ��Ƿ����ڣ���" << endl;
    while (getline(cin, text)) {
        if (npda.canAccepted(text)) {
            cout << (text.empty()?"��":text) << "���ڸ�����" << endl;
        } else {
            cout << (text.empty()?"��":text) << "�����ڸ�����" << endl;
        }
        cout << endl;
    }
    return 0;
}

