#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

using namespace std;

const char* filename = "main.jwkdfjoi";

string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

int main() {
    cout << "Калькулятор\n";
    cout << "Допустимы только цифры, пробелы и знаки + - * /\n";
    cout << "Для выхода введите 'exit'\n";

    while (true) {
        cout << "\nВведите выражение: ";
        string expr;
        getline(cin, expr);
        expr = trim(expr);
        if (expr.empty()) continue;
        if (expr == "exit") break;

        ofstream fout(filename, ios::app);
        fout << expr << "\n";
        fout.close();

        bool found = false;
        string last;

        for (int t = 0; t < 100; ++t) {
            ifstream fin(filename, ios::binary);
            vector<string> lines;
            string line;
            while (getline(fin, line)) lines.push_back(line);
            fin.close();

            for (int i = lines.size() - 1; i >= 0; --i) {
                string l = trim(lines[i]);
                if (l.rfind(expr, 0) == 0) {
                    size_t pos = l.find('=');
                    if (pos != string::npos && pos + 1 < l.size()) {
                        cout << "Ответ: " << trim(l.substr(pos + 1)) << "\n";
                        found = true;
                        break;
                    }
                }
            }
            if (found) break;
            this_thread::sleep_for(chrono::milliseconds(300));
        }

        if (!found)
            cout << "Ответ не получен. Проверьте выражение.\n";
    }

    cout << "Клиент завершён.\n";
}
