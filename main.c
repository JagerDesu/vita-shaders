#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "log.h"
#include "resolve.h"

#define SHACC_BASE0 0x84700550
#define SHACC_BASE1 0x81cda6b0

int load_binary(uintptr_t addr, int prot, const char *filename) {
	uintptr_t aligned_addr = addr & ~0xFFF;
	FILE *input = fopen(filename, "rb");
	if (!input) {
		printf("Failed to fopen(%s), errno %d\n", filename, errno);
		exit(-1);
	}
	fseek(input, 0, SEEK_END);
	size_t size = ftell(input);
	size_t aligned_size = (size + (addr - aligned_addr) + 0xFFF) & ~0xFFF;
	fseek(input, 0, SEEK_SET);
	uintptr_t mapped = (uintptr_t)mmap((void*)aligned_addr, aligned_size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
	if (mapped != aligned_addr) {
		printf("Failed to mmap(): mapped %08x != aligned %08x, terminating.\n", mapped, aligned_addr);
		exit(-1);
	}
	fread((void*)addr, size, 1, input);
	fclose(input);
}

void init_nullpage(void) {
	void *nullpage = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
	if (nullpage != NULL) {
		printf("Failed to mmap NULL page, result was %08x.\nPerhaps your Linux is too secure? try `echo 0 > /proc/sys/vm/mmap_min_addr`", nullpage);
		exit(-1);
	}
	*(uint*)nullpage = 0x11223344; // stack cookie
}

void load_binaries(void) {
	load_binary(SHACC_BASE0, PROT_READ | PROT_EXEC, "0x40020521_0x84700550_SceShaccCg_0.bin");
	load_binary(SHACC_BASE1, PROT_READ | PROT_WRITE, "0x40020521_0x81cda6b0_SceShaccCg_1.bin");

	resolve_stubs((void*)SHACC_BASE0);

	init_nullpage();
}

typedef struct {
	int field_0;
	int field_4;
	int field_8;
	int field_C;
	int field_10;
	int field_14;
} shader_first;

typedef struct {
	int field_0;
	int field_4;
	char* field_8;
	int field_C;
	int field_10;
	int field_14;
	const char** field_18;
	int field_1C;
	int field_20;
	int field_24;
	int field_28;
	int field_2C;
	int field_30;
	int field_34;
	int field_38;
	int field_3C;
	int field_40;
	int field_44;
	int field_48;
	int field_4C;
	int field_50;
	int field_54;
	int field_58;
	int field_5C;
	int field_60;
	int field_64;
} shader_second;

typedef struct {
	int unk;
	int x;
	int y;
} shader_error_pos;

typedef struct {
	int type;
	int unk2;
	shader_error_pos *pos;
	char *message;
} shader_error;

typedef struct {
	void *buf;
	int size;
	int error_count;
	shader_error *errors;
} shader_return;

typedef struct {
	char *unk;
	char *program;
	int size;
} shader_input;

shader_input g_input;

void *get_input(void) {
	return &g_input;
}

shader_return *get_shader(int type) {
	register_syscalls();
	load_binaries();

	func_ptr SceShaccCg_935CD196 = find_export(0x935cd196);
	func_ptr SceShaccCg_A8C2C1C8 = find_export(0xA8C2C1C8);
	func_ptr SceShaccCg_3B58AFA0 = find_export(0x3B58AFA0);
	func_ptr SceShaccCg_66814F35 = find_export(0x66814F35);
	func_ptr SceShaccCg_6F01D573 = find_export(0x6F01D573);

	g_input.unk = "<built-in>";

	int ret = 0;
	
	// Init the library
	ret = SceShaccCg_935CD196(0);
	LOG("SceShaccCg_935CD196: 0x%08x\n", ret);

	// Register malloc and free function pointers
	ret = SceShaccCg_6F01D573(malloc, free);
	LOG("SceShaccCg_6F01D573: 0x%08x\n", ret);

	shader_first first = {0};
	shader_second second = {0};
	ret = SceShaccCg_A8C2C1C8(first, 1);
	LOG("SceShaccCg_A8C2C1C8: 0x%08x\n", ret);
	ret = SceShaccCg_3B58AFA0(second);
	LOG("SceShaccCg_3B58AFA0: 0x%08x\n", ret);

	first.field_0 = (int)get_input;

	second.field_0 = (int)g_input.unk;
	second.field_4 =  type; // 0 -- vertex, 1 -- fragment
	second.field_8 = "main";
	second.field_14 = 1;
	const char *str = "SHADER_API_PSM";
	second.field_18 = &str;
	second.field_30 = 1;
	second.field_38 = 3;
	second.field_3C = 1;
	second.field_44 = 0;
	second.field_4C = 0;
	second.field_50 = 1;
	second.field_54 = 3;
	shader_return *r = (void*)SceShaccCg_66814F35(&second, &first, 0);
	LOG("SceShaccCg_66814F35: %p\n", r);
	return r;
}

void dump_heap(void) {
	char buf[4096];
	FILE *fin = fopen("/proc/self/maps", "rb");
	for (int i = 0; i < 3; ++i)
		fgets(buf, sizeof(buf), fin);
	// 4th line is the heap
	unsigned heap_start, heap_end;
	fscanf(fin, "%x-%x", &heap_start, &heap_end);
	printf("Heap: 0x%08x - 0x%08x\n", heap_start, heap_end);
	fclose(fin);
	FILE *fout = fopen("heap.bin", "wb");
	fwrite((void*)heap_start, heap_end - heap_start, 1, fout);
	fclose(fout);
}

void print_usage(void) {
	printf("Usage: shacc --vertex/--fragment input.cg output.gxp\n");
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
		print_usage();
		return -1;
	}
	int shader_type;
	if (strcmp(argv[1], "--vertex") == 0)
		shader_type = 0;
	else if (strcmp(argv[1], "--fragment") == 0)
		shader_type = 1;
	else {
		print_usage();
		return -1;
	}

	FILE *input = fopen(argv[2], "rb");
	if (!input) {
		fprintf(stderr, "%s: cannot open, errno %d\n", argv[2], errno);
		return -2;
	}
	fseek(input, 0, SEEK_END);
	g_input.size = ftell(input);
	fseek(input, 0, SEEK_SET);
	g_input.program = malloc(g_input.size);
	fread(g_input.program, g_input.size, 1, input);
	fclose(input);

	shader_return *r = get_shader(shader_type);
#ifdef DEBUG
	dump_heap();
#endif
	if (!r) {
		fprintf(stderr, "Unknown error\n");
		return -2;
	}

	// print infos/warnings/errors
	for (int i = 0; i < r->error_count; ++i) {
		shader_error *e = &r->errors[i];
		int x = 0, y = 0;
		if (e->pos) {
			x = e->pos->x;
			y = e->pos->y;
		}
		switch (e->type) {
		case 0:
			printf("INFO (%d, %d):\t", x, y);
			break;
		case 1:
			printf("WARN (%d, %d):\t", x, y);
			break;
		case 2:
			printf("ERROR (%d, %d):\t", x, y);
			break;
		}
		printf("%s\n", e->message);
	}


	if (r->buf) {
		FILE *output = fopen(argv[3], "wb");
		if (!output) {
			fprintf(stderr, "%s: cannot open for write, errno %d\n", argv[3], errno);
			return -2;
		}
		fwrite(r->buf, r->size, 1, output);
		fclose(output);
	} else {
		return -1;
	}

	return 0;
}
