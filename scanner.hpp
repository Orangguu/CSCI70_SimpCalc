#ifndef SCANNER_HPP
#define SCANNER_HPP

#include <string>
#include "token.hpp"
using namespace std;

void scan(string inputFileName, string outputFileName);
void processAllFiles();

// Parser interface:
void openTokenStream(const string& scanFile, const string& originalFile);
Token gettoken();
Token peektoken();
string getOriginalFilename();

// Mapping support:
string getOriginalFromScan(const string& scanFile);

#endif
