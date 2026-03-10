#include "parser.h"
#include "utils.h"

#include <stdio.h>

/*
	Helpers
*/

static Token peek(ParserState *ps) {
	return *(Token *)get_darray(&ps->tokens, ps->curr);
}

static Token advance(ParserState *ps) {
	Token tok = peek(ps);
	if (tok.token != TOK_EOF) ps->curr++;
	return tok;
}

static bool is_next(ParserState * ps, TokenKind expected) {
	Token next = peek(ps);
	return (next.token == expected) ? true : false;
}

static Token expect(ParserState *ps, TokenKind expected) {
	if (peek(ps).token == expected) {
		Token t = advance(ps);
		return t;
	} else {
		char msg_buffer[128];
		snprintf(msg_buffer, 128, "Expected %s! Found: %s", token_print[expected], token_print[peek(ps).token]);
		parse_error_with_line(msg_buffer, peek(ps).line);
	}
}

static Token previous_token(ParserState *ps) {
	return *(Token *)get_darray(&ps->tokens, ps->curr - 1);
}

static bool match(ParserState *ps, TokenKind expected) {
	if (ps->curr >= ps->tokens.size || peek(ps).token != expected) return false;
	ps->curr++;
	return true;
}

/*
	Parser
*/

AstNode *primary(ParserState *ps) {
	if (match(ps, TOK_INTEGER)) {
		Token tok = previous_token(ps);
		float f = 0;
		if (str8_to_float(tok.lexeme, &f) < 0) {
			parse_error_with_line("Failed to convert string to float!", tok.line);
		}
		return make_integer(ps->nodes_arena, ps->ast_nodes_arena, f, tok.line);
	}
	
	else if (match(ps, TOK_FLOAT)) 	{
		Token tok = previous_token(ps);
		float f = 0;
		if (str8_to_float(tok.lexeme, &f) < 0) {
			parse_error_with_line("Failed to convert string to float!", tok.line);
		}
		return make_float(ps->nodes_arena, ps->ast_nodes_arena, f, tok.line);
	}

	else if (match(ps, TOK_TRUE)) {
		Token tok = previous_token(ps);
		return make_bool(ps->nodes_arena, ps->ast_nodes_arena, true, tok.line);
	}

	else if (match(ps, TOK_FALSE)) {
		Token tok = previous_token(ps);
		return make_bool(ps->nodes_arena, ps->ast_nodes_arena, false, tok.line);
	}

	else if (match(ps, TOK_STRING)) {
		Token tok = previous_token(ps);
		return make_string(ps->nodes_arena, ps->ast_nodes_arena, tok.lexeme, tok.line);
	}

	else if (match(ps, TOK_LPAREN)) {
		Token tok = previous_token(ps);
		AstNode *ast_node = expr(ps);
		if (!match(ps, TOK_RPAREN)) {
			parse_error_with_line("Error: ')' expected.", tok.line);
		} else {
			return make_grouping(ps->nodes_arena, ps->ast_nodes_arena, ast_node, tok.line);
		}
	}

	else {
		Token tok_id = expect(ps, TOK_IDENTIFIER);
		AstNode *id = make_identifier(ps->nodes_arena, ps->ast_nodes_arena, tok_id.lexeme, tok_id.line);

		if (match(ps, TOK_LPAREN)) {
			DArray args = {0};
			init_darray(&args, sizeof(AstNode *), 2);
			
			while (peek(ps).token != TOK_RPAREN) {
				AstNode *arg = expr(ps);
				PUSH_DARRAY(&args, arg, AstNode *);

				if (peek(ps).token == TOK_COMMA) {
					advance(ps);
				} else {
					break;
				}
			}

			expect(ps, TOK_RPAREN);
			return make_func_call(ps->nodes_arena, ps->ast_nodes_arena, id, args, tok_id.line);
		} else {
			Token tok = previous_token(ps);
			return make_identifier(ps->nodes_arena, ps->ast_nodes_arena, tok.lexeme, tok.line);
		}
	}

	return NULL;
}

AstNode *exponent(ParserState *ps) {
	AstNode *left = primary(ps);

	while (match(ps, TOK_CARET)) {
		Token op = previous_token(ps);
		AstNode *right = exponent(ps);
		left = make_binop(ps->nodes_arena, ps->ast_nodes_arena, op, left, right);
	}

	return left;
}

