#!/usr/bin/env python3
"""Turn a stub story chapter into a chapter folder of its own (Documentation/Features/StoryStubs.md).

    python3 Tools/chapter/new_chapter.py 12 [--dry-run] [--force]

It writes CustomCampaign/Chapters/<NN>/events/:
  events.c    the chapter's event group, Chapter<NN>Event. It starts as a copy of the stub: the opening shows the
              placeholder line and moves the story on to the same next chapter, so the game plays as before. Its
              comments say what to change to write the chapter.
  units.c     an empty party list for the chapter's units.
  worldmap.c  the stub's world-map scripts (EventScrWM_Ch<N>_SET_NODE and _TRAVEL_TO_NODE), written out, unless the
              folder already has a worldmap.c (Chapter 10's).

and wires them in:
  - CustomCampaign/Chapters/Stubs/Stubs.event: the chapter's event slot line goes.
  - CustomCampaign/Event/Event_Installer.event: the slot points at Chapter<NN>Event, and events.lyn.event and
    worldmap.lyn.event are included.
  - CustomCampaign/Chapters/Stubs/events/events.c: the STUB_CHAPTER line goes.
  - CustomCampaign/Chapters/Stubs/events/worldmap.c: the STUB_SET_NODE line goes.

The script names stay the same, so EventScrWM_SET_NODE and EventScrWM_TRAVEL_TO_NODE (World_Map_Events.c) need no
change. Build with Tools/cloud/build.sh (or make): it builds the new .lyn.event files and rebuilds the stubs'. Then
`python3 Tools/cloud/smoketest.py --only stubs` should still pass.

--dry-run prints what would change. --force overwrites an existing events.c or units.c in the folder.
"""
import argparse
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
CHAPTERS = os.path.join(ROOT, "CustomCampaign", "Chapters")
STUBS_EVENT = os.path.join(CHAPTERS, "Stubs", "Stubs.event")
STUBS_EVENTS_C = os.path.join(CHAPTERS, "Stubs", "events", "events.c")
STUBS_WORLDMAP_C = os.path.join(CHAPTERS, "Stubs", "events", "worldmap.c")
INSTALLER = os.path.join(ROOT, "CustomCampaign", "Event", "Event_Installer.event")
STORY_STUBS_MD = os.path.join(ROOT, "Documentation", "Features", "StoryStubs.md")

STUBS_MARKER = "/* Chapter 10 and the chapters after it are stubs"

EVENTS_C = """#include "../../_shared/headers.h"
#include "../../_shared/empty-event-lists.h"
#include "units.c"

/*
 * Chapter {n}: {title} (made from its stub by Tools/chapter/new_chapter.py, Documentation/Features/StoryStubs.md).
 *
 * It still plays as the stub did: the opening shows the placeholder line and moves the story on. To write it:
 * - Put the party and the enemies in units.c, and load them in the opening (LOAD_WAIT) after the scene.
 * - Take TEXT(MSG_STORY_STUB_CHAPTER) and the {next_name} line out of the opening. The ending scene keeps it: the
 *   misc events' win condition (DEFEAT_ALL, a boss's death, a seize) starts the ending.
 * - The map, tileset and title card are the vanilla chapter's (its row in the chapter table). Add chapter.c and a map
 *   folder, as Chapter 9 has, to change them.
 * - The world map before the chapter is in worldmap.c.
 */

static const EventScr EventScr_Ch{n}_Opening[] = {{
    STAL(30)
    TEXT(MSG_STORY_STUB_CHAPTER)
    {next}
    ENDA
}};

static const EventScr EventScr_Ch{n}_Ending[] = {{
    {next}
    ENDA
}};

static const EventListScr EventListScr_Ch{n}_Misc[] = {{
    /* DEFEAT_ALL(EventScr_Ch{n}_Ending) */
    CAUSE_GAME_OVER_IF_LORD_DIES
    END_MAIN
}};

const struct ChapterEventGroup Chapter{n}Event = {{
    .turnBasedEvents               = EventListScr_Empty,
    .characterBasedEvents          = EventListScr_Empty,
    .locationBasedEvents           = EventListScr_Empty,
    .miscBasedEvents               = EventListScr_Ch{n}_Misc,
    .specialEventsWhenUnitSelected = EventListScr_Empty,
    .specialEventsWhenDestSelected = EventListScr_Empty,
    .specialEventsAfterUnitMoved   = EventListScr_Empty,
    .tutorialEvents                = EventListScr_Tutorial_None,

    .traps            = TrapData_None,
    .extraTrapsInHard = TrapData_None,

    .playerUnitsInNormal = Ch{n}_PlayerUnits,
    .playerUnitsInHard   = Ch{n}_PlayerUnits,

    .playerUnitsChoice1InEncounter = NULL,
    .playerUnitsChoice2InEncounter = NULL,
    .playerUnitsChoice3InEncounter = NULL,

    .enemyUnitsChoice1InEncounter = NULL,
    .enemyUnitsChoice2InEncounter = NULL,
    .enemyUnitsChoice3InEncounter = NULL,

    .beginningSceneEvents = EventScr_Ch{n}_Opening,
    .endingSceneEvents    = EventScr_Ch{n}_Ending,
}};
"""

