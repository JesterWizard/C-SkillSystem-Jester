"""The EXP bar styles. Tools/expbar/expbar.py draws each into Kernel/Wizardry/ExpBarStyles/Gfx/<name>.png.

A style is a dict:
  name, desc   shown on the Options screen (also the PNG's name); Source/ExpBarStyles.c lists the styles in game
  pal          colour name -> RGB hex, at most 15 (colour 0, transparent, is added first)
  frame        the window: profile (border colours from the outside in), interior, corner (square, round, cut),
               radius, deco (dots, rivets, art)
  label        the "EXP" label in the top border: plate (x0, x1, y0, y1), fill, edge, cut, font (bold, slim), col,
               outline, shadow, x, y, pixels
  bar          legend (art letter -> colour name), full and empty art for the left cap (L), a body tile (B) and the
               right cap (R), 8 rows of 8 each ('.' shows the window), and edge (the column just after the fill)
  digits       col, low (lower half), low2 (bottom row), outline, shadow
The fill never reaches the left cap's column 0 or the right cap's columns 6 and 7: they must match full and empty.
"""


def rows(*r):
    return list(r)


def recolour(art, m):
    return [''.join(m.get(c, c) for c in row) for row in art]


# ------------------------------------------------------------------ 1 Blade
BLADE_B = rows("........", "oooooooo", "WWWWWWWW", "SSSSSSSS", "FFFFFFFF", "SSSSSSSS", "oooooooo", "........")
BLADE_L = rows("Kk......", "Kkoooooo", "KkWWWWWW", "KkSSSSSS", "KkFFFFFF", "KkSSSSSS", "Kkoooooo", "Kk......")
BLADE_R = rows("........", "oooo....", "WWWWoo..", "SSSSSSo.", "FFFFFFo.", "SSSSoo..", "oooo....", "........")
DULL = {'W': 'g', 'S': 'G', 'F': 'd'}
BLADE = dict(
    name='Blade', desc='A sword that charges up: hilt, blade and point',
    pal=dict(o='000000', iron='3a404c', ironl='6a7484', gold='e0b040', goldd='9a6a18', int='181c26', W='ffffff',
             S='9cd4ff', F='3c84e0', g='8c94a0', G='5a6270', d='343a46', Y='fff4a0'),
    frame=dict(profile=['o', 'ironl', 'iron', 'iron', 'gold'], interior='int',
               deco=[('rivets', (2, 'goldd', 'gold'))]),
    label=dict(plate=(1, 18, 1, 7), fill='gold', edge='goldd', font='bold', col='o', x=3, y=2),
    bar=dict(legend=dict(o='o', W='W', S='S', F='F', g='g', G='G', d='d', K='gold', k='goldd', Y='Y'),
             full=dict(L=BLADE_L, B=BLADE_B, R=BLADE_R),
             empty=dict(L=recolour(BLADE_L, DULL), B=recolour(BLADE_B, DULL), R=recolour(BLADE_R, DULL)),
             edge="..YYYY.."),
    digits=dict(col='W', outline='o', low='S', low2='F'),
)

# ------------------------------------------------------------------ 2 Gems
GEM_B = rows("........", "..ooo...", ".oWhRo..", "ohRRRdo.", "oRRRddok", ".oRddo..", "..ooo...", "........")
GEM_E = rows("........", "..ooo...", ".oxsso..", "oxsssso.", "ossssok", ".ossso..", "..ooo...", "........")
GEM_E[4] = "osssssok"
CLASP = ['.', 'k', 'G', 'G', 'G', 'G', 'k', '.']


def gem_l(b):
    return [CLASP[y] + b[y][:7] for y in range(8)]


def gem_r(b):
    return [b[y][:7] + CLASP[y] for y in range(8)]


GEMS = dict(
    name='Gems', desc='A string of rubies set in a gold jewel box',
    pal=dict(o='1a0a20', gold='f0c850', goldd='a07020', velvet='3c1440', velvetd='2a0c2e', R='e82850',
             Rh='ff9cb0', Rd='900c30', W='ffffff', s='1a0618', sh='5a2a60'),
    frame=dict(profile=['o', 'gold', 'goldd', 'velvetd'], interior='velvet', corner='round', radius=4),
    label=dict(plate=(1, 18, 1, 7), fill='gold', edge='goldd', cut=2, font='bold', col='velvetd', x=3, y=2),
    bar=dict(legend=dict(o='o', W='W', h='Rh', R='R', d='Rd', k='goldd', G='gold', x='sh', s='s'),
             full=dict(L=gem_l(GEM_B), B=GEM_B, R=gem_r(GEM_B)),
             empty=dict(L=gem_l(GEM_E), B=GEM_E, R=gem_r(GEM_E))),
    digits=dict(col='gold', outline='o', low='goldd'),
)

