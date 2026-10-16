"""Додаткові перевірки стрибків і обмеженого пошуку."""
exec(open("tests/check_machine.py").read().split("for n in")[0])
out=run(Path("checks/lab-04.txt").read_text())
assert "3 2 1 " in out and "steps=11" in out and "PC=8 A=0" in out and "Z=1" in out,out
for op in [0x33,0x34,0x31,0x32]:
    for a,b in [(1,2),(2,1),(2,2)]:
        take={0x33:a<b,0x34:a>=b,0x31:a==b,0x32:a!=b}[op]
        out=run(code([0x20,a,0x21,b,0x1a,op,11,0,0x20,77,0,0x20,88,0])+"run\nregs\nquit\n")
        assert f"A={88 if take else 77} " in out,out
out=run(code([0x20,0,0x31,7,0,0x20,9,0])+"run\nregs\nquit\n")
assert "A=9 " in out,"LOADI must preserve Z"
for unknown in [0x05,0x1c,0x2e,0x35,0xff]:
    out=run(code([unknown])+"step\nregs\nquit\n")
    assert f"unknown opcode 0x{unknown:02x}" in out and "PC=0 " in out,out
out=run(code([0x30,0,0])+"run 7\nregs\nquit\n")
assert "steps=7" in out and "step limit reached (7)" in out and "PC=0 " in out
out=run(code([0x30,0,16])+"step\nregs\nquit\n")
assert "address outside memory" in out and "PC=0 " in out
out=run(Path("checks/lab-04.txt").read_text().split("run")[0]+"b 3\nrun\nregs\nstep\nb off\nrun\nregs\nquit\n")
assert "breakpoint at 3" in out and "PC=3 A=3" in out and "PC=8 A=0" in out,out
search=[0x28,0,8,0x21,65,0x29,0x1a,0x31,21,0,0x2b,0x2d,0x21,8,0x1a,0x31,24,0,0x30,3,0,0x2d,3,0,0x20,255,3,0]
for hit in range(-1,8):
    data=[1]*8
    if hit>=0:data[hit]=65
    cmd="".join(f"set {2048+i} {x}\n" for i,x in enumerate(data))+"set 2056 65\n"+code(search)
    out=run(cmd+"find 2048 2056 65\nrun\nregs\nquit\n")
    if hit<0:assert "not found" in out and "255 " in out and "H=2056" in out,out
    else:assert f"found {2048+hit} index={hit}" in out and f"\n{hit} \n" in out,out
assert "not found" in run("find 4096 4096 1\nquit\n")
assert "usage:" in run("find 5 4 1\nquit\n")
print("PASS lab-04: countdown11, all branch conditions, stale flags, unknown opcodes, bounded run, breakpoint, search all8/miss")
