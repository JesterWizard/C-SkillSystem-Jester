#include "common-chax.h"
#include "types.h"
#include "eventinfo.h"
#include "constants/texts.h"
#include "jester_headers/maps.h"

// CONFIG_MULTIPLE_DEATH_QUOTES

const struct BattleTalkExtEnt gNewBattleTalkList[] = {
    {
        .pidA    = CHARACTER_VALTER_PROLOGUE,
        .pidB    = CHARACTER_SETH,
        .chapter = OUTSIDE_RENAIS,
        .msg     = MSG_BATTLE_QUOTE_VALTER_PROLOGUE,
    },
    {
        .pidA    = CHARACTER_EIRIKA,
        .pidB    = CHARACTER_ONEILL,
        .chapter = PROLOGUE,
        .flag    = EVFLAG_BATTLE_QUOTES,
        .msg     = MSG_BATTLE_QUOTE_EIRIKA_ONEILL,
    },
    {
        .pidA    = CHARACTER_SETH,
        .pidB    = CHARACTER_ONEILL,
        .chapter = PROLOGUE,
        .flag    = EVFLAG_BATTLE_QUOTES,
        .msg     = MSG_BATTLE_QUOTE_SETH_ONEILL,
    },
    {
        .pidA    = CHARACTER_BREGUET,
        .pidB    = CHARACTER_FRELIAN,
        .chapter = CHAPTER_01,
        .msg     = MSG_BATTLE_QUOTE_FRELIAN_SOLIDER_BORDER_MULAN,
    },    
    {
        .pidA    = CHARACTER_BREGUET,
        .pidB    = CHARACTER_GILLIAM,
        .chapter = CHAPTER_01,
        .flag    = EVFLAG_BATTLE_QUOTES,
        .msg     = MSG_BATTLE_QUOTE_GILLIAM_BREGUET,
    },
    {
        .pidA    = CHARACTER_BONE,
        .pidB    = CHARACTER_ROSS,
        .chapter = CHAPTER_02,
        .flag    = EVFLAG_BATTLE_QUOTES,
        .msg     = MSG_BATTLE_QUOTE_ROSS_BONE,
    },
    {
        .pidA    = CHARACTER_BAZBA,
        .pidB    = CHARACTER_ROSS,
        .chapter = CHAPTER_03,
        .flag    = EVFLAG_BATTLE_QUOTES,
        .msg     = MSG_BATTLE_QUOTE_ROSS_BAZBA,
    },
    {
        .pidA    = CHARACTER_SAAR,
        .pidB    = CHARACTER_NATASHA,
        .chapter = CHAPTER_05,
        .flag    = EVFLAG_BATTLE_QUOTES,
        .msg     = MSG_BATTLE_QUOTE_NATASHA_SAAR,
    },
    {
        .pidA    = CHARACTER_SAAR,
        .pidB    = CHARACTER_EIRIKA,
        .chapter = CHAPTER_05,
        .flag    = EVFLAG_BATTLE_QUOTES,
        .msg     = MSG_BATTLE_QUOTE_EIRIKA_SAAR,
    },
    {
        .pidA    = CHARACTER_NOVALA,
        .pidB    = CHARACTER_ARTUR,
        .chapter = CHAPTER_06,
        .flag    = EVFLAG_BATTLE_QUOTES,
        .msg     = MSG_BATTLE_QUOTE_ARTUR_NOVALA,
    },
    {
        .pidA    = CHARACTER_NOVALA,
        .pidB    = CHARACTER_EIRIKA,
        .chapter = CHAPTER_06,
        .flag    = EVFLAG_BATTLE_QUOTES,
        .msg     = MSG_BATTLE_QUOTE_EIRIKA_NOVALA,
    },
    {
        .pidA    = CHARACTER_TIRADO,
        .pidB    = CHARACTER_SETH,
        .chapter = CHAPTER_08,
        .flag    = EVFLAG_BATTLE_QUOTES,
        .msg     = MSG_BATTLE_QUOTE_SETH_TIRADO,
    },
    {
        .pidA    = 0xffff,
        .chapter = 0x00,
        .msg     = 0x0000,
    },

};

//! FE8U = 0x0808464C
LYN_REPLACE_CHECK(GetBattleQuoteEntry);
struct BattleTalkExtEnt* GetBattleQuoteEntry(u16 pidA, u16 pidB) {
    const struct BattleTalkExtEnt* it;

    for (it = gNewBattleTalkList; it->pidA != 0xFFFF; it++) {

        if (it->chapter != 0xff && it->chapter != gPlaySt.chapterIndex) {
            if (it->chapter != 0xfe || BattleIsTriangleAttack() != 1) {
                continue;
            }
        }

        if (GetEventTriggerState(it->flag)) {
            continue;
        }

        if (it->pidA != 0) {

            if (it->pidB == 0) {
                if (pidA == it->pidA) {
                    return (struct BattleTalkExtEnt *)it;
                }
                continue;
            }

        } else {
            if (it->pidB == 0) {
                continue;
            }

            if (pidB == it->pidB) {
                return (struct BattleTalkExtEnt *)it;
            }

            continue;
        }

        if ((pidA == it->pidA) && (pidB == it->pidB)) {
            return (struct BattleTalkExtEnt *)it;
        }

        if ((pidB == it->pidA) && (pidA == it->pidB)) {
            return (struct BattleTalkExtEnt *)it;
        }
    }

    return NULL;
}