#include "resolve.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "module.h"
#include "syscalls.h"

function_entry syscalls[MAX_SYSCALLS];
function_entry exports[MAX_EXPORTS];

function_entry *find_common(function_entry *entries, uint cnt, uint nid) {
	for (int i = 0; i < cnt; ++i)
		if (entries[i].nid == nid)
			return &entries[i];
	return &entries[0];
}

function_entry *find_syscall(int nid) {
	return find_common(syscalls, MAX_SYSCALLS, nid);
}

func_ptr find_export(int nid) {
	return find_common(exports, MAX_EXPORTS, nid)->func;
}

int unimplemented_syscall(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9, int a10) { // will probably crash and burn with more than 10 args
	int lr;
	asm("mov %0,lr\n" : "=r" (lr));
	printf("Unimplemented, called from 0x%08x, please file a bug report.\n", lr);
	return -1;
}

int unimplemented_func() {
	printf("Unimplemented func called!\n");
	exit(-3);
	return -1;
}

unsigned char moduleinfo_header[14] = {0x00, 0x00, 0x01, 0x01, 0x53, 0x63, 0x65, 0x53, 0x68, 0x61, 0x63, 0x63, 0x43, 0x67};
void resolve_stubs(void *start) {
	int export_cnt = 1;
	exports[0].func = unimplemented_func;

	module_info_t *module_info = (module_info_t*)memmem(start, -1, moduleinfo_header, sizeof(moduleinfo_header));
	LOG("Resolving module '%s' version %d\n", module_info->modname, module_info->modversion);

	module_imports_3x_t *import = (module_imports_3x_t*)((char*)start + module_info->stub_top);
	module_imports_3x_t *import_end = (module_imports_3x_t*)((char*)start + module_info->stub_end);
	for (; import < import_end; ++import) {
		if (import->size != sizeof(module_imports_3x_t)) {
			printf("Invalid import size 0x%08x, we only support 0x%08x\n", import->size, sizeof(module_imports_3x_t));
			exit(-1);
		}
		LOG("Library name %s. %d functions, %d variables.\n", import->lib_name, import->num_functions, import->num_vars);
		for (int i = 0; i < import->num_functions; ++i) {
			function_entry *entry = find_syscall(import->func_nid_table[i]);
			LOG("Resolving %08x at %p => %s (at %p)\n", import->func_nid_table[i], import->func_entry_table[i], entry->name, entry->func);
			int *patch = import->func_entry_table[i];
			*patch++ = 0xe59ff000; // ldr	pc, [pc]
			*patch++ = 0;
			*patch++ = (int)entry->func;
		}
	}

	module_exports_t *ex = (void*)((char*)start + module_info->ent_top);
	module_exports_t *export_end = (void*)((char*)start + module_info->ent_end);
	for (; ex < export_end; ++ex) {
		if (ex->size != sizeof(module_exports_t)) {
			printf("Invalid export size 0x%08x, we only support 0x%08x\n", ex->size, sizeof(module_exports_t));
			exit(-1);
		}
		LOG("Library name %s. %d functions. %d variables.\n", ex->lib_name ? ex->lib_name : "NULL", ex->num_functions, ex->num_vars);
		for (int i = 0; i < ex->num_functions; ++i) {
			LOG("NID %08x => func at %p\n", ex->nid_table[i], ex->entry_table[i]);
			exports[export_cnt].nid = ex->nid_table[i];
			exports[export_cnt].func = ex->entry_table[i];
			++export_cnt;
		}
	}
}

void register_syscalls(void) {
	int count = 0;
#define REGISTER(nid_, func_) syscalls[count].nid = nid_; syscalls[count].func = (void*)func_; syscalls[count].name = #func_; ++count;
	REGISTER(0xFFFFFFFF, unimplemented_syscall);
	REGISTER(0x632980d7, sceClibMemset);
	REGISTER(0x736753c8, sceClibMemmove);
	REGISTER(0x9cc2bfdf, sceClibMemcmp);
	REGISTER(0x2E3B02A1, sceClibMemcpy_safe);
	REGISTER(0xda6ec8ef, sceKernelCreateLwMutex);
	REGISTER(0x46e7be7b, sceKernelLockLwMutex);
	REGISTER(0xb295eb61, sceKernelGetTLSAddr);
	REGISTER(0x2f2c6046, sceClibAbort);
	REGISTER(0x91FA6614, SceLibKernel_91FA6614);
	REGISTER(0xfa6be467, sceClibVsnprintf);
	REGISTER(0xa2fb4d9d, sceClibStrcmp);
	REGISTER(0x660d1f6d, sceClibStrncmp);
	REGISTER(0xb110c123, sceKernelGetProcessTimeWide);
#undef REGISTER
}