UNITS_C = """/*
 * Chapter {n}'s units. A row: UNIT_ENTRY(character, class, faction, drop, autolevel, level, x, y, ai, REDAs,
 * ..., items), as in Chapters/09/events/units.c. The list ends with {{}}.
 */

static const struct UnitDefinition Ch{n}_PlayerUnits[] = {{
    {{}},
}};
"""

WORLDMAP_C = """#include "../../_shared/worldmap-include.h"

/*
 * The world map before Chapter {n} (written out from its stub, Stubs/events/worldmap.c): the lord on the last chapter's
 * node, this chapter's node shown, set as the destination and joined to it by a road (WM_REVEAL_DEST). The road is a
 * vanilla path, so the two nodes must be neighbours on the vanilla map. The destination is what makes arriving start
 * the chapter (Documentation/Features/WorldMapStoryChapters.md). EventScrWM_SET_NODE and EventScrWM_TRAVEL_TO_NODE
 * (CustomCampaign/Event/World_Map_Events.c) list these by the chapter's gmapEventId.
 */
const EventScr EventScrWM_Ch{n}_SET_NODE[] = {{
    WM_OPEN_MAP(CHARACTER_EIRIKA, {src})
    WM_SETUNITONNODE(WM_MU_0, {src})
    WM_REVEAL_DEST({dst})
    WM_CLOSE_SET_NODE()
}};

/* Travelling there. A scene can go here (see Chapters/09/events/worldmap.c); it must leave the lord on the node. */
const EventScr EventScrWM_Ch{n}_TRAVEL_TO_NODE[] = {{
    WM_SETUNITONNODE(WM_MU_0, {dst})
    STAL(20)
    ENDA
}};
"""


def read(path):
    with open(path) as f:
        return f.read()


def fail(msg):
    print("new_chapter: " + msg, file=sys.stderr)
    sys.exit(1)


