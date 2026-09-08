#include "common-chax.h"
#include "hooks.h"
#include "stat-screen.h"
#include "skill-system.h"
#include "bwl.h"
#include "utf8.h"

typedef struct
{
    const int key;
    const char * quotes[2];   // up to 2 lines of quotes
    const char * age;         // character age (approximate where unknown)
    const char * height_cm;   // height in centimeters
    const char * likes[2];    // up to 2 likes
    const char * dislikes[2]; // up to 2 dislikes
} CharacterTraits;

static const CharacterTraits character_traits[] = 
{ 
    {0},

    { CHARACTER_EIRIKA,    {"I'll fight on for my", "friends with all I am."}, "17", "160cm", {"Peace", "Family"}, {"War", "Betrayal"} },
    { CHARACTER_SETH,      {"My blade guards you,", "and only you, my liege."}, "27", "185cm", {"Duty", "Order"}, {"Injustice", "Cowards"} },
    { CHARACTER_GILLIAM,   {"Steel may fail, but", "I'll hold the line firm."}, "28", "190cm", {"Discipline", "Loyalty"}, {"Neglect", "Disorder"} },
    { CHARACTER_FRANZ,     {"I'll train hard until", "I'm the knight I dream of."}, "15", "165cm", {"Training", "Heroes"}, {"Fear", "Failure"} },
    { CHARACTER_MOULDER,   {"My body may tire, yet", "my faith keeps me moving."}, "45", "178cm", {"Prayer", "Guidance"}, {"Doubt", "Weakness"} },
    { CHARACTER_VANESSA,   {"With wings high, I'll", "strike before they react."}, "18", "165cm", {"Flying", "Freedom"}, {"Restraint", "Storms"} },
    { CHARACTER_ROSS,      {"I'll grow strong so", "Dad will be proud of me."}, "13", "150cm", {"Adventure", "Dad"}, {"Losing", "Waiting"} },
    { CHARACTER_NEIMI,     {"My hands shake, but", "I'll aim true for you."}, "14", "155cm", {"Archery", "Friends"}, {"Loneliness", "Fear"} },
    { CHARACTER_COLM,      {"No lock stops me, for", "I slip past any wall."}, "15", "160cm", {"Freedom", "Gold"}, {"Rules", "Guards"} },
    { CHARACTER_GARCIA,    {"I fight on so my son", "sees the man I was."}, "40", "185cm", {"Family", "Training"}, {"Regret", "Weakness"} },
    { CHARACTER_INNES,     {"My arrows fly true,", "for I'm prince of the bow."}, "20", "182cm", {"Archery", "Honor"}, {"Failure", "Disrespect"} },
    { CHARACTER_LUTE,      {"The world will soon", "see my brilliance shine."}, "18", "162cm", {"Study", "Magic"}, {"Ignorance", "Mediocrity"} },
    { CHARACTER_NATASHA,   {"If the gods guide me,", "no darkness will stand."}, "19", "165cm", {"Faith", "Kindness"}, {"Cruelty", "Despair"} },
    { CHARACTER_CORMAG,    {"For my brother's honor,", "I drive my lance on."}, "24", "183cm", {"Honor", "Family"}, {"Betrayal", "Tyranny"} },
    { CHARACTER_EPHRAIM,   {"My spear carves the", "path to peace ahead."}, "18", "175cm", {"Battle", "Leadership"}, {"Cowardice", "Oppression"} },
    { CHARACTER_FORDE,     {"I may jest, yet when", "duty calls I fight hard."}, "23", "178cm", {"Painting", "Rest"}, {"Discipline", "War"} },
    { CHARACTER_KYLE,      {"Discipline guides each", "step I take to the end."}, "22", "182cm", {"Duty", "Training"}, {"Laziness", "Folly"} },
    { CHARACTER_AMELIA,    {"Someday I'll stand with", "the knights I admire."}, "13", "150cm", {"Knights", "Justice"}, {"Weakness", "Doubt"} },
    { CHARACTER_ARTUR,     {"With light before me,", "I'll banish all shadow."}, "19", "172cm", {"Light", "Books"}, {"Darkness", "Corruption"} },
    { CHARACTER_GERIK,     {"I'm a merc, but I'll", "shield my crew with steel."}, "29", "185cm", {"Loyalty", "Freedom"}, {"Greed", "Dishonor"} },
    { CHARACTER_TETHYS,    {"If my dance lifts you,", "then my steps have worth."}, "22", "165cm", {"Dance", "Beauty"}, {"Cruelty", "Silence"} },
    { CHARACTER_MARISA,    {"My blade speaks softly", "in silence sharper still."}, "19", "168cm", {"Swordplay", "Quiet"}, {"Noise", "Distraction"} },
    { CHARACTER_SALEH,     {"Ages of wisdom will", "light the road I walk."}, "30", "180cm", {"Study", "Teaching"}, {"Arrogance", "Chaos"} },
    { CHARACTER_EWAN,      {"I'll prove even I can", "stand tall as a mage."}, "14", "155cm", {"Magic", "Adventure"}, {"Doubt", "Failure"} },
    { CHARACTER_LARACHEL,  {"I was born perfect, so", "I'll show true divinity."}, "18", "162cm", {"Perfection", "Attention"}, {"Neglect", "Dirt"} },
    { CHARACTER_DOZLA,     {"Ha! Battle and laughs", "are joys I can't resist!"}, "45", "190cm", {"Laughter", "Battle"}, {"Sadness", "Cowards"} },
    { CHARACTER_ENEMY_1B,  {"", ""}, "", "", {"", ""}, {"", ""} },
    { CHARACTER_RENNAC,    {"If the pay is right,", "I'll stick around awhile."}, "24", "178cm", {"Money", "Freedom"}, {"Work", "Risk"} },
    { CHARACTER_DUESSEL,   {"Age may dull strength,", "but my honor stands firm."}, "50", "188cm", {"Honor", "Mentorship"}, {"Treachery", "Decay"} },
    { CHARACTER_MYRRH,     {"Though power scares me,", "I'll guard the friends I love."}, "1200+", "140cm", {"Friends", "Nature"}, {"Loneliness", "War"} },
    { CHARACTER_KNOLL,     {"The dark shares truths,", "but each one has a price."}, "28", "178cm", {"Research", "Truth"}, {"Naivety", "Faith"} },
    { CHARACTER_JOSHUA,    {"If luck stays with me,", "fortune will guide my hand."}, "20", "180cm", {"Gambling", "Freedom"}, {"Fate", "Obligation"} },
    { CHARACTER_SYRENE,    {"With sky and lance, I", "shield all I must protect."}, "27", "170cm", {"Duty", "Flight"}, {"Fear", "Cowardice"} },
    { CHARACTER_TANA,      {"Even if I stumble,", "I'll rise and soar again."}, "16", "162cm", {"Freedom", "Family"}, {"Losing", "Restraint"} },
    { CHARACTER_LYON_CC,      {"This power twists me,", "yet I bear it alone."}, "19", "175cm", {"Research", "Peace"}, {"Corruption", "Loneliness"} },
    { CHARACTER_ORSON_CC,     {"Though she is gone, my", "heart fights on for her."}, "28", "182cm", {"Love", "Memories"}, {"Reality", "Loss"} },
    { CHARACTER_ORSON_CH5X,   {"Her memory drives me,", "though the world took her."}, "28", "182cm", {"Love", "Memories"}, {"Reality", "Loss"} },
    { CHARACTER_GLEN_CC,      {"I'll uphold my duty,", "though its weight is heavy."}, "26", "185cm", {"Honor", "Duty"}, {"Corruption", "Lies"} },
    { CHARACTER_SELENA_CC,    {"The storm heeds me, and", "I'll wield it to the end."}, "25", "172cm", {"Loyalty", "Justice"}, {"Treachery", "Cruelty"} },
    { CHARACTER_VALTER_CC,    {"Battle feeds my hunger,", "and killing thrills me."}, "29", "184cm", {"Battle", "Chaos"}, {"Peace", "Calm"} },
    { CHARACTER_RIEV_CC,      {"The void whispers, and", "I answer every call."}, "40", "178cm", {"Darkness", "Power"}, {"Faith", "Light"} },
    { CHARACTER_CAELLACH_CC,  {"I'll climb until I", "claim a mighty king's power."}, "25", "183cm", {"Power", "Ambition"}, {"Weakness", "Fate"} },
    { CHARACTER_FADO_CC,      {"A king must stand when", "his people need him most."}, "35", "185cm", {"Duty", "Peace"}, {"Cowardice", "Betrayal"} },
    { CHARACTER_ISMAIRE_CC,   {"My sword strikes with", "honor, my grace its edge."}, "32", "170cm", {"Grace", "Swordplay"}, {"Dishonor", "Violence"} },
    { CHARACTER_HAYDEN_CC,    {"Age weighs on me, yet", "my crown keeps me steady."}, "50", "182cm", {"Stability", "Wisdom"}, {"Chaos", "Loss"} },
};

