#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

using namespace std;

// Structure to hold a production rule
struct Production {
    string lhs; // Left-hand side (non-terminal)
    vector<string> rhs; // Right-hand side (productions)
};

// Function to split a string by delimiter and trim whitespace
vector<string> split(const string &s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        // Trim whitespace
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

// Function to read CFG from file
map<string, Production> readCFG(const string &filename) {
    map<string, Production> cfg;
    ifstream file(filename);
    string line;

    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return cfg;
    }

    while (getline(file, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        // Split line into LHS and RHS
        auto parts = split(line, '-');
        if (parts.size() < 2) continue; // Invalid line
        string lhs = parts[0];
        string rhs_part = parts[1].substr(parts[1].find('>') + 1); // Remove '>'

        Production prod;
        prod.lhs = lhs;
        // Split RHS by '|' and trim each production
        auto productions = split(rhs_part, '|');
        for (auto &p : productions) {
            p.erase(0, p.find_first_not_of(" \t"));
            p.erase(p.find_last_not_of(" \t") + 1);
            if (p.empty()) p = "ε"; // Handle empty productions
        }
        prod.rhs = productions;
        cfg[lhs] = prod;
    }

    file.close();
    return cfg;
}

// Function to remove immediate left recursion for a single non-terminal
void removeImmediateLeftRecursion(const string &nonTerminal, map<string, Production> &cfg) {
    Production &prod = cfg[nonTerminal];
    vector<string> recursive, nonRecursive;

    // Separate recursive and non-recursive productions
    for (const auto &rhs : prod.rhs) {
        // Check if the production starts with the non-terminal
        if (rhs.length() >= nonTerminal.length() && rhs.substr(0, nonTerminal.length()) == nonTerminal) {
            string remaining = rhs.substr(nonTerminal.length());
            // Trim leading space if present
            remaining.erase(0, remaining.find_first_not_of(" \t"));
            if (!remaining.empty()) {
                recursive.push_back(remaining); // Get α in A -> Aα
            } else {
                recursive.push_back(""); // Handle cases where α might be empty
            }
        } else {
            nonRecursive.push_back(rhs);
        }
    }

    // If no left recursion, return
    if (recursive.empty()) return;

    // Create new non-terminal (e.g., A' for A)
    string newNonTerminal = nonTerminal + "'";

    // Update productions
    Production newProd;
    newProd.lhs = newNonTerminal;
    vector<string> newRhs, updatedRhs;

    // For non-recursive productions: A -> β becomes A -> βA'
    for (const auto &beta : nonRecursive) {
        string newRule = beta;
        if (beta != "ε") { // Avoid adding A' to epsilon
            newRule += " " + newNonTerminal;
        }
        updatedRhs.push_back(newRule);
    }

    // For recursive productions: A -> Aα becomes A' -> αA' | ε
    for (const auto &alpha : recursive) {
        if (alpha.empty()) continue; // Skip if α is empty
        string newRule = alpha + " " + newNonTerminal;
        newRhs.push_back(newRule);
    }
    newRhs.push_back("ε"); // Add epsilon production

    // Update CFG
    prod.rhs = updatedRhs;
    newProd.rhs = newRhs;
    cfg[newNonTerminal] = newProd;
}

// Function to remove all left recursion
void removeLeftRecursion(map<string, Production> &cfg) {
    vector<string> nonTerminals;
    for (const auto &pair : cfg) {
        nonTerminals.push_back(pair.first);
    }

    // Process each non-terminal
    for (size_t i = 0; i < nonTerminals.size(); ++i) {
        string A_i = nonTerminals[i];
        // Handle indirect recursion by substituting A_j productions (j < i)
        for (size_t j = 0; j < i; ++j) {
            string A_j = nonTerminals[j];
            vector<string> newRhs;
            bool modified = false;

            for (const auto &rhs : cfg[A_i].rhs) {
                if (!rhs.empty() && rhs.length() >= A_j.length() && rhs.substr(0, A_j.length()) == A_j) {
                    string suffix = rhs.substr(A_j.length());
                    suffix.erase(0, suffix.find_first_not_of(" \t"));
                    if (suffix.empty() || suffix[0] == ' ') {
                        modified = true;
                        suffix = suffix.empty() ? "" : suffix;
                        // Substitute A_j's productions
                        for (const auto &beta : cfg[A_j].rhs) {
                            string newRule = beta;
                            if (!suffix.empty() && !beta.empty() && beta != "ε") {
                                newRule += " " + suffix;
                            } else if (suffix.empty()) {
                                newRule = beta;
                            }
                            if (!newRule.empty() && newRule != "ε") {
                                newRhs.push_back(newRule);
                            }
                        }
                    } else {
                        newRhs.push_back(rhs);
                    }
                } else {
                    newRhs.push_back(rhs);
                }
            }

            if (modified) {
                cfg[A_i].rhs = newRhs;
            }
        }

        // Remove immediate left recursion for A_i
        removeImmediateLeftRecursion(A_i, cfg);
    }
}

// Function to write CFG to file
void writeCFG(const map<string, Production> &cfg, const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    for (const auto &pair : cfg) {
        const Production &prod = pair.second;
        file << prod.lhs << " -> ";
        bool first = true;
        for (const auto &rhs : prod.rhs) {
            if (!rhs.empty()) {
                if (!first) {
                    file << " | ";
                }
                file << rhs;
                first = false;
            }
        }
        // If all productions were empty, ensure at least ε is written
        if (first) {
            file << "ε";
        }
        file << endl;
    }

    file.close();
}

int main() {
    string inputFile = "input_original_CFG_left_recursion.txt";
    string outputFile = "fine_tuned_CFG_left_recursion.txt";

    // Step a: Read CFG from input file
    map<string, Production> cfg = readCFG(inputFile);
    if (cfg.empty()) {
        cerr << "No valid CFG found in input file." << endl;
        return 1;
    }

    // Step b: Remove left recursion
    removeLeftRecursion(cfg);

    // Step c: Write fine-tuned CFG to output file
    writeCFG(cfg, outputFile);

    cout << "Left recursion removed successfully. Output written to " << outputFile << endl;

    return 0;
}