def stub_title(n):
    """The vanilla title from StoryStubs.md's table, or ''."""
    m = re.search(r"^\| %d \| `0x[0-9A-Fa-f]+` \| ([^|]+?) \|" % int(n), read(STORY_STUBS_MD), re.M)
    return m.group(1) if m else ""


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("chapter", help="the story chapter's number, e.g. 12")
    ap.add_argument("--dry-run", action="store_true", help="print the changes, write nothing")
    ap.add_argument("--force", action="store_true", help="overwrite the folder's events.c and units.c")
    a = ap.parse_args()
    if not re.fullmatch(r"\d{1,2}", a.chapter):
        fail("the chapter is a number, e.g. 12")
    n = "%02d" % int(a.chapter)
    stub = "StubChapter" + n

    stubs_event, stubs_c, stubs_wm, installer = (read(p) for p in (STUBS_EVENT, STUBS_EVENTS_C, STUBS_WORLDMAP_C,
                                                                  INSTALLER))
    slot_re = re.compile(r"^NewEventPointerTable_Event\((0x[0-9A-Fa-f]+), %sEvent\)[^\n]*\n" % stub, re.M)
    m = slot_re.search(stubs_event)
    if not m:
        fail("Chapter %s is not a stub: no %sEvent line in %s" % (n, stub, os.path.relpath(STUBS_EVENT, ROOT)))
    slot = m.group(1)
    line_re = re.compile(r"^STUB_CHAPTER\(%s, (.+)\)[ \t]*\n" % stub, re.M)
    m = line_re.search(stubs_c)
    if not m:
        fail("no STUB_CHAPTER(%s, ...) line in %s" % (stub, os.path.relpath(STUBS_EVENTS_C, ROOT)))
    nxt = m.group(1)
    wm_re = re.compile(r"^STUB_SET_NODE\(Ch%d, *(\w+), *(\w+)\)[ \t]*\n" % int(n), re.M)
    wm = wm_re.search(stubs_wm)

    folder = os.path.join(CHAPTERS, n, "events")
    writes = []
    for name, text in (("events.c", EVENTS_C), ("units.c", UNITS_C)):
        path = os.path.join(folder, name)
        if os.path.exists(path) and not a.force:
            fail("%s exists (an older version of the chapter?): move it away, or pass --force to overwrite it" %
                 os.path.relpath(path, ROOT))
        writes.append((path, text.format(n=n, title=stub_title(n) or "(untitled)", next=nxt,
                                         next_name=nxt.split("(")[0])))
    wm_path = os.path.join(folder, "worldmap.c")
    if wm and not os.path.exists(wm_path):
        writes.append((wm_path, WORLDMAP_C.format(n=int(n), src=wm.group(1), dst=wm.group(2))))
    elif wm:
        fail("%s exists and Stubs/events/worldmap.c still has Ch%d's scripts: keep one of them" %
             (os.path.relpath(wm_path, ROOT), int(n)))

    edits = []
    edits.append((STUBS_EVENT, slot_re.sub("", stubs_event, 1)))
    edits.append((STUBS_EVENTS_C, line_re.sub("", stubs_c, 1)))
    if wm:
        edits.append((STUBS_WORLDMAP_C, wm_re.sub("", stubs_wm, 1)))
    if STUBS_MARKER not in installer:
        fail("can't find where to add the chapter in %s (the line starting %r)" %
             (os.path.relpath(INSTALLER, ROOT), STUBS_MARKER))
    group = 'NewEventPointerTable_Event(%s, Chapter%sEvent)\n#include "../Chapters/%s/events/events.lyn.event"\n' % (
        slot, n, n)
    new = installer.replace(STUBS_MARKER, group + "\n" + STUBS_MARKER, 1)
    wm_inc = '#include "../Chapters/%s/events/worldmap.lyn.event"' % n
    if wm and wm_inc not in new:
        last = list(re.finditer(r'^#include "\.\./Chapters/\w+/events/worldmap\.lyn\.event"\n', new, re.M))
        if not last:
            fail("no world-map includes in %s" % os.path.relpath(INSTALLER, ROOT))
        new = new[:last[-1].end()] + wm_inc + "\n" + new[last[-1].end():]
    edits.append((INSTALLER, new))

    for path, text in writes:
        print("write %s (%d lines)" % (os.path.relpath(path, ROOT), text.count("\n")))
    for path, text in edits:
        print("edit  %s" % os.path.relpath(path, ROOT))
    print("Chapter %s: event slot %s, then %s%s" % (n, slot, nxt,
                                                   ", world map %s -> %s" % wm.groups() if wm else ""))
    if a.dry_run:
        return
    os.makedirs(folder, exist_ok=True)
    for path, text in writes + edits:
        with open(path, "w") as f:
            f.write(text)
    print("Next: build (Tools/cloud/build.sh or make), run `python3 Tools/cloud/smoketest.py --only stubs`, and move "
          "Chapter %s's row out of StoryStubs.md's stub table." % int(n))


if __name__ == "__main__":
    main()
