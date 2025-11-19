#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

struct Token {
    string type;    // e.g. Identifier, Number, Plus, etc.
    string lexeme;  // actual lexeme
};

vector<Token> tokens;
int indexPtr = 0;
bool errorFound = false;


Token currentToken() {
    if (indexPtr < tokens.size()) return tokens[indexPtr];
    return {"EndofFile", ""};
}

Token nextToken() {
    if (indexPtr < tokens.size()) return tokens[indexPtr++];
    return {"EndofFile", ""};
}


bool match(const string& expected, ofstream& out) {
    if (errorFound) return false;
    Token t = currentToken();
    if (t.type == expected) {
        nextToken();
        return true;
    }
    out << "Parse Error: " << expected << " expected.\n";
    errorFound = true;
    return false;
}


void Prg(ofstream&);
void Blk(ofstream&);
void Stm(ofstream&);
void Exp(ofstream&);
void Trm(ofstream&);
void Trmfollow(ofstream&);
void Fac(ofstream&);
void Facfollow(ofstream&);
void Lit(ofstream&);
void Litfollow(ofstream&);
void Val(ofstream&);
void Arg(ofstream&);
void Argfollow(ofstream&);
void Cnd(ofstream&);
void Rel(ofstream&);
void Iffollow(ofstream&);


// Procedures
void Prg(ofstream& out) {
    if (errorFound) return;
    Blk(out);
    match("EndofFile", out);
}

void Blk(ofstream& out) {
    if (errorFound) return;
    Token t = currentToken();

    if ((t.type == "Identifier" || t.type == "Print"
        || t.type == "If")) {
        Stm(out);
        Blk(out);
    }
    // else epsilon
}

void Stm(ofstream& out) {
    if (errorFound) return;
    Token t = currentToken();

    // IDENTIFIER :=
    if (t.type == "Identifier") {
        nextToken();
        if (!match("Assign", out)) return;
        Exp(out);
        if (!match("Semicolon", out)) return;
        if (!errorFound)
            out << "Assignment Statement Recognized\n";
        return;
    }

    // PRINT(...)
    if (t.type == "Print") {
        nextToken();
        if (!match("LeftParen", out)) return;
        Arg(out);
        Argfollow(out);
        if (!match("RightParen", out)) return;
        if (!match("Semicolon", out)) return;
            
        if (!errorFound)
            out << "Print Statement Recognized\n";
        return;
    }

    // IF Cnd : Blk Iffollow
    if (t.type == "If") {
        nextToken();
        out << "If Statement Begins\n";
        Cnd(out);
        if (!match("Colon", out)) return;
        Blk(out);
        Iffollow(out);
        if (!errorFound)
            out << "If Statement Ends\n";
        return;
    }

    out << "Invalid Statement\n";
    errorFound = true;
}

void Arg(ofstream& out) {
    if (errorFound) return;
    Token t = currentToken();

    if (t.type == "String") {
        nextToken();
        return;
    }

    Exp(out);
}

void Argfollow(ofstream& out) {
    if (errorFound) return;

    if ((currentToken().type == "RightParen") || (currentToken().type == "EndofFile")) return; 

    if (currentToken().type == "Comma") {
        nextToken();
        Arg(out);
        if (errorFound) return;
        Argfollow(out);
        return;
    }
}

void Iffollow(ofstream& out) {
    if (errorFound) return;
    Token t = currentToken();

    if (t.type == "Endif") {
        nextToken();
        if (!match("Semicolon", out)) return;
        return;
    }

    if (t.type == "Else") {
        nextToken();
        Blk(out);
        if (!match("Endif", out)) return;
        if (!match("Semicolon", out)) return;
        return;
    }

    out << "Incomplete if Statement\n";
    errorFound = true;
}

void Exp(ofstream& out) {
    if (errorFound) return;

    if (currentToken().type == "Comma" || currentToken().type == "RightParen") return;

    Trm(out);
    if (errorFound) return;
    Trmfollow(out);
}

