"""Власні перевірки Lab08: офіційного checks/lab-08.txt у джерелі немає."""
from pathlib import Path
import subprocess,sys,tempfile,re
binary=str(Path(sys.argv[1] if len(sys.argv)>1 else "build/ember").resolve())
def cli(*args,ok=True):
    p=subprocess.run([binary,*map(str,args)],text=True,capture_output=True,timeout=15)
    assert (p.returncode==0)==ok,(args,p.returncode,p.stdout,p.stderr)
    if ok:assert not p.stderr,p.stderr
    else:assert "Sanitizer" not in p.stderr,p.stderr
    return p
assert cli("programs/hello.asm").stdout.startswith("EMBER\n")
assert cli("programs/fib.asm").stdout.startswith("8 \n")
assert cli("programs/search.asm").stdout.startswith("3 \n")
screen=cli("programs/bounce.asm").stdout.splitlines()
screen=[s for s in screen if re.fullmatch("[.#]{64}",s)]
assert len(screen)==192
for i,x in enumerate([61,62,63,62,61,60]):
    frame=screen[i*32:(i+1)*32]
    assert frame[16]=="."*x+"#"+"."*(63-x) and sum(s.count("#") for s in frame)==1
with tempfile.TemporaryDirectory() as td:
    td=Path(td);f=td/"case.asm"
    for n,want in [(0,0),(1,1),(2,1),(6,8),(10,55)]:
        f.write_text(Path("programs/fib.asm").read_text().replace(".define N 6",f".define N {n}"))
        assert cli(f).stdout.startswith(f"{want} \n")
    f.write_text(Path("programs/search.asm").read_text().replace(".define TARGET 65",".define TARGET 66"))
    assert cli(f).stdout.startswith("255 \n")
    for source,line in [("LOADI A,0xGG",1),("\nLOADI A,0x",2),("HALT\n@",2),
                        ("LOADI A 12",1),("JMP absent",1),("LOADI A,12ab",1),
                        ("LOADI A,\x01",1),('LOADI A,"unfinished',1)]:
        f.write_text(source)
        result=cli(f,ok=False)
        assert f"line {line}:" in result.stderr,result
    for program in sorted(Path("programs").glob("*.asm")):
        values=bytes.fromhex(cli("asm",program).stdout)
        raw=td/"program.bin";raw.write_bytes(values)
        listing=cli("disasm",raw).stdout;f.write_text(listing)
        assert bytes.fromhex(cli("asm",f).stdout)==values
    f.write_text("JMP done\nNOP\ndone: HALT")
    assert cli("asm",f).stdout.strip()=="30 04 00 01 00"
    raw=td/"invalid.bin"
    for data in [b"\xff",b"\x20",b"\x30\x01"]:
        raw.write_bytes(data);assert "byte 0:" in cli("disasm",raw,ok=False).stderr
    f.write_text("HALT")
    p=subprocess.run([binary],input=f"set 2048 99\nset 0 80\nstep\nasm {f}\nregs\nstack\nget 2048\nquit\n",capture_output=True,text=True)
    assert p.returncode==0 and not p.stderr and "loaded 1 bytes" in p.stdout
    assert "PC=0 A=0" in p.stdout and "SP=4095 used=0" in p.stdout and "0  0x00" in p.stdout
assert "cannot open file" in cli("missing-file.asm",ok=False).stderr
tokens=cli("lex","programs/hello.asm").stdout
assert "Ident \"LOADH\"" in tokens and "Number \"0x800\"" in tokens and "Eof" in tokens
print("PASS lab-08: hello/search/fib0,1,2,6,10, six bouncing frames, line errors, all programs binary round trips, REPL reset")
