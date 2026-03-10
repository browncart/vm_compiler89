#include "arrays/dynamic_array.h"
#include "compile.h"
#include "string.h"
#include "utils.h"

#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

char *op_codes[] = {
	"PUSH",
	"POP",
	"ADD",
	"SUB",
	"MUL",
	"DIV",
	"OR",
	"AND",
	"XOR",
	"NEG",
	"EXP",
	"MOD",
	"EQ",
	"NE",
	"GT",
	"GE",
	"LT",
	"LE",
	"LOAD",
	"STORE",
	"LOAD_GLOBAL",
	"STORE_GLOBAL",
	"LOAD_LOCAL",
	"STORE_LOCAL",
	"LABEL",
	"JMP",
	"JMPZ", 
	"JSR",
	"RTS",
	"ADJSP",
	"PUSH_RET",
	"PRINT",
	"PRINTLN",
	"HALT"
};

char *op_types[] = {
	"OP_NONE",
	"OP_NUMBER",
	"OP_BOOL",
	"OP_STRING"
};

void print_compiler_instructions(InstructionArray *code) {
	int indent = 0;

	int i;
	for (i = 0; i < code->size; ++i) {
		Instruction inst = get_instruction_array(code, i);

		printf("%.8d  ", i);

		if (inst.name == JMP || inst.name == JMPZ || inst.name == JSR) {
			printf("%*s%s %d\n", indent, "", op_codes[inst.name], inst.target);
			continue;
		}

		if (inst.name == LOAD_LOCAL || inst.name == STORE_LOCAL) {
			if (inst.value.val_num >= 0) {
				printf("%*s%s [+%g]\n", indent, "", op_codes[inst.name], inst.value.val_num);
			} else {
				printf("%*s%s [%g]\n", indent, "", op_codes[inst.name], inst.value.val_num);
			}
			continue;
		}

		if (inst.name == LOAD_GLOBAL || inst.name == STORE_GLOBAL) {
			printf("%*s%s [%g]\n", indent, "", op_codes[inst.name], inst.value.val_num);
			continue;
		}

		switch (inst.value.type) {
			case (VAL_NONE): {
				if (op_codes[inst.name] == "PUSH") {
					printf("%*s%s NONE\n", indent, "", op_codes[inst.name]);
				} else {
					printf("%*s%s\n", indent, "", op_codes[inst.name], op_types[VAL_NONE]);
				}
				break;
			}

			case (VAL_NUM): {
				if (inst.name == LABEL) {
					indent = 0;
				}

				printf("%*s%s %g", indent, "", op_codes[inst.name], inst.value.val_num);
				
				if (inst.name == LABEL) printf(":");
				printf("\n");
				break;
			}

			case (VAL_BOOL): {
				if (inst.value.val_bool) {
					printf("%*s%s true\n", indent, "",op_codes[inst.name]);
				} else {
					printf("%*s%s false\n", indent, "", op_codes[inst.name]);
				}
				break;
			}

			case (VAL_STRING): {
				String8 str = inst.value.val_string;

				printf("%*s%s ", indent, "",op_codes[inst.name]);

				printf("\"");
				int j;
				for (j = 0; j < str.size; ++j) {
					printf("%c", str.str[j]);
				}
				printf("\"\n");

				break;
			}
		}

		if (inst.name == LABEL) indent += 4;
	}
}

void emit_instruction(Instruction inst, InstructionArray *code) {
	push_back_instruction_array(code, inst);
}

int make_label(LabelsArray *l, int pc, int idx) {
	insert_label(l, pc, idx);
	return pc;
}

int make_emit_label(LabelsArray *l, InstructionArray *code, int label_idx) {
	Instruction label;
	label.name			= LABEL;
	label.value.type 	= VAL_NUM;
	label.value.val_num = label_idx;
	insert_label(l, code->size, label_idx);
	emit_instruction(label, code);
}

int make_emit_jmp(OP_CODES jmp_type, LabelFixupArray *fixups, InstructionArray *code, usize field_offset, int label_id) {
	Instruction jmp;
	jmp.name 	= jmp_type;
	jmp.target	= field_offset;
	emit_instruction(jmp, code);
	record_fixup(fixups, code->size - 1, field_offset, label_id);
}

int record_fixup(LabelFixupArray *fixups, int inst_idx, usize field_offset, int label_id) {
	LabelFixup fixup;
	fixup.inst_idx 		= inst_idx;
	fixup.field_offset 	= field_offset;
	fixup.label_id 		= label_id;

	push_back_label_fixup_array(fixups, fixup);
}

void fixup_pass(InstructionArray *code, LabelFixupArray *fixups, LabelsArray *labels) {
	int fixups_count = fixups->size;
	int i;

	for(i = 0; i < fixups_count; ++i) {
		LabelFixup fixup	= get_label_fixup_array(fixups, i);
		int inst_idx 		= fixup.inst_idx;
		int label_id 		= fixup.label_id;
		usize offset		= fixup.field_offset;

		int label 			= get_label(labels, label_id);
		Instruction *inst 	= get_pointer_instruction_array(code, inst_idx);

		*(int *)((char *)inst + offset) = label;
	}
}

