#ifndef MODELS_H
#define MODELS_H

#include "tokens.h"
#include "arena_allocator.h"
#include "string.h"
#include "arrays/dynamic_array.h"

typedef uint32_t u32;

extern const char *value_strings[];

typedef enum OP_CODES OP_CODES;
enum OP_CODES {
	PUSH,
	POP,
	ADD,
	SUB,
	MUL,
	DIV,
	OR,
	AND,
	XOR,
	NEG,
	EXP,
	MOD,
	EQ,
	NE,
	GT,
	GE,
	LT,
	LE,
	LOAD,
	STORE,
	LOAD_GLOBAL,
	STORE_GLOBAL,
	LOAD_LOCAL,
	STORE_LOCAL,
	LABEL,
	JMP,
	JMPZ, 
	JSR,
	RTS,
	ADJSP,
	PUSH_RET,
	PRINT,
	PRINTLN,
	HALT
};

typedef enum ValueType ValueType;
enum ValueType {
	VAL_NONE,
    VAL_NUM,
    VAL_BOOL,
    VAL_STRING
};

typedef struct Value Value;
struct Value {
	union {
        float 	val_num;
        bool 	val_bool;
        String8 val_string;
    };
	ValueType type;
};

typedef struct Instruction Instruction;
struct Instruction {
	Value 		value;
	OP_CODES 	name;
	int 		target;
};

typedef struct Symbol Symbol;
struct Symbol {
	String8 name;
	usize 	slot;
	int 	prev_index;
};

typedef struct FunctionSymbol FunctionSymbol;
struct FunctionSymbol {
	String8 name;
	usize 	index;
	usize 	arity;
};

typedef struct LabelFixup LabelFixup;
struct LabelFixup {
	int 	inst_idx;
	usize 	field_offset;
	int 	label_id;
};

typedef enum NodeType NodeType;
enum NodeType {
	_Integer,
	_Float,
	_Boolean,
	_String,
	_Identifier,
	_UnOp,
	_BinOp,
	_Grouping,
	_Assignment,
	_LocalAssignment,
	_IfStmt,
	_ForStmt,
	_WhileStmt,
	_PrintStmt,
	_FuncDecl,
	_FuncCall,
	_FuncCallStmt,
	_ReturnStmt,
	_Stmts
};

typedef struct AstNode AstNode;
struct AstNode {
	u32 offset;
	u32 line;
	NodeType kind;
};

typedef struct Integer Integer;
struct Integer {
	float value;
};

typedef struct Float Float;
struct Float {
	float value;
};

typedef struct Bool Bool;
struct Bool {
	bool value;
};

typedef struct String String;
struct String {
	String8 value;
};

typedef struct Identifier Identifier;
struct Identifier {
	String8 id;
};

typedef struct UnOp UnOp;
struct UnOp {
	Token op;
	AstNode *operand;
};

typedef struct BinOp BinOp;
struct BinOp {
	Token op;
	AstNode *left;
	AstNode *right;
};

typedef struct Grouping Grouping;
struct Grouping {
	AstNode *value;
};

typedef struct Assignment Assignment;
struct Assignment {
	AstNode *lvalue;
	AstNode *rvalue;
};

typedef struct LocalAssignment LocalAssignment;
struct LocalAssignment {
	AstNode *lvalue;
	AstNode *rvalue;
};

typedef struct IfStmt IfStmt;
struct IfStmt {
	Token token;
	AstNode *condition;
	AstNode *then_stmts;
	AstNode *else_stmts;
};

typedef struct ForStmt ForStmt;
struct ForStmt {
	Token 		token;
	AstNode    *start;
	AstNode    *end;
	AstNode    *opt_step;
	AstNode    *do_stmts;
	Token 		direction;
};

typedef struct WhileStmt WhileStmt;
struct WhileStmt {
	Token token;
	AstNode *condition;
	AstNode *do_stmts;
};

typedef struct PrintStmt PrintStmt;
struct PrintStmt {
	bool new_line;
	AstNode *expr;
};

typedef struct FuncDecl FuncDecl;
struct FuncDecl {
	AstNode *name;
	DArray params;
	AstNode *stmts;
};

