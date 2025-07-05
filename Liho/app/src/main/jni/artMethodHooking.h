#include <jni.h>
#include <string>

extern int (*orig_doCall[4])(void*, void*, void*, void*, int, void*);
extern std::string(*orig_PrettyMethod)(void*, bool);

int hooked_doCall_0(void* a, void* b, void* c, void* d, int e, void* f);
int hooked_doCall_1(void* a, void* b, void* c, void* d, int e, void* f);
int hooked_doCall_2(void* a, void* b, void* c, void* d, int e, void* f);
int hooked_doCall_3(void* a, void* b, void* c, void* d, int e, void* f);

std::string PrettyMethod(void* arg1, bool arg2);