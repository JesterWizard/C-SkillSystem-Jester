# Rescue Penalty

The Skl and Spd penalty for the unit that carries another unit.

## Vanilla

A unit carrying another (a rescue) has its Skl and Spd halved. The penalty is the same whatever the two units weigh.

## Con-based penalty

The designer-config toggle `rescue_penalty_by_con` (`Data/DesignerConfig/designer-config.c`) switches the penalty to depend on Con. The default is false, which is vanilla.

With the toggle on:

- pct = `rescue_penalty_pct` * carried Con / rescuer Con
- pct is clamped to [`rescue_penalty_min_pct`, `rescue_penalty_max_pct`]
- the rescuer's Skl and Spd become status * (100 - pct) / 100

At 50% this is exactly vanilla's halving, rounding included.

Con is `ConGetter` (`include/kernel/status-getter.h`), so Con skills such as Heavy Metal count.

Defaults: `rescue_penalty_pct` 50, `rescue_penalty_min_pct` 10, `rescue_penalty_max_pct` 75.

### Examples (defaults)

| Rescuer Con | Carried Con | pct | Skl 20 becomes |
|---|---|---|---|
| 10 | 10 | 50% | 10 (vanilla) |
| 15 | 10 | 33% | 13 |
| 8 | 7 | 43% | 11 |
| 20 | 5 | 12% | 17 |

The minimum (10%) applies only when the computed pct falls below it.

A carried unit heavier than the rescuer can happen with mounted Aid. Then pct rises toward the maximum, up to 75%.

## Where it applies

`RescuePenalty_Scale` (`Kernel/Wizardry/RescuePenalty/Source/RescuePenalty.c`, declared in `include/kernel/rescue-penalty.h`) replaces the two `status / 2` in `PairUp_RescueStatScale` (`Kernel/Wizardry/PairUp/Source/PairUp.c`).

- Before Pair Up is unlocked, it applies to the rescue.
- Once Pair Up is unlocked, it applies to a regular rescue. It does not apply to a pair-up.

Pair Up's own bonuses are not changed.

`PairUp_RescueStatScale` runs in the Skl and Spd getters. So the change shows everywhere those stats are used: the stat screen, battle, and the AI.

## Tuning

The three percentages are the knobs.

- A lower `rescue_penalty_pct` makes every rescue lighter. For example, 40 gives a lighter penalty than the default 50 at equal Con.
- `rescue_penalty_max_pct` 50 means the penalty is never worse than vanilla.

## Testing

Checked by hand in the emulator. Seth (Con 11) carries Eirika (Con 5).

- Toggle off: Skl 13 and Spd 12, halved (vanilla).
- Toggle on: 22% off, Skl 10 and Spd 9. This is before Seth's early-chapter handicap.

`Tools/cloud/smoketest.py` has no test for this yet.
