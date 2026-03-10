#include "models.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *value_strings[] = {
	"number",
	"boolean",
	"string"
};

void print_ast_node(AstNode *ast, Arena *a, int indent) {
	switch (ast->kind) {
		case _Integer: {
			Integer *int_node = (Integer *)(&a->buffer[ast->offset]);
			print_integer(int_node, a, indent);
			break;
		}
		case _Float: {
			Float *float_node = (Float *)(&a->buffer[ast->offset]);
			print_float(float_node, a, indent);
			break;
		}
		case _Boolean: {
			Bool *bool_node = (Bool *)(&a->buffer[ast->offset]);
			print_bool(bool_node, a, indent);
			break;
		}
		case _String: {
			String *string_node = (String *)(&a->buffer[ast->offset]);
			print_string(string_node, a, indent);
			break;
		}
		case _Identifier: {
			Identifier *identifier_node = (Identifier *)(&a->buffer[ast->offset]);
			print_identifier(identifier_node, a, indent);
			break;
		}
		case _UnOp: {
			UnOp *unop_node = (UnOp *)(&a->buffer[ast->offset]);
			print_unop(unop_node, a, indent);
			break;
		}
		case _BinOp: {
			BinOp *binop_node = (BinOp *)(&a->buffer[ast->offset]);
			print_binop(binop_node, a, indent);
			break;
		}
		case _Grouping: {
			Grouping *grouping_node = (Grouping *)(&a->buffer[ast->offset]);
			print_grouping(grouping_node, a, indent);
			break;
		}
		case _Assignment: {
			Assignment *assignment_node = (Assignment *)(&a->buffer[ast->offset]);
			print_assignment(assignment_node, a, indent);
			break;
		}
		case _LocalAssignment: {
			LocalAssignment *local_assignment_node = (LocalAssignment *)(&a->buffer[ast->offset]);
			print_local_assignment(local_assignment_node, a, indent);
			break;
		}
		case _IfStmt: {
			IfStmt *if_stmt_node = (IfStmt *)(&a->buffer[ast->offset]);
			print_if_stmt(if_stmt_node, a, indent);
			break;
		}
		case _ForStmt: {
			ForStmt *for_stmt_node = (ForStmt *)(&a->buffer[ast->offset]);
			print_for_stmt(for_stmt_node, a, indent);
			break;
		}
		case _WhileStmt: {
			WhileStmt *while_stmt_node = (WhileStmt *)(&a->buffer[ast->offset]);
			print_while_stmt(while_stmt_node, a, indent);
			break;
		}
		case _PrintStmt: {
			PrintStmt *print_stmt_node = (PrintStmt *)(&a->buffer[ast->offset]);
			print_print_stmt(print_stmt_node, a, indent);
			break;
		}
		case _FuncDecl: {
			FuncDecl *func_decl_node = (FuncDecl *)(&a->buffer[ast->offset]);
			print_func_decl(func_decl_node, a, indent);
			break;
		}
		case _FuncCall: {
			FuncCall *func_call_node = (FuncCall *)(&a->buffer[ast->offset]);
			print_func_call(func_call_node, a, indent);
			break;
		}
		case _FuncCallStmt: {
			FuncCallStmt *func_call_stmt_node = (FuncCallStmt *)(&a->buffer[ast->offset]);
			print_func_call_stmt(func_call_stmt_node, a, indent);
			break;
		}
		case _ReturnStmt: {
			ReturnStmt *return_node = (ReturnStmt *)(&a->buffer[ast->offset]);
			print_return_stmt(return_node, a, indent);
			break;
		}
		case _Stmts: {
			Stmts *stmts_node = (Stmts *)(&a->buffer[ast->offset]);
			print_stmts(stmts_node, a, indent);
			break;
		}
		default:
			break;
	}
}

void print_node_array(Arena *a, DArray stmts, int indent) {
	if (stmts.size == 0) return;
	u32 last_stmt = stmts.size - 1;

	int i;
	for (i = 0; i < last_stmt; ++i) {
		AstNode **item = (AstNode **)get_darray(&stmts, i);
		print_ast_node(*item, a, indent);
		DEBUG_PRINT(",\n");
	}
	AstNode *item = *(AstNode **)get_darray(&stmts, last_stmt);
	print_ast_node(item, a, indent);
}

void print_integer(Integer *i, Arena *a, int indent) {
	DEBUG_PRINT("%*sInteger: (%.f)", indent, "", i->value);
}

void print_float(Float *f, Arena *a, int indent) {
	DEBUG_PRINT("%*sFloat: (%g)", indent, "", f->value);
}