# ------------------------------------------------------------------ 3 Pearls
PEARL_B = rows("........", "..pPPp..", ".PWWPPp.", ".PWPPPq.", "sPPPPpqs", ".pPPpqq.", "..qqqq..", "........")
PEARL_E = rows("........", "..mmmm..", ".m....m.", ".m....m.", "sm....ms", ".m....m.", "..mmmm..", "........")
KNOT = ['.', '.', '.', 'k', 'k', 'k', '.', '.']
KNOT_R = ['..', '..', '..', '.k', 'sk', '.k', '..', '..']


def pearl_l(b):
    return [KNOT[y] + b[y][1:] for y in range(8)]


def pearl_r(b):
    return [b[y][1:7] + KNOT_R[y] for y in range(8)]


PEARLS = dict(
    name='Pearls', desc='Pearls on a thread that light up one by one',
    pal=dict(o='061418', teal='4a9aa0', n2='1a4a52', n='0c2c34', W='ffffff', P='f0ece4', p='c4bcd8',
             q='8a80a8', m='2e6670', s='a0d0d0', k='e0b860'),
    frame=dict(profile=['o', 'teal', 'n2'], interior='n', corner='round', radius=6),
    label=dict(plate=(1, 16, 1, 7), fill='n2', edge='teal', cut=2, font='slim', col='P', x=4, y=2),
    bar=dict(legend=dict(p='p', P='P', W='W', q='q', s='s', m='m', k='k'),
             full=dict(L=pearl_l(PEARL_B), B=PEARL_B, R=pearl_r(PEARL_B)),
             empty=dict(L=pearl_l(PEARL_E), B=PEARL_E, R=pearl_r(PEARL_E))),
    digits=dict(col='P', outline='o', low='p', low2='q'),
)

# ------------------------------------------------------------------ 4 Tech
TECH_B = rows("........", "........", "WWW.WWW.", "CCC.CCC.", "CCC.CCC.", "ddd.ddd.", "........", "........")
TECH_L = rows("........", "cc......", "c...WWW.", "c...CCC.", "c...CCC.", "c...ddd.", "cc......", "........")
TECH_R = rows("........", ".....cc.", "WWW...c.", "CCC...c.", "CCC...c.", "ddd...c.", ".....cc.", "........")
OFF = {'W': 'x', 'C': 'e', 'd': 'e'}
TECH = dict(
    name='Tech', desc='Sci-fi HUD: chamfered frame and LED segments',
    pal=dict(c='30e0ff', cd='16708a', n='08121e', W='e8ffff', C='2cc8ec', d='1888a8', x='1c5468', e='10303e'),
    frame=dict(profile=['c', 'n', 'cd'], interior='n', corner='cut', radius=5),
    label=dict(plate=(1, 16, 1, 7), fill='n', edge='c', cut=2, font='slim', col='W', x=4, y=2),
    bar=dict(legend=dict(c='c', W='W', C='C', d='d', x='x', e='e'),
             full=dict(L=TECH_L, B=TECH_B, R=TECH_R),
             empty=dict(L=recolour(TECH_L, OFF), B=recolour(TECH_B, OFF), R=recolour(TECH_R, OFF)),
             edge="..WWWW.."),
    digits=dict(col='W', low='C', low2='d'),
)

# ------------------------------------------------------------------ 5 Scroll
ROLL_L = (["__bhhb..", "__bHHb..", "_bbbbbb."] + ["_b1123b."] * 18 + ["_bbbbbb.", "__bHHb..", "__bhhb.."])
ROLL_R = (["..bhhb__", "..bHHb__", ".bbbbbb_"] + [".b1123b_"] * 18 + [".bbbbbb_", "..bHHb__", "..bhhb__"])
SCROLL_LEG = dict(b='b', h='h', H='H', **{'1': 'r1', '2': 'r2', '3': 'r3'})
RIB_B = rows("........", "........", "llllllll", "RRRRRRRR", "RRRRRRRR", "dddddddd", "........", "........")
RIB_L = rows("........", "........", ".lllllll", "..RRRRRR", "..RRRRRR", ".ddddddd", "........", "........")
RIB_R = rows("........", "........", "llllll..", "RRRRR...", "RRRRR...", "dddddd..", "........", "........")
PEN_B = rows("........", "........", "e.e.e.e.", "........", "........", ".e.e.e.e", "........", "........")
PEN_L = rows("........", "........", "..e.e.e.", "........", "........", ".e.e.e.e", "........", "........")
PEN_R = rows("........", "........", "e.e.e...", "........", "........", ".e.e.e..", "........", "........")
SCROLL = dict(
    name='Scroll', desc='A parchment scroll on wooden rollers, with a red ribbon',
    pal=dict(b='5a3418', p='f2e2b8', p2='e2cc98', h='9a6030', H='6a3a18', r1='fff6dc', r2='dcc490', r3='a88454',
             R='d02830', l='f06464', d='8a1018', e='b89868', k='3a2410'),
    frame=dict(profile=['b', 'p2'], interior='p',
               deco=[('art', (0, 0, ROLL_L, SCROLL_LEG)), ('art', (16, 0, ROLL_R, SCROLL_LEG))]),
    label=dict(font='bold', col='k', x=3, y=2, pixels=[(x, 7, 'r3') for x in range(2, 17)]),
    bar=dict(legend=dict(l='l', R='R', d='d', e='e'),
             full=dict(L=RIB_L, B=RIB_B, R=RIB_R), empty=dict(L=PEN_L, B=PEN_B, R=PEN_R)),
    digits=dict(col='k', low='k'),
)

