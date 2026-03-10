#ifndef TOKENS_H
#define TOKENS_H

#include "string.h"
#include <stdint.h>

typedef uint32_t u32;

typedef enum TokenKind TokenKind;
enum TokenKind {
	/*Single char tokens*/
    TOK_LPAREN,    
    TOK_RPAREN,    
    TOK_LCURLY,    
    TOK_RCURLY,    
    TOK_LSQUAR,    
    TOK_RSQUAR,    
    TOK_COMMA,     
    TOK_DOT,       
    TOK_PLUS,      
    TOK_MINUS,     
    TOK_STAR,      
    TOK_SLASH,     
    TOK_CARET,     
    TOK_MOD,       
    TOK_COLON,     
    TOK_SEMICOLON, 
    TOK_QUESTION,  
    TOK_NOT,       
    TOK_GT,        
    TOK_LT,        
    TOK_EQ,        

    /*Two-char tokens*/
    TOK_GE,         
    TOK_LE,         
    TOK_NE,         
    TOK_EQEQ,       
    TOK_ASSIGN,     
    TOK_GTGT,       
    TOK_LTLT,       

    /*Literals*/
    TOK_IDENTIFIER,
    TOK_STRING,
    TOK_INTEGER,
    TOK_FLOAT,

    /*Keywords*/
    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_TRUE,
    TOK_FALSE,
    TOK_AND,
    TOK_OR,
	TOK_LOCAL,
    TOK_WHILE,
    TOK_DO,
    TOK_FOR,
    TOK_FUNC,
    TOK_NULL,
    TOK_END,
    TOK_PRINT,
    TOK_PRINTLN,
    TOK_RET,

	/*EOF Sentinel*/
	TOK_EOF
};

extern const char *token_lexemes[];
extern const char *token_print[];

extern const TokenKind keywords_tt[];
extern const char *keywords_str[];
extern const size_t keywords_size;

typedef struct Token Token;
struct Token {
	TokenKind token;
	String8 lexeme;
	u32 line;
};

#endif