"""Власні перевірки Lab06: окремого офіційного lab-06.txt немає."""
exec(open("tests/check_machine.py").read().split("for n in")[0])
out=run("sprite 0 62 1 1 -1\nsprite 1 1 30 -1 1\ntick\nsprites\ntick\nsprites\nsprite 8 0 0 1 1\nquit\n")
for value in ["sprite 0 x=63 y=0 vx=-1 vy=1","sprite 1 x=0 y=31 vx=1 vy=-1",
              "sprite 0 x=62 y=1 vx=-1 vy=1","sprite 1 x=1 y=30 vx=1 vy=-1","usage: sprite"]:
    assert value in out,value
for speed in [-127,-50,0,50,127]:
    out=run(f"sprite 0 32 16 {speed} {speed}\ntick 10\nsprites\nquit\n")
    import re
    x,y=map(int,re.findall(r"sprite 0 x=(\d+) y=(\d+)",out)[0])
    assert 0<=x<64 and 0<=y<32
out=run(code([0x20,255,0x60,0x60,0x60,0x20,3,0x60,0x20,1,0x60,0])+"run\nregs\nquit\n")
assert "heap=3840" in out and "H=3837" in out and "C=1" in out,out
print("PASS lab-06: refactor regression, two sprites/both axes, large speeds, heap exact fit and failed ALLOC preserving H")
