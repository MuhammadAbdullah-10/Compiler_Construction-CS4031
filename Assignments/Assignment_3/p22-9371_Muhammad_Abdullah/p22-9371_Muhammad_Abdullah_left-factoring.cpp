#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <algorithm>

using namespace std;

map<string, vector<vector<string>>> outputRules;
queue<pair<string, vector<vector<string>>>> pending;
char nextNonTerminal = 'B'; // Start after 'A'

// Utility: split by delimiter
vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        while (!token.empty() && token.front() == ' ') token.erase(0, 1);
        while (!token.empty() && token.back() == ' ') token.pop_back();
        tokens.push_back(token);
    }
    return tokens;
}

// Utility: split by space
vector<string> splitBySpace(const string& s) {
    vector<string> tokens;
    istringstream iss(s);
    string token;
    while (iss >> token)
        tokens.push_back(token);
    return tokens;
}

// Utility: join tokens to string
string join(const vector<string>& vec) {
    string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        result += vec[i];
        if (i != vec.size() - 1) result += " ";
    }
    return result;
}

// Longest common prefix
vector<string> getCommonPrefix(const vector<vector<string>>& prods) {
    if (prods.empty()) return {};
    vector<string> prefix = prods[0];
    for (size_t i = 1; i < prods.size(); ++i) {
        vector<string> temp;
        for (size_t j = 0; j < min(prefix.size(), prods[i].size()); ++j) {
            if (prefix[j] == prods[i][j])
                temp.push_back(prefix[j]);
            else break;
        }
        prefix = temp;
        if (prefix.empty()) break;
    }
    return prefix;
}

// Generate new non-terminal: A, B, C, ...
string generateNonTerminal() {
    string nt(1, nextNonTerminal);
    nextNonTerminal++;
    return nt;
}

// Recursive left factoring
void processLeftFactoring(string nonTerminal, vector<vector<string>> prods) {
    if (prods.size() <= 1) {
        outputRules[nonTerminal] = prods;
        return;
    }

    while (!prods.empty()) {
        vector<vector<string>> group;
        group.push_back(prods[0]);
        vector<string> base = prods[0];
        prods.erase(prods.begin());

        for (int i = prods.size() - 1; i >= 0; --i) {
            vector<string> common;
            for (size_t j = 0; j < min(base.size(), prods[i].size()); ++j) {
                if (base[j] == prods[i][j])
                    common.push_back(base[j]);
                else break;
            }
            if (!common.empty()) {
                group.push_back(prods[i]);
                prods.erase(prods.begin() + i);
            }
        }

        vector<string> prefix = getCommonPrefix(group);
        if (group.size() > 1 && !prefix.empty()) {
            string newNT = generateNonTerminal();
            vector<string> rule = prefix;
            rule.push_back(newNT);
            outputRules[nonTerminal].push_back(rule);

            vector<vector<string>> suffixes;
            for (auto& g : group) {
                vector<string> suffix(g.begin() + prefix.size(), g.end());
                if (suffix.empty()) suffix.push_back("ε");
                suffixes.push_back(suffix);
            }
            pending.push({ newNT, suffixes });
        } else {
            outputRules[nonTerminal].push_back(group[0]);
        }
    }
}

// Format and write output
void writeOutputFormatted(const string& filename) {
    ofstream out(filename);
    for (const auto& [nonTerminal, productions] : outputRules) {
        out << nonTerminal << " -> ";
        for (size_t i = 0; i < productions.size(); ++i) {
            out << join(productions[i]);
            if (i != productions.size() - 1)
                out << " | ";
        }
        out << endl;
    }
    out.close();
}

// Main processing function
void writeLeftFactoredCFG(const string& inputFileName, const string& outputFileName) {
    ifstream inputFile(inputFileName);
    string line;

    while (getline(inputFile, line)) {
        if (line.empty()) continue;
        size_t pos = line.find("->");
        if (pos == string::npos) continue;

        string nonTerminal = line.substr(0, pos - 1);
        string right = line.substr(pos + 2);
        vector<string> rawProds = split(right, '|');

        vector<vector<string>> prods;
        for (auto& raw : rawProds)
            prods.push_back(splitBySpace(raw));

        pending.push({ nonTerminal, prods });
    }

    while (!pending.empty()) {
        auto [nt, prod] = pending.front();
        pending.pop();
        processLeftFactoring(nt, prod);
    }

    inputFile.close();
    writeOutputFormatted(outputFileName);
}

int main() {
    writeLeftFactoredCFG("input_original_CFG_left_factoring.txt", "fine_tuned_CFG_left_factoring.txt");
    cout << "✅ Left factoring complete!" << endl;
    return 0;
}
