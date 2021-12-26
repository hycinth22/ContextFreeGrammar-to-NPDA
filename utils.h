//
// Created by pinelliar on 2021-12-26.
//

#ifndef CFG2NPDA_UTILS_H
#define CFG2NPDA_UTILS_H

#include <string>
#include <set>

inline bool isNonTerminal(const char x) { return x >= 'A' && x <= 'Z'; }

inline bool isTerminal(const char x) { return !isNonTerminal(x); }

// �ڴ������ʽʱ����Ҫƴ���Ҳ���
// concatRight��������������һ���ǿյ����⣬��ֹ���֦�A��A�ţ�Ӧ��A��
inline std::string concatRight(const std::string &a, const std::string &b) {
    if (a == "0") return b; else if (b == "0") return a; else return a + b;
}

inline std::string concatRight(const std::string &a, char b) {
    if (a == "0") return {b}; else if (b == '0') return a; else return a + b;
}

std::set<std::string> splitStringToSet(const std::string &str, char delim);

#endif //CFG2NPDA_UTILS_H
