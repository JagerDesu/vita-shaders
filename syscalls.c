#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG(fmt, ...) fprintf(stderr, "    " fmt, ##__VA_ARGS__)

void *sceClibMemset(void *s, int c, size_t n) {
	LOG("sceClibMemset(%p, 0x%02x, 0x%08x)\n", s, c, n);
	return memset(s, c, n);
}

void *sceClibMemmove(void *dest, const void *src, size_t n) {
	LOG("sceClibMemmove(%p, %p, 0x%08x)\n", dest, src, n);
	return memmove(dest, src, n);
}

int sceClibMemcmp(const void *s1, const void *s2, size_t n) {
	return memcmp(s1, s2, n);
}

void *sceClibMemcpy_safe(void *dest, const void *src, size_t n) {
	return memcpy(dest, src, n);
}

int sceClibStrcmp(const char *s1, const char *s2) {
	LOG("sceClibStrcmp(%s, %s)\n", s1, s2);
	return strcmp(s1, s2);
}

int sceKernelCreateLwMutex(void *mem, const char *name, int attr, int count, void *opt) {
	LOG("STUB sceKernelCreateLwMutex(%p, %s, %d, %d, %p)\n", mem, name, attr, count, opt);
	return 0;
}

int sceKernelLockLwMutex(void *mem, int count, int *timeout) {
	LOG("STUB sceKernelLockLwMutex(%p, %d, %08x)\n", mem, count, timeout ? *timeout : -1);
	return 0;
}

int tlsbuf[0x1000];
void *sceKernelGetTLSAddr(int pos) {
	// LOG("sceKernelGetTLSAddr(0x%08x)\n", pos);
	return &tlsbuf[pos];
}

void sceClibAbort(void) {
	LOG("abort() called OOOOPS\n");
	exit(-2);
}

int SceLibKernel_91FA6614(int a1, int a2) {
	LOG("STUB SceLibKernel_91FA6614(0x%08x, 0x%08x)\n", a1, a2);
	return 0;
}

int sceClibVsnprintf(char *str, size_t size, const char *format, va_list ap) {
	LOG("sceClibVsnprintf(%p, %d, %s, ...)", str, size, format);
	int ret = vsnprintf(str, size, format, ap);
	LOG(" => printed %s\n", str);
	return ret;
}

unsigned long long sceKernelGetProcessTimeWide(void) {
	LOG("STUB sceKernelGetProcessTimeWide()\n");
	return 0;
}
