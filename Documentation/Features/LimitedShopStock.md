# Limited Shop Stock

<p align="center">
  <img src="https://media.giphy.com/media/l2Je66zG6mAAZxgqI/giphy.gif" />
</p>

---

## Overview

This hack allows you to **limit how many of each item a player may purchase**.

These limits:

- Apply **globally to every shop in a chapter**
- Are **set per chapter**
- Persist via SRAM
- Visually display remaining stock in shops

---

## Core Logic

### File: `LimitedShopStock.c`

Handles:

- Stock initialization
- SRAM save/load
- Shop UI drawing
- Purchase restriction logic
- Color coding for availability

---

## Stock System

| Function | Purpose |
|----------|--------|
| `InitShopStock()` | Loads chapter stock |
| `GetItemStock()` | Returns remaining stock |
| `IsItemInStock()` | Determines if item is purchasable |
| `ReduceItemStock()` | Deducts stock after purchase |

---

## UI Behavior

When `limited_shop_items` is enabled:

| Stock Status | Name Color | Stat Color |
|-------------|------------|------------|
| In Stock    | Gold       | Blue       |
| Out of Stock| Gray       | Gray       |
| Unlimited   | Default    | Default    |

Stock count is shown in the shop list.

---

## Purchase Handling

Shop buying is modified to:

- Block purchase when stock = 0
- Display Out of Stock text
- Deduct stock after purchase
- Save updated stock

---

## Shop Initialization Hook

`Shop_Init()` now:

- Loads chapter stock
- Adjusts UI layout
- Enables stock display window

---

## Save Support

| Function | Purpose |
|----------|--------|
| `SU_SaveShopStock()` | Writes stock to SRAM |
| `SU_LoadShopStock()` | Loads stock from SRAM |

---

## Integration Points

| Hook | Purpose |
|------|--------|
| `DrawShopItemPriceLine` | Displays stock |
| `Shop_Loop_BuyKeyHandler` | Blocks purchase if empty |
| `HandleShopBuyAction` | Reduces stock |
| `Shop_Init` | Loads chapter limits |

---

## Chapter Configuration

To assign stock to a chapter do this in ``LimitedShopStock_Installer.event``:

```
ALIGN 4
Ch2ShopStock:
StockEntry(SlimSword, 10)
StockEntry(SlimLance, 7)
StockEntry(IronLance, 1)
StockListEnd

SetChapterShopStock(2, Ch2ShopStock)
```

---

## Limitations

- Maximum of **20 stocked items per chapter**
- Items not listed behave normally
- Stock is shared across all shops in a chapter

---