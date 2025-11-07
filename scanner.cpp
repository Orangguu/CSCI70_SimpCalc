#include <fstream>    
#include <string>     
#include <iomanip>    
#include <filesystem> 
#include <cctype>     
#include <iostream>
#include <unordered_map>
#include <vector>
using namespace std;
namespace fs = std::filesystem;

unordered_map<string, string> keywords = {
    {"IF", "If"}, {"ELSE", "Else"}, {"ENDIF", "Endif"},
    {"PRINT", "Print"}, {"SQRT", "Sqrt"},
    {"OR", "Or"}, {"AND", "And"}, {"NOT", "Not"}
};

unordered_map<char, string> single_tokens = {
    {';', "Semicolon"}, {',', "Comma"},
    {'(', "LeftParen"}, {')', "RightParen"},
    {'+', "Plus"}, {'-', "Minus"}, {'=', "Equal"}
};

void scan(string inputFileName, string outputFileName) {
    ifstream input(inputFileName);
    ofstream output(outputFileName);

    if (!input.is_open()) {
        cerr << "Error: Cannot open file " << inputFileName << endl;
        return;
    }

    char ch0, ch1;
    string lexeme, token;

    while (input.get(ch0)) {
        lexeme = "";
        token = "";

        // --- Skip whitespace ---
        if (isspace(ch0)) continue;

        // --- IDENTIFIER or KEYWORD ---
        if (isalpha(ch0) || ch0 == '_') {
            lexeme += ch0;
            while (input.get(ch1)) {
                if (isalnum(ch1) || ch1 == '_') lexeme += ch1;
                else { input.unget(); break; }
            }

          
            if (keywords.find(lexeme) != keywords.end())
                token = keywords[lexeme];
            else
                token = "Identifier";
        }

        // --- NUMBER ---
        else if (isdigit(ch0)) {
            lexeme += ch0;

            // integer part
            while (input.get(ch1)) {
                if (isdigit(ch1)) lexeme += ch1;
                else { input.unget(); break; }
            }

            // decimal part
            if (input.get(ch1)) {
                if (ch1 == '.') {
                    lexeme += ch1;
                    if (input.get(ch1) && isdigit(ch1)) {
                        lexeme += ch1;
                        while (input.get(ch1)) {
                            if (isdigit(ch1)) lexeme += ch1;
                            else { input.unget(); break; }
                        }
                    } else {
                        output << "Lexical Error: Invalid number format\nError\n";
                        continue;
                    }
                } else input.unget();
            }

            // exponent part
            if (input.get(ch1)) {
                if (ch1 == 'e' || ch1 == 'E') {
                    lexeme += ch1;

                    if (input.get(ch1) && (ch1 == '+' || ch1 == '-')) {
                        lexeme += ch1;
                        input.get(ch1);
                    }

                    if (isdigit(ch1)) {
                        lexeme += ch1;
                        while (input.get(ch1)) {
                            if (isdigit(ch1)) lexeme += ch1;
                            else { input.unget(); break; }
                        }
                    } else {
                        output << "Lexical Error: Invalid number format\nError\n";
                        continue;
                    }
                } else input.unget();
            }

            token = "Number";
        }

        // --- STRING ---
        else if (ch0 == '"') {
            lexeme += ch0;
            bool unterminated = true;

            while (input.get(ch1)) {
                if (ch1 == '"') {
                    lexeme += ch1;
                    unterminated = false;
                    break;
                }
                if (ch1 == '\n' || input.eof()) break;
                lexeme += ch1;
            }

            if (unterminated) {
                output << "Lexical Error: Unterminated string\nError\n";
                continue;
            }

            token = "String";
        }

        // --- MULTI-CHARACTER OPERATORS ---
        else if (ch0 == ':') {
            if (input.get(ch1) && ch1 == '=') {
                lexeme = ":="; token = "Assign";
            } else { input.unget(); lexeme = ":"; token = "Colon"; }
        }

        else if (ch0 == '*') {
            if (input.get(ch1) && ch1 == '*') {
                lexeme = "**"; token = "Raise";
            } else { input.unget(); lexeme = "*"; token = "Multiply"; }
        }

        else if (ch0 == '<') {
            if (input.get(ch1) && ch1 == '=') {
                lexeme = "<="; token = "LTEqual";
            } else { input.unget(); lexeme = "<"; token = "LessThan"; }
        }

        else if (ch0 == '>') {
            if (input.get(ch1) && ch1 == '=') {
                lexeme = ">="; token = "GTEqual";
            } else { input.unget(); lexeme = ">"; token = "GreaterThan"; }
        }

        else if (ch0 == '!') {
            if (input.get(ch1) && ch1 == '=') {
                lexeme = "!="; token = "NotEqual";
            } else {
                output << "Lexical Error: Illegal character/character sequence\nError\n";
                continue;
            }
        }

        // --- COMMENT or DIVIDE ---
        else if (ch0 == '/') {
            if (input.get(ch1) && ch1 == '/') {
                // skip comment
                while (input.get(ch1) && ch1 != '\n');
                continue;
            } else {
                input.unget();
                lexeme = "/";
                token = "Divide";
            }
        }

        // --- SINGLE CHARACTER TOKENS ---
        else if (single_tokens.find(ch0) != single_tokens.end()) {
            lexeme = ch0;
            token = single_tokens[ch0];
        }

        // --- UNKNOWN CHARACTER ---
        else {
            output << "Lexical Error: Illegal character/character sequence\nError\n";
            continue;
        }

        // --- OUTPUT TOKEN ---
        if (!token.empty())
            output << left << setw(17) << token << lexeme << "\n";
    }

    output << left << setw(17) << "EndofFile" << "\n";
    input.close();
    output.close();
}

void processAllFiles() {
    const string inputDir = "input_files";
    const string outputDir = "output_files";

    // Create output directory if it doesn't exist
    if (!fs::exists(outputDir)) {
        fs::create_directory(outputDir);
    }

    // Get all input files
    vector<string> inputFiles;
    for (const auto& entry : fs::directory_iterator(inputDir)) {
        if (entry.is_regular_file()) {
            string filename = entry.path().filename().string();
            if (filename.find("sample_input_") == 0) {
                inputFiles.push_back(filename);
            }
        }
    }

    // Process each file
    for (const string& inputFile : inputFiles) {
        string outputFile = inputFile;
        size_t pos = outputFile.find("input");
        if (pos != string::npos) {
            outputFile.replace(pos, 5, "output_scan");
        }

        string inputPath = inputDir + "/" + inputFile;
        string outputPath = outputDir + "/" + outputFile;

        cout << "Processing " << inputFile << " -> " << outputFile << endl;
        scan(inputPath, outputPath);
    }
}

int main() {
    processAllFiles();
    return 0;
}