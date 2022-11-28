#include "Lexer.hpp"

std::string Lexer::Text;
unsigned int Lexer::Position;
char Lexer::CurrentChar;
std::vector<int> Lexer::Tokens;
std::vector<std::string> Lexer::GlobalStringInfo;