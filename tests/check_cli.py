"""Перевірки значень CLI; офіційний сценарій лежить у checks/."""
from pathlib import Path
import subprocess
import sys

binary = str(Path(sys.argv[1] if len(sys.argv) > 1 else "build/ember").resolve())

def run(commands):
    result = subprocess.run([binary], input=commands, text=True, capture_output=True, timeout=10)
    assert result.returncode == 0, result.stderr
    assert not result.stderr, result.stderr
    return result.stdout.replace("ember> ", "")

official = run(Path("checks/lab-01.txt").read_text())
for expected in ["65  0x41  0b01000001  'A'", "66  0x42", "|AB..............|",
                 "address 5000 is outside", "a byte is 0..255"]:
    assert expected in official, expected
zero = run("dump\nquit\n")
rows = [line for line in zero.splitlines() if "  |" in line]
assert len(rows) == 256
assert all(row.split("  |")[0].split()[1:] == ["00"] * 16 for row in rows)
edge = run("set 4095 255\ninc 4095\nget 4095\nset16 4094 0x1234\n"
           "get 4094\nget 4095\nset16 4095 65535\nget 4095\n"
           "get 4096\ninc -1\nset 0 -1\nset 0 256\nget xyz\nset16 0 65536\nquit\n")
for expected in ["0  0x00  0b00000000", "52  0x34", "18  0x12",
                 "two-byte address", "address 4096 is outside", "usage: get",
                 "a 16-bit value is 0..65535"]:
    assert expected in edge, expected
assert edge.count("18  0x12") == 2, "invalid set16 must not partially write"
for value in range(256):
    out = run(f"set 0 {value}\nget 0\nquit\n")
    assert f"0b{value:08b}" in out
print("PASS lab-01: official checks, 4096 zero bytes, boundaries, atomic set16, all 256 byte views")
