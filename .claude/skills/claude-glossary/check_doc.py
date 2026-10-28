#!/usr/bin/env python3
"""Check Claude reference docs against the repo they describe.

Every repo path a doc mentions must exist, and every backticked identifier that looks
like a code symbol must appear in at least one tracked file outside the glossary docs.

usage (from the repo root):
    python3 check_doc.py [--exclude-dir DIR ...] DOC.md [DOC.md ...]

Names containing "example" (any case) or <placeholders> are template names and are skipped.
Paths are checked relative to the repo root, and markdown links relative to the doc.
Symbols are searched with `git grep`, so untracked copies such as worktrees are ignored.
"""
import argparse, os, re, subprocess, sys
from concurrent.futures import ThreadPoolExecutor

LINK_RE = re.compile(r"\]\(([^)#\s]+)(?:#[^)]*)?\)")
TICK_RE = re.compile(r"`([^`\n]+)`")
SYM_RE = re.compile(r"^[A-Za-z_][A-Za-z0-9_]{3,}$")
LINE_SUFFIX_RE = re.compile(r":\d+(?:-\d+)?(?:,\s*\d+(?:-\d+)?)*$")
# Only these tracked files are searched for symbols; large binary-heavy repos are slow otherwise.
TEXT_EXTS = [".c", ".h", ".cpp", ".hpp", ".s", ".S", ".asm", ".inc", ".event", ".txt", ".py",
             ".sh", ".md", ".mk", ".ld", ".json", ".toml", ".yaml", ".yml", ".rs", ".lua"]
SKIP = {"true", "false", "NULL", "void", "bool", "static", "const", "return", "struct",
        "enum", "include", "define", "ifdef", "ifndef", "endif", "make", "grep", "python3"}


def top_dirs():
    out = subprocess.run(["git", "ls-files"], capture_output=True, text=True).stdout
    return {p.split("/", 1)[0] for p in out.splitlines() if "/" in p}


_found_cache = {}


def symbol_found(sym, excludes):
    """True if sym occurs as a whole word in a tracked text file outside the excludes."""
    if sym not in _found_cache:
        cmd = ["git", "grep", "-q", "-w", "-I", "-F", "-e", sym, "--"]
        cmd += ["*" + x for x in TEXT_EXTS] + ["*makefile", "*Makefile"]
        cmd += [":!" + e.rstrip("/") for e in excludes]
        _found_cache[sym] = subprocess.run(cmd).returncode == 0
    return _found_cache[sym]


def symbols_found(syms, excludes):
    """Return the subset of syms that is found; runs the greps in parallel."""
    with ThreadPoolExecutor(max_workers=8) as pool:
        hits = pool.map(lambda s: (s, symbol_found(s, excludes)), syms)
    return {s for s, ok in hits if ok}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--exclude-dir", action="append", default=[],
                    help="glossary dirs whose own mentions do not count as evidence")
    ap.add_argument("docs", nargs="+")
    a = ap.parse_args()
    excludes = a.exclude_dir + [os.path.dirname(d) for d in a.docs if os.path.dirname(d)]
    roots = top_dirs()
    bad = 0
    for doc in a.docs:
        text = open(doc).read()
        docdir = os.path.dirname(doc)
        paths, syms = set(), set()
        for m in LINK_RE.finditer(text):
            if not m.group(1).startswith(("http", "mailto:")) and "<" not in m.group(1):
                paths.add(os.path.normpath(os.path.join(docdir, m.group(1))))
        for m in TICK_RE.finditer(text):
            raw = m.group(1).strip()
            tok = LINE_SUFFIX_RE.sub("", raw.rstrip(".,;"))
            if raw.endswith("...") or "example" in tok.lower() or any(c in tok for c in "<* "):
                continue
            if "/" in tok and tok.split("/", 1)[0] in roots:
                paths.add(tok.rstrip("/"))
            elif SYM_RE.match(tok.rstrip("()")) and tok.rstrip("()") not in SKIP:
                syms.add(tok.rstrip("()"))
        missing = sorted(p for p in paths if not os.path.exists(p))
        unfound = sorted(syms - symbols_found(sorted(syms), excludes))
        print(f"== {doc}: {len(paths)} paths, {len(syms)} symbols")
        for p in missing:
            print(f"  MISSING PATH: {p}")
        for s in unfound:
            print(f"  UNFOUND SYMBOL: {s}")
        bad += len(missing) + len(unfound)
    print("RESULT:", "OK" if bad == 0 else f"{bad} problems")
    sys.exit(1 if bad else 0)


if __name__ == "__main__":
    main()
