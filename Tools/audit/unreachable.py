#!/usr/bin/env python3
"""Lists content that is built into the ROM but that nothing in the game reaches.

    python3 Tools/audit/unreachable.py [--all]

Snek was a whole minigame with texts, records and two guild quests, but no event started it. This looks for the same
pattern, in the sources (comments stripped, so a commented-out call does not count):

  asmc      C functions named *ASMC (event entry points) that no event, table or other C code names
  messages  messages from the hack's text files (## NAME) that no source file names
  items     items given a row in Data/ItemSys/Source/Items.c whose ITEM_ constant nothing else names

A name can still be reached in ways a text search cannot see (a computed message id, a table built by a script, an
item word assembled at run time), so each hit is a lead to check, not a verdict. Known false leads are listed in
KNOWN (with the reason) and left out unless --all is given. Build first: messages come from Contents/Texts/build/msgs.h.
"""
import os
import re
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SKIP_DIRS = {".git", ".claude", "build", "Tools", "Standalone"}
SOURCE_EXT = (".c", ".h", ".event", ".s", ".asm", ".txt")

# name: why it is not a real gap
KNOWN = {
    "ChangeControlledUnitASMC": "free movement API for events that switch the controlled unit; none does yet",
    "StartCreditsProc_ASMC": "the credits roll: the campaign has no ending yet (Chapter 10 is a stub)",
    "CC_DEFEAT_ALL_BANDITS": "chapter goal text for a chapter not built yet",
    "CC_DEFEAT_ONEILL": "chapter goal text for a chapter not built yet",
}

# (regex, why): groups of names that are not real gaps
KNOWN_PATTERNS = [
    (r"^Epilogue_", "paired endings: the campaign has no ending yet"),
    (r"^MSG_HEROES_CARDS_", "biography pages; most are empty placeholders, the written ones are in CharacterBiographies.c"),
    (r"_Description_(Two|Three)$", "later description stages, empty placeholders for characters without them"),
    (r"^Chapter_10_", "Chapter 10 is a stub: its event group is commented out"),
    (r"^[ABC]_\w+_\w+$", "support conversations of units not recruitable in the campaign so far"),
    (r"^MSG_COMART_", "an older copy of the combat art names; CombatArtInfo.c uses MSG_SKILL_COMBAT_*"),
    (r"^AFFINITY_", "affinity descriptions of an affinity screen that is not installed"),
    (r"^Puzzle", "Lights Out puzzle texts named from its asm"),
    (r"^ITEM_UNK_", "vanilla's unused item slots"),
    (r"^ITEM_(BLACKGEM|GOLDGEM|JUNAFRUIT|DELPHISHIELD)$", "vanilla items the campaign has not placed yet"),
    (r"^MSG_FISHING_COL_", "column labels of the first fishing journal layout"),
    (r"^MSG_ITEM_\w+_STAFF_SUBTITLE$", "staff subtitles of a menu that is not installed"),
]


def known(name):
    if name in KNOWN:
        return KNOWN[name]
    for pat, why in KNOWN_PATTERNS:
        if re.search(pat, name):
            return why
    return None


def sources(exts=SOURCE_EXT, skip_files=()):
    for d, dirs, files in os.walk(ROOT):
        dirs[:] = [x for x in dirs if x not in SKIP_DIRS]
        for f in files:
            if f.endswith(exts) and not f.endswith(".lyn.event"):
                p = os.path.join(d, f)
                if p not in skip_files:
                    yield p


def strip_comments(text, path):
    if path.endswith((".c", ".h", ".event", ".s", ".asm")):
        text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
        text = re.sub(r"//[^\n]*", " ", text)
    return text


def corpus(skip_files=()):
    """Every source's text, comments stripped, by path."""
    out = {}
    for p in sources(skip_files=skip_files):
        try:
            out[p] = strip_comments(open(p, encoding="utf-8", errors="replace").read(), p)
        except OSError:
            pass
    return out


_TOKENS = {}


def tokens(texts, exclude=()):
    """How often each identifier occurs in texts (minus the excluded files); built once per exclude set."""
    key = frozenset(exclude)
    if key not in _TOKENS:
        counts = {}
        for p, t in texts.items():
            if p in exclude:
                continue
            for w in re.findall(r"[A-Za-z_]\w*", t):
                counts[w] = counts.get(w, 0) + 1
        _TOKENS[key] = counts
    return _TOKENS[key]


def count_refs(name, texts, exclude=()):
    return tokens(texts, exclude).get(name, 0)


def audit_asmc(texts):
    defs = {}
    for p, t in texts.items():
        if p.endswith(".c"):
            for m in re.finditer(r"^\s*(?:static\s+)?void\s+(\w*ASMC\w*)\s*\(", t, re.M):
                defs.setdefault(m.group(1), p)
    out = []
    for name, p in sorted(defs.items()):
        # the definition itself and its prototype do not count
        refs = count_refs(name, texts) - 1
        protos = sum(len(re.findall(r"\b%s\s*\([^;{)]*\)\s*;" % re.escape(name), t))
                     for t in texts.values() if name in t)
        if refs - protos <= 0:
            out.append((name, os.path.relpath(p, ROOT)))
    return out


def text_names():
    """## NAME messages of the hack's own text files (not vanilla's)."""
    names = {}
    roots = [os.path.join(ROOT, "CustomCampaign", "Text"), os.path.join(ROOT, "CustomCampaign", "Chapters"),
             os.path.join(ROOT, "Contents", "Texts", "Source", "texts")]
    for r in roots:
        for d, dirs, files in os.walk(r):
            for f in files:
                if f.endswith(".txt") and not f.startswith("_vanilla"):
                    p = os.path.join(d, f)
                    for m in re.finditer(r"^##\s+(\w+)", open(p, encoding="utf-8", errors="replace").read(), re.M):
                        names.setdefault(m.group(1), p)
    return names


def audit_messages(texts):
    names = text_names()
    txt_files = {p for p in texts if p.endswith(".txt")}
    out = []
    for name, p in sorted(names.items()):
        if count_refs(name, texts, exclude=txt_files) == 0:
            out.append((name, os.path.relpath(p, ROOT)))
    return out


def audit_items(texts):
    items_c = os.path.join(ROOT, "Data", "ItemSys", "Source", "Items.c")
    rows = re.findall(r"^\s*\[(ITEM_\w+)\]\s*=", open(items_c).read(), re.M)
    headers = {p for p in texts if p.endswith(".h")}
    out = []
    for name in sorted(set(rows)):
        if count_refs(name, texts, exclude={items_c} | headers) == 0:
            out.append((name, os.path.relpath(items_c, ROOT)))
    return out


def main():
    show_all = "--all" in sys.argv
    texts = corpus()
    total = 0
    for title, hits in (("ASMC entry points nothing calls", audit_asmc(texts)),
                        ("Messages nothing names", audit_messages(texts)),
                        ("Items nothing gives, sells or names", audit_items(texts))):
        shown = [h for h in hits if show_all or not known(h[0])]
        print("== %s: %d (%d known)" % (title, len(shown), len(hits) - len([h for h in hits if not known(h[0])])))
        for name, where in shown:
            note = "  (known: %s)" % known(name) if known(name) else ""
            print("  %-48s %s%s" % (name, where, note))
        total += len(shown)
    return 0 if total == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
