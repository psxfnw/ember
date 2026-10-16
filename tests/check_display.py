"""Додаткові перевірки VRAM, рядків, сортування та MMIO."""
exec(open("tests/check_machine.py").read().split("for n in")[0])
import re
def screens(out):return [line for line in out.splitlines() if re.fullmatch("[.#]{64}",line)]
out=run(Path("checks/lab-05.txt").read_text())
screen=screens(out)
assert len(screen)==32 and screen[0]=="#"+"."*63 and all(x=="."*64 for x in screen[1:]),out
assert "128  0x80" in out
out=run("plot 0 1\nget 2568\nrowmin 1\nrowmin 0\nplot 64 1\nquit\n")
assert "128  0x80" in out and "rowmin=0" in out and "rowmin=-1" in out and "usage: plot" in out
out=run(code([0x20,64,0x21,0,0x41,0])+"run\nregs\nget 2560\nquit\n")
assert "C=1" in out and "0  0x00" in out
out=run("set 2048 72\nset 2049 69\nset 2050 76\nset 2051 76\nset 2052 79\n"+code([4,0,8,0])+"run\nquit\n")
assert "HELLO" in out
out=run("".join(f"set {2048+i} 88\n" for i in range(257))+code([4,0,8,0])+"run\nquit\n")
assert out.count("X")==256
out=run("set 4095 65\n"+code([4,255,15,0])+"run\nquit\n")
assert "\nA\n" in out
values=[8,3,7,1,6,2,5,4]
setup="".join(f"set {2048+i} {v}\n" for i,v in enumerate(values))
out=run(setup+"sortshow 2048 2056\n"+"".join(f"get {2048+i}\n" for i in range(8))+"quit\n")
assert [int(v) for v in re.findall(r"^(\d+)  0x",out,re.M)]==list(range(1,9)),out
assert len(screens(out))>=64 and len(screens(out))%32==0
out=run(code([0x20,72,0x24,0,11,0x20,73,0x24,0,11,0])+"run\nget 2816\nquit\n")
assert "HI" in out and "73  0x49" in out
out=run("set 2816 88\n"+code([0x20,65,0x24,0,11,0x24,0,11,0])+"run\nquit\n")
assert "AA" in out and "X" not in out
out=run(code([1,0x22,1,11,0x03,0])+"run\nregs\nquit\n")
assert "\n2 " in out and "A=2 " in out,out
print("PASS lab-05: exact64x32/stride, bounds/C, OUTS256/end, sort frames, port side effects, timer including current instruction")
