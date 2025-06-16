#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Structure to hold CFG production rules
struct CFG {
    vector<string> nonTerminals;
    vector<vector<string>> productions;
};

// Function to split a string by delimiter
vector<string> split(const string& str, char delim) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(str);
    while (getline(tokenStream, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Function to trim whitespace from a string
string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    if (first == string::npos) return "";
    return str.substr(first, last - first + 1);
}

// Function to find index of a non-terminal
int findNonTerminalIndex(const vector<string>& nonTerminals, const string& nt) {
    for (size_t i = 0; i < nonTerminals.size(); ++i) {
        if (nonTerminals[i] == nt) return i;
    }
    return -1;
}

// Function to check if a string exists in a vector
bool contains(const vector<string>& vec, const string& str) {
    for (const auto& item : vec) {
        if (item == str) return true;
    }
    return false;
}

// Function to read CFG from file
CFG readCFG(const string& filename) {
    CFG cfg;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cout << "Error opening file: " << filename << endl;
        return cfg;
    }

    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        // Split production rule into left and right parts
        auto parts = split(line, '-');
        if (parts.size() < 2) continue;

        string nonTerminal = trim(parts[0]);
        string rhs = trim(parts[1].substr(1)); // Remove '>' from "->"

        // Add non-terminal if not already present
        if (!contains(cfg.nonTerminals, nonTerminal)) {
            cfg.nonTerminals.push_back(nonTerminal);
            cfg.productions.push_back(vector<string>());
        }

        // Split right-hand side by '|' for multiple productions
        auto productions = split(rhs, '|');
        int ntIndex = findNonTerminalIndex(cfg.nonTerminals, nonTerminal);
        for (auto& prod : productions) {
            cfg.productions[ntIndex].push_back(trim(prod));
        }
    }

    file.close();
    return cfg;
}

// Function to compute FIRST for a given symbol
vector<string> computeFirst(const string& symbol, const CFG& cfg, vector<vector<string>>& firstSets) {
    // Find if symbol is a non-terminal
    int ntIndex = findNonTerminalIndex(cfg.nonTerminals, symbol);

    // If already computed, return cached result
    if (ntIndex != -1 && !firstSets[ntIndex].empty()) {
        return firstSets[ntIndex];
    }

    vector<string> first;

    // If symbol is a terminal (not a non-terminal in CFG)
    if (ntIndex == -1) {
        first.push_back(symbol);
        return first;
    }

    // Process each production of the non-terminal
    for (const auto& prod : cfg.productions[ntIndex]) {
        if (prod == "epsilon") {
            if (!contains(first, "epsilon")) {
                first.push_back("epsilon");
            }
            continue;
        }

        // Split production into symbols
        auto symbols = split(prod, ' ');
        bool allHaveEpsilon = true;

        for (size_t i = 0; i < symbols.size(); ++i) {
            string sym = trim(symbols[i]);
            if (sym.empty()) continue;

            // Compute FIRST for the current symbol
            auto subFirst = computeFirst(sym, cfg, firstSets);

            // Add all terminals except epsilon to the result
            for (const auto& terminal : subFirst) {
                if (terminal != "epsilon" && !contains(first, terminal)) {
                    first.push_back(terminal);
                }
            }

            // If epsilon is not in FIRST(sym), stop processing this production
            if (!contains(subFirst, "epsilon")) {
                allHaveEpsilon = false;
                break;
            }
        }

        // If all symbols in production can derive epsilon, add epsilon to FIRST
        if (allHaveEpsilon && !contains(first, "epsilon")) {
            first.push_back("epsilon");
        }
    }

    // Cache the result
    firstSets[ntIndex] = first;
    return first;
}

// Function to compute FIRST sets for all non-terminals
vector<vector<string>> computeAllFirst(const CFG& cfg) {
    vector<vector<string>> firstSets(cfg.nonTerminals.size());

    // Compute FIRST for each non-terminal
    for (size_t i = 0; i < cfg.nonTerminals.size(); ++i) {
        computeFirst(cfg.nonTerminals[i], cfg, firstSets);
    }

    return firstSets;
}

// Function to write FIRST sets to file
void writeFirstSets(const vector<string>& nonTerminals, const vector<vector<string>>& firstSets, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file: " << filename << endl;
        return;
    }

    for (size_t i = 0; i < nonTerminals.size(); ++i) {
        file << nonTerminals[i] << " -> { ";
        bool firstItem = true;
        for (const auto& terminal : firstSets[i]) {
            if (!firstItem) file << ", ";
            file << terminal;
            firstItem = false;
        }
        file << " }\n";
    }

    file.close();
}

int main() {
    // Read CFG from file
    CFG cfg = readCFG("fine-tuned_CFG.txt");

    // Compute FIRST sets
    auto firstSets = computeAllFirst(cfg);

    // Write FIRST sets to file
    writeFirstSets(cfg.nonTerminals, firstSets, "First_function.txt");

    cout << "FIRST sets have been computed and written to First_function.txt" << endl;

    return 0;
}