int begin_block(SymbolArray *locals, UsizeArray *scope_markers) {
	assert(locals->size <= INT_MAX);
	push_back_usize_array(scope_markers, locals->size);
}

void end_block(SymbolArray *locals, UsizeArray *scope_markers, HashTable *local_prev_indexes, InstructionArray *code, int *curr_slot) {
	Instruction pop;
	pop.name 		= POP;
	pop.value.type 	= VAL_NONE;

	usize scope_start = get_usize_array(scope_markers, scope_markers->size - 1);
	pop_back_usize_array(scope_markers);

	usize i;
	for (i = locals->size; i > scope_start; --i) {
		Symbol local_var = get_symbol_array(locals, i - 1);
		pop_back_symbol_array(locals);
		
		if (local_var.prev_index != -1) {
			int *prev_index = (int *)find_ht(local_prev_indexes, local_var.name);
			*prev_index 	= local_var.prev_index;
		} else {
			remove_ht(local_prev_indexes, local_var.name);
		}

		emit_instruction(pop, code);
	}

	*curr_slot = locals->size;
}

void end_func_decl_block(SymbolArray *locals, UsizeArray *scope_markers, HashTable *local_prev_indexes, InstructionArray *code, int *curr_slot, int num_of_args) {
	usize scope_start = get_usize_array(scope_markers, scope_markers->size - 1);
	pop_back_usize_array(scope_markers);

	usize i;
	for (i = locals->size; i > scope_start; --i) {
		Symbol local_var = get_symbol_array(locals, i - 1);
		pop_back_symbol_array(locals);
		
		if (local_var.prev_index != -1) {
			int *prev_index = (int *)find_ht(local_prev_indexes, local_var.name);
			*prev_index 	= local_var.prev_index;
		} else {
			remove_ht(local_prev_indexes, local_var.name);
		}
	}

	*curr_slot = locals->size;
}

int derive_variable_offset(UsizeArray *func_arity, int slot_number) {	
	assert(func_arity->size > 0 && "Code does not live inside a function!");

	int arity = (int)get_usize_array(func_arity, func_arity->size - 1);
	if (slot_number < arity) {
		/*we subtract 1 because of the return address*/
		return slot_number - arity - 1;
	} else {
		/*we add one because fp is at 0*/
		return slot_number - arity + 1;
	}
}