# ------------------------------------------------------------------ 6 Vial
VIAL_B = rows("........", "gggggggg", "llllllll", "LLgLLLLL", "LLLLLgLL", "kkkkkkkk", "GGGGGGGG", "........")
VIAL_E = rows("........", "gggggggg", "wwwwwwww", "eeeeeeee", "eeeeeeee", "eeeeeeee", "GGGGGGGG", "........")
BOTTOM = ['.', '.', 'g', 'g', 'g', 'g', '.', '.']
RIM = ['.', 'g', 'g', 'g', 'g', 'g', 'g', '.']
CORK = ['..', '..', 'cc', 'cC', 'cC', 'CC', '..', '..']


def vial_l(b):
    return [BOTTOM[y] + b[y][1:] for y in range(8)]


def vial_r(b):
    return [b[y][:5] + RIM[y] + CORK[y] for y in range(8)]


VIAL = dict(
    name='Vial', desc='A corked potion vial filling with green liquid, on a wooden shelf',
    pal=dict(o='1a0e04', w1='8a5a2c', w2='6a4220', wi='3e2614', g='cce4f2', G='7090a8', L='4cc86a', l='b4ff94',
             k='1e7a3c', e='24323c', w='4e6676', pp='f2e6c6'),
    frame=dict(profile=['o', 'w1', 'w2', 'w2', 'o'], interior='wi', deco=[('rivets', (2, 'G', 'g'))]),
    label=dict(plate=(1, 15, 1, 7), fill='pp', edge='o', cut=1, font='slim', col='wi', x=3, y=2),
    bar=dict(legend=dict(g='g', G='G', L='L', l='l', k='k', e='e', w='w', c='w1', C='w2'),
             full=dict(L=vial_l(VIAL_B), B=VIAL_B, R=vial_r(VIAL_B)),
             empty=dict(L=vial_l(VIAL_E), B=VIAL_E, R=vial_r(VIAL_E)),
             edge="..llll.."),
    digits=dict(col='pp', outline='o', low='l', low2='L'),
)

# ------------------------------------------------------------------ 7 Chain
CHAIN_B = rows("........", "........", ".oooo...", "oHHHHooo", "oH..MoMM", "oMMMLooo", ".oooo...", "........")
IRON = {'H': 'h', 'M': 'm', 'L': 'l'}
PLATE_L = ['.', 'o', 'A', 'B', 'B', 'C', 'o', '.']
PLATE_R = ['..', 'oo', 'Ao', 'Bo', 'Bo', 'Co', 'oo', '..']


def chain_l(b):
    return [PLATE_L[y] + b[y][:6] + b[y][6] for y in range(8)]


def chain_r(b):
    return [b[y][:6] + PLATE_R[y] for y in range(8)]


CHAIN_E = recolour(CHAIN_B, IRON)
CHAIN = dict(
    name='Chain', desc='Iron links that turn to gold, on a bolted iron plate',
    pal=dict(o='000000', i1='8a9098', i2='5a6068', i3='3a3e46', ii='26282c', H='ffe080', M='e0a830', L='9a6a10',
             h='9aa0a8', m='626870', l='3c4148'),
    frame=dict(profile=['o', 'i1', 'i2', 'i2', 'i3'], interior='ii', deco=[('rivets', (2, 'i3', 'i1'))]),
    label=dict(plate=(1, 18, 1, 7), fill='i3', edge='o', font='bold', col='i1', shadow='o', x=3, y=2),
    bar=dict(legend=dict(o='o', H='H', M='M', L='L', h='h', m='m', l='l', A='i1', B='i2', C='i3'),
             full=dict(L=chain_l(CHAIN_B), B=CHAIN_B, R=chain_r(CHAIN_B)),
             empty=dict(L=chain_l(CHAIN_E), B=CHAIN_E, R=chain_r(CHAIN_E))),
    digits=dict(col='H', outline='o', low='M', low2='L'),
)

