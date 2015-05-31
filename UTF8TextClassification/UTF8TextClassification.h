#include "ucharLib.h"

#define TYPE_COUNT 4

struct text
{
	struct ustring text;
	bool types[TYPE_COUNT];
};