Value evaluate_constant(Arena *base_nodes_arena, Arena *scratch, AstNode *ast) {
	switch (ast->kind) {
		case _Integer: {
			Integer *node = (Integer *)(&base_nodes_arena->buffer[ast->offset]);

			Value val;
			val.type 	= VAL_NUM;
			val.val_num = node->value;
			return val;
		}
				  
		case _Float: {
			Float *node = (Float *)(&base_nodes_arena->buffer[ast->offset]);
			
			Value val;
			val.type 	= VAL_NUM;
			val.val_num = node->value;
			return val;
		}         
				  
		case _Boolean: {
			Bool *node = (Bool *)(&base_nodes_arena->buffer[ast->offset]);
						
			Value val;
			val.type 		= VAL_BOOL;
			val.val_bool 	= node->value;
			return val;
		}         
				  
		case _String: {
			String *node = (String *)(&base_nodes_arena->buffer[ast->offset]);
			
			Value val;
			val.type 		= VAL_STRING;
			val.val_string 	= node->value;
			return val;
		}         
				  
		case _Grouping: {
			Grouping *node = (Grouping *)(&base_nodes_arena->buffer[ast->offset]);
			Value val = evaluate_constant(base_nodes_arena, scratch, node->value);
			return val;
		}         

		case _UnOp: {
			UnOp *node = (UnOp *)(&base_nodes_arena->buffer[ast->offset]);

			Value operand;
			operand = evaluate_constant(base_nodes_arena, scratch, node->operand);

			switch(node->op.token) {
				case TOK_PLUS: {
					if (operand.type == VAL_NUM) {						
						Value val;
						val.type 	= VAL_NUM;
						val.val_num = +operand.val_num;
						return val;
					}
					break;
				}
				
				case TOK_MINUS: {
					if (operand.type == VAL_NUM) {
						Value val;
						val.type 	= VAL_NUM;
						val.val_num = -operand.val_num;
						return val;
					}
					break;
				}

				case TOK_NOT: {
					if (operand.type == VAL_BOOL) {
						Value val;
						val.type 	= VAL_BOOL;
						val.val_num = !operand.val_num;
						return val;
					}
					break;
				}
			}

			compile_error_with_line("Unary operator not recognized!", ast->line);
		}

		case _BinOp: {
			BinOp *node = (BinOp *)(&base_nodes_arena->buffer[ast->offset]);
			Value left = evaluate_constant(base_nodes_arena, scratch, node->left);

			/*early out*/
			if (node->op.token == TOK_AND) {
				if (left.type == VAL_BOOL && !left.val_bool) {
					Value val;
					val.type 		= VAL_BOOL;
					val.val_bool 	= false;
					return val;
				}
			} else if (node->op.token == TOK_OR) {
				if (left.type == VAL_BOOL && left.val_bool) {
					Value val;
					val.type 	= VAL_BOOL;
					val.val_bool = true;
					return val;
				}
			}

			Value right;
			evaluate_constant(base_nodes_arena, scratch, node->right);

			switch (node->op.token) {
				case TOK_NE: {
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_num != right.val_num);
						return val;
					} 
					
					else if (left.type == VAL_STRING && right.type == VAL_STRING) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= !str8_match(left.val_string, right.val_string);
						return val;
					} 
					
					else if (left.type == VAL_BOOL && right.type == VAL_BOOL) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (right.val_bool != left.val_bool);
						return val;
					} 
					
					else {
						runtime_op_error(_BinOp, token_lexemes[TOK_NE], node->op.line, 2, left, right);
					}

					break;
				}
				
				case TOK_EQEQ: {
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_num == right.val_num);
						return val;
					} 
					
					else if (left.type == VAL_STRING && right.type == VAL_STRING) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= str8_match(left.val_string, right.val_string);
						return val;
					} 
					
					else if (left.type == VAL_BOOL && right.type == VAL_BOOL) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (right.val_bool == left.val_bool);
						return val;
					} 
					
					else {
						runtime_op_error(_BinOp, token_lexemes[TOK_EQEQ], node->op.line, 2, left, right);
					}

					break;
				}

				case TOK_GE: {
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_num >= right.val_num);
						return val;
					} 
					
					else if (left.type == VAL_STRING && right.type == VAL_STRING) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_string.size >= right.val_string.size);
						return val;
					} 
					
					else {
						runtime_op_error(_BinOp, token_lexemes[TOK_GE], node->op.line, 2, left, right);
					}
					
					break;
				}

				case TOK_GT: {					
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_num > right.val_num);
						return val;
					} 
					
					else if (left.type == VAL_STRING && right.type == VAL_STRING) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_string.size > right.val_string.size);
						return val;
					} 
					
					else {
						runtime_op_error(_BinOp, token_lexemes[TOK_GT], node->op.line, 2, left, right);
					}
					
					break;
				}
				
				case TOK_LE: {
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_num <= right.val_num);
						return val;
					} 
					
					else if (left.type == VAL_STRING && right.type == VAL_STRING) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_string.size <= right.val_string.size);
						return val;
					} 
					
					else {
						runtime_op_error(_BinOp, token_lexemes[TOK_LE], node->op.line, 2, left, right);
					}
					
					break;
				}

				case TOK_LT: {
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_num < right.val_num);
						return val;
					} 
					
					else if (left.type == VAL_STRING && right.type == VAL_STRING) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_string.size < right.val_string.size);
						return val;
					} 
					
					else {
						runtime_op_error(_BinOp, token_lexemes[TOK_LT], node->op.line, 2, left, right);
					}
					
					break;
				}
				
				case TOK_PLUS: {
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_NUM;
						val.val_num 	= left.val_num + right.val_num;
						return val;
					} 
					
					else if (left.type == VAL_STRING || right.type == VAL_STRING) {
						String8 left_str;
						String8 right_str;

						if (left.type == VAL_NUM) 			left_str = float_to_str8(scratch, left.val_num);
						else if (left.type == VAL_BOOL) 	left_str = bool_to_str8(left.val_bool);
						else 								left_str = left.val_string;

						if (right.type == VAL_NUM) 			right_str = float_to_str8(scratch, right.val_num);
						else if (right.type == VAL_BOOL) 	right_str = bool_to_str8(right.val_bool);
						else 								right_str = right.val_string;


						String8 new_str8 = str8_concat(scratch, left_str, right_str);						

						Value val;
						val.type 		= VAL_STRING;
						val.val_string 	= new_str8;
						return val;
					} 
					
					else {
						runtime_op_error(_BinOp, token_lexemes[TOK_PLUS], node->op.line, 2, left, right);
					}

					break;
				}
				
				case TOK_MINUS: {
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_NUM;
						val.val_num 	= left.val_num - right.val_num;
						return val;
					} else {
						runtime_op_error(_BinOp, token_lexemes[TOK_MINUS], node->op.line, 2, left, right);
					}

					break;
				}
				
				case TOK_STAR: {
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_NUM;
						val.val_num 	= left.val_num * right.val_num;
						return val;
					} else {
						runtime_op_error(_BinOp, token_lexemes[TOK_STAR], node->op.line, 2, left, right);
					}

					break;
				}
				
				case TOK_SLASH: {					
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						if (right.val_num == 0) {
							compile_error_with_line("Division by zero!", ast->line);
						}
						Value val;
						val.type 		= VAL_NUM;
						val.val_num 	= left.val_num / right.val_num;
						return val;
					} else {
						runtime_op_error(_BinOp, token_lexemes[TOK_SLASH], node->op.line, 2, left, right);
					}
					
					break;
				}
				
				case TOK_CARET: {
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_NUM;
						val.val_num 	= pow(left.val_num, right.val_num);
						return val;
					} else {
						runtime_op_error(_BinOp, token_lexemes[TOK_CARET], node->op.line, 2, left, right);
					}

					break;
				}
				
				case TOK_MOD: {
					if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_NUM;
						val.val_num 	= fmod(left.val_num, right.val_num);
						return val;
					} else {
						runtime_op_error(_BinOp, token_lexemes[TOK_MOD], node->op.line, 2, left, right);
					}

					break;
				}

				case TOK_AND: {
					if (left.type == VAL_BOOL && right.type == VAL_BOOL) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_bool && right.val_bool);
						return val;
					} 

					else if (left.type == VAL_NUM && right.type == VAL_BOOL) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_num && right.val_bool);
						return val;
					}

					else if (left.type == VAL_BOOL && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_bool && right.val_num);
						return val;
					}

					else if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_num && right.val_num);
						return val;
					}
					
					else {
						runtime_op_error(_BinOp, token_lexemes[TOK_AND], node->op.line, 2, left, right);
					}

					break;
				}

				case TOK_OR: {
					if (left.type == VAL_BOOL && right.type == VAL_BOOL) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_bool || right.val_bool);
						return val;
					} 
					
					else if (left.type == VAL_NUM && right.type == VAL_BOOL) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_num || right.val_bool);
						return val;
					}

					else if (left.type == VAL_BOOL && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_bool || right.val_num);
						return val;
					}

					else if (left.type == VAL_NUM && right.type == VAL_NUM) {
						Value val;
						val.type 		= VAL_BOOL;
						val.val_bool 	= (left.val_num || right.val_num);
						return val;
					}
					
					else {
						runtime_op_error(_BinOp, token_lexemes[TOK_OR], node->op.line, 2, left, right);
					}

					break;
				}
			}

			compile_error_with_line("Binary operator not recognized in constant folding!", ast->line);
		}
	}

	compile_error_with_line("Compiler Error: Global value is not constant at line [%d]!", ast->line);
}

