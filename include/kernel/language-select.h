#pragma once

#include "common-chax.h"

/* Order matches the flags on the boot language selector (2 rows x 3 columns) */
enum kernel_language {
	KERNEL_LANG_ENGLISH,
	KERNEL_LANG_FRENCH,
	KERNEL_LANG_GERMAN,
	KERNEL_LANG_SPANISH,
	KERNEL_LANG_ITALIAN,
	KERNEL_LANG_JAPANESE,

	KERNEL_LANG_MAX
};

/* Language picked on the boot selector, kept in the global save (SRAM) */
int GetKernelLanguage(void);
void SetKernelLanguage(int lang);