void print_bool(Bool *b, Arena *a, int indent) {
	if (b->value) DEBUG_PRINT("%*sBool: (true)", indent, "");
	else DEBUG_PRINT("%*sBool: (false)", indent, "");
}

void print_string(String *s, Arena *a, int indent) {
	DEBUG_PRINT("%*sString: (%.*s)", indent, "", s->value.size, s->value.str);
}

void print_identifier(Identifier *i, Arena *a, int indent) {
	DEBUG_PRINT("%*sIdentifier: (%.*s)", indent, "", i->id.size, i->id.str);
}

void print_unop(UnOp *u, Arena *a, int indent) {
	DEBUG_PRINT("%*sUnOp: (%s,\n", indent, "", token_lexemes[u->op.token]);
	print_ast_node(u->operand, a, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_binop(BinOp *b, Arena *a, int indent) {
	DEBUG_PRINT("%*sBinOp: (%s,\n", indent, "", token_lexemes[b->op.token]);
	print_ast_node(b->left, a, indent + 2);
	DEBUG_PRINT(",\n");
	print_ast_node(b->right, a, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_assignment(Assignment *as, Arena *a, int indent) {
	DEBUG_PRINT("%*sAssignment: (\n", indent, "");
	print_ast_node(as->lvalue, a, indent + 2);
	DEBUG_PRINT(",\n");
	print_ast_node(as->rvalue, a, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_local_assignment(LocalAssignment *la, Arena *a, int indent) {
	DEBUG_PRINT("%*sLocalAssignment: (\n", indent, "");
	print_ast_node(la->lvalue, a, indent + 2);
	DEBUG_PRINT(",\n");
	print_ast_node(la->rvalue, a, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_grouping(Grouping *g, Arena *a, int indent) {
	DEBUG_PRINT("%*sGrouping: (\n", indent, "");
	print_ast_node(g->value, a, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_if_stmt(IfStmt *i, Arena *a, int indent) {
	DEBUG_PRINT("%*sIfStmt: (\n", indent, "");
	print_ast_node(i->condition, a, indent + 2);
	DEBUG_PRINT(",\n");
	print_ast_node(i->then_stmts, a, indent + 2);
	
	if (i->else_stmts) {
		DEBUG_PRINT(",\n");
		print_ast_node(i->else_stmts, a, indent + 2);
	}

	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_for_stmt(ForStmt *f, Arena *a, int indent) {
	DEBUG_PRINT("%*sForStmt: (\n", indent, "");
	print_ast_node(f->start, a, indent + 2);
	DEBUG_PRINT(",\n");
	print_ast_node(f->end, a, indent + 2);
	DEBUG_PRINT(",\n");

	if (f->opt_step) {
		print_ast_node(f->opt_step, a, indent + 2);
		DEBUG_PRINT(",\n");
	}
	print_ast_node(f->do_stmts, a, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_while_stmt(WhileStmt *w, Arena *a, int indent) {
	DEBUG_PRINT("%*sWhile: (\n", indent, "");
	print_ast_node(w->condition, a, indent + 2);
	DEBUG_PRINT(",\n");
	print_ast_node(w->do_stmts, a, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_print_stmt(PrintStmt *p, Arena *a, int indent) {
	if (p->new_line) DEBUG_PRINT("%*sPrintLnStmt: (\n", indent, "");
	else DEBUG_PRINT("%*sPrintStmt: (\n", indent, "");
	print_ast_node(p->expr, a, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_func_decl(FuncDecl *f, Arena *a, int indent) {
	DEBUG_PRINT("%*sFuncDecl: (\n", indent, "");
	print_ast_node(f->name, a, indent + 2);
	
	DEBUG_PRINT(",\n%*sParams: (\n", indent + 2, "");
	print_node_array(a, f->params, indent + 4);
	DEBUG_PRINT("\n%*s),\n", indent + 2, "");

	print_ast_node(f->stmts, a, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_func_call(FuncCall *f, Arena *a, int indent) {
	DEBUG_PRINT("%*sFuncCall: (\n", indent, "");
	print_ast_node(f->name, a, indent + 2);	
	DEBUG_PRINT(",\n%*sArgs: (\n", indent + 2, "");
	print_node_array(a, f->args, indent + 4);
	DEBUG_PRINT("\n%*s)\n%*s)", indent + 2, "", indent, "");
}

void print_func_call_stmt(FuncCallStmt *f, Arena *a, int indent) {
	DEBUG_PRINT("%*sFuncCallStmt: (\n", indent, "");
	print_ast_node(f->func_call, a, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_return_stmt(ReturnStmt *r, Arena *a, int indent) {
	DEBUG_PRINT("%*sReturnStmt: (\n", indent, "");
	print_ast_node(r->result, a, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

void print_stmts(Stmts *s, Arena *a, int indent) {
	DEBUG_PRINT("%*sStmts: (\n", indent, "");
	print_node_array(a, s->stmts, indent + 2);
	DEBUG_PRINT("\n%*s)", indent, "");
}

AstNode *make_integer(Arena *nodes_arena, Arena *ast_nodes_arena, float value, u32 line_number) {
	Integer *int_node 	= (Integer *)(arena_alloc_align(nodes_arena, sizeof(Integer), sizeof(Integer)));
	int_node->value 	= value;
	u32 offset 			= nodes_arena->curr - sizeof(Integer);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _Integer;

	return ast_node;
}

AstNode *make_float(Arena *nodes_arena, Arena *ast_nodes_arena, float value, u32 line_number) {
	Float *float_node 	= (Float *)(arena_alloc_align(nodes_arena, sizeof(Float), sizeof(Float)));	
	float_node->value 	= value;
	u32 offset 			= nodes_arena->curr - sizeof(Float);
	
	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _Float;

	return ast_node;
}

AstNode *make_bool(Arena *nodes_arena, Arena *ast_nodes_arena, bool value, u32 line_number) {
	Bool *bool_node 	= (Bool *)(arena_alloc_align(nodes_arena, sizeof(Bool), sizeof(Bool)));	
	bool_node->value 	= value;
	u32 offset 			= nodes_arena->curr - sizeof(Bool);
	
	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _Boolean;

	return ast_node;
}

AstNode *make_string(Arena *nodes_arena, Arena *ast_nodes_arena, String8 value, u32 line_number) {
	char *new_str = (char *)(malloc(value.size + 1));
	new_str[value.size] = '\0';
	strncpy(new_str, value.str, value.size);
	value.str 			= new_str;

	String *string_node = (String *)(arena_alloc_default(nodes_arena, sizeof(String)));
	string_node->value 	= value;
	u32 offset 			= nodes_arena->curr - sizeof(String);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _String;

	return ast_node;
}

AstNode *make_identifier(Arena *nodes_arena, Arena *ast_nodes_arena, String8 value, u32 line_number) {
	Identifier *id_node = (Identifier *)(arena_alloc_default(nodes_arena, sizeof(Identifier)));
	id_node->id 		= value;
	u32 offset 			= nodes_arena->curr - sizeof(Identifier);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _Identifier;

	return ast_node;
}

AstNode *make_unop(Arena *nodes_arena, Arena *ast_nodes_arena, Token op, AstNode *operand) {
	UnOp *unop_node 	= (UnOp *)(arena_alloc_default(nodes_arena, sizeof(UnOp)));
	unop_node->op 		= op;
	unop_node->operand 	= operand;
	u32 offset 			= nodes_arena->curr - sizeof(UnOp);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= op.line;
	ast_node->kind 		= _UnOp;

	return ast_node;
}

AstNode *make_binop(Arena *nodes_arena, Arena *ast_nodes_arena, Token op, AstNode *left, AstNode *right) {
	BinOp *binop_node 	= (BinOp *)(arena_alloc_default(nodes_arena, sizeof(BinOp)));
	binop_node->op 		= op;
	binop_node->left 	= left;
	binop_node->right 	= right;
	u32 offset 			= nodes_arena->curr - sizeof(BinOp);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= op.line;
	ast_node->kind 		= _BinOp;

	return ast_node;
}

AstNode *make_grouping(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *value, u32 line_number) {
	Grouping *grouping 	= (Grouping *)(arena_alloc_default(nodes_arena, sizeof(Grouping)));
	grouping->value 	= value;
	u32 offset 			= nodes_arena->curr - sizeof(Grouping);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _Grouping;

	return ast_node;
}

AstNode *make_assignment(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *lvalue, AstNode *rvalue, u32 line_number) {
	Assignment *assign 	= (Assignment *)(arena_alloc_default(nodes_arena, sizeof(Assignment)));
	assign->lvalue 		= lvalue;
	assign->rvalue 		= rvalue;
	u32 offset 			= nodes_arena->curr - sizeof(Assignment);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _Assignment;

	return ast_node;
}

AstNode *make_local_assignment(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *lvalue, AstNode *rvalue, u32 line_number) {
	LocalAssignment *assign = (LocalAssignment *)(arena_alloc_default(nodes_arena, sizeof(LocalAssignment)));
	assign->lvalue			= lvalue;
	assign->rvalue 			= rvalue;
	u32 offset 				= nodes_arena->curr - sizeof(LocalAssignment);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _LocalAssignment;

	return ast_node;
}

AstNode *make_if_statement(Arena *nodes_arena, Arena *ast_nodes_arena, Token token, AstNode *condition, AstNode *then_stmts, AstNode *else_stmts) {
	IfStmt *if_stmt 	= (IfStmt *)(arena_alloc_default(nodes_arena, sizeof(IfStmt)));
	if_stmt->condition 	= condition;
	if_stmt->then_stmts = then_stmts;
	if_stmt->else_stmts = else_stmts;
	u32 offset 			= nodes_arena->curr - sizeof(IfStmt);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= token.line;
	ast_node->kind 		= _IfStmt;

	return ast_node;
}

AstNode *make_for_statement(Arena *nodes_arena, Arena *ast_nodes_arena, Token token, AstNode *start, AstNode *end, AstNode *opt_step, AstNode *do_stmts) {
	ForStmt *for_stmt 		= (ForStmt *)(arena_alloc_default(nodes_arena, sizeof(ForStmt)));
	for_stmt->start 		= start;
	for_stmt->end 			= end;
	for_stmt->opt_step 		= opt_step;
	for_stmt->do_stmts 		= do_stmts;
	u32 offset 				= nodes_arena->curr - sizeof(ForStmt);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= token.line;
	ast_node->kind 		= _ForStmt;

	return ast_node;
}

AstNode *make_while_statement(Arena *nodes_arena, Arena *ast_nodes_arena, Token token, AstNode *condition, AstNode *do_stmts) {
	WhileStmt *while_stmt 	= (WhileStmt *)(arena_alloc_default(nodes_arena, sizeof(WhileStmt)));
	while_stmt->condition 	= condition;
	while_stmt->do_stmts 	= do_stmts;
	u32 offset 				= nodes_arena->curr - sizeof(WhileStmt);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= token.line;
	ast_node->kind 		= _WhileStmt;

	return ast_node;
}

AstNode *make_print_statement(Arena *nodes_arena, Arena *ast_nodes_arena, bool new_line, AstNode *expr, u32 line_number) {
	PrintStmt *stmt = (PrintStmt *)(arena_alloc_default(nodes_arena, sizeof(PrintStmt)));
	stmt->new_line	= new_line;
	stmt->expr 		= expr;
	u32 offset 		= nodes_arena->curr - sizeof(PrintStmt);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _PrintStmt;

	return ast_node;
}

AstNode *make_func_decl(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *id, DArray params, AstNode *stmts, u32 line_number) {
	FuncDecl *func_decl = (FuncDecl *)(arena_alloc_default(nodes_arena, sizeof(FuncDecl)));
	func_decl->name 	= id;
	func_decl->params 	= params;
	func_decl->stmts 	= stmts;
	u32 offset 			= nodes_arena->curr - sizeof(FuncDecl);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _FuncDecl;

	return ast_node;
}

AstNode *make_func_call(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *id, DArray args, u32 line_number) {
	FuncCall *func_call = (FuncCall *)(arena_alloc_default(nodes_arena, sizeof(FuncCall)));
	func_call->name 	= id;
	func_call->args 	= args;
	u32 offset 			= nodes_arena->curr - sizeof(FuncCall);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _FuncCall;

	return ast_node;
}

AstNode *make_func_call_stmt(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *func_call, u32 line_number) {
	FuncCallStmt *func_call_stmt	= (FuncCallStmt *)(arena_alloc_default(nodes_arena, sizeof(FuncCallStmt)));
	func_call_stmt->func_call 		= func_call;
	u32 offset 						= nodes_arena->curr - sizeof(FuncCallStmt);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _FuncCallStmt;

	return ast_node;
}

AstNode *make_return_stmt(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *expr, u32 line_number) {
	ReturnStmt *return_stmt = (ReturnStmt *)(arena_alloc_default(nodes_arena, sizeof(ReturnStmt)));
	return_stmt->result		= expr;
	u32 offset 				= nodes_arena->curr - sizeof(ReturnStmt);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _ReturnStmt;

	return ast_node;
}

AstNode *make_statements(Arena *nodes_arena, Arena *ast_nodes_arena, DArray stmt_nodes, u32 line_number) {
	Stmts *stmts 		= (Stmts *)(arena_alloc_default(nodes_arena, sizeof(Stmts)));
	stmts->stmts 		= stmt_nodes;
	u32 offset 			= nodes_arena->curr - sizeof(Stmts);

	AstNode *ast_node 	= (AstNode *)(arena_alloc_default(ast_nodes_arena, sizeof(AstNode)));
	ast_node->offset 	= offset;
	ast_node->line 		= line_number;
	ast_node->kind 		= _Stmts;

	return ast_node;
}