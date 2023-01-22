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

  TK_Ptr = -26,

  TK_Struct = -27,

  TK_GenericPointer = -28,

  TK_Import = -29,

  TK_Shelf = -30,
};

struct SourceLocation
{
  int Line;
  int Column;
};

struct Lexer
{
    static std::string EntireScriptContent;
    static std::string IdentifierStr; // Filled in if TK_Identifier
    static std::string NumValString;  // Filled in if TK_Number
    static char CharVal;              // Filled in if TK_Char
    static std::string StringString;  // Filled in if TK_String

    static SourceLocation CurrentLocation;
    static SourceLocation LexerLocation;

    static std::string GetSavedString;
    static bool RecordString;

    static int LastECChar;

    static int Advance()
    {
      if(RecordString)
        GetSavedString += EntireScriptContent[LastECChar];

      LastECChar++;

      int LastChar = EntireScriptContent[LastECChar];

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

    static void StartStringRecording()
    {
      RecordString = true;
    }

    static std::string FinishStringRecording()
    {
      RecordString = false;
      std::string finalStr = GetSavedString;
      GetSavedString = "";
      return finalStr;
    }

    static void AddToStringRecording(std::string s)
    {
      if(RecordString)
          GetSavedString += s;
    }

    static void Reset()
    {
      IdentifierStr = "";
      NumValString = "";
      CharVal = '\0';
      StringString = "";

      CurrentToken = ' ';

      CurrentLocation.Line = 0;
      CurrentLocation.Column = 0;

      LexerLocation.Line = 0;
      LexerLocation.Column = 0;

      EntireScriptContent = "";

      LastECChar = 0;
    }

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
        {
          return Token::TK_Extern;
        }

        if(IdentifierStr == "int")
        {
          //AddToStringRecording(IdentifierStr);
          return Token::TK_Integer;
        }
        if(IdentifierStr == "double")
        {
          //AddToStringRecording(IdentifierStr);
          return Token::TK_Double;
        }
        if(IdentifierStr == "float")
        {
          //AddToStringRecording(IdentifierStr);
          return Token::TK_Float;
        }

        if(IdentifierStr == "if")
          return Token::TK_If;
        if(IdentifierStr == "then")
          return Token::TK_Then;
        if(IdentifierStr == "else")
          return Token::TK_Else;

        if(IdentifierStr == "for")
          return Token::TK_For;

        if(IdentifierStr == "var")
        {
          //AddToStringRecording(IdentifierStr);
          return Token::TK_Var;
        }
        if(IdentifierStr == "ptr")
        {
          //AddToStringRecording(IdentifierStr);
          return Token::TK_Ptr;
        }

        if(IdentifierStr == "binary")
        {
          //AddToStringRecording(IdentifierStr);
          return Token::TK_Binary;
        }
        if(IdentifierStr == "unary")
        {
          //AddToStringRecording(IdentifierStr);
          return Token::TK_Unary;
        }

        if(IdentifierStr == "char")
        {
          //AddToStringRecording(IdentifierStr);
          return Token::TK_Char;
        }
        if(IdentifierStr == "string")
        {
          //AddToStringRecording(IdentifierStr);
          return Token::TK_String;
        }

        if(IdentifierStr == "bool")
        {
          //AddToStringRecording(IdentifierStr);
          return Token::TK_Bool;
        }

        if(IdentifierStr == "GENERIC")
        {
          //AddToStringRecording(IdentifierStr);
          return Token::TK_GenericPointer;
        }

        if(IdentifierStr == "true")
          return Token::TK_True;
        if(IdentifierStr == "false")
          return Token::TK_False;

        if(IdentifierStr == "struct")
          return Token::TK_Struct;

        if(IdentifierStr == "import")
          return Token::TK_Import;

        if(IdentifierStr == "shelf")
          return Token::TK_Shelf;

        //AddToStringRecording(IdentifierStr);
        return Token::TK_Identifier;
      }

      if(LastChar == ';')
      {
        LastChar = Advance();
        //Lexer::AddToStringRecording(";");
        return Token::TK_DotComma;
      }

      if(LastChar == ',')
      {
        LastChar = Advance();
        //Lexer::AddToStringRecording(",");
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
    
      if (isdigit(LastChar)) { // Number: [0-9.]+
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

      if(ThisChar < 0)
      {
        //std::cout << "Be careful! ThisChar returned less than 0!";
        ThisChar = 0;
      }

      return ThisChar;
    }
};

#endif