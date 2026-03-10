#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"
#include "arrays/dynamic_array.h"

typedef struct LexerState LexerState;
struct LexerState {
	String8 	source;
	size_t 		start;
	size_t 		curr;
	size_t 		line;
	DArray 		tokens;
};

DArray tokenize(LexerState *ls);

#endif