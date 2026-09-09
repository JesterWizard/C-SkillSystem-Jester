#include "common-chax.h"
#include "kernel-lib.h"

extern const u8 Chname[];

typedef void (*ChapterNameDrawFunc)(int pos, int chapter);

void Chname_Gate(int pos, int chapter)
{
	ChapterNameDrawFunc draw = gpKernelDesignerConfig->chapter_names != false
		? (ChapterNameDrawFunc)((uintptr_t)Chname | 1)
		: (ChapterNameDrawFunc)0x08089625;

	draw(pos, chapter);
}
