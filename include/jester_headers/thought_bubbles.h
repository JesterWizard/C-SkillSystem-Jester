// EIRIKA
extern u8 Gfx_Chapter_02_Thought_Bubble_Eirika_Left[];
extern u8 Gfx_Chapter_02_Thought_Bubble_Eirika_Right[];
extern u8 Gfx_Chapter_03_Thought_Bubble_Eirika_Left[];
extern u8 Gfx_Chapter_03_Thought_Bubble_Eirika_Right[];
extern u8 Gfx_Chapter_04_Thought_Bubble_Eirika_Left[];
extern u8 Gfx_Chapter_04_Thought_Bubble_Eirika_Right[];
extern u8 Gfx_Chapter_05_Thought_Bubble_Eirika_Left[];
extern u8 Gfx_Chapter_05_Thought_Bubble_Eirika_Right[];
extern u8 Gfx_Chapter_06_Thought_Bubble_Eirika_Left[];
extern u8 Gfx_Chapter_06_Thought_Bubble_Eirika_Right[];
extern u8 Gfx_Chapter_07_Thought_Bubble_Eirika_Left[];
extern u8 Gfx_Chapter_07_Thought_Bubble_Eirika_Right[];
extern u8 Gfx_Chapter_09_Thought_Bubble_Eirika_Left[];
extern u8 Gfx_Chapter_09_Thought_Bubble_Eirika_Right[];

// SETH
extern u8 Gfx_Chapter_02_Thought_Bubble_Seth_Left[];
extern u8 Gfx_Chapter_02_Thought_Bubble_Seth_Right[];
extern u8 Gfx_Chapter_03_Thought_Bubble_Seth_Left[];
extern u8 Gfx_Chapter_03_Thought_Bubble_Seth_Right[];
extern u8 Gfx_Chapter_04_Thought_Bubble_Seth_Left[];
extern u8 Gfx_Chapter_04_Thought_Bubble_Seth_Right[];
extern u8 Gfx_Chapter_05_Thought_Bubble_Seth_Left[];
extern u8 Gfx_Chapter_05_Thought_Bubble_Seth_Right[];
extern u8 Gfx_Chapter_06_Thought_Bubble_Seth_Left[];
extern u8 Gfx_Chapter_06_Thought_Bubble_Seth_Right[];
extern u8 Gfx_Chapter_07_Thought_Bubble_Seth_Left[];
extern u8 Gfx_Chapter_07_Thought_Bubble_Seth_Right[];
extern u8 Gfx_Chapter_09_Thought_Bubble_Seth_Left[];
extern u8 Gfx_Chapter_09_Thought_Bubble_Seth_Right[];

typedef struct {
    u8 * const bubble[2];
} WorldMapThoughtBubbleEntryGraphics;

static const WorldMapThoughtBubbleEntryGraphics WorldMapThoughtBubbleEirika[] = {
    { .bubble = {}}, // Prologue
    { .bubble = {}}, // Chapter 1
    { .bubble = { Gfx_Chapter_02_Thought_Bubble_Eirika_Left, Gfx_Chapter_02_Thought_Bubble_Eirika_Right } },
    { .bubble = { Gfx_Chapter_03_Thought_Bubble_Eirika_Left, Gfx_Chapter_03_Thought_Bubble_Eirika_Right } },
    { .bubble = { Gfx_Chapter_04_Thought_Bubble_Eirika_Left, Gfx_Chapter_04_Thought_Bubble_Eirika_Right } },
    { .bubble = {}}, // Chapter 5x
    { .bubble = { Gfx_Chapter_05_Thought_Bubble_Eirika_Left, Gfx_Chapter_05_Thought_Bubble_Eirika_Right } },
    { .bubble = { Gfx_Chapter_06_Thought_Bubble_Eirika_Left, Gfx_Chapter_06_Thought_Bubble_Eirika_Right } },
    { .bubble = { Gfx_Chapter_07_Thought_Bubble_Eirika_Left, Gfx_Chapter_07_Thought_Bubble_Eirika_Right } },
    { .bubble = {}}, // Chapter 8
    { .bubble = { Gfx_Chapter_09_Thought_Bubble_Eirika_Left, Gfx_Chapter_09_Thought_Bubble_Eirika_Right } },
};

static const WorldMapThoughtBubbleEntryGraphics WorldMapThoughtBubbleSeth[] = {
    { .bubble = {}}, // Prologue
    { .bubble = {}}, // Chapter 1
    { .bubble = { Gfx_Chapter_02_Thought_Bubble_Seth_Left, Gfx_Chapter_02_Thought_Bubble_Seth_Right } },
    { .bubble = { Gfx_Chapter_03_Thought_Bubble_Seth_Left, Gfx_Chapter_03_Thought_Bubble_Seth_Right } },
    { .bubble = { Gfx_Chapter_04_Thought_Bubble_Seth_Left, Gfx_Chapter_04_Thought_Bubble_Seth_Right } },
    { .bubble = {}}, // Chapter 5x
    { .bubble = { Gfx_Chapter_05_Thought_Bubble_Seth_Left, Gfx_Chapter_05_Thought_Bubble_Seth_Right } },
    { .bubble = { Gfx_Chapter_06_Thought_Bubble_Seth_Left, Gfx_Chapter_06_Thought_Bubble_Seth_Right } },
    { .bubble = { Gfx_Chapter_07_Thought_Bubble_Seth_Left, Gfx_Chapter_07_Thought_Bubble_Seth_Right } },
    { .bubble = {}}, // Chapter 8
    { .bubble = { Gfx_Chapter_09_Thought_Bubble_Seth_Left, Gfx_Chapter_09_Thought_Bubble_Seth_Right } },
};