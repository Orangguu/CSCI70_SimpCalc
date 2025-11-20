#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <filesystem>
#include "token.hpp"
#include "scanner.hpp"

using namespace std;
namespace fs = std::filesystem;

// From scanner module:
void openTokenStream(const string&);
Token gettoken();
Token peektoken();

// GLOBAL ERROR FLAG
bool errorFound = false;

// FORWARD DECLARATIONS
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


// ======================================================
// MATCH FUNCTION
// ======================================================
bool match(const string& expected, ofstream& out) {
    if (errorFound) return false;

    Token t = peektoken();
    if (t.type == expected) {
        gettoken();
        return true;
    }

    out << "Parse Error: " << expected << " expected.\n";
    errorFound = true;
    return false;
}


// ======================================================
// PARSER IMPLEMENTATION
// ======================================================

void Prg(ofstream& out) {
    if (errorFound) return;
    Blk(out);
    match("EndofFile", out);
}

void Blk(ofstream& out) {
    if (errorFound) return;

    Token t = peektoken();
    if (t.type == "Identifier" || t.type == "Print" || t.type == "If") {
        Stm(out);
        Blk(out);
    }
}

void Stm(ofstream& out) {
    if (errorFound) return;

    Token t = peektoken();

    if (t.type == "Identifier") {
        gettoken();
        if (!match("Assign", out)) return;
        Exp(out);
        if (!match("Semicolon", out)) return;
        if (!errorFound) out << "Assignment Statement Recognized\n";
        return;
    }

    if (t.type == "Print") {
        gettoken();
        if (!match("LeftParen", out)) return;
        Arg(out);
        Argfollow(out);
        if (!match("RightParen", out)) return;
        if (!match("Semicolon", out)) return;
        if (!errorFound) out << "Print Statement Recognized\n";
        return;
    }

    if (t.type == "If") {
        gettoken();
        out << "If Statement Begins\n";
        Cnd(out);
        if (!match("Colon", out)) return;
        Blk(out);
        Iffollow(out);
        if (!errorFound) out << "If Statement Ends\n";
        return; 
    }

    out << "Invalid Statement\n";
    errorFound = true;
}

void Arg(ofstream& out) {
    if (errorFound) return;
    if (peektoken().type == "String") {
        gettoken();
        return;
    }
    Exp(out);
}

void Argfollow(ofstream& out) {
    if (errorFound) return;

    if ((peektoken().type == "RightParen") || (peektoken().type == "EndofFile")) return;

    if (peektoken().type == "Comma") {
        gettoken();
        Arg(out);
        if (errorFound) return;
        Argfollow(out);
        return;
    }
}

void Iffollow(ofstream& out) {
    if (errorFound) return;
    Token t = peektoken();

    if (t.type == "Endif") {
        gettoken();
        if (!match("Semicolon", out)) return;
        return;
    }

    if (t.type == "Else") {
        gettoken();
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

    Token t = peektoken();

    // Grammar DOES NOT support boolean operators.
    if (t.type == "And" || t.type == "Or" || t.type == "Not") {
        out << "Parse Error: Colon expected.\n";
        errorFound = true;
        return;
    }

    if (t.type=="Comma" || t.type=="RightParen") return;

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

    if (peektoken().type == "Comma" || peektoken().type == "RightParen") return;

    Token t = peektoken();
    if (t.type=="Plus" || t.type=="Minus") {
        gettoken();
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

    if (peektoken().type == "Comma" || peektoken().type == "RightParen") return;

    Token t = peektoken();
    if (t.type=="Multiply" || t.type=="Divide") {
        gettoken();
        Fac(out);
        if (errorFound) return;
        Facfollow(out);
    }
}

void Litfollow(ofstream& out) {
    if (errorFound) return;

    if (peektoken().type == "Comma" || peektoken().type == "RightParen") return;    

    if (peektoken().type=="Raise") {
        gettoken();
        Lit(out);
        if (errorFound) return;
        Litfollow(out);
    }
}

void Lit(ofstream& out) {
    if (errorFound) return;
    if (peektoken().type=="Minus") {
        gettoken();
        Val(out);
        return;
    }
    Val(out);
}

void Val(ofstream& out) {
    if (errorFound) return;
    Token t = peektoken();

    if (t.type=="Identifier" || t.type=="Number") {
        gettoken();
        return;
    }

    if (t.type=="Sqrt") {
        gettoken();
        if (!match("LeftParen", out)) return;
        Exp(out);
        if (!match("RightParen", out)) return;
        return;
    }

    if (t.type=="LeftParen") {
        gettoken();
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
    Token t = peektoken();

    if (t.type=="LessThan" || t.type=="Equal" || t.type=="GreaterThan" ||
        t.type=="GTEqual" || t.type=="NotEqual" || t.type=="LTEqual") {
        gettoken();
        return;
    }

    out << "Missing relational operator\n";
    errorFound = true;
}



// ======================================================
// DRIVER — PROCESS ALL SCANNER OUTPUT FILES
// ======================================================

void processParser() {
    const string outDir="output_files";

    for (auto &entry : fs::directory_iterator(outDir)){
        string fname = entry.path().filename().string();
        if (fname.find("output_scan")==string::npos) continue;

        string parseName=fname;
        size_t pos=parseName.find("scan");
        parseName.replace(pos,4,"parse");

        ofstream out(outDir+"/"+parseName);

        string scanFile = outDir + "/" + fname;
        string original = getOriginalFromScan(scanFile);

        openTokenStream(scanFile, original);

        errorFound=false;
        Prg(out);

        if (!errorFound)
            out<< original <<" is a valid SimpCalc program";
    }
}