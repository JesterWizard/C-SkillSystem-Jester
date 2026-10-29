#!/usr/bin/env python3
# Writes Source/TextAliasTable.h, the "[Name]" table TextAliases.c reads,
# from the aliases in textdefs.txt.  Run by make.
import re
import sys

src, out = sys.argv[1], sys.argv[2]
rows = []
for m in re.finditer(r'^\[([A-Za-z0-9_]+)\]\s*=\s*(.+)$', open(src).read(), re.M):
    name = m.group(1)
    code = [int(x, 16) for x in m.group(2).split(',')]
    if not 2 <= len(code) <= 4 or 0 in code:
        sys.exit('%s: %s must be 2 to 4 non-zero bytes' % (src, name))
    if len(code) > len(name) + 2:
        sys.exit('%s: [%s] is shorter than its bytes' % (src, name))
    rows.append('\t{ "%s", { %s } },' % (name, ', '.join('0x%02X' % b for b in code)))

with open(out, 'w') as f:
    f.write('/* Generated from textdefs.txt by Source/MakeAliasTable.py. Don\'t edit. */\n\n')
    f.write('static const struct TextAlias sTextAliases[] = {\n')
    f.write('\n'.join(rows) + '\n')
    f.write('\t{ NULL, { 0 } },\n};\n')
