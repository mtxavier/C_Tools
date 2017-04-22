#include <Classes.h>
#include <StackEnv.h>

typedef struct { struct RpgIntVal *Static; } RpgIntVal;
struct RpgIntVal {
	int (*Eval)(RpgIntVal *this);
};


