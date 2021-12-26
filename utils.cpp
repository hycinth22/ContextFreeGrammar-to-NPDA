//
// Created by pinelliar on 2021-12-26.
//

#include<cassert>
#include<cstdlib>
#include<algorithm>
#include<map>
#include<set>
#include<vector>
#include<stack>
#include<queue>
#include<string>
using namespace std;

std::set<std::string> splitStringToSet(const std::string &str, char delim) {
    std::set<std::string> s;
    std::size_t kb = 0, ke = str.find(delim);
    while (ke != std::string::npos) {
        s.insert(str.substr(kb, ke - kb));
        kb = ke + 1;
        ke = str.find(delim, kb);
    }
    s.insert(str.substr(kb));
    return s;
}