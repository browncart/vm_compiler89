#include "vm.h"
#include "compile.h"
#include "utils.h"
#include "arrays/int_array.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void run_VM(VMState vm_state) {
	InstructionArray *code 	= vm_state.code;
	Arena *scratch 			= vm_state.scratch;
	Arena *string_storage	= vm_state.string_storage;

	IntArray func_call_pcs = {0};
	init_int_array(&func_call_pcs, 10);

	Value return_value;

	Value *stack = (Value *)malloc(sizeof(Value) * 1024 * 1024);
	usize sp 				= 0;
	usize fp				= 0;
	usize pc				= 0;
	bool running 			= true;

	while (running && pc < code->size) {
		size_t scratch_save = arena_save(scratch);

		Instruction instruction = get_instruction_array(code, pc++);

		switch (instruction.name) {
			case (PUSH): {
				stack[sp] = instruction.value;
				sp++;
				break;
			}

			case (POP): {
				sp--;
				break;
			}

			case (ADD): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];				

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type 	= VAL_NUM;
					result.val_num 	= left.val_num + right.val_num;
					stack[sp] = result;
					sp++;
				}

				else if ((left.type == VAL_STRING || right.type == VAL_STRING) && (left.type != VAL_NONE && right.type != VAL_NONE)) {
					String8 left_str;
					String8 right_str;

					if (left.type == VAL_NUM) 			left_str = float_to_str8(scratch, left.val_num);
					else if (left.type == VAL_BOOL) 	left_str = bool_to_str8(left.val_bool);
					else 								left_str = left.val_string;

					if (right.type == VAL_NUM) 			right_str = float_to_str8(scratch, right.val_num);
					else if (right.type == VAL_BOOL) 	right_str = bool_to_str8(right.val_bool);
					else 								right_str = right.val_string;

					String8 new_str8 = str8_concat(string_storage, left_str, right_str);						

					arena_restore_from_save(scratch, scratch_save);

					result.type = VAL_STRING;
					result.val_string = new_str8;
					stack[sp] = result;
					sp++;
				}
				
				else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on ADD between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}

				break;
			}

			case (SUB): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type = VAL_NUM;
					result.val_num = left.val_num - right.val_num;
					stack[sp] = result;
					sp++;
				} else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on SUB between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (MUL): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type = VAL_NUM;
					result.val_num = left.val_num * right.val_num;
					stack[sp] = result;
					sp++;
				} else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on MUL between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (DIV): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type = VAL_NUM;
					result.val_num = left.val_num / right.val_num;
					stack[sp] = result;
					sp++;
				} else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on DIV between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (MOD): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type 	= VAL_NUM;
					result.val_num 	= fmod(left.val_num, right.val_num);
					stack[sp] = result;
					sp++;
				} else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on MOD between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (AND): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_num && right.val_num;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_BOOL && left.type == VAL_BOOL){
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_bool && right.val_bool;
					stack[sp] = result;
					sp++;
				} 

				else if (right.type == VAL_NUM && left.type == VAL_BOOL){
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_bool && right.val_num;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_BOOL && left.type == VAL_NUM){
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_num && right.val_bool;
					stack[sp] = result;
					sp++;
				} 
				
				else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on AND between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (OR): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type 	= VAL_BOOL;
					result.val_bool	= left.val_num || right.val_num;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_BOOL && left.type == VAL_BOOL){
					result.type 	= VAL_BOOL;
					result.val_bool	= left.val_bool || right.val_bool;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_NUM && left.type == VAL_BOOL){
					result.type 	= VAL_BOOL;
					result.val_bool	= right.val_num || left.val_bool;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_BOOL && left.type == VAL_NUM){
					result.type 	= VAL_BOOL;
					result.val_bool	= right.val_bool || left.val_num;
					stack[sp] = result;
					sp++;
				} 
				
				else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on OR between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (XOR): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				union {float f; u32 u;} r, l;
				r.f = right.val_num;
				l.f = left.val_num;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type 	= VAL_BOOL;
					result.val_bool	= l.u ^ r.u;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_BOOL && left.type == VAL_BOOL){
					result.type 	= VAL_BOOL;
					result.val_bool	= left.val_bool ^ right.val_bool;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_NUM && left.type == VAL_BOOL){
					result.type 	= VAL_BOOL;
					result.val_bool	= left.val_bool ^ r.u;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_BOOL && left.type == VAL_NUM){
					result.type 	= VAL_BOOL;
					result.val_bool	= l.u ^ right.val_bool;
					stack[sp] = result;
					sp++;
				} 
				
				else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on XOR between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (NEG): {
				sp--;
				Value solo_val = stack[sp];

				Value result;

				if (solo_val.type == VAL_NUM) {
					result.type 	= VAL_NUM;
					result.val_num 	= -(solo_val.val_num);
					stack[sp] = result;
					sp++;
				} 
				
				else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on NEG with SOLO TYPE: %s.", op_types[solo_val.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (GT): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_num > right.val_num;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_STRING && left.type == VAL_STRING) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_string.size > right.val_string.size;
					stack[sp] = result;
					sp++;
				} 
				
				else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on GT between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (LT): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_num < right.val_num;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_STRING && left.type == VAL_STRING) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_string.size < right.val_string.size;
					stack[sp] = result;
					sp++;
				} 
				
				else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on LT between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}
			
			case (GE): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_num >= right.val_num;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_STRING && left.type == VAL_STRING) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_string.size >= right.val_string.size;
					stack[sp] = result;
					sp++;
				} 
				
				else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on GE between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (LE): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_num <= right.val_num;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_STRING && left.type == VAL_STRING) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_string.size <= right.val_string.size;
					stack[sp] = result;
					sp++;
				} 
				
				else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on LE between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (EQ): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_num == right.val_num;
					stack[sp] = result;
					sp++;
				} 
				
				else if (right.type == VAL_BOOL && left.type == VAL_BOOL) {
					result.type 	= VAL_BOOL;
					result.val_bool = right.val_bool == left.val_bool;
					stack[sp] = result;
					sp++;
				}
				
				else if (right.type == VAL_STRING && left.type == VAL_STRING) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_string.size == right.val_string.size;
					stack[sp] = result;
					sp++;
				} 
				
				else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on EQ between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (NE): {
				sp--;
				Value right = stack[sp];

				sp--;
				Value left = stack[sp];

				Value result;

				if (right.type == VAL_NUM && left.type == VAL_NUM) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_num != right.val_num;
					stack[sp] = result;
					sp++;
				} 

				else if (right.type == VAL_BOOL && left.type == VAL_BOOL) {
					result.type 	= VAL_BOOL;
					result.val_bool = right.val_bool != left.val_bool;
					stack[sp] = result;
					sp++;
				}
				
				else if (right.type == VAL_STRING && left.type == VAL_STRING) {
					result.type 	= VAL_BOOL;
					result.val_bool = left.val_string.size != right.val_string.size;
					stack[sp] = result;
					sp++;
				} 
				
				else {
					char msg_buffer[256];
					sprintf(msg_buffer, "Error on NE between LEFT TYPE: %s and RIGHT TYPE: %s.", op_types[left.type], op_types[right.type]);
					vm_runtime_error(msg_buffer, pc - 1); 
				}
				break;
			}

			case (PRINT): {
				sp--;
				Value inst = stack[sp];

				switch (inst.type) {
					case VAL_NONE: {
						compile_error("Tried to print NONE value.");
						break;
					}
					case VAL_NUM: {
						String8 str8 = float_to_str8(scratch, inst.val_num);
						printf("%s", str8.str);
						break;
					}
					case VAL_BOOL: {
						inst.val_bool ? printf("true") : printf("false");
						break;
					}
					case VAL_STRING: {
						printf("%.*s", inst.val_string.size, inst.val_string.str);
						break;
					}
				}

				arena_restore_from_save(scratch, scratch_save);

				break;
			}

			case (PRINTLN): {
				sp--;
				Value inst = stack[sp];

				switch (inst.type) {
					case VAL_NONE: {
						compile_error("Tried to println NONE value.");
						break;
					}
					case VAL_NUM: {
						String8 str8 = float_to_str8(scratch, inst.val_num);
						printf("%s", str8.str);
						break;
					}
					case VAL_BOOL: {
						inst.val_bool ? printf("true") : printf("false");
						break;
					}
					case VAL_STRING: {
						printf("%.*s", inst.val_string.size, inst.val_string.str);
						break;
					}
				}

				arena_restore_from_save(scratch, scratch_save);

				printf("\n");
				break;
			}

			case (LOAD_GLOBAL): {
				int idx = (int)instruction.value.val_num;
				Value global_val = get_value_array(vm_state.globals, idx);
				stack[sp] = global_val;
				sp++;

				break;
			}

			case (STORE_GLOBAL): {
				sp--;
				int idx 		= (int)instruction.value.val_num;
				Value *global_val = get_pointer_value_array(vm_state.globals, idx);
				*global_val = stack[sp];

				break;
			}

			case (LOAD_LOCAL): {
				int offset 			= (int)instruction.value.val_num;
				Value load_val = stack[fp + offset];
				stack[sp] 			= load_val;
				sp++;

				break;
			}

			case (STORE_LOCAL): {				
				sp--;
				Value store_val = stack[sp];

				int offset = (int)instruction.value.val_num;

				stack[fp + offset] = store_val;

				break;
			}

			case (JMP): {
				pc = instruction.target;
				break;
			}

			case (JMPZ): {
				sp--;
				Value inst = stack[sp];

				if ((inst.type == VAL_BOOL && !inst.val_bool) || (inst.type == VAL_NUM && !inst.val_num)) {
					pc = instruction.target;
				}
				break;
			}

			case (JSR): {
				Value return_addr;
				return_addr.type 	= VAL_NUM;
				return_addr.val_num = pc;

				stack[sp++] = return_addr;

				Value frame;
				frame.type = VAL_NUM;
				frame.val_num = fp;

				stack[sp++] = frame;

				fp = sp - 1;

				pc = instruction.target;
				break;
			}

			case (RTS): {
				sp--;
				return_value = stack[sp];

				sp = fp;
				fp = stack[fp].val_num;

				sp--;
				Value return_addr = stack[sp];
				pc = return_addr.val_num;

				break;
			}

			case (ADJSP): {
				sp += instruction.value.val_num;
				break;
			}

			case (PUSH_RET): {
				stack[sp++] = return_value;
				break;
			}

			case (HALT): {
				running = false;
				break;
			}
		}
	}
}