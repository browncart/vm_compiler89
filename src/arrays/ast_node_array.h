#ifndef AST_NODE_ARRAY_H
#define AST_NODE_ARRAY_H

#include "../common.h"
#include "../models.h"

#include <string.h>

typedef struct AstNodeArray AstNodeArray;
struct AstNodeArray {
	AstNode    *data;
	usize 		size;
	usize 		capacity;
};

int 	init_ast_node_array			(AstNodeArray *a, usize capacity);
int 	resize_ast_node_array		(AstNodeArray *a);
int 	push_back_ast_node_array	(AstNodeArray *a, AstNode num);
void 	pop_back_ast_node_array		(AstNodeArray *a);
AstNode get_ast_node_array			(AstNodeArray *a, usize idx);
void 	clear_ast_node_array		(AstNodeArray *a);
void 	free_ast_node_array			(AstNodeArray *a);

#endif