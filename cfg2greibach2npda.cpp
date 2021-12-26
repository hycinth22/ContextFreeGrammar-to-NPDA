//
// ��C++11��׼���롣
// �涨A-ZΪ���ս�����Ҳ�Ϊ0����ղ���ʽ��
// �Ҳ������|�ָ��ʾ�������ʽ������Ҫ�ж���ո�
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
                cerr << "�ķ������Ҳ��0ֻ�����ڱ�ʾ�ղ���ʽ";
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

    g.removeUselessSymbol();
    cout << "�������÷��ź�:" << endl;
    g.output();

    g.eliminateEpsilon();
    cout << "�����ղ���ʽ��:" << endl;
    g.output();

    g.eliminateOnlySingle();
    cout << "������һ����ʽ��:" << endl;
    g.output();

    g.eliminateLeftRecursion();
    cout << "������ݹ��:" << endl;
    g.output();

    g.transformFirstSymbolToTerminal();
    cout << "�������ս����ͷ��:" << endl;
    g.output();

    g.transformIntoNonTerminalExceptFirst();
    g.removeUselessSymbol();
    cout << "Greibach��ʽ:" << endl;
    g.output();

    NPDA npda(g);
    cout << "NPDAת�ƹ���:" << endl;
    npda.showRules();

    string text;
    cout << "����һ�о����ж��Ƿ����ڴ��ķ����������ԣ�ֱ�ӻس������жϿմ��Ƿ����ڣ���" << endl;
    while (getline(cin, text)) {
        if (npda.canAccepted(text)) {
            cout << "�� " << (text.empty() ? "��" : text) << "���ڸ�����" << endl;
        } else {
            cout << "�� " << (text.empty() ? "��" : text) << "�����ڸ�����" << endl;
        }
        cout << endl;
    }
    return 0;
}

