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

  TK_For = -14,
  TK_Var = -15,

  TK_Binary = -16,
  TK_Unary = -17,

  TK_Then = -18,

  TK_Char = -19,
  TK_Bool = -20,

  TK_CharValue = -21,

  TK_True = -22,
  TK_False = -23,

  TK_String = -24,
  TK_StringContent = -25,
};

struct SourceLocation
{
  int Line;
  int Column;
};

struct Lexer
{
    static std::string IdentifierStr; // Filled in if TK_Identifier
    static std::string NumValString;  // Filled in if TK_Number
    static char CharVal;              // Filled in if TK_Char
    static std::string StringString;  // Filled in if TK_String

    static SourceLocation CurrentLocation;
    static SourceLocation LexerLocation;

    static int Advance()
    {
      int LastChar = getchar();

      if(LastChar == '\n' || LastChar == '\r')
      {
        LexerLocation.Line++;
        LexerLocation.Column = 0;
      }
      else
      {
        LexerLocation.Column++;
      }

      return LastChar;
    }

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
        LastChar = Advance();

      CurrentLocation = LexerLocation;
    
      if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        IdentifierStr = LastChar;
        while (isalnum((LastChar = Advance())))
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
        if(IdentifierStr == "then")
          return Token::TK_Then;
        if(IdentifierStr == "else")
          return Token::TK_Else;

        if(IdentifierStr == "for")
          return Token::TK_For;

        if(IdentifierStr == "var")
          return Token::TK_Var;

        if(IdentifierStr == "binary")
          return Token::TK_Binary;
        if(IdentifierStr == "unary")
          return Token::TK_Unary;

        if(IdentifierStr == "char")
          return Token::TK_Char;
        if(IdentifierStr == "string")
          return Token::TK_String;

        if(IdentifierStr == "bool")
          return Token::TK_Bool;

        if(IdentifierStr == "true")
          return Token::TK_True;
        if(IdentifierStr == "false")
          return Token::TK_False;

        return Token::TK_Identifier;
      }

      if(LastChar == ';')
      {
        LastChar = Advance();
        return Token::TK_DotComma;
      }

      if(LastChar == ',')
      {
        LastChar = Advance();
        return Token::TK_Comma;
      }

      if(LastChar == '\"')
      {
        StringString = "";
        LastChar = Advance();

        while (LastChar != '\"')
        {
          StringString += LastChar;
          LastChar = Advance();
        }

        LastChar = Advance();

        return Token::TK_StringContent;
      }

      if(LastChar == '\'')
      {
        std::string CharStr;

        LastChar = Advance();

        do
        {
          CharStr += LastChar;
          LastChar = Advance();
        } while (isalpha(LastChar) && LastChar != '\'');

        if(LastChar == '\'')
          LastChar = Advance();

        if(CharStr.size() == 2)
        {
          if(CharStr[0] == '\\')
          {
            if(CharStr[1] == 'n')
              CharVal = '\n';
            else if(CharStr[1] == 'r')
              CharVal = '\r';
            else if(CharStr[1] == 't')
              CharVal = '\t';
            else if(CharStr[1] == '0')
              CharVal = '\0';
            else if(CharStr[1] == '\'')
              CharVal = '\'';
          }
        }
        else if(CharStr.size() == 1)
        {
          CharVal = CharStr[0];
        }
        else
          CharVal = ' ';

        return Token::TK_CharValue;

      }
    
      if (isdigit(LastChar) || LastChar == '.' || LastChar == 'f') { // Number: [0-9.]+
        std::string NumStr;
        do {
          NumStr += LastChar;
          LastChar = Advance();
        } while (isdigit(LastChar) || LastChar == '.' || LastChar == 'f');
    
        NumValString = NumStr;
        return Token::TK_Number;
      }
    
      if (LastChar == '#') {
        // Comment until end of line.
        do
          LastChar = Advance();
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
    
        if (LastChar != EOF)
          return GetToken();
      }
    
      // Check for end of file.  Don't eat the EOF.
      if (LastChar == EOF)
        return Token::TK_EndOfFile;
    
      // Otherwise, just return the character as its ascii value.
      int ThisChar = LastChar;
      LastChar = Advance();
      return ThisChar;
    }
};

#endif