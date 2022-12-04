#ifndef LEXER_H
#define LEXER_H

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <iostream>

enum Token
{
  TK_EndOfFile = -1,

  // Commands
  TK_Define = -2,
  TK_Extern = -3,

  // Primary
  TK_Identifier = -4,
  TK_Number = -5,
  TK_Comma = -6,
  TK_DotComma = -7,

  TK_Integer = -8,
  TK_Double = -9,
  TK_Float = -10,

  TK_Arrow = -11,

  TK_If = -12,
  TK_Else = -13,
};

struct Lexer
{
    static std::string IdentifierStr; // Filled in if tok_identifier
    static std::string NumValString;  // Filled in if tok_number

    static int CurrentToken;
    static int GetNextToken()
    {
      CurrentToken = GetToken();
      //std::cout << "Next Token: " << CurrentToken << "\n";
      return CurrentToken;
    }

    static int GetToken()
    {
      static int LastChar = ' ';

      while (isspace(LastChar))
        LastChar = getchar();
    
      if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        IdentifierStr = LastChar;
        while (isalnum((LastChar = getchar())))
          IdentifierStr += LastChar;
    
        if (IdentifierStr == "func")
          return Token::TK_Define;
        if (IdentifierStr == "extern")
          return Token::TK_Extern;

        if(IdentifierStr == "int")
          return Token::TK_Integer;
        if(IdentifierStr == "double")
          return Token::TK_Double;
        if(IdentifierStr == "float")
          return Token::TK_Float;

        if(IdentifierStr == "if")
          return Token::TK_If;
        if(IdentifierStr == "else")
          return Token::TK_Else;

        return Token::TK_Identifier;
      }

      if(LastChar == ';')
      {
        LastChar = getchar();
        return Token::TK_DotComma;
      }

      if(LastChar == ',')
      {
        LastChar = getchar();
        return Token::TK_Comma;
      }
    
      if (isdigit(LastChar) || LastChar == '.' || LastChar == 'f') { // Number: [0-9.]+
        std::string NumStr;
        do {
          NumStr += LastChar;
          LastChar = getchar();
        } while (isdigit(LastChar) || LastChar == '.' || LastChar == 'f');
    
        NumValString = NumStr;
        return Token::TK_Number;
      }
    
      if (LastChar == '#') {
        // Comment until end of line.
        do
          LastChar = getchar();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
    
        if (LastChar != EOF)
          return GetToken();
      }
    
      // Check for end of file.  Don't eat the EOF.
      if (LastChar == EOF)
        return Token::TK_EndOfFile;
    
      // Otherwise, just return the character as its ascii value.
      int ThisChar = LastChar;
      LastChar = getchar();
      return ThisChar;
    }
};

#endif