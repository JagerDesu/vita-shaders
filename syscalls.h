#pragma once

void *sceClibMemset(void *s, int c, size_t n);
void *sceClibMemmove(void *dest, const void *src, size_t n);
int sceClibMemcmp(const void *s1, const void *s2, size_t n);
void *sceClibMemcpy_safe(void *dest, const void *src, size_t n);
int sceKernelCreateLwMutex(void *mem, const char *name, int attr, int count, void *opt);
int sceKernelLockLwMutex(void *mem, int count, int *timeout);
void *sceKernelGetTLSAddr(int pos);
void sceClibAbort(void);
int SceLibKernel_91FA6614(int a1, int a2);
int sceClibVsnprintf(char *str, size_t size, const char *format, va_list ap);
int sceClibStrcmp(const char *s1, const char *s2);
int sceClibStrncmp(const char *s1, const char *s2, size_t n);
unsigned long long sceKernelGetProcessTimeWide(void);