AstNode *unary(ParserState *ps) {
	if (match(ps, TOK_NOT) || match(ps, TOK_MINUS) || match(ps, TOK_PLUS)) {
		Token op = previous_token(ps);
		AstNode *operand = unary(ps);
		return make_unop(ps->nodes_arena, ps->ast_nodes_arena, op, operand);
	}

	return exponent(ps);
}

AstNode *multiplication(ParserState *ps) {
	AstNode *left = unary(ps);

	while (match(ps, TOK_STAR) || match(ps, TOK_SLASH) || match(ps, TOK_MOD)) {
		Token op = previous_token(ps);
		AstNode *right = unary(ps);
		left = make_binop(ps->nodes_arena, ps->ast_nodes_arena, op, left, right);
	}

	return left;
}

AstNode *addition(ParserState *ps) {
	AstNode *left = multiplication(ps);

	while (match(ps, TOK_PLUS) || match(ps, TOK_MINUS)) {
		Token op = previous_token(ps);
		AstNode *right = multiplication(ps);
		left = make_binop(ps->nodes_arena, ps->ast_nodes_arena, op, left, right);
	}

	return left;
}

AstNode *comparison(ParserState *ps) {
	AstNode *left = addition(ps);

	while (match(ps, TOK_GT) || match(ps, TOK_LT) || match(ps, TOK_GE) || match(ps, TOK_LE)) {
		Token op = previous_token(ps);
		AstNode *right = addition(ps);
		left = make_binop(ps->nodes_arena, ps->ast_nodes_arena, op, left, right);
	}

	return left;
}

AstNode *equality(ParserState *ps) {
	AstNode *left = comparison(ps);

	while (match(ps, TOK_EQEQ) || match(ps, TOK_NE)) {
		Token op = previous_token(ps);
		AstNode *right = comparison(ps);
		left = make_binop(ps->nodes_arena, ps->ast_nodes_arena, op, left, right);
	}

	return left;
}

AstNode *logical_and(ParserState *ps) {
	AstNode *left = equality(ps);

	while (match(ps, TOK_AND)) {
		Token op = previous_token(ps);
		AstNode *right = equality(ps);
		left = make_binop(ps->nodes_arena, ps->ast_nodes_arena, op, left, right);
	}

	return left;
}

AstNode *logical_or(ParserState *ps) {
	AstNode *left = logical_and(ps);

	while (match(ps, TOK_OR)) {
		Token op = previous_token(ps);
		AstNode *right = logical_and(ps);
		left = make_binop(ps->nodes_arena, ps->ast_nodes_arena, op, left, right);
	}

	return left;
}

AstNode *expr(ParserState *ps) {
	return logical_or(ps);
}