typedef struct FuncCall FuncCall;
struct FuncCall {
	AstNode *name;
	DArray args;
};

typedef struct FuncCallStmt FuncCallStmt;
struct FuncCallStmt {
	AstNode *func_call;
};

typedef struct ReturnStmt ReturnStmt;
struct ReturnStmt {
	AstNode *result;
};

typedef struct Stmts Stmts;
struct Stmts {
	DArray stmts;
};

#define MAX_NODES (1024 * 1024)

void print_node_array		(Arena *a, DArray stmts, int indent);

void print_ast_node			(AstNode *ast, Arena *a, int indent);
void print_integer			(Integer *i, Arena *a, int indent);
void print_float			(Float *f, Arena *a, int indent);
void print_bool				(Bool *b, Arena *a, int indent);
void print_string			(String *s, Arena *a, int indent);
void print_identifier		(Identifier *i, Arena *a, int indent);
void print_unop				(UnOp *u, Arena *a, int indent);
void print_binop			(BinOp *b, Arena *a, int indent);
void print_assignment		(Assignment *as, Arena *a, int indent);
void print_local_assignment	(LocalAssignment *la, Arena *a, int indent);
void print_grouping			(Grouping *g, Arena *a, int indent);
void print_if_stmt			(IfStmt *i, Arena *a, int indent);
void print_for_stmt			(ForStmt *f, Arena *a, int indent);
void print_while_stmt		(WhileStmt *w, Arena *a, int indent);
void print_print_stmt		(PrintStmt *p, Arena *a, int indent);
void print_func_decl		(FuncDecl *f, Arena *a, int indent);
void print_func_call		(FuncCall *f, Arena *a, int indent);
void print_func_call_stmt	(FuncCallStmt *f, Arena *a, int indent);
void print_return_stmt		(ReturnStmt *r, Arena *a, int indent);
void print_stmts			(Stmts *s, Arena *a, int indent);

AstNode *make_integer			(Arena *nodes_arena, Arena *ast_nodes_arena, float value, u32 line_number);
AstNode *make_float				(Arena *nodes_arena, Arena *ast_nodes_arena, float value, u32 line_number);
AstNode *make_bool				(Arena *nodes_arena, Arena *ast_nodes_arena, bool value, u32 line_number);
AstNode *make_string			(Arena *nodes_arena, Arena *ast_nodes_arena, String8 value, u32 line_number);
AstNode *make_identifier		(Arena *nodes_arena, Arena *ast_nodes_arena, String8 value, u32 line_number);
AstNode *make_unop				(Arena *nodes_arena, Arena *ast_nodes_arena, Token op, AstNode *operand);
AstNode *make_binop				(Arena *nodes_arena, Arena *ast_nodes_arena, Token op, AstNode *left, AstNode *right);
AstNode *make_grouping			(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *operand, u32 line_number);
AstNode *make_assignment		(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *lvalue, AstNode *rvalue, u32 line_number);
AstNode *make_local_assignment	(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *lvalue, AstNode *rvalue, u32 line_number);
AstNode *make_if_statement		(Arena *nodes_arena, Arena *ast_nodes_arena, Token token, AstNode *condition, AstNode *then_stmts, AstNode *else_stmts);
AstNode *make_for_statement		(Arena *nodes_arena, Arena *ast_nodes_arena, Token token, AstNode *start, AstNode *end, AstNode *opt_step, AstNode *do_stmts);
AstNode *make_while_statement	(Arena *nodes_arena, Arena *ast_nodes_arena, Token token, AstNode *condition, AstNode *do_stmts);
AstNode *make_print_statement	(Arena *nodes_arena, Arena *ast_nodes_arena, bool new_line, AstNode *expr, u32 line_number);
AstNode *make_func_decl			(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *id, DArray params, AstNode *stmts, u32 line_number);
AstNode *make_func_call			(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *id, DArray args, u32 line_number);
AstNode *make_func_call_stmt	(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *func_call, u32 line_number);
AstNode *make_return_stmt		(Arena *nodes_arena, Arena *ast_nodes_arena, AstNode *expr, u32 line_number);
AstNode *make_statements		(Arena *nodes_arena, Arena *ast_nodes_arena, DArray stmt_nodes, u32 line_number);

#endif