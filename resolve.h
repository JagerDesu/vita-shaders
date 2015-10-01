#pragma once

void resolve_stubs(void *start);
void register_syscalls(void);

typedef int(*func_ptr)();

func_ptr find_export(int nid);

typedef struct {
	int nid;
	func_ptr func;
	const char* name;
} function_entry;

#define MAX_SYSCALLS 64
#define MAX_EXPORTS 128