AstNode *stmt(ParserState *ps) {
	if (match(ps, TOK_PRINT) || match(ps, TOK_PRINTLN)) {
		Token tok = previous_token(ps);
		AstNode *node = expr(ps);

		if (tok.token == TOK_PRINTLN) return make_print_statement(ps->nodes_arena, ps->ast_nodes_arena, true, node, tok.line);
		else return make_print_statement(ps->nodes_arena, ps->ast_nodes_arena, false, node, tok.line);
	}

	else if (match(ps, TOK_IF)) {
		Token tok_if 		= previous_token(ps);
		AstNode *condition 	= expr(ps);
		AstNode *then_stmts;
		AstNode *else_stmts = NULL;

		expect(ps, TOK_THEN);
		then_stmts = stmts(ps);

		if (match(ps, TOK_ELSE)) {
			else_stmts = stmts(ps);
		}

		expect(ps, TOK_END);

		return make_if_statement(ps->nodes_arena, ps->ast_nodes_arena, tok_if, condition, then_stmts, else_stmts);
	}

	else if (match(ps, TOK_FOR)) {
		Token tok_for 		= previous_token(ps);
		AstNode *start		= stmt(ps);
		AstNode *opt_step 	= NULL;
		AstNode *end;
		AstNode *do_stmts;

		expect(ps, TOK_COMMA);
		end = expr(ps);
		if (match(ps, TOK_COMMA)) {
			opt_step = expr(ps);
		}
		expect(ps, TOK_DO);

		if (peek(ps).token != TOK_END) {
			do_stmts = stmts(ps);
		}
		expect(ps, TOK_END);

		return make_for_statement(ps->nodes_arena, ps->ast_nodes_arena, tok_for, start, end, opt_step, do_stmts);
	}

	else if (match(ps, TOK_WHILE)) {
		Token tok_while 	= previous_token(ps);
		AstNode *condition 	= expr(ps);
		AstNode *do_stmts;

		expect(ps, TOK_DO);
		if (peek(ps).token != TOK_END) {
			do_stmts = stmts(ps);
		}
		expect(ps, TOK_END);

		return make_while_statement(ps->nodes_arena, ps->ast_nodes_arena, tok_while, condition, do_stmts);
	}

	else if (match(ps, TOK_FUNC)) {
		Token func_tok 	= previous_token(ps);
		Token tok_id 	= expect(ps, TOK_IDENTIFIER);
		AstNode *id 	= make_identifier(ps->nodes_arena, ps->ast_nodes_arena, tok_id.lexeme, tok_id.line);

		DArray params = {0};
		init_darray(&params, sizeof(AstNode *), 2);

		AstNode *func_stmts;

		expect(ps, TOK_LPAREN);

		int param_count = 0;
		while (peek(ps).token != TOK_RPAREN) {
			param_count++;
			if (param_count > 255) {
				parse_error_with_line("Parameter count is greater than 255!", tok_id.line);
			}

			AstNode *param = primary(ps);
			PUSH_DARRAY(&params, param, AstNode *);

			if (!is_next(ps, TOK_RPAREN)) {
				expect(ps, TOK_COMMA);

				if (is_next(ps, TOK_RPAREN)) {
					parse_error_with_line("No arg follows comma in arg list!", tok_id.line);
				}
			}
			param_count++;
		}

		expect(ps, TOK_RPAREN);

		if (peek(ps).token != TOK_END) {
			func_stmts = stmts(ps);	
		}

		expect(ps, TOK_END);

		return make_func_decl(ps->nodes_arena, ps->ast_nodes_arena, id, params, func_stmts, func_tok.line);
	}
	
	else if (match(ps, TOK_RET)) {
		Token tok = previous_token(ps);
		AstNode *return_val = expr(ps);
		return make_return_stmt(ps->nodes_arena, ps->ast_nodes_arena, return_val, tok.line);
	}

	else if (match(ps, TOK_LOCAL)) {
		Token tok_id = expect(ps, TOK_IDENTIFIER);
		AstNode *left = make_identifier(ps->nodes_arena, ps->ast_nodes_arena, tok_id.lexeme, tok_id.line);
		if (match(ps, TOK_ASSIGN)) {
			Token assign_tok = previous_token(ps);
			AstNode *right = expr(ps);
			return make_local_assignment(ps->nodes_arena, ps->ast_nodes_arena, left, right, assign_tok.line);
		} else {
			parse_error_with_line("Assignment operator (:=) does not follow 'local' keyword and identifier!", tok_id.line);
		}
	}

	else {
		AstNode *left = expr(ps);

		if (left->kind == _Identifier) {
			Token tok = expect(ps, TOK_ASSIGN);
			AstNode *right = expr(ps);
			return make_assignment(ps->nodes_arena, ps->ast_nodes_arena, left, right, tok.line);
		}
		
		else if (left->kind == _FuncCall) {
			return make_func_call_stmt(ps->nodes_arena, ps->ast_nodes_arena, left, left->line);
		}

		return left;
	}

	return NULL;
}

AstNode *stmts(ParserState *ps) {
	DArray stmt_nodes = {0};
	init_darray(&stmt_nodes, sizeof(AstNode *), 2);

	while (peek(ps).token != TOK_EOF && !is_next(ps, TOK_ELSE) && !is_next(ps, TOK_END)) {
		AstNode *stmt_node = stmt(ps);
		if (stmt_node) {
			PUSH_DARRAY(&stmt_nodes, stmt_node, AstNode *);
		}
	}

	return make_statements(ps->nodes_arena, ps->ast_nodes_arena, stmt_nodes, 0);
}

AstNode *parse(ParserState *ps) {
	AstNode *ast = stmts(ps);
	return ast;
}