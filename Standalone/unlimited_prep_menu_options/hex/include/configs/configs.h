/*
 * Empty stand-in for the kernel's configs/configs.h. FE-CLib's bmunit.h and prepscreen.h
 * include it. The Hex Cards add-on defines no CONFIG_* switches, so struct Unit has its
 * vanilla layout (CONFIG_MISC_UNIT_COUNTERS would change the conBonus bit-field only).
 * No kernel header is used.
 */
#pragma once
