"""Record one captioned scene of a demo GIF with gbarun.

    import sys; sys.path.insert(0, '<repo>/.claude/skills/demo-gif')
    from scene import Scene, unit_items
    s = Scene(WORK, 's2', setup=unit_items(EIRIKA, [0x2E01]), start='ready.ss')
    s.caption('Eirika has 4 weapons.|The soldier drops an Iron Sword...')
    s.rec(40)                     # record 40 frames on the current caption
    s.key('A', 4, 200)            # press A (held 4 frames), record 200 frames
    s.caption('Slot 5 (outlined)...', box='slot5')   # draw box 'slot5' from now on
    s.save()                      # writes WORK/s2.txt and WORK/s2/caps.json

Then, from WORK:  gbarun ROM s2.txt
Frames land in WORK/<scene>/fNNNN.png, one every `step` game frames.
"""
import json
import os

ANIMS_OFF = 'w8 0x0202BD32 2'  # gPlaySt options: battle animations off (vanilla layout)


def unit_items(base, items):
    """w16 lines that set a unit's five item slots (struct Unit +0x1E). Item = id | uses << 8."""
    items = list(items) + [0] * (5 - len(items))
    return ['w16 %#x %#x' % (base + 0x1E + 2 * k, it) for k, it in enumerate(items)]


class Scene:
    def __init__(self, work, name, setup=(), start='ready.ss', prelude=(ANIMS_OFF,), step=2):
        self.work, self.name, self.step = work, name, step
        self.lines = ['load %s' % start] + list(prelude) + list(setup)
        self.caps, self.cap, self.box, self.n = [], '', None, 0
        os.makedirs(os.path.join(work, name), exist_ok=True)

    def caption(self, text, box=None):
        """Caption for the frames recorded from now on. '|' splits lines (two at most).
        box names a highlight box from the compose config, or None."""
        self.cap, self.box = text, box

    def rec(self, frames):
        for _ in range(frames // self.step):
            self.lines += ['wait %d' % self.step, 'shot %s/f%04d.png' % (self.name, self.n)]
            self.caps.append([self.cap, self.box])
            self.n += 1

    def wait(self, frames):
        """Run frames without recording (skips dead time)."""
        self.lines.append('wait %d' % frames)

    def raw(self, *lines):
        """Extra gbarun lines (w8/w16/w32 ...), not recorded."""
        self.lines += list(lines)

    def key(self, keys, hold=4, after=30, record=True):
        self.lines.append('press %s %d' % (keys, hold))
        if record:
            self.rec(after)
        else:
            self.wait(after)

    def save(self):
        with open(os.path.join(self.work, self.name + '.txt'), 'w') as f:
            f.write('\n'.join(self.lines) + '\n')
        with open(os.path.join(self.work, self.name, 'caps.json'), 'w') as f:
            json.dump(self.caps, f)
