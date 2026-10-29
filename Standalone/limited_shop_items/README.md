# Limited Shop Items

Shops can sell some items in limited quantities per chapter. Each chapter can have a stock list; a listed item can be bought only as many times as its stock during that chapter, from any armory, vendor or secret shop of the chapter. Items not listed are unlimited, and chapters without a list behave as vanilla.

The buy list gets a stock column. The item window is three tiles wider than vanilla to make room (the sell list uses the same wider window):

- a stocked item's name is gold, with its remaining stock at the right;
- a sold-out item is all gray, with stock `0`;
- choosing a sold-out item shows an "out of stock" line ("Sorry, we don't have that." / "You took the last one!" / "Hmm... We're out.") and returns to the list.

The remaining stock survives leaving and re-entering the shop and Suspend / Resume Chapter. It refills when the chapter map starts again (new chapter, Restart Chapter, or loading a save from the preparations screen).

This is the integrated C Skill System `limited_shop_items` designer-config flag (default `true` there), always on here.

## Configuration

Edit `Installer.event`, then reinstall the patch. No C recompile is needed.

### Stock lists

```
Ch2ShopStock:
StockEntry(SLIM_SWORD, 10)   // item ID, how many can be bought this chapter
StockEntry(SLIM_LANCE, 7)
StockEntry(IRON_LANCE, 1)
StockListEnd

SetChapterShopStock(2, Ch2ShopStock)   // chapter ID, list
```

