#include "common-chax.h"
#include "jester_headers/custom-functions.h"

struct KillRewards {
    u8 unitWinner;
    u8 unitLoser;
    u8 item;
};

static const struct KillRewards gKillRewards[] = {
    {
        .unitWinner    = CHARACTER_ARTUR,
        .unitLoser     = CHARACTER_NOVALA,
        .item          = ITEM_LIGHT_DIVINE,
    },
};

u8 GetItemReward(struct Unit * winner, struct Unit * loser)
{
    for (u8 i = 0; i < ARRAY_COUNT(gKillRewards); i++)
    {
        if (winner->pCharacterData->number == gKillRewards[i].unitWinner && loser->pCharacterData->number == gKillRewards[i].unitLoser)
            return gKillRewards[i].item;
    }

    return 0;
}