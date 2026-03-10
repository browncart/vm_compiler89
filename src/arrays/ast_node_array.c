#include "ast_node_array.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int init_ast_node_array(AstNodeArray *a, usize capacity) {
	a->size 		= 0;
	a->capacity 	= capacity;
	a->data 		= (AstNode *)malloc(a->capacity * sizeof(AstNode));
	return a->data ? 0 : -1;
}

int resize_ast_node_array(AstNodeArray *a) {
	int new_capacity 	= a->capacity * 2 + 1;
	AstNode *new_data 	= (AstNode *)realloc(a->data, new_capacity * sizeof(AstNode));
	if (!new_data) return -1;
	a->data 			= new_data;
	a->capacity 		= new_capacity;
}

int push_back_ast_node_array(AstNodeArray *a, AstNode ast_node) {
	if (a->size == a->capacity) {
		resize_ast_node_array(a);
	}
	a->data[a->size] = ast_node;
	a->size++;
	return 0;
}

void pop_back_ast_node_array(AstNodeArray *a) {
	if (a->size > 0) a->size--;
}

AstNode get_ast_node_array(AstNodeArray *a, usize idx) {
	assert(idx < a->size && idx >= 0);
	return a->data[idx];
}

void clear_ast_node_array(AstNodeArray *a) {
	a->size = 0;
}

void free_ast_node_array(AstNodeArray *a) {
	free(a->data);
	a->data 		= NULL;
	a->size 		= 0;
	a->capacity 	= 0;
}