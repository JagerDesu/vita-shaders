#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

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

struct shader_first
{
  int field_0;
  int field_4;
  int field_8;
  int field_C;
  int field_10;
  int field_14;
};

struct shader_second
{
  int field_0;
  int field_4;
  int field_8;
  int field_C;
  int field_10;
  int field_14;
  int field_18;
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
};

int gbuf[0x1000];

void *return_some_addr(void) {
	return gbuf;
}

void do_stuff(void) {
	func_ptr SceShaccCg_935CD196 = find_export(0x935cd196);
	func_ptr SceShaccCg_A8C2C1C8 = find_export(0xA8C2C1C8);
	func_ptr SceShaccCg_3B58AFA0 = find_export(0x3B58AFA0);
	func_ptr SceShaccCg_66814F35 = find_export(0x66814F35);
	func_ptr SceShaccCg_6F01D573 = find_export(0x6F01D573);

	int ret = 0;
	
	// Init the library
	ret = SceShaccCg_935CD196(0);
	printf("SceShaccCg_935CD196: 0x%08x\n", ret);

	// Register malloc and free function pointers
	ret = SceShaccCg_6F01D573(malloc, free);
	printf("SceShaccCg_6F01D573: 0x%08x\n", ret);

	struct shader_first first = {0};
	struct shader_second second = {0};
	ret = SceShaccCg_A8C2C1C8(first, 1);
	printf("SceShaccCg_A8C2C1C8: 0x%08x\n", ret);
	ret = SceShaccCg_3B58AFA0(second);
	printf("SceShaccCg_3B58AFA0: 0x%08x\n", ret);

	first.field_0 = return_some_addr;

	second.field_0 = 0;
	second.field_8 = "sqrt";
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
	ret = SceShaccCg_66814F35(&second, &first, 0);
	printf("SceShaccCg_66814F35: 0x%08x\n", ret);

	int *arr = (int*)ret;
	printf("%p %p %p %p %p %p %p %p\n", arr[0], arr[1], arr[2], arr[3], arr[4], arr[5], arr[6], arr[7]);
	arr = arr[3];
	printf("%08x %08x %08x %08x\n", arr[0], arr[1], arr[2], arr[3]);
}

int main(int argc, char *argv[]) {
	register_syscalls();
	load_binaries();
	printf("--------------------------------------------------------------------------------\n");
	do_stuff();
	printf("done\n");

	return 0;
}