const int character_traits_size = sizeof(character_traits) / sizeof(character_traits[0]);

// ---------------------------------------------------------------------------
// Core helper: find traits by charId
// ---------------------------------------------------------------------------
static const CharacterTraits *GetCharacterTraits(int charId)
{
    for (int i = 0; i < character_traits_size; i++)
    {
        if (character_traits[i].key == charId)
            return &character_traits[i];
    }
    return NULL;
}

// ---------------------------------------------------------------------------
// Unified getters
// ---------------------------------------------------------------------------
static inline const char *SafeStr(const char *s)
{
    return (s && s[0] != '\0') ? s : "";
}

const char *GetCharacterQuote(int charId, int index)
{
    const CharacterTraits *t = GetCharacterTraits(charId);
    if (!t || index < 0 || index >= 2) return "";
    return SafeStr(t->quotes[index]);
}

const char *GetCharacterAge(int charId)
{
    const CharacterTraits *t = GetCharacterTraits(charId);
    return t ? SafeStr(t->age) : "";
}

const char *GetCharacterHeight(int charId)
{
    const CharacterTraits *t = GetCharacterTraits(charId);
    return t ? SafeStr(t->height_cm) : "";
}

const char *GetCharacterLikes(int charId, int index)
{
    const CharacterTraits *t = GetCharacterTraits(charId);
    if (!t || index < 0 || index >= 2) return "";
    return SafeStr(t->likes[index]);
}

