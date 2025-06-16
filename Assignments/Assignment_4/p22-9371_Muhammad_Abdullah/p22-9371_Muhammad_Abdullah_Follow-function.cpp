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

// Function to add a terminal to a vector if not already present
void addUnique(vector<string>& vec, const string& str) {
    if (!contains(vec, str)) {
        vec.push_back(str);
    }
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

// Function to compute FIRST for a given symbol (needed for FOLLOW)
vector<string> computeFirst(const string& symbol, const CFG& cfg, vector<vector<string>>& firstSets) {
    int ntIndex = findNonTerminalIndex(cfg.nonTerminals, symbol);

    if (ntIndex != -1 && !firstSets[ntIndex].empty()) {
        return firstSets[ntIndex];
    }

    vector<string> first;

    if (ntIndex == -1) {
        first.push_back(symbol);
        return first;
    }

    for (const auto& prod : cfg.productions[ntIndex]) {
        if (prod == "ε") {
            addUnique(first, "ε");
            continue;
        }

        auto symbols = split(prod, ' ');
        bool allHaveEpsilon = true;

        for (size_t i = 0; i < symbols.size(); ++i) {
            string sym = trim(symbols[i]);
            if (sym.empty()) continue;

            auto subFirst = computeFirst(sym, cfg, firstSets);

            for (const auto& terminal : subFirst) {
                if (terminal != "ε") {
                    addUnique(first, terminal);
                }
            }

            if (!contains(subFirst, "ε")) {
                allHaveEpsilon = false;
                break;
            }
        }

        if (allHaveEpsilon) {
            addUnique(first, "ε");
        }
    }

    firstSets[ntIndex] = first;
    return first;
}

// Function to compute FIRST sets for all non-terminals
vector<vector<string>> computeAllFirst(const CFG& cfg) {
    vector<vector<string>> firstSets(cfg.nonTerminals.size());

    for (size_t i = 0; i < cfg.nonTerminals.size(); ++i) {
        computeFirst(cfg.nonTerminals[i], cfg, firstSets);
    }

    return firstSets;
}

// Function to compute FOLLOW sets
vector<vector<string>> computeFollow(const CFG& cfg, const vector<vector<string>>& firstSets) {
    vector<vector<string>> followSets(cfg.nonTerminals.size());

    // Step 1: Add $ to FOLLOW of start symbol (assume first non-terminal is start)
    if (!cfg.nonTerminals.empty()) {
        addUnique(followSets[0], "$");
    }

    bool changed = true;
    while (changed) {
        changed = false;

        // Step 2: Process each production
        for (size_t i = 0; i < cfg.nonTerminals.size(); ++i) {
            string A = cfg.nonTerminals[i];
            for (const auto& prod : cfg.productions[i]) {
                auto symbols = split(prod, ' ');

                // For each symbol B in production
                for (size_t j = 0; j < symbols.size(); ++j) {
                    string B = trim(symbols[j]);
                    if (B.empty()) continue;

                    int bIndex = findNonTerminalIndex(cfg.nonTerminals, B);
                    if (bIndex == -1) continue; // Skip if B is a terminal

                    // Compute FIRST of the string β (symbols after B)
                    vector<string> betaFirst;
                    bool betaCanBeEpsilon = true;

                    for (size_t k = j + 1; k < symbols.size(); ++k) {
                        string sym = trim(symbols[k]);
                        if (sym.empty()) continue;

                        int symIndex = findNonTerminalIndex(cfg.nonTerminals, sym);
                        vector<string> symFirst;

                        if (symIndex != -1) {
                            symFirst = firstSets[symIndex];
                        } else {
                            symFirst.push_back(sym);
                        }

                        for (const auto& terminal : symFirst) {
                            if (terminal != "ε") {
                                addUnique(betaFirst, terminal);
                            }
                        }

                        if (!contains(symFirst, "ε")) {
                            betaCanBeEpsilon = false;
                            break;
                        }
                    }

                    // Add FIRST(β) - {ε} to FOLLOW(B)
                    size_t oldSize = followSets[bIndex].size();
                    for (const auto& terminal : betaFirst) {
                        addUnique(followSets[bIndex], terminal);
                    }

                    // If β can be ε or β is empty, add FOLLOW(A) to FOLLOW(B)
                    if (betaCanBeEpsilon || j + 1 == symbols.size()) {
                        for (const auto& terminal : followSets[i]) {
                            addUnique(followSets[bIndex], terminal);
                        }
                    }

                    if (followSets[bIndex].size() > oldSize) {
                        changed = true;
                    }
                }
            }
        }
    }

    return followSets;
}

// Function to write FOLLOW sets to file
void writeFollowSets(const vector<string>& nonTerminals, const vector<vector<string>>& followSets, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file: " << filename << endl;
        return;
    }

    for (size_t i = 0; i < nonTerminals.size(); ++i) {
        file << nonTerminals[i] << " -> { ";
        bool firstItem = true;
        for (const auto& terminal : followSets[i]) {
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

    // Compute FIRST sets (needed for FOLLOW)
    auto firstSets = computeAllFirst(cfg);

    // Compute FOLLOW sets
    auto followSets = computeFollow(cfg, firstSets);

    // Write FOLLOW sets to file
    writeFollowSets(cfg.nonTerminals, followSets, "Follow_function.txt");

    cout << "FOLLOW sets have been computed and written to Follow_function.txt" << endl;

    return 0;
}