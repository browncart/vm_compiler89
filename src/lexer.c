#include "lexer.h"
#include "utils.h"

#include <stdio.h>

/*
	Helpers
*/

static void add_token(LexerState *ls, TokenKind tt) {
	String8 substr;
	if (tt == TOK_STRING) {
		substr = (String8) {
			.str 	= &ls->source.str[ls->start + 1], 
			.size 	= (ls->curr - ls->start - 2)
		};
	} else {
		substr = (String8) {
			.str 	= &ls->source.str[ls->start], 
			.size 	= (ls->curr - ls->start)
		};
	}

	Token tok = {
		.token 	= tt,
		.lexeme = substr,
		.line 	= (u32)ls->line
	};

	push_back_darray(&ls->tokens, &tok);
}

static char advance(LexerState *ls) {
	return ls->source.str[ls->curr++];
}

static char peek(LexerState *ls) {
	if (ls->curr >= ls->source.size) return '\0';
	return ls->source.str[ls->curr];
}

static char lookahead(LexerState *ls, size_t n) {
	if (ls->curr + n >= ls->source.size) return '\0';
	return ls->source.str[ls->curr + n];
}

static bool match(LexerState *ls, char c) {
	if (ls->curr >= ls->source.size) return false;
	if (ls->source.str[ls->curr] != c) return false;
	
	ls->curr++;
	return true;
}

static bool is_alpha(char c) {
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return true;
	return false;
}

static bool is_digit(char c) {
	if (c >= '0' && c <= '9') return true;
	return false;
}

static void handle_number(LexerState *ls) {
	while (is_digit(peek(ls))) {
		advance(ls);
	}
	
	if (peek(ls) == '.' && is_digit(lookahead(ls, 1))) {
		advance(ls);
		while (is_digit(peek(ls))) {
			advance(ls);
		}
		add_token(ls, TOK_FLOAT);
	} 
	
	else {
		add_token(ls, TOK_INTEGER);
	}
}

static void handle_string(LexerState *ls, char start_quote) {
	while (peek(ls) != start_quote && ls->curr < ls->source.size) {
		advance(ls);
	}
	if (ls->curr >= ls->source.size) {
		lexer_error("Unterminated string.", ls->line);
	}
	advance(ls);
	add_token(ls, TOK_STRING);
}

static void handle_identifier(LexerState *ls) {
	while (is_digit(peek(ls)) || is_alpha(peek(ls)) || peek(ls) == '_') {
		advance(ls);
	}

	char *substr 		= &ls->source.str[ls->start];
	size_t substr_size 	= ls->curr - ls->start;

	int i;
	for (i = 0; i < keywords_size; ++i) {
		const char *keyword = keywords_str[i];
		size_t keyword_size = 0;
		int j 				= 0;

		while (keyword[j] != '\0') {
			++keyword_size;
			++j;
		}
		
		String8 substr8 = (String8){
			.str 	= substr,
			.size 	= substr_size
		};

		String8 keyword_str8 = (String8){
			.str 	= (char *)keyword,
			.size 	= keyword_size
		};

		if (keyword_size == substr_size && str8_match(substr8, keyword_str8)) {
			add_token(ls, keywords_tt[i]);
			return;
		}
	}

	add_token(ls, TOK_IDENTIFIER);
}

/*
	Lexer
*/

DArray tokenize(LexerState *ls) {
	while (ls->curr < ls->source.size) {
		ls->start = ls->curr;
		char ch = advance(ls);

		if (ch == '\n') ls->line++;
		else if (ch == ' ') continue;
		else if (ch == '\t') continue;
		else if (ch == '\r') continue;
		else if (ch == '\0') continue;
		else if (ch == '(') add_token(ls, TOK_LPAREN);
		else if (ch == ')') add_token(ls, TOK_RPAREN);
		else if (ch == '{') add_token(ls, TOK_LCURLY);
		else if (ch == '}') add_token(ls, TOK_RCURLY);
		else if (ch == '[') add_token(ls, TOK_LSQUAR);
		else if (ch == ']') add_token(ls, TOK_RSQUAR);
		else if (ch == '.') add_token(ls, TOK_DOT);
		else if (ch == ',') add_token(ls, TOK_COMMA);
		else if (ch == '+') add_token(ls, TOK_PLUS);
		else if (ch == '*') add_token(ls, TOK_STAR);
		else if (ch == '^') add_token(ls, TOK_CARET);
		else if (ch == '/') add_token(ls, TOK_SLASH);
		else if (ch == ';') add_token(ls, TOK_SEMICOLON);
		else if (ch == '?') add_token(ls, TOK_QUESTION);
		else if (ch == '%') add_token(ls, TOK_MOD);
		else if (ch == '-') {
			if (match(ls, '-')) {
				while (peek(ls) != '\n' && !(ls->curr >= ls->source.size)) {
					advance(ls);
				}
			} else {
				add_token(ls, TOK_MINUS);
			}
		} else if (ch == '=') {
			if (match(ls, '=')) {
				add_token(ls, TOK_EQEQ);
			} else {
				add_token(ls, TOK_EQ);
			}
		} else if (ch == '~') {
			if (match(ls, '=')) {
				add_token(ls, TOK_NE);
			} else {
				add_token(ls, TOK_NOT);
			}
		} else if (ch == '<') {
			if (match(ls, '=')) {
				add_token(ls, TOK_LE);
			} else {
				add_token(ls, TOK_LT);
			}
		} else if (ch == '>') {
			if (match(ls, '=')) {
				add_token(ls, TOK_GE);
			} else {
				add_token(ls, TOK_GT);
			}
		} else if (ch == ':') {
			if (match(ls, '=')) {
				add_token(ls, TOK_ASSIGN);
			} else {
				add_token(ls, TOK_COLON);
			}
		} else if (ch == '"' || ch == '\'') {
			handle_string(ls, ch);
		} else if (is_digit(ch)) {
			handle_number(ls);
		} else if (is_alpha(ch) || ch == '_') {
			handle_identifier(ls);
		} else {
			char msg_buffer[32];
			snprintf(msg_buffer, 32, "Unexpected character: %s", ch);
			lexer_error(msg_buffer, ls->line);
		}
	}

	Token tok_eof = (Token){
		.token 	= TOK_EOF,
		.lexeme = (String8){
			.str = (char *)"",
			.size = 0
		},
		.line = (u32)ls->line
	};

	push_back_darray(&ls->tokens, &tok_eof);
	return ls->tokens;
}