---
name: operation-duration-tracking
description: "Use when the user wants elapsed time measured for an operation by capturing start and end timestamps, subtracting them, and reporting the result as HH:MM:SS."
---

# Operation Duration Tracking

Use this skill when you need to measure how long an operation takes and report the elapsed time in `HH:MM:SS`.

## Use This Skill When

- The user wants timing around a task you are about to perform.
- The user wants a duration estimate for a completed operation.
- The user wants the measurement returned in `HH:MM:SS` format.

## Workflow

1. Capture the start time with `date +%s` immediately before the operation begins.
2. Capture the end time with `date +%s` immediately after the operation finishes.
3. Subtract the start value from the end value to get elapsed seconds.
4. Format the elapsed seconds as `HH:MM:SS` and as a more human-readable string (e.g., "1 minute and 2 seconds").
5. Return the formatted duration to the user.

## Implementation Notes

- Use epoch seconds for subtraction so the result is stable across minute and hour boundaries.
- Do not subtract formatted clock strings directly.
- If the user wants the wall-clock timestamps too, report them separately from the elapsed duration.

## Reporting Format

- Start Time: `date +%s`
- End Time: `date +%s`
- Elapsed Time: `HH:MM:SS`

## Example

- Start Time: `1743350400`
- End Time: `1743350462`
- Elapsed Time (Format 1): `00:01:02`
- Elapsed Time (Format 2): `1 minute and 2 seconds`