void Trm(ofstream& out) {
    if (errorFound) return;
    Fac(out);
    if (errorFound) return;
    Facfollow(out);
}

void Trmfollow(ofstream& out) {
    if (errorFound) return;

    if (currentToken().type == "Comma" || currentToken().type == "RightParen") return;

    Token t = currentToken();

    if (t.type == "Plus" || t.type == "Minus") {
        nextToken();
        Trm(out);
        Trmfollow(out);
    }
}

void Fac(ofstream& out) {
    if (errorFound) return;
    Lit(out);
    if (errorFound) return;
    Litfollow(out);
}

void Facfollow(ofstream& out) {
    if (errorFound) return;

    if (currentToken().type == "Comma" || currentToken().type == "RightParen") return;

    Token t = currentToken();

    if (t.type == "Multiply" || t.type == "Divide") {
        nextToken();
        Fac(out);
        if (errorFound) return;
        Facfollow(out);
    }
}

void Litfollow(ofstream& out) {
    if (errorFound) return;
    if (currentToken().type == "Comma" || currentToken().type == "RightParen") return;
   
    if (currentToken().type == "Raise") {
        nextToken();
        Lit(out);
        if (errorFound) return;
        Litfollow(out);
    }
}

void Lit(ofstream& out) {
    if (errorFound) return;
    if (currentToken().type == "Minus") {
        nextToken();
        Val(out);
        return;
    }
    Val(out);
}

void Val(ofstream& out) {
    if (errorFound) return;
    Token t = currentToken();

    if (t.type == "Identifier" || t.type == "Number") {
        nextToken();
        return;
    }

    if (t.type == "Sqrt") {
        nextToken();
        if (!match("LeftParen", out)) return;
        Exp(out);
        if (!match("RightParen", out)) return;
        return;
    }

    if (t.type == "LeftParen") {
        nextToken();
        Exp(out);
        if (!match("RightParen", out)) return;
        return;
    }

    out << "Symbol expected\n";
    errorFound = true;
}

void Cnd(ofstream& out) {
    Exp(out);
    Rel(out);
    Exp(out);
}

void Rel(ofstream& out) {
    Token t = currentToken();

    if (t.type == "LessThan" ||
        t.type == "Equal" ||
        t.type == "GreaterThan" ||
        t.type == "GTEqual" ||
        t.type == "NotEqual" ||
        t.type == "LTEqual") {
        nextToken();
        return;
    }

    out << "Missing relational operator\n";
    errorFound = true;
}


vector<Token> loadTokens(const string& filename) {
    ifstream in(filename);
    vector<Token> list;
    string line;

    while (getline(in, line)) {
        if (line.empty()) continue;

        size_t pos = line.find_first_of(" \t");
        if (pos == string::npos) {
            // Only token name present (just in case)
            list.push_back({line, ""});
            continue;
        }

        string type = line.substr(0, pos);
        string lex = "";

        // The remainder of the line is the lexeme (trim leading space)
        if (pos + 1 < line.size())
            lex = line.substr(pos + 1);

        list.push_back({type, lex});
    }

    return list;
}


void processParser(const string& scanOutputFile, const string& parseOutputFile) {
    ofstream out(parseOutputFile);

    tokens = loadTokens(scanOutputFile);
    indexPtr = 0;
    errorFound = false;

    Prg(out);

    string filename = scanOutputFile.substr(scanOutputFile.find_last_of("/\\") + 1);
    if (!errorFound)
        out << filename << " is a valid SimpCalc program\n";

    out.close();
}

int main() {
    const string outputDir = "output_files";

    for (const auto& entry : fs::directory_iterator(outputDir)) {
        string fname = entry.path().filename().string();
        if (fname.find("output_scan") != string::npos) {
            string outname = fname;
            size_t pos = outname.find("scan");
            outname.replace(pos, 4, "parse");

            string parsePath = outputDir + "/" + outname;

            processParser(outputDir + "/" + fname, parsePath);
        }
    }

    return 0;
}
