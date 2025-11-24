struct DefeatTalkEntNew {
             u16 pidA;
             u16 pidB;
    /* 02 */ u8 route;
    /* 03 */ u8 chapter;
    /* 04 */ u16 flag;
    /* 06 */ u16 msg;
    /* 08 */ EventScr * event;
};

extern struct DefeatTalkEntNew* GetDefeatTalkEntry_NEW(u16);

/* START - Page 7 - Unit promotions list */
typedef struct
{
    int classId;     // Promotion class
    int skills[3];   // Up to 3 skills for this promotion
} PromotionEntry;

typedef struct
{
    int key;                       // Unit ID
    PromotionEntry promotions[3];  // Up to 3 promotions
} UnitPromotions;

#define UNIT_PROMOS(unit_id, ...) \
    { .key = (unit_id), .promotions = { __VA_ARGS__ } }
#define PROMO(class_id, ...) \
    { .classId = (class_id), .skills = { __VA_ARGS__ } }

extern const UnitPromotions unit_promotions[];
/* END - Page 7 - Unit promotions list */

/* Global flags use */
enum {
    GLOBAL_FLAG_BASE_CHAPTER_INTRO_SKIP = 0xEC,
};