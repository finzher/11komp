#include <iostream>
#include <fstream>
#include <string>
#include <sys/stat.h>
#include <thread>
#include <chrono>
#include <sstream>
#include <vector>
#include <cctype>
#include <cmath>

using namespace std;

const char* filename = "main.jwkdfjoi";

string trim(const string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

// Проверка был ли записан результат
bool isProcessed(const string& line) {
    if (line.find('=') != string::npos) return true;
    if (line.find("[Ошибка") != string::npos) return true;
    return false;
}

// Разбиение выражения на токены и проверка корректности
vector<string> tokenize_and_validate(const string& raw, bool& ok) {
    ok = false;
    string s = trim(raw);
    vector<string> tokens;
    size_t i = 0, n = s.size();
    bool expectNumber = true;
    while (i < n) {
        if (isspace((unsigned char)s[i])) { ++i; continue; }
        if (expectNumber) {
            string num;
            if (s[i] == '-') {
                num.push_back('-');
                ++i;
                if (i >= n || isspace((unsigned char)s[i])) return {};
            }
            bool sawDigit = false, sawDot = false;
            while (i < n && (isdigit((unsigned char)s[i]) || s[i] == '.')) {
                if (s[i] == '.') {
                    if (sawDot) return {};
                    sawDot = true;
                } else sawDigit = true;
                num.push_back(s[i]);
                ++i;
            }
            if (!sawDigit) return {};
            tokens.push_back(num);
            expectNumber = false;
        } else {
            char c = s[i];
            if (c == '+' || c == '-' || c == '*' || c == '/') {
                tokens.push_back(string(1, c));
                ++i;
                expectNumber = true;
            } else return {};
        }
    }
    if (expectNumber) return {};
    ok = true;
    return tokens;
}

// Перевод токенов в RPN и вычисление
bool evaluateTokens(const vector<string>& tokens, double& result) {
    auto prec = [](const string& op) {
        if (op == "+" || op == "-") return 1;
        if (op == "*" || op == "/") return 2;
        return 0;
    };
    vector<string> out, ops;
    for (auto& t : tokens) {
        if (isdigit(t[0]) || (t[0] == '-' && t.size() > 1 && isdigit(t[1]))) {
            out.push_back(t);
        } else {
            while (!ops.empty() && prec(ops.back()) >= prec(t)) {
                out.push_back(ops.back());
                ops.pop_back();
            }
            ops.push_back(t);
        }
    }
    while (!ops.empty()) {
        out.push_back(ops.back());
        ops.pop_back();
    }
    vector<double> st;
    for (auto& tk : out) {
        if (isdigit(tk[0]) || (tk[0] == '-' && tk.size() > 1 && isdigit(tk[1]))) {
            st.push_back(stod(tk));
        } else {
            if (st.size() < 2) return false;
            double b = st.back(); st.pop_back();
            double a = st.back(); st.pop_back();
            if (tk == "+") st.push_back(a + b);
            else if (tk == "-") st.push_back(a - b);
            else if (tk == "*") st.push_back(a * b);
            else if (tk == "/") {
                if (fabs(b) < 1e-12) return false;
                st.push_back(a / b);
            }
        }
    }
    if (st.size() != 1) return false;
    result = st.back();
    return true;
}

// Красивый вид ответа
string formatResult(double value) {
    if (fabs(value - round(value)) < 1e-9) {
        return to_string((long long)llround(value));
    }
    ostringstream ss;
    ss.precision(6);
    ss << fixed << value;
    string s = ss.str();
    while (!s.empty() && s.back() == '0') s.pop_back();
    if (!s.empty() && s.back() == '.') s.pop_back();
    return s;
}

// Обработка файла
void processFile() {
    ifstream fin(filename, ios::binary);
    if (!fin) return;
    vector<string> lines;
    string line;
    bool changed = false;
    while (getline(fin, line)) {
        string original = line;
        if (trim(original).empty()) continue;
        if (isProcessed(original)) {
            lines.push_back(original);
            continue;
        }
        bool ok = false;
        vector<string> tokens = tokenize_and_validate(original, ok);
        if (!ok) {
            lines.push_back(original + " = [Ошибка: некорректное выражение]");
            changed = true;
            continue;
        }
        double res;
        if (!evaluateTokens(tokens, res)) {
            lines.push_back(original + " = [Ошибка вычисления]");
            changed = true;
            continue;
        }
        lines.push_back(original + " = " + formatResult(res));
        changed = true;
    }
    fin.close();
    if (changed) {
        ofstream fout(filename, ios::trunc);
        for (auto& l : lines) fout << l << "\n";
        fout.close();
    }
}

int main() {
    cout << "Сервер запущен. Ожидание изменений файла main.txt...\n";
    struct stat info;
    time_t lastMod = 0;
    ofstream init(filename, ios::app | ios::binary);
    init.close();
    while (true) {
        if (stat(filename, &info) == 0) {
            if (info.st_mtime != lastMod) {
                lastMod = info.st_mtime;
                processFile();
            }
        }
        this_thread::sleep_for(chrono::milliseconds(300));
    }
}
