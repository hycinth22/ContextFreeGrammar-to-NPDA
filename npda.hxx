#ifndef CFG2NPDA_NPDA_HXX
#define CFG2NPDA_NPDA_HXX

#include "cfg.hxx"
#include "utils.h"
#include <set>
#include <vector>
#include <string>
#include <tuple>
#include <iostream>
typedef int NPDAState;
typedef pair<NPDAState, string> NPDANext; // ����Ԫ�طֱ������һ��״̬����ջ����
const NPDAState FINAL = 0x7f7f7f7f;
const char EMPTY = 'z';
const char STACK_BOTTOM = 'z';
const string EMPTY_STACK = {STACK_BOTTOM};
class NPDA {
    std::map<std::tuple<NPDAState, char, char>, std::set<NPDANext> > rules; // tuple����Ԫ�طֱ����ǰ״̬����ǰ���룬ջ�����š�

public:
    NPDA(const ContextFreeGrammar &g) {
        rules[make_tuple(0, EMPTY, STACK_BOTTOM)].insert(make_pair(1, std::string("S") + STACK_BOTTOM));
        rules[make_tuple(1, EMPTY, STACK_BOTTOM)].insert(make_pair(FINAL, EMPTY_STACK));
        for (auto &p: g.prod) {
            const char &left = p.first;
            for (auto pr = p.second.begin(); pr != p.second.end(); ++pr) {
                const std::string &right = *pr;
                char ch = right[0];
                if (right.empty() || isNonTerminal(ch)) {
                    std::cerr << "�����ķ����󣬲���Greibach��ʽ" << std::endl;
                    return;
                }
                if (ch == '0') {
                    if (left != 'S') {
                        std::cerr << "Greibach��ʽ�Ŀղ���ʽ���ֻ��Ϊ��ʼ����S" << std::endl;
                        return;
                    }
                    rules[make_tuple(1, EMPTY, 'S')].insert(make_pair(FINAL, ""));
                } else {
                    rules[make_tuple(1, ch, left)].insert(make_pair(1, right.substr(1)));
                }
            }
        }
    }

    bool canAccepted(const std::string &text) {
        std::vector<std::pair<NPDAState, std::string> > current, next; //  �����Ƿ�ȷ���ͣ���Ҫ�洢һ�鵱ǰ��Ϣ
        current.emplace_back(0, EMPTY_STACK); // ��ʼ״̬
        bool matched = false;
        for (int i = 0; i < text.size(); ++i) {
            next.clear();
            for (int k = 0; k < current.size(); ++k) {
                std::pair<NPDAState, std::string> cur = current[k];
                NPDAState state = cur.first;
                const std::string &stack = cur.second;

                auto epsilonRule = rules.find(make_tuple(state, EMPTY, stack[0]));
                if (epsilonRule != rules.end()) {
                    for (const std::pair<NPDAState, std::string> &transfer: epsilonRule->second) {
                        std::pair<NPDAState, std::string> o = make_pair(transfer.first,
                                                                        nextStack(stack, transfer.second));
                        if (std::find(current.begin(), current.end(), o) == current.end()) {
                            // cout << o.first << "," << o.second << endl;
                            current.push_back(o);
                        }
                    }
                }

                auto r = rules.find(make_tuple(state, text[i], stack[0]));
                if (r != rules.end()) {
                    for (const std::pair<NPDAState, std::string> &transfer: r->second) {
                        std::pair<NPDAState, std::string> o = make_pair(transfer.first,
                                                                        nextStack(std::get<1>(cur), transfer.second));
                        if (std::find(next.begin(), next.end(), o) == next.end()) {
                            next.push_back(o);
                        }
                    }
                    matched = true;
                }
            }
            if (!matched) {
                std::cout << "��ǰ״̬û���κι������ƥ���������" << text[i] << std::endl;
                return false;
            }
            std::cout << "��ȡ������" << text[i] << std::endl;
            for (std::pair<NPDAState, std::string> &s: next) {
                std::cout << "{״̬" << s.first << ", " << s.second.substr(0, s.second.size()) << "} ";
            }
            std::cout << std::endl;
            current = next;
            matched = false;
        }
        for (int k = 0; k < current.size(); ++k) {
            std::pair<NPDAState, std::string> cur = current[k];
            NPDAState state = cur.first;
            std::string &stack = cur.second;
            auto epsilonRule = rules.find(make_tuple(state, EMPTY, stack[0]));
            if (epsilonRule != rules.end()) {
                for (const std::pair<NPDAState, std::string> &transfer: epsilonRule->second) {
                    current.emplace_back(transfer.first, nextStack(cur.second, transfer.second));
                    if (current.back().first == FINAL && current.back().second == EMPTY_STACK) return true;
                }
            }
        }
        std::cout << "������Ŵ���û�е������״̬" << std::endl;
        return false;
    }

    static std::string nextStack(const std::string &oldStack, const std::string &x) {
        return x + oldStack.substr(1);
    }

    void showRules() {
        for (const auto &p: rules) {
            int state;
            char inputChar;
            char stackTop;
            std::tie(state, inputChar, stackTop) = p.first;

            std::cout << "��(" << state << ",";
            if (inputChar == EMPTY) {
                std::cout << "��";
            } else {
                std::cout << inputChar;
            }
            std::cout << ",";
            if (stackTop == EMPTY) {
                std::cout << "z";
            } else {
                std::cout << stackTop;
            }
            std::cout << ")= { ";
            bool first = true;
            for (const auto &next: p.second) {
                if (first) first = false;
                else std::cout << ", ";
                std::cout << "(";
                if (next.first == FINAL) {
                    std::cout << "F";
                } else {
                    std::cout << next.first;
                }
                std::string pushStack = next.second;
                if (pushStack.empty()) pushStack = "��";
                else if (pushStack.back() == STACK_BOTTOM) pushStack.back() = 'z';
                std::cout << ", " << pushStack << ")";
            }
            std::cout << " }" << std::endl;
        }
        std::cout << std::endl;
    }
};
#endif //CFG2NPDA_NPDA_HXX