void compile(CompilerState c_state) {
	InstructionArray 	*code 	= c_state.code;
	LabelsArray 		*labels = c_state.labels;
	LabelFixupArray 	*fixups = c_state.fixups;
	HashTable 			*funcs	= c_state.funcs;
	
	Instruction start_label;
	start_label.name 		= LABEL;
	start_label.value.type 	= VAL_NONE;
	emit_instruction(start_label, code);

	usize main_idx = code->size;

	Instruction jump_to_main;
	jump_to_main.name 			= JSR;
	jump_to_main.target			= 0;
	emit_instruction(jump_to_main, code);

	Instruction halt;
	halt.name 		= HALT;
	halt.value.type = VAL_NONE;
	emit_instruction(halt, code);

	compile_ast(c_state);

	fixup_pass(code, fixups, labels);

	String8 main;
	main.str 	= "main";
	main.size 	= 4;

	FunctionSymbol *main_func = (FunctionSymbol *)find_ht(funcs, main);
	if (!main_func) {
		char buffer[256];
		sprintf(buffer, "No main() function defined!");
		compile_error(buffer);
	}

	Instruction *main_jump = get_pointer_instruction_array(code, main_idx);
	assert(main_jump);
	main_jump->target = main_func->index;
}

void compile_ast(CompilerState c_state) {
	Arena 				*base_nodes_arena 	= c_state.base_nodes_arena;
	Arena 				*scratch 			= c_state.scratch;
	AstNode 			*ast 				= c_state.ast;
	HashTable 			*globals 			= c_state.globals;
	ValueArray			*global_data		= c_state.global_data;
	SymbolArray 		*locals 			= c_state.locals;
	HashTable 			*local_indexes 		= c_state.local_indexes;
	UsizeArray 			*scope_markers 		= c_state.scope_markers;
	UsizeArray			*func_arity			= c_state.func_arity;
	InstructionArray 	*code				= c_state.code;
	LabelsArray			*labels				= c_state.labels;
	LabelFixupArray		*fixups				= c_state.fixups;
	HashTable 			*funcs				= c_state.funcs;
	int					*max_locals			= c_state.max_locals;
	int 				*max_globals		= c_state.max_globals;
	int					*global_slot_count	= c_state.global_slot_count;
	int					*label_counter		= c_state.label_counter;
	int 				*local_slot_count 	= c_state.local_slot_count;
	int 				*scope_depth		= c_state.scope_depth;

	switch (ast->kind) {
		case (_Identifier): {
			Identifier *node = (Identifier *)(&base_nodes_arena->buffer[ast->offset]);

			Instruction inst;

			int *local_idx = (int *)find_ht(local_indexes, node->id);

			if (local_idx) {
				Symbol sym = get_symbol_array(locals, *local_idx);

				inst.name 				= LOAD_LOCAL;
				inst.value.type 		= VAL_NUM;
				inst.value.val_num		= derive_variable_offset(func_arity, sym.slot);
			} else {
				int *var = (int *)find_ht(globals, node->id);

				if (!var) {
					char buffer[256];
					sprintf(buffer, "Variable [%.*s] was not defined!", (int)node->id.size, node->id.str);
					compile_error(buffer);
				}
				
				inst.name 				= LOAD_GLOBAL;
				inst.value.type 		= VAL_NUM;
				inst.value.val_num 		= *var;
			}

			emit_instruction(inst, code);
			break;
		}

		case (_Integer): {
			Integer *node = (Integer *)(&base_nodes_arena->buffer[ast->offset]);

			Instruction inst;
			inst.name 				= PUSH;
			inst.value.type 		= VAL_NUM;
			inst.value.val_num 		= node->value;

			emit_instruction(inst, code);
			break;
		}

		case (_Float): {
			Float *node = (Float *)(&base_nodes_arena->buffer[ast->offset]);

			Instruction inst;
			inst.name 				= PUSH;
			inst.value.type 		= VAL_NUM;
			inst.value.val_num 		= node->value;

			emit_instruction(inst, code);
			break;
		}

		case (_Boolean): {
			Bool *node = (Bool *)(&base_nodes_arena->buffer[ast->offset]);

			Instruction inst;
			inst.name 			= PUSH;
			inst.value.type 	= VAL_BOOL;
			inst.value.val_bool = node->value;

			emit_instruction(inst, code);
			break;
		}

		case (_String): {
			String *node = (String *)(&base_nodes_arena->buffer[ast->offset]);

			Instruction inst;
			inst.name 				= PUSH;
			inst.value.type 		= VAL_STRING;
			inst.value.val_string 	= node->value;

			emit_instruction(inst, code);
			break;
		}

		case (_UnOp): {
			UnOp *node = (UnOp*)(&base_nodes_arena->buffer[ast->offset]);

			c_state.ast = node->operand;
			
			compile_ast(c_state);

			Instruction inst;
			inst.value.type = VAL_NONE;

			switch (node->op.token) {
				case (TOK_MINUS): {
					inst.name = NEG;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_NOT): {
					inst.name 				= PUSH;
					inst.value.type 		= VAL_BOOL;
					inst.value.val_bool 	= true;

					emit_instruction(inst, code);

					inst.name 				= XOR;
					inst.value.type 		= VAL_NONE;

					emit_instruction(inst, code);
					break;
				}
			}

			break;
		}

		case (_BinOp): {
			BinOp *node = (BinOp *)(&base_nodes_arena->buffer[ast->offset]);

			c_state.ast = node->left;
			compile_ast(c_state);

			c_state.ast = node->right;
			compile_ast(c_state);

			Instruction inst;
			inst.value.type = VAL_NONE;

			switch (node->op.token) {
				case (TOK_PLUS): {
					inst.name = ADD;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_MINUS): {
					inst.name = SUB;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_STAR): {
					inst.name = MUL;
					inst.value.type = VAL_NONE;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_SLASH): {
					inst.name = DIV;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_MOD): {
					inst.name = MOD;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_CARET): {
					inst.name = EXP;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_EQEQ): {
					inst.name = EQ;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_NE): {
					inst.name = NE;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_GT): {
					inst.name = GT;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_GE): {
					inst.name = GE;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_LT): {
					inst.name = LT;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_LE): {
					inst.name = LE;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_OR): {
					inst.name = OR;
					emit_instruction(inst, code);
					break;
				}

				case (TOK_AND): {
					inst.name = AND;
					emit_instruction(inst, code);
					break;
				}
			}
			break;
		}

		case (_Grouping): {
			Grouping *group = (Grouping *)(&base_nodes_arena->buffer[ast->offset]);

			c_state.ast = group->value;
			compile_ast(c_state);
			break;
		}

		case (_PrintStmt): {
			PrintStmt *stmt = (PrintStmt *)(&base_nodes_arena->buffer[ast->offset]);

			c_state.ast = stmt->expr;
			compile_ast(c_state);

			Instruction inst;
			inst.value.type = VAL_NONE;

			if (stmt->new_line) {
				inst.name = PRINTLN;
			} else {
				inst.name = PRINT;
			}

			emit_instruction(inst, code);
			break;
		}

		case (_IfStmt): {
			IfStmt *stmt = (IfStmt *)(&base_nodes_arena->buffer[ast->offset]);

			c_state.ast = stmt->condition;
			compile_ast(c_state);

			(*scope_depth)++;

			bool has_else_stmts = stmt->else_stmts != NULL;

			int else_label_idx;
			int exit_label_idx;

			if (has_else_stmts) {
				else_label_idx = (*label_counter)++;
				exit_label_idx = (*label_counter)++;
				make_emit_jmp(JMPZ, fixups, code, offsetof(Instruction, target), else_label_idx);
			} else {
				exit_label_idx = (*label_counter)++;
				make_emit_jmp(JMPZ, fixups, code, offsetof(Instruction, target), exit_label_idx);
			}
			
			c_state.ast = stmt->then_stmts;
			begin_block(locals, scope_markers);
			compile_ast(c_state);
			end_block(locals, scope_markers, local_indexes, code, local_slot_count);

			if (has_else_stmts) {
				make_emit_jmp(JMP, fixups, code, offsetof(Instruction, target), exit_label_idx);
				make_emit_label(labels, code, else_label_idx);
				c_state.ast = stmt->else_stmts;
				begin_block(locals, scope_markers);
				compile_ast(c_state);
				end_block(locals, scope_markers, local_indexes, code, local_slot_count);
			}

			make_emit_label(labels, code, exit_label_idx);

			(*scope_depth)--;
			break;
		}

		case (_ForStmt): {
			ForStmt *node 		= (ForStmt *)(&base_nodes_arena->buffer[ast->offset]);
			Assignment *assign 	= (Assignment *)(&base_nodes_arena->buffer[node->start->offset]);
			Identifier *id 		= (Identifier *)(&base_nodes_arena->buffer[assign->lvalue->offset]);

			(*scope_depth)++;
			begin_block(locals, scope_markers);

			int i_slot = *local_slot_count;
			c_state.ast = node->start;
			compile_ast(c_state);

			int loop_label_idx 	= (*label_counter)++;
			int decr_label_idx 	= (*label_counter)++;
			int exit_label_idx 	= (*label_counter)++;

			Instruction load_i;
			load_i.name 			= LOAD_LOCAL;
			load_i.value.type 		= VAL_NUM;
			load_i.value.val_num 	= derive_variable_offset(func_arity, i_slot);
			emit_instruction(load_i, code);

			c_state.ast = node->end;
			compile_ast(c_state);

			Instruction determine_direction;
			determine_direction.name = LE;
			determine_direction.value.type = VAL_NONE;
			emit_instruction(determine_direction, code);
			make_emit_jmp(JMPZ, fixups, code, offsetof(Instruction, target), decr_label_idx);

			/*
				Increment Segment
			*/

			make_emit_label(labels, code, loop_label_idx);

			emit_instruction(load_i, code);
			
			c_state.ast = node->end;
			compile_ast(c_state);

			emit_instruction(determine_direction, code);

			make_emit_jmp(JMPZ, fixups, code, offsetof(Instruction, target), exit_label_idx);

			begin_block(locals, scope_markers);
			c_state.ast = node->do_stmts;
			compile_ast(c_state);
			end_block(locals, scope_markers, local_indexes, code, local_slot_count);

			emit_instruction(load_i, code);

			if (node->opt_step) {
				c_state.ast = node->opt_step;
				compile_ast(c_state);
			} else {
				Instruction step_inst;
				step_inst.name 			= PUSH;
				step_inst.value.type	= VAL_NUM;
				step_inst.value.val_num = 1; 
				emit_instruction(step_inst, code);
			}

			Instruction inc_direction;
			inc_direction.value.type = VAL_NONE;
			inc_direction.name = ADD;
			emit_instruction(inc_direction, code);

			Instruction store_i;
			store_i.name = STORE_LOCAL;
			store_i.value.type = VAL_NUM;
			store_i.value.val_num = derive_variable_offset(func_arity, i_slot);
			emit_instruction(store_i, code);

			make_emit_jmp(JMP, fixups, code, offsetof(Instruction, target), loop_label_idx);

			/*
				Decrement Segment
			*/

			make_emit_label(labels, code, decr_label_idx);

			emit_instruction(load_i, code);

			c_state.ast = node->end;
			compile_ast(c_state);

			Instruction for_cond;
			for_cond.name = GE;
			for_cond.value.type = VAL_NONE;
			emit_instruction(for_cond, code);

			make_emit_jmp(JMPZ, fixups, code, offsetof(Instruction, target), exit_label_idx);

			begin_block(locals, scope_markers);
			c_state.ast = node->do_stmts;
			compile_ast(c_state);
			end_block(locals, scope_markers, local_indexes, code, local_slot_count);

			emit_instruction(load_i, code);

			if (node->opt_step) {
				c_state.ast = node->opt_step;
				compile_ast(c_state);
			} else {
				Instruction step_inst;
				step_inst.name 			= PUSH;
				step_inst.value.type	= VAL_NUM;
				step_inst.value.val_num = 1; 
				emit_instruction(step_inst, code);
			}

			Instruction step_direction;
			step_direction.value.type = VAL_NONE;
			step_direction.name = SUB;
			emit_instruction(step_direction, code);

			emit_instruction(store_i, code);

			make_emit_jmp(JMP, fixups, code, offsetof(Instruction, target), decr_label_idx);

			make_emit_label(labels, code, exit_label_idx);

			end_block(locals, scope_markers, local_indexes, code, local_slot_count);
			(*scope_depth)--;
			break;
		}

		case (_WhileStmt): {
			WhileStmt *node = (WhileStmt *)(&base_nodes_arena->buffer[ast->offset]);

			int enter_label_idx = (*label_counter)++;
			int exit_label_idx 	= (*label_counter)++;

			make_emit_label(labels, code, enter_label_idx);

			c_state.ast = node->condition;
			compile_ast(c_state);

			begin_block(locals, scope_markers);
			(*scope_depth)++;

			make_emit_jmp(JMPZ, fixups, code, offsetof(Instruction, target), exit_label_idx);

			c_state.ast = node->do_stmts;
			compile_ast(c_state);

			end_block(locals, scope_markers, local_indexes, code, local_slot_count);

			make_emit_jmp(JMP, fixups, code, offsetof(Instruction, target), enter_label_idx);

			make_emit_label(labels, code, exit_label_idx);

			(*scope_depth)--;
			break;
		}

		case (_LocalAssignment): {
			LocalAssignment *node 	= (LocalAssignment *)(&base_nodes_arena->buffer[ast->offset]);
			Identifier *id			= (Identifier *)(&base_nodes_arena->buffer[node->lvalue->offset]);

			c_state.ast = node->rvalue;
			compile_ast(c_state);

			Instruction inst;

			if (*scope_depth == 0) {
				int *possible_global = (int *)find_ht(globals, id->id);

				if (possible_global) {
					char buffer[256];
					sprintf(buffer, "Double global variable declaration! Identifier: %s\n", id->id);
					compile_error(buffer);
				}

				Value global_val = evaluate_constant(base_nodes_arena, scratch, node->rvalue);

				int *new_global_var = (int *)upsert_ht(globals, id->id);

				if (new_global_var) {
					*new_global_var = *global_slot_count;
					push_back_value_array(global_data, global_val);
				} else {
					char buffer[256];
					sprintf(buffer, "Globals hash table table is full!\n");
					compile_error(buffer);
				}

				(*global_slot_count)++;
			}

			else {
				Symbol new_sym;
				new_sym.name 		= id->id;
				new_sym.slot 		= *local_slot_count;

				int *idx = (int *)find_ht(local_indexes, id->id);

				if (idx) {
					int scope_start = get_int_array(scope_markers, scope_markers->size - 1);

					if (*idx >= scope_start) {
						char buffer[256];
						sprintf(buffer, "Double `local` variable assignment in same scope. Identifier: %s\n", id->id.str);
						compile_error(buffer);
					}
					
					new_sym.prev_index 	= *idx;
					*idx 				= *local_slot_count;
				}
				
				else {				
					new_sym.prev_index 	= -1;

					int *new_idx 	= (int *)upsert_ht(local_indexes, id->id);
					*new_idx 		= *local_slot_count;
				}

				push_back_symbol_array(locals, new_sym);
				(*local_slot_count)++;
				*max_locals = MAX(*local_slot_count, *max_locals);
			}
			
			break;
		}

		case (_Assignment): {
			Assignment *node 	= (Assignment *)(&base_nodes_arena->buffer[ast->offset]);
			Identifier *id		= (Identifier *)(&base_nodes_arena->buffer[node->lvalue->offset]);

			int *possible_global = (int *)find_ht(globals, id->id);

			if (*scope_depth == 0) {

				if (possible_global) {
					char buffer[256];
					sprintf(buffer, "Double global variable declaration! Identifier: %s\n", id->id);
					compile_error(buffer);
				}

				Value global_val = evaluate_constant(base_nodes_arena, scratch, node->rvalue);

				int *new_global_var = (int *)upsert_ht(globals, id->id);

				if (new_global_var) {
					*new_global_var = *global_slot_count;
					push_back_value_array(global_data, global_val);
				} else {
					char buffer[256];
					sprintf(buffer, "Globals hash table table is full!\n");
					compile_error(buffer);
				}

				(*global_slot_count)++;
			}

			else {
				c_state.ast = node->rvalue;
				compile_ast(c_state);

				int *possible_local = (int *)find_ht(local_indexes, id->id);

				if (possible_global && !possible_local) {
					Instruction inst;
					inst.name 			= STORE_GLOBAL;
					inst.value.type 	= VAL_NUM;
					inst.value.val_num 	= (float)*possible_global;
					emit_instruction(inst, code);
				} 
				
				else if (possible_local) {
					Instruction inst;
					inst.name 			= STORE_LOCAL;
					inst.value.type 	= VAL_NUM;
					inst.value.val_num 	= derive_variable_offset(func_arity, *possible_local);
					emit_instruction(inst, code);
				} else {
					Symbol new_sym;
					new_sym.name 		= id->id;
					new_sym.prev_index 	= -1;
					new_sym.slot 		= *local_slot_count;
					
					int *new_local_var 	= (int *)upsert_ht(local_indexes, id->id);
					
					if (new_local_var) {
						*new_local_var = *local_slot_count;
					} else {
						char buffer[256];
						sprintf(buffer, "Locals hash table is full!\n");
						compile_error(buffer);
					}
					
					push_back_symbol_array(locals, new_sym);
					(*local_slot_count)++;
					*max_locals = MAX(*local_slot_count, *max_locals);
				}
			}

			break;
		}

		case (_FuncDecl): {
			FuncDecl *func_decl = (FuncDecl *)(&base_nodes_arena->buffer[ast->offset]);
			Identifier *func_id = (Identifier *)(&base_nodes_arena->buffer[func_decl->name->offset]);

			void *name_taken = find_ht(funcs, func_id->id);
			if (name_taken) {
				char buffer[256];
				sprintf(buffer, "Function name [%.*s] already in use.", func_id->id.size, func_id->id.str);
				compile_error(buffer);
			}

			FunctionSymbol *func_symbol = (FunctionSymbol *)upsert_ht(funcs, func_id->id);
			func_symbol->name 			= func_id->id;
			func_symbol->index 			= code->size;
			func_symbol->arity			= func_decl->params.size;

			make_emit_label(labels, code, (*label_counter)++);

			int prev_local_slot_count 	= *local_slot_count;
			*local_slot_count 			= 0;

			(*scope_depth)++;

			begin_block(locals, scope_markers);

			usize function_arity = func_decl->params.size;

			push_back_usize_array(func_arity, function_arity);

			usize i;
			for (i = 0; i < function_arity; ++i) {
				AstNode **ast_param 	= (AstNode **)get_darray(&func_decl->params, i);
				Identifier *param_id 	= (Identifier *)(&base_nodes_arena->buffer[(*ast_param)->offset]);

				Symbol new_sym;
				new_sym.name 		= param_id->id;
				new_sym.slot 		= *local_slot_count;

				int *idx = (int *)find_ht(local_indexes, param_id->id);

				if (idx) {
					int scope_start = get_int_array(scope_markers, scope_markers->size - 1);

					if (*idx >= scope_start) {
						char buffer[256];
						sprintf(buffer, "Double variable assignment. Identifier: %.*s\n", param_id->id.size, param_id->id.str);
						compile_error(buffer);
					}
					
					new_sym.prev_index 	= *idx;
					*idx 				= *local_slot_count;
				}
				
				else {				
					new_sym.prev_index 	= -1;

					int *new_idx 	= (int *)upsert_ht(local_indexes, param_id->id);
					*new_idx 		= *local_slot_count;
				}

				push_back_symbol_array(locals, new_sym);
				(*local_slot_count)++;
				*max_locals = MAX(*local_slot_count, *max_locals);
			}

			c_state.ast = func_decl->stmts;
			compile_ast(c_state);

			(*scope_depth)--;
			end_func_decl_block(locals, scope_markers, local_indexes, code, local_slot_count, function_arity);
			*local_slot_count = prev_local_slot_count;

			Instruction default_return_value;
			default_return_value.name = PUSH;
			default_return_value.value.type = VAL_NONE;
			emit_instruction(default_return_value, code);

			Instruction rts;
			rts.name = RTS;
			rts.value.type = VAL_NONE;
			emit_instruction(rts, code);

			pop_back_usize_array(func_arity);

			break;
		}

		case (_FuncCall): {
			FuncCall *node 	= (FuncCall *)(&base_nodes_arena->buffer[ast->offset]);
			Identifier *id 	= (Identifier *)(&base_nodes_arena->buffer[node->name->offset]);

			FunctionSymbol *func_symbol = (FunctionSymbol *)find_ht(funcs, id->id);

			if (!func_symbol) {
				char buffer[256];
				sprintf(buffer, "Function %.*s was not defined before call!", (int)id->id.size, id->id.str);
				compile_error(buffer);
			}

			usize function_arity = node->args.size;
			if (function_arity != func_symbol->arity) {
				char buffer[256];
				if (node->args.size < func_symbol->arity) {
					sprintf(buffer, "Not enough arguments passed to function %.*s!", (int)func_symbol->name.size, func_symbol->name.str);
				} else {
					sprintf(buffer, "Too many arguments passed to function %.*s!", (int)func_symbol->name.size, func_symbol->name.str);
				}
				compile_error_with_line(buffer, ast->line);
			}

			int i;
			for (i = 0; i < function_arity; ++i) {
				AstNode **arg = (AstNode **)get_darray(&node->args, i);
				c_state.ast = *arg;
				compile_ast(c_state);
			}

			Instruction jsr;
			jsr.name 	= JSR;
			jsr.target 	= func_symbol->index;
			emit_instruction(jsr, code);

			Instruction adjsp;
			adjsp.name	= ADJSP;
			adjsp.value.type = VAL_NUM;
			adjsp.value.val_num = -(int)function_arity;
			emit_instruction(adjsp, code);

			Instruction push_ret;
			push_ret.name		= PUSH_RET;
			push_ret.value.type = VAL_NONE;
			emit_instruction(push_ret, code);

			break;
		}

		case (_FuncCallStmt): {
			FuncCallStmt *func_call_stmt 	= (FuncCallStmt *)(&base_nodes_arena->buffer[ast->offset]);
			c_state.ast 					= func_call_stmt->func_call;
			compile_ast(c_state);

			Instruction pop;
			pop.name 		= POP;
			pop.value.type 	= VAL_NONE;
			emit_instruction(pop, code);

			break;
		}

		case (_ReturnStmt): {
			ReturnStmt *return_stmt = (ReturnStmt *)(&base_nodes_arena->buffer[ast->offset]);
			c_state.ast = return_stmt->result;
			compile_ast(c_state);

			Instruction rts;
			rts.name 		= RTS;
			rts.value.type 	= VAL_NONE;
			emit_instruction(rts, code);

			break;
		}

		case (_Stmts): {
			Stmts *node = (Stmts *)(&base_nodes_arena->buffer[ast->offset]);

			int i;
			for (i = 0; i < node->stmts.size; ++i) {
				AstNode *stmt = *(AstNode **)get_darray(&node->stmts, i);				
				c_state.ast = stmt;
				compile_ast(c_state);
			}
			break;
		}
	}

	*max_globals = *global_slot_count;
}