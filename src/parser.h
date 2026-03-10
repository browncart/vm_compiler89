#ifndef PARSER_H
#define PARSER_H

#include "tokens.h"
#include "models.h"
#include "arrays/dynamic_array.h"

typedef struct ParserState ParserState;
struct ParserState {
	DArray 	tokens;
	int 	curr;
	Arena  *nodes_arena;
	Arena  *ast_nodes_arena;
};

AstNode *primary		(ParserState *ps);
AstNode *exponent		(ParserState *ps);
AstNode *unary			(ParserState *ps);
AstNode *multiplication	(ParserState *ps);
AstNode *addition		(ParserState *ps);
AstNode *comparison		(ParserState *ps);
AstNode *equality		(ParserState *ps);
AstNode *expr			(ParserState *ps);
AstNode *stmt			(ParserState *ps);
AstNode *stmts			(ParserState *ps);
AstNode *parse			(ParserState *ps);

#endif