#include <iostream>
#include <fstream>
#include <cctype>
#include <cstring>

using namespace std;

#define MAX_TAPE_SIZE 1000  // Maximum input size

char inputTape[MAX_TAPE_SIZE];  // Input tape (character array)
int start_point = 0, end_point = 0;  // Two pointers

// List of reserved keywords
string keywords[] = {"int", "float", "string", "if", "else", "while", "return"};
int numKeywords = sizeof(keywords) / sizeof(keywords[0]);

// Function to check if a string is a keyword
bool isKeyword(string lexeme) {
    for (int i = 0; i < numKeywords; i++) {
        if (lexeme == keywords[i]) return true;
    }
    return false;
}

// Function to check if a character is an operator
bool isOperator(char ch) {
    return (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=' ||
            ch == '<' || ch == '>' || ch == '!' || ch == '%');
}

// Function to check if a character is a symbol (parentheses, brackets, etc.)
bool isSymbol(char ch) {
    return (ch == '(' || ch == ')' || ch == '{' || ch == '}');
}

int main() {
    ifstream inputFile("input.txt");
    ofstream tokenFile("tokens.txt");

    if (!inputFile.is_open() || !tokenFile.is_open()) {
        cout << "Error opening file!" << endl;
        return 1;
    }

    // Read entire file into input tape
    inputFile.read(inputTape, MAX_TAPE_SIZE);
    int tapeLength = inputFile.gcount();
    inputFile.close();

    while (end_point < tapeLength) {
        // Skip spaces, tabs, and new lines
        while (isspace(inputTape[end_point])) {
            end_point++;
        }

        start_point = end_point;  // Set start of lexeme

        // Identifiers & Keywords
        if (isalpha(inputTape[end_point]) || inputTape[end_point] == '_') {
            while (isalnum(inputTape[end_point]) || inputTape[end_point] == '_') {
                end_point++;
            }
            string lexeme(inputTape + start_point, inputTape + end_point);
            if (isKeyword(lexeme)) {
                tokenFile << "Lexeme: " << lexeme << ", Token: Reserved Word\n";
            } else {
                tokenFile << "Lexeme: " << lexeme << ", Token: Identifier\n";
            }
        }
        // Numbers (Integer & Float)
        else if (isdigit(inputTape[end_point])) {
            bool isFloat = false;
            while (isdigit(inputTape[end_point]) || inputTape[end_point] == '.') {
                if (inputTape[end_point] == '.') {
                    if (isFloat) break; // Prevent multiple dots
                    isFloat = true;
                }
                end_point++;
            }
            string lexeme(inputTape + start_point, inputTape + end_point);
            tokenFile << "Lexeme: " << lexeme << ", Token: " << (isFloat ? "Float Number\n" : "Integer\n");
        }
        // Operators
        else if (isOperator(inputTape[end_point])) {
            char first = inputTape[end_point];
            char second = inputTape[end_point + 1];

            if ((first == '=' && second == '=') || (first == '!' && second == '=') ||
                (first == '<' && second == '=') || (first == '>' && second == '=')) {
                tokenFile << "Lexeme: " << first << second << ", Token: Operator\n";
                end_point += 2;
            } else {
                tokenFile << "Lexeme: " << first << ", Token: Operator\n";
                end_point++;
            }
        }
        // Statement Terminator `#`
        else if (inputTape[end_point] == '#') {
            tokenFile << "Lexeme: #, Token: Statement Terminator\n";
            end_point++;
        }
        // Parentheses & Block Symbols
        else if (isSymbol(inputTape[end_point])) {
            tokenFile << "Lexeme: " << inputTape[end_point] << ", Token: Symbol\n";
            end_point++;
        }
        // Unknown Symbols (Now Fixed)
        else {
            if (isprint(inputTape[end_point])) {
                cout << "Lexical Error: Unknown Symbol -> " << inputTape[end_point] << endl;
            } else {
                cout << "Lexical Error: Unknown Non-Printable Character Detected" << endl;
            }
            end_point++;
        }
    }

    cout << "Lexical analysis complete. Tokens stored in tokens.txt\n";

    tokenFile.close();
    return 0;
}