- Up to 20 entries per list (extra entries are unlimited). Item IDs are FE8U item IDs (FEBuilder's item editor shows them).
- Keep new lists and `SetChapterShopStock` lines above the final `PROTECT` line of `Installer.event`.
- The installer ships the integrated example for Chapter 2 (Slim Sword 10, Slim Lance 7, Iron Lance 1). Delete it if you don't want it.

### Settings

| Constant | Default | Meaning |
|----------|---------|---------|
| `LIMITED_SHOP_TRAP_TYPE` | `0xF5` | Trap type used to store the stock (see below). Change it only if another patch uses custom trap type `0xF5` |
| `LIMITED_SHOP_OUT_OF_STOCK_TEXT` | `0` | `0`: built-in English lines. Otherwise the text ID of the armory line; the vendor line is ID+1 and the secret shop line ID+2, like vanilla shop texts. Edit those three text IDs in FEBuilder |
| `LIMITED_SHOP_HIDE_WEAPON_USES` | `0` | Set to `1` together with [infinite_durability](../infinite_durability/) so the shop lists also hide weapon uses |

## How the stock is saved

The save layout is unchanged and no RAM is reserved. The remaining stock is kept in the vanilla trap pool (`sTrapPool`, 64 entries at `0x0203A614`):

- The suspend save already writes and reads the whole trap pool, so the stock survives Suspend / Resume Chapter.
- Vanilla clears the pool when a chapter map starts (`StartBattleMap`, and event map changes), which is exactly when the stock should refill.
- Vanilla already keeps positionless state in this pool (enabled map changes are traps at `(0, 0)`), and every vanilla loop over the pool only acts on the trap types it knows.

Records use trap type `LIMITED_SHOP_TRAP_TYPE` at x = `0xFF` (off every map) and y = record number. Record 0 holds the chapter ID and the stock of entries 0-3; each further record holds 5 more entries. A 20-entry list uses 5 of the 64 trap slots, created the first time a shop of the chapter is opened. If the pool is too full (more than 63 traps), the chapter's items stay unlimited.

## Target ROM

- **FE8U (USA)** clean ROM

## Build

No build step is required to install the patch. `Source/LimitedShopItems.lyn.event` is checked in, so you can install `Installer.event` as-is.

Run `make` only if you edit `Source/LimitedShopItems.c`:

```bash
make -C Standalone/limited_shop_items
```

That requires [devkitARM](https://devkitpro.org/wiki/Getting_Started) and this repo's [FE-CLib](https://github.com/MokhaLeee/FE-CLib-Mokha) / [Event Assembler](https://github.com/MokhaLeee/EventAssembler/tree/mokha-fix) tools. See [Documentation/Setup.md](../../Documentation/Setup.md) for full setup.

## Installation

This folder is self-contained. It does not include `EAstdlib.event` or `Hack Installation.txt`. FEBuilder’s bundled Event Assembler is enough.

### FEBuilderGBA

1. Copy the `limited_shop_items` folder (or download this standalone package).
2. Open a clean FE8U ROM in FEBuilder.
3. Go to **Advanced Editors → Insert EA**.
4. Click **Select File**, choose `Installer.event`, then click **Load Script**.

### Event Assembler

From a copy of clean `fe8.gba`, with this folder as the working directory:

```bash
ColorzCore A FE8 -input:Installer.event -output:/path/to/fe8-limited-shop-items.gba
```

## Files

| File | Purpose |
|------|---------|
| `Installer.event` | EA installer: settings, addresses, hooks, free-space placement, stock lists |
| `Source/LimitedShopItems.c` | C implementation |
| `Source/LimitedShopItems.lyn.event` | Checked-in lyn output (rebuild with `make` after editing `.c`) |
| `makefile` | Compiles C to `.lyn.event` |

## Hooks & Free Space

| Site | `ORG` | Overwritten |
|------|-------|-------------|
| `ShopDrawBuyItemLine` (buy line redraw while scrolling) | `$B43BC` | 8 bytes (`jumpToHack`) |
| `ShopDrawSellItemLine` (sell line redraw while scrolling) | `$B4418` | 8 bytes (`jumpToHack`) |
| `Shop_Loop_BuyKeyHandler` (hand position, out-of-stock check) | `$B44A8` | 8 bytes (`jumpToHack`) |
| `DrawShopSoldItems` (buy list) | `$B4F90` | 8 bytes (`jumpToHack`) |
| `ShopInitTexts_OnSell` (clears the moved icon column) | `$B505C` | 8 bytes (`jumpToHack`) |
| `ShopDrawDefaultSellItemLine` (sell list) | `$B50C8` | 8 bytes (`jumpToHack`) |
| `HandleShopBuyAction` (spends one stock) | `$B5440` | 8 bytes (`jumpToHack`) |
| `Shop_Loop_SellKeyHandler`: hand x `56` → `24` | `$B4800` | 1 byte |
| `Shop_Loop_UnkKeyHandler` (view-only list): hand x `56` → `24` | `$B4AAE` | 1 byte |
| `Shop_Init`: item frame x `6` → `3` | `$B4CAA` | 1 byte |
| `Shop_Init`: item frame width `20` → `23` | `$B4CAE` | 1 byte |
| `Shop_Init`: window 0 left `56` → `32` (`mov r5, r8` → `movs r5, #32`) | `$B4D42` | 2 bytes |

The vanilla line drawers `DrawShopItemPriceLine` and `DrawShopItemLine` take 4 arguments, so `jumpToHack` (which clobbers r3) can't replace them. The patch replaces their four callers instead, which are their only callers.

- Free space: `$1000000` (about 0xE10 bytes: code, settings, the 0x80-entry chapter table and the stock lists). No RAM.
- `Installer.event` uses `PROTECT` on every patched site and on the free-space body.

## Conflicts

- Any patch that replaces the shop functions above or edits the shop window layout, including the full C Skill System.
- Any patch that uses trap type `0xF5` (change `LIMITED_SHOP_TRAP_TYPE`), or that assumes every trap in the pool is a map object.
- [infinite_durability](../infinite_durability/) hides weapon uses through `DrawItemMenuLine`, which these shop lists no longer call: set `LIMITED_SHOP_HIDE_WEAPON_USES` to `1` when both are installed.
- Free space at `$1000000` overlaps with other standalone patches from this repo. Install only one body at `$1000000`, or move this patch's `FE8U_StandaloneFreeSpace` to the next free region after any already-installed standalone code.

## Limitations

- Shops opened from the world map use the stock list of `gPlaySt.chapterIndex` at that moment. The game save (unlike the suspend save) doesn't store the trap pool, so stock bought there refills after saving and reloading, and when the next map starts.
- The out-of-stock line is drawn from built-in ASCII strings unless you set `LIMITED_SHOP_OUT_OF_STOCK_TEXT`.

### Differences from the integrated code

- Integrated `Shop_Init` reloads the full stock every time a shop opens, and its save/load helpers were never registered, so stock never lasted beyond one visit. This patch keeps it for the whole chapter and through suspend, which is what those helpers were written for.
- Integrated `LimitedShopStock_Installer.event` places the `DrawShopItemPriceLine` hook at `$B514` instead of `$B5164`. That overwrites 16 bytes of an unrelated vanilla function at `0x0800B514`, and scrolled buy lines were redrawn in the old layout. Here every buy line uses the new layout.
- The price stays gray when you can't afford the item, as in vanilla (the integrated code dropped that), and is gray when the item is sold out.
- Sell list colors are vanilla (white/gray by usability). The integrated sell list tinted items that happen to be on the stock list gold, or gray when sold out.
- With infinite durability on, the integrated code also hid the stock number of weapons; here only the uses are hidden.
- The view-only shop list (shop opened without a unit) moves its hand cursor to the wider window too.

## Credits

Extracted from [C Skill System](https://github.com/JesterWizard/C-SkillSystem-Jester).
