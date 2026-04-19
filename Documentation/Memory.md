# Memory Space

This page documents the ROM and RAM layout used by the kernel, along with the fixed data locations that other tools and patches depend on.

## ROM Space

ROM space distribution is configured in [config-memmap.h](../include/configs/config-memmap.h).

<!-- markdownlint-disable MD060 -->
| Address     | Size      | Usage                                    |
|-------------|-----------|------------------------------------------|
| `0x00E8414` | `0x785A8` | Kernel text section (**in-BL range**)    |
| `0x09875AC` | `0x0F000` | Kernel data section (secure)             |
| `0x0B2A604` | `0xD59FC` | Common data section (non-secure)         |
| `0x0EFB2E0` | `0xE4D20` | Font                                     |
| `0x1000000` | ---       | ***Reserved for DEMO***                  |
<!-- markdownlint-enable MD060 -->

To improve performance, the kernel places its text section in **in-BL range** space starting at `0xE8414`, which differs from many older custom build setups.

Most other data is placed in free space starting at `0xB2A604`.

To support FEBuilderGBA integration and kernel-based demo projects, several locations are kept fixed.

### Magic Pattern

[A series of characters](../Preload/Magic.event) is set at the head of kernel free space (`0xB2A604`), which can be used as an identifier for [FEBuilder patch](../Patches/PATCH_SkillInfo.txt#L4).

### Pointer List

There is a pointer list after the magic pattern, starting at `0xB2A614` with size = `0x400`. Both wizard C-hacks and FEBuilder patches can find the data via this list, so you can expand the data via FEB.

### Text Table

The TextTable is repointed at the end of the data section, [0xBFBBF4](../include/configs/config-memmap.h#L64) (the tail of the non-secure data section) with [size = `0x2000 * sizeof(uintptr_t)`](../Preload/AllocMsgTable.event) free space allocated.

> [!NOTE]
> For now, we use the vanilla message data location to put the kernel `.text` section, so that the kernel can run at in-BL range space. In exchange, the vanilla TextTable and text contents have to be recompiled and redirected to the `.data` section of the kernel. You can use the config [`CONFIG_CROP_VANILLA_MSG`](../include/configs/configs.h#L18) to crop out vanilla story-related texts to save space (~470KB).

## Font Space

Free space at `0x0EFB2E0` is used to insert font data for further multi-language support, which is also a reserved space.

## RAM Space

RAM space distribution is configured in [config-memmap.s](../include/link/config-memmap.s).

<!-- markdownlint-disable MD060 -->
| Address      | Size     | Usage                  |
|--------------|----------|------------------------|
| `0x02026E30` | `0x2028` | Kernel                 |
| `0x0203F150` | `0x0C40` | ***Reserved for DEMO*** |
<!-- markdownlint-enable MD060 -->

Because the whole source tree is compiled together, CHAX can manage free RAM more deliberately than many older patch stacks. Previously identified vanilla free RAM can still be referenced through [StanH's DOC](https://github.com/StanHash/DOC/blob/master/FREE-RAM-SPACE.md), but this kernel mainly uses the vanilla debug print buffer at `0x02026E30` with size `0x2028`.

In the kernel, free-RAM space is allocated from the bottom to the top:

```assembly
0x02026E30, FreeRamSpaceTop

<--- gKernelUsedFreeRamSpaceTop
<------------
<!used kernel space>
0x02028E58, FreeRamSpaceBottom
```

Developers must ensure that used free RAM does not overflow. In addition, RAM allocations must always be even numbered to prevent allocations being misaligned. In practice, `(gKernelUsedFreeRamSpaceTop > FreeRamSpaceTop)` must remain true. The kernel also detects RAM overflow during [game init](../Kernel/Wizardry/Common/GameInitHook/source/GameInit.c#L14).

### Example

Here is a typical RAM allocation workflow.

Suppose you need a 4-byte allocation such as `u8 NewAlloc4Bytes[4]`:

1. Open [config-memmap.s](../include/link/config-memmap.s)
2. Insert new allocation:

```assembly
_kernel_malloc NewAlloc4Bytes, 4
```

> [!WARNING]
> Make sure the allocated space remains 32-bit aligned.

1. Declare the variable in your own C file: `extern u8 NewAlloc4Bytes[4];`

## Other Kernel Built-in RAM Space Usage

<!-- markdownlint-disable MD050 MD060 -->
| Part | Function Name        | Start        | End          | Max Size | Real Size |
|------|---------------------|--------------|--------------|----------|-----------|
| [a]  | ARM_MapFloodCoreRe  | `0x03003CAC` | `0x03003F94` | `0x2E8`  | `0x2E8`   |
| [a]  | ARM_MapTask         | `0x03003F94` | `0x03003FF0` | `0x05C`  | `0x05C`   |
| [a]  | ARM_SkillTester     | `0x03003FF0` | `0x03004150` | `0x138`  | `0x160`   |
| [b]  | ARM_UnitList        | `0x0300428C` | `0x0300438C` | `0x100`  | `0xEC`    |
| [b]  | ARM_SkillList       | `0x0300438C` | `0x030043B4` | `0x03C`  | `0x3C`    |
| [b]  | `__free__`          | ---          | `0x03004960` | `0x5A0`  | ---       |
<!-- markdownlint-enable MD050 MD060 -->
