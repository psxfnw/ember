"""Додаткові сценарії процесора, не офіційні checks викладача."""
from pathlib import Path
import subprocess, sys
binary=str(Path(sys.argv[1] if len(sys.argv)>1 else "build/ember").resolve())
def run(commands):
    p=subprocess.run([binary],input=commands,text=True,capture_output=True,timeout=10)
    assert p.returncode==0 and not p.stderr,p.stderr
    return p.stdout.replace("ember> ","")
def code(values):
    return "".join(f"set {i} {v}\n" for i,v in enumerate(values))
for n in [1,2,3]:
    out=run(Path(f"checks/lab-{n:02d}.txt").read_text())
    if n==2:assert "PC=2 A=44" in out and "C=1" in out and "halted\n" in out
    if n==3:assert "AB" in out and "PC=9" in out and "H=2049" in out and "steps=7" in out
out=run(code([0x20,65,0x24,0,8,0x21,66,0x25,1,8,0x22,0,8,0x23,1,8,0x26,0x03,0])+"run\nregs\nget 2048\nget 2049\nquit\n")
assert "66 " in out and "65  0x41" in out and "66  0x42" in out,out
out=run("set 4095 0x20\nreg pc 4095\nstep\nregs\nquit\n")
assert "truncated instruction" in out and "PC=4095" in out,out
out=run(code([0x28,0,16,0x29])+"run\nregs\nquit\n")
assert "address outside memory" in out and "H=4096" in out,out
out=run("set16 4094 65535\nget16 4094\nset16 4095 0\nget16 4094\nquit\n")
assert out.count("65535")==2,out
print("PASS lab-03: checks 01-03, transfers, sizes, H, truncated operand, invalid address, 16-bit boundary")