const char *GetCharacterDislikes(int charId, int index)
{
    const CharacterTraits *t = GetCharacterTraits(charId);
    if (!t || index < 0 || index >= 2) return "";
    return SafeStr(t->dislikes[index]);
}

// ---------------------------------------------------------------------------
// Display helpers
// ---------------------------------------------------------------------------
static void DisplayLabeledText(int textIndex, int x, int y, int color, const char *label)
{
    PutDrawText(&gStatScreen.text[textIndex], gUiTmScratchA + TILEMAP_INDEX(x, y), color, 0, 0, label);
}

static void DisplayValueText(int textIndex, int x, int y, int color, const char *value)
{
    PutDrawText(&gStatScreen.text[textIndex], gUiTmScratchA + TILEMAP_INDEX(x, y), color, 0, 0, value);
}

void DisplayQuote(struct Unit *unit, int charId)
{
    DisplayLabeledText(15, 0x1, 0xB, TEXT_COLOR_SYSTEM_GREEN, "Quote:");
    DisplayValueText(  2, 0x1, 0xD, TEXT_COLOR_SYSTEM_WHITE, GetCharacterQuote(charId, 0));
    DisplayValueText( 34, 0x1, 0xF, TEXT_COLOR_SYSTEM_WHITE, GetCharacterQuote(charId, 1));
}

void DisplayAge(struct Unit *unit, int charId)
{
    DisplayLabeledText(16, 0x1, 0x9, TEXT_COLOR_SYSTEM_GOLD, "Age:");
    DisplayValueText( 17, 0x4, 0x9, TEXT_COLOR_SYSTEM_WHITE, GetCharacterAge(charId));
}

void DisplayHeight(struct Unit *unit, int charId)
{
    DisplayLabeledText(18, 0x9, 0x9, TEXT_COLOR_SYSTEM_GOLD, "Height:");
    DisplayValueText( 19, 0xD, 0x9, TEXT_COLOR_SYSTEM_WHITE, GetCharacterHeight(charId));
}

void DisplayLikes(struct Unit *unit, int charId)
{
    DisplayLabeledText(31, 0x1, 0x1, TEXT_COLOR_SYSTEM_BLUE, "Likes:");
    DisplayValueText( 32, 0x9, 0x1, TEXT_COLOR_SYSTEM_WHITE, GetCharacterLikes(charId, 0));
    DisplayValueText( 33, 0x9, 0x3, TEXT_COLOR_SYSTEM_WHITE, GetCharacterLikes(charId, 1));
}

void DisplayDislikes(struct Unit *unit, int charId)
{
    DisplayLabeledText(14, 0x1, 0x5, TEXT_COLOR_SYSTEM_BLUE, "Dislikes:");
    DisplayValueText( 29, 0x9, 0x5, TEXT_COLOR_SYSTEM_WHITE, GetCharacterDislikes(charId, 0));
    DisplayValueText( 30, 0x9, 0x7, TEXT_COLOR_SYSTEM_WHITE, GetCharacterDislikes(charId, 1));
}

// ---------------------------------------------------------------------------
// Page draw
// ---------------------------------------------------------------------------
void DrawPage6Rework(void)
{
    struct Unit *unit = gStatScreen.unit;
    int charId = UNIT_CHAR_ID(unit);

    for (int i = 14; i < 21; i++)
        ClearText(&gStatScreen.text[i]);
    for (int i = 29; i < 35; i++)
        ClearText(&gStatScreen.text[i]);

    TileMap_FillRect(gUiTmScratchA + 0x34, 12, 1, 0);

    DisplayQuote(unit, charId);
    DisplayAge(unit, charId);
    DisplayHeight(unit, charId);
    DisplayLikes(unit, charId);
    DisplayDislikes(unit, charId);
}