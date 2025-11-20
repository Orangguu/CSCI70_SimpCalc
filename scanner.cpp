#include <fstream>
#include <string>
#include <iomanip>
#include <filesystem>
#include <cctype>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <map>
#include "token.hpp"

using namespace std;
namespace fs = std::filesystem;

// ======================================================
// TOKEN TABLES
// ======================================================

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

// ======================================================
// MAPPING: scanFile → originalInputFile
// ======================================================

static map<string,string> fileMapping;

string getOriginalFromScan(const string& scanFile) {
    if (fileMapping.count(scanFile)) return fileMapping[scanFile];
    return "";
}

// ======================================================
// SCANNER THAT PRODUCES OUTPUT FILE
// ======================================================

string currentSourceFilename = "";

void scan(string inputFileName, string outputFileName) {
    currentSourceFilename = inputFileName;
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

        if (isspace(ch0)) continue;

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

        else if (isdigit(ch0)) {
            lexeme += ch0;

            while (input.get(ch1)) {
                if (isdigit(ch1)) lexeme += ch1;
                else { input.unget(); break; }
            }

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

            if (input.get(ch1)) {
                if (ch1 == 'e' || ch1 == 'E') {
                    lexeme += ch1;
                    if (input.get(ch1) && (ch1=='+'||ch1=='-')) {
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

        else if (ch0 == ':') {
            if (input.get(ch1) && ch1 == '=') {
                lexeme = ":="; token = "Assign";
            } else { input.unget(); lexeme = ":"; token = "Colon"; }
        }

        else if (ch0 == '*') {
            if (input.get(ch1) && ch1 == '*') {
                lexeme="**"; token="Raise";
            } else { input.unget(); lexeme="*"; token="Multiply"; }
        }

        else if (ch0 == '<') {
            if (input.get(ch1) && ch1 == '=') {
                lexeme="<="; token="LTEqual";
            } else { input.unget(); lexeme="<"; token="LessThan"; }
        }

        else if (ch0 == '>') {
            if (input.get(ch1) && ch1 == '=') {
                lexeme=">="; token="GTEqual";
            } else { input.unget(); lexeme=">"; token="GreaterThan"; }
        }

        else if (ch0 == '!') {
            if (input.get(ch1) && ch1 == '=') {
                lexeme="!="; token="NotEqual";
            } else {
                output << "Lexical Error: Illegal character/character sequence\nError\n";
                continue;
            }
        }

        else if (ch0 == '/') {
            if (input.get(ch1) && ch1 == '/') {
                while (input.get(ch1) && ch1!='\n');
                continue;
            }
            input.unget();
            lexeme="/";
            token="Divide";
        }

        else if (single_tokens.find(ch0)!=single_tokens.end()) {
            lexeme=ch0;
            token=single_tokens[ch0];
        }

        else {
            output << "Lexical Error: Illegal character/character sequence\nError\n";
            continue;
        }

        output << left << setw(17) << token << lexeme << "\n";
    }

    output << left << setw(17) << "EndofFile" << "\n";
    input.close();
    output.close();
}



// ======================================================
// PART 2 — PROVIDE gettoken() FOR PARSER
// ======================================================

static ifstream tokenStream;
static Token saved = {"",""};
static bool hasSaved = false;

static string originalFilename = "";
void openTokenStream(const string& scanFile, const string& originalFile) {
    originalFilename = originalFile;
    tokenStream.close();
    tokenStream.clear();
    tokenStream.open(scanFile);
    hasSaved = false;
}

string getOriginalFilename() {
    return originalFilename;
}


static Token readNextToken() {
    string type, lexeme;

    if (!(tokenStream >> type))
        return {"EndofFile", ""};

    if (type == "EndofFile")
        return {"EndofFile", ""};

    if (type == "Lexical" || type == "Error")
        return {"Error", ""};

    // If the token is a string, read until closing quote
    if (type == "String") {
        char ch;
        lexeme = "";  

        // Skip whitespace before the first quote
        tokenStream >> std::ws;  
        tokenStream.get(ch);  // Should be the opening quote
        if (ch != '"') return {"Error", ""};  

        // Read until the closing quote
        while (tokenStream.get(ch) && ch != '"') {
            lexeme += ch;
        }
    } else {
        tokenStream >> lexeme;
    }
    
    return {type, lexeme};
}

Token gettoken() {
    if (hasSaved) {
        hasSaved = false;
        return saved;
    }
    return readNextToken();
}

Token peektoken() {
    if (!hasSaved) {
        saved = readNextToken();
        hasSaved = true;
    }
    return saved;
}


// ======================================================
// PROCESS ALL INPUT FILES
// ======================================================

void processAllFiles() {
    const string inputDir="input_files";
    const string outputDir="output_files";

    if (!fs::exists(outputDir)) fs::create_directory(outputDir);

    for (auto &entry : fs::directory_iterator(inputDir)) {
        if (!entry.is_regular_file()) continue;

        string in = entry.path().filename().string();
        if (in.find("sample_input_") != 0) continue;

        string out = in;
        size_t pos = out.find("input");
        out.replace(pos, 5, "output_scan");

        string inPath = inputDir + "/" + in;
        string outPath = outputDir + "/" + out;

        fileMapping[outPath] = inPath;

        scan(inPath, outPath);
        cout << "Scanned " << in << endl;
    }
}

// int main() {
//     processAllFiles();
//     return 0;
// }
