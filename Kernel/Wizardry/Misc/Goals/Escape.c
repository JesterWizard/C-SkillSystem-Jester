#include "common-chax.h"
#include "jester_headers/custom-functions.h"
#include "jester_headers/procs.h"

bool HasEscapeObjective(int chapterIndex)
{
    switch (chapterIndex)
    {
    case CHAPTER_L_PROLOGUE:
    case CHAPTER_L_1:
    case CHAPTER_L_2:
    case CHAPTER_L_3:
    case CHAPTER_L_4:
    case CHAPTER_L_5:
    case CHAPTER_L_5X:
    case CHAPTER_L_6:
    case CHAPTER_L_7:
    case CHAPTER_L_8:
    case CHAPTER_E_9:
    case CHAPTER_E_10:
    case CHAPTER_E_11:
    case CHAPTER_E_12:
    case CHAPTER_E_13:
    case CHAPTER_E_14:
    case CHAPTER_E_15:
    case CHAPTER_E_16:
    case CHAPTER_E_17:
    case CHAPTER_E_18:
        return true;

    default:
        return false;
    }
}

bool IsEscapeTile(int chapterIndex, int x, int y)
{
    switch (chapterIndex)
    {
    case CHAPTER_L_PROLOGUE:
    case CHAPTER_L_1:
    case CHAPTER_L_2:
    case CHAPTER_L_3:
    case CHAPTER_L_4:
    case CHAPTER_L_5:
    case CHAPTER_L_5X:
    case CHAPTER_L_6:
    case CHAPTER_L_7:
    case CHAPTER_L_8:
    case CHAPTER_E_9:
    case CHAPTER_E_10:
    case CHAPTER_E_11:
    case CHAPTER_E_12:
    case CHAPTER_E_13:
    case CHAPTER_E_14:
    case CHAPTER_E_15:
    case CHAPTER_E_16:
    case CHAPTER_E_17:
    case CHAPTER_E_18:
        return x == 3 && y == 3;

    default:
        return false;
    }
}