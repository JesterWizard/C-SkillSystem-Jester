# FE8 map tooling: LZ77 decode, plist lookup, metatile render.
# Reads fe8.gba and CustomCampaign/Event/PListBins/*.bin from the repo root (this file is Tools/district/tilegen/).
import os, struct
WT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
ROM=open(WT+"/fe8.gba","rb").read()
def lz77(d, off=0):
    assert d[off]==0x10, hex(d[off])
    size=d[off+1]|d[off+2]<<8|d[off+3]<<16; out=bytearray(); i=off+4
    while len(out)<size:
        flags=d[i]; i+=1
        for b in range(8):
            if len(out)>=size: break
            if flags&(0x80>>b):
                v=d[i]<<8|d[i+1]; i+=2; n=(v>>12)+3; disp=(v&0xFFF)+1
                for _ in range(n): out.append(out[-disp])
            else: out.append(d[i]); i+=1
    return bytes(out)
def binptr(name, idx):
    d=open(WT+"/CustomCampaign/Event/PListBins/"+name,"rb").read()
    return struct.unpack("<I",d[4*idx:4*idx+4])[0]
def rom_at(p): return p-0x08000000
def load_map(mapid=None, raw=None):
    d = lz77(raw) if raw is not None else lz77(ROM, rom_at(binptr("map-plist.bin",mapid)))
    w,h=d[0],d[1]; t=struct.unpack("<%dH"%(w*h), d[2:2+2*w*h])
    return w,h,[x>>2 for x in t]          # metatile indices
def tileset(obj, pal, tcfg):
    gfx=lz77(ROM, rom_at(binptr("obj-palette.bin",obj)))
    palp=rom_at(binptr("obj-palette.bin",pal))
    pals=[[ (lambda c:((c&31)*8,((c>>5)&31)*8,((c>>10)&31)*8))(struct.unpack("<H",ROM[palp+2*(16*p+i):palp+2*(16*p+i)+2])[0]) for i in range(16)] for p in range(10)]
    cfg=lz77(ROM, rom_at(binptr("tile-configs.bin",tcfg)))
    return gfx,pals,cfg
def render(w,h,tiles,ts,path,scale=1,grid=False):
    from PIL import Image, ImageDraw
    gfx,pals,cfg=ts
    im=Image.new("RGB",(w*16,h*16)); px=im.load()
    cache={}
    def meta(m):
        if m in cache: return cache[m]
        mi=Image.new("RGB",(16,16)); mp=mi.load()
        for q in range(4):
            e=struct.unpack("<H",cfg[8*m+2*q:8*m+2*q+2])[0]
            tno=e&0x3FF; hf=(e>>10)&1; vf=(e>>11)&1; pl=(e>>12)&15
            base=tno*32
            for y in range(8):
                for x in range(8):
                    sx=7-x if hf else x; sy=7-y if vf else y
                    if base+sy*4+sx//2 >= len(gfx): c=0
                    else:
                        b=gfx[base+sy*4+sx//2]; c=(b>>4) if sx&1 else b&15
                    col=pals[pl][c] if pl<len(pals) else (255,0,255)
                    mp[(q&1)*8+x,(q>>1)*8+y]=col
        cache[m]=mi; return mi
    for i,m in enumerate(tiles):
        im.paste(meta(m),((i%w)*16,(i//w)*16))
    if grid:
        dr=ImageDraw.Draw(im)
        for x in range(0,w*16,16*5): dr.line([(x,0),(x,h*16)],fill=(255,0,0))
        for y in range(0,h*16,16*5): dr.line([(0,y),(w*16,y)],fill=(255,0,0))
    if scale!=1: im=im.resize((w*16*scale,h*16*scale),Image.NEAREST)
    im.save(path)
def terrain(ts, m):
    cfg=ts[2]; return cfg[0x2000+m] if len(cfg)>0x2000+m else None
def render_grid(w,h,tiles,ts,path,scale=2):
    from PIL import Image, ImageDraw
    tmp=path+".tmp.png"; render(w,h,tiles,ts,tmp,scale)
    im=Image.open(tmp); os.remove(tmp); dr=ImageDraw.Draw(im); s=16*scale
    for x in range(w+1): dr.line([(x*s,0),(x*s,h*s)],fill=(255,0,0) if x%5==0 else (90,90,90))
    for y in range(h+1): dr.line([(0,y*s),(w*s,y*s)],fill=(255,0,0) if y%5==0 else (90,90,90))
    for x in range(0,w,5):
        for y in range(0,h,5): dr.text((x*s+2,y*s+1),"%d,%d"%(x,y),fill=(255,255,255))
    im.save(path)