# ------------------------------------------------------------------ 8 Stars
STAR_B = rows("...W....", "...Y....", "..YYY...", "YYYWYYY.", ".YYYYY..", "..YYD...", ".YD.YD..", ".D...D..")
STAR_R = rows("........", "........", "..W.....", ".YYY....", "YYWYY...", ".YDY....", ".D.D....", "........")
UNLIT = {'W': 'w', 'Y': 'm', 'D': 'M'}


def star_l(b):
    return ['.' + b[y][:7] for y in range(8)]


STARS = dict(
    name='Stars', desc='A row of stars that light up across a night sky',
    pal=dict(o='060a18', Y2='d8b048', n3='2a3868', n2='1c2850', n='101838', sp='8a9ae0', W='ffffff', Y='f8e050',
             D='c09020', w='5a6aa0', m='34426e', M='26325a'),
    frame=dict(profile=['Y2', 'n3', 'n2'], interior='n', corner='round', radius=4,
               deco=[('dots', (1, 'sp', 5, 2))]),
    label=dict(font='bold', col='Y', outline='o', x=3, y=2, pixels=[(19, 2, 'W'), (21, 4, 'sp')]),
    bar=dict(legend=dict(W='W', Y='Y', D='D', w='w', m='m', M='M'),
             full=dict(L=star_l(STAR_B), B=STAR_B, R=STAR_R),
             empty=dict(L=star_l(recolour(STAR_B, UNLIT)), B=recolour(STAR_B, UNLIT), R=recolour(STAR_R, UNLIT))),
    digits=dict(col='Y', outline='o', low='D'),
)

# ------------------------------------------------------------------ 9 Arrow
ARROW_B = rows("........", "........", ".y...y..", "WWWWWWWW", "YYYYYYYY", "...y...y", "........", "........")
ARROW_E = rows("........", "........", "........", "bbbbbbbb", "BBBBBBBB", "........", "........", "........")
ARROW_L = rows("........", "rR.rR...", ".rR.rR..", "BWWWWWWW", "BYYYYYYY", ".rR.rR..", "rR.rR...", "........")
ARROW_R = rows("........", "..s.....", "..sSs...", "WWsSSSs.", "YYsSSSs.", "..sSs...", "..s.....", "........")
WOOD = {'W': 'b', 'Y': 'B'}
ARROW = dict(
    name='Arrow', desc='A fletched arrow that glows from feather to point, on stitched leather',
    pal=dict(o='0a1a0c', L1='5a8a3a', L2='3a6a28', f='183a20', st='d8e8a0', W='fff8d0', Y='f0c040', y='ffe890',
             b='6a5030', B='4a3820', R='c83828', r='f08060', s='7c8494', S='e0e8f0'),
    frame=dict(profile=['o', 'L1', 'L2', 'L2'], interior='f', corner='round', radius=3,
               deco=[('dots', (2, 'st', 3, 0))]),
    label=dict(plate=(1, 18, 1, 7), fill='L1', edge='o', cut=2, font='bold', col='st', x=3, y=2),
    bar=dict(legend=dict(W='W', Y='Y', y='y', b='b', B='B', R='R', r='r', s='s', S='S'),
             full=dict(L=ARROW_L, B=ARROW_B, R=ARROW_R),
             empty=dict(L=recolour(ARROW_L, WOOD), B=ARROW_E, R=recolour(ARROW_R, WOOD))),
    digits=dict(col='st', outline='o', low='Y'),
)

# ------------------------------------------------------------------ 10 Minimal
MIN_B = rows("........", "........", "oooooooo", "WWWWWWWW", "YYYYYYYY", "oooooooo", "........", "........")
MIN_L = rows("........", "........", ".ooooooo", "oWWWWWWW", "oYYYYYYY", ".ooooooo", "........", "........")
MIN_R = rows("........", "........", "oooooo..", "WWWWWWo.", "YYYYYYo.", "oooooo..", "........", "........")
GREY = {'W': 'g', 'Y': 'G'}
MINIMAL = dict(
    name='Minimal', desc='A borderless dark pill with a slim bar',
    pal=dict(o='000000', W='ffffff', Y='ffd040', g='5a626c', G='3a4048', n='1a1c22'),
    frame=dict(profile=[], interior='n', corner='round', radius=7),
    label=dict(font='slim', col='W', outline='o', x=2, y=2),
    bar=dict(legend=dict(o='o', W='W', Y='Y', g='g', G='G'),
             full=dict(L=MIN_L, B=MIN_B, R=MIN_R),
             empty=dict(L=recolour(MIN_L, GREY), B=recolour(MIN_B, GREY), R=recolour(MIN_R, GREY))),
    digits=dict(col='W', outline='o', low='Y'),
)

STYLES = [BLADE, GEMS, PEARLS, TECH, SCROLL, VIAL, CHAIN, STARS, ARROW, MINIMAL]
