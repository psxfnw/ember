"""Власні перевірки Lab07 разом з офіційним checks/lab-07.txt."""
exec(open("tests/check_machine.py").read().split("for n in")[0])
import re
fast=[0x20,5,0x54,7,0,3,0,0x21,0,0x1A,0x31,27,0,0x21,1,0x1A,0x31,27,0,0x50,0x19,0x54,7,0,0x53,0x1B,0x55,0x20,1,0x55]
out=run(Path("checks/lab-07.txt").read_text())
assert "120 " in out and "steps=60" in out and "SP=4095" in out,out
for n,result,steps in [(0,1,9),(1,1,12),(6,208,72)]:
    data=fast.copy();data[1]=n
    out=run(code(data)+"run\nregs\nquit\n")
    assert f"A={result} " in out and f"steps={steps}" in out and "SP=4095" in out,out
    if n==6:assert "C=1" in out
for op in [0x51,0x53,0x55,0x57]:
    out=run(code([op])+"step\nregs\nquit\n")
    assert "stack underflow" in out and "SP=4095" in out,out
data=fast.copy();data[10:13]=[1]*3;data[16:19]=[1]*3
out=run(code(data)+"run\nregs\nquit\n")
assert "stack overflow" in out and "SP=3840" in out,out
out=run(Path("examples/fact-add.txt").read_text())
assert "A=120 " in out and "H=2304" in out and "SP=4095" in out,out
slow=Path("examples/fact-add.txt").read_text().replace("set 1 5\n","set 1 6\n")
out=run(slow)
assert "A=208 " in out and "C=1" in out,out
out=run("reg a 65\n"+code([0x54,4,0,0,2,0x55])+"step\nstack\nstep\nstep\nregs\nstep\nregs\nquit\n")
assert "SP=4093" in out and "0fff: 03" in out and "0ffe: 00" in out and "PC=3 A=65" in out and "SP=4095" in out,out
print("PASS lab-07: official factorial, 0/1/6, CALL byte order, RET/POP underflow, recursion overflow, ADD subroutine and H restoration")
