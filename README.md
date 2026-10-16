# ember — віртуальний комп'ютер на C++17

Навчальний проєкт курсу «Основи програмування»: від пам'яті на 4096 байтів до процесора й асемблера.
Версія **1.0.0 — Lab 08, Advanced**. Один проєкт послідовно розвивається в тегах
lab-01 … lab-08: пам'ять → ALU → адресація → цикли → екран → структури →
виклики → власна мова. Нижче є самодостатній опис фінальної версії та
перевірені результати кожного етапу.

[Lab 01](#lab-01--коробка-байтів) · [Фінальна мова й архітектура](#lab-08--мова-для-ember) ·
[Повний перелік перевірок](CHECKS.md)

## Збірка й запуск

Перевірено в Ubuntu 26.04 / WSL2, x86_64, GCC 15.2.0, CMake 4.2.3.
На Windows відкрийте проєкт через VS Code WSL; команди виконуйте в Ubuntu.

~~~bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/ember programs/fib.asm
~~~

C++17, -Wall -Wextra -Werror; у Debug увімкнені -fsanitize=address,undefined, -fno-omit-frame-pointer і -g.
Попередження та санітайзери не вимкнені. Без аргументів ./build/ember відкриває REPL;
файл .asm в аргументі збирається та виконується. Для перевірки: ctest --test-dir build --output-on-failure.

~~~text
ember 1.0 - 4096 bytes of memory you can see. Type `help`.
~~~
| Команда | Дія |
|---|---|
| dump | усі 4096 байтів, по 16 на рядок, hex та ASCII |
| get addr | байт десятково, hex, binary та символом |
| set addr value | запис 0–255 |
| inc addr | збільшення за модулем 256 |
| set16 addr value | запис 0–65535 молодшим байтом уперед |
| help / quit | допомога та завершення |

Адреси байта — 0–4095, двобайтового запису — 0–4094. Числа десяткові або з префіксом 0x.
Некоректний set16 не змінює навіть перший байт.

## Lab 01 — Коробка байтів

Використано [офіційний starter](https://github.com/rmalkevy/Programming-Practice-Projects/tree/main/courses/programming-fundamentals/starter).
Memory містить Byte data[4096]{}. Byte — std::uint8_t, один 8-бітний байт у цьому середовищі.
Ширина int не задає ширину клітинки VM.

### Розміри типів — виміряні Notes §1

~~~text
1 4 4 8 8
1 2
~~~
| Тип | Байти |
|---|---:|
| char / uint8_t | 1 |
| uint16_t | 2 |
| int / float | 4 |
| double / void* | 8 |

### M2–M3: початкові нулі й представлення

Перші три рядки свіжого дампа:

~~~text
0000  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
0010  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
0020  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
~~~
Фрагмент фактичного офіційного сценарію checks/lab-01.txt:

~~~text
65  0x41  0b01000001  'A'
66  0x42  0b01000010  'B'
0  0x00  0b00000000  '.'
address 5000 is outside 0..4095
a byte is 0..255, got 300
0000  41 42 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |AB..............|
~~~
ASCII 0x20–0x7e показується символом, решта — крапкою.

### M4: чотири досліди

**1. Загортання.** Команди set 0 255, get 0, inc 0, get 0:

~~~text
ember 0.1 - 4096 bytes of memory you can see. Type `help`.
255  0xff  0b11111111  '.'
0  0x00  0b00000000  '.'
~~~
Після додавання у ширшому цілому перетворення до uint8_t лишає молодші 8 бітів: 256 mod 256 = 0.
Знаковий int такої гарантії не має. Окремий фрагмент Notes §2, зібраний з ASan+UBSan:

~~~text
0
-2147483648
/home/psxfnw/pf-private/ember/experiments/lab-01/overflow.cpp:4:166: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
~~~
Цей навмисно неправильний фрагмент не входить до готового застосунку.

**2. Дроби (Notes §3).**

~~~text
0.3
false
0.30000000000000004
~~~
0.1 і 0.2 не мають скінченного двійкового представлення. Звичайний друк округлює до 0.3,
точніше форматування показує похибку; точне порівняння з 0.3 дає false. Адреси й байти зберігаємо цілими.

**3. C-рядок.** Після set 0 65, set 1 66, set 2 0:

~~~text
0000  41 42 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |AB..............|
~~~
Байти 41 42 00 — A, B та завершувач C-рядка \0. Нуль — не цифра '0' з кодом 48.

**4. Пам'ять без {}.** В ізольованій копії прибрано {} з масиву, копію реально перезібрано
з ASan+UBSan і виконано dump. Початок:

~~~text
0000  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
0010  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
0020  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |................|
~~~
Перші байти цього запуску були нульовими. Це не гарантує ініціалізацію:
читання невизначених значень у C++17 некоректне, а ASan/UBSan не є універсальним детектором таких читань.
Робоча версія має {}; експериментальна копія не входить до проєкту.

Notes §4: число, символ і ділення:

~~~text
65 65 65 A
B
2 2.5
~~~
### Advanced: set16 і порядок байтів

set16 0 0x1234; get 0; get 1:

~~~text
52  0x34  0b00110100  '4'
18  0x12  0b00010010  '.'
~~~
Дамп:

~~~text
0000  34 12 00 00 00 00 00 00 00 00 00 00 00 00 00 00  |4...............|
~~~
Молодший байт 0x34 за меншою адресою, 0x12 — за наступною. Маска value & 0xff і зсув value >> 8
задають порядок VM явно, незалежно від порядку хоста.
.clang-format — LLVM, відступ 4, межа 100; застосовано до всіх cpp/hpp.

### Перевірки

~~~bash
./build/ember < checks/lab-01.txt
python3 tests/check_cli.py build/ember
clang-format --dry-run --Werror src/*.cpp src/*.hpp
~~~

checks/lab-01.txt — офіційний сценарій; tests/check_cli.py — додаткові перевірки.
Фактичний результат:

~~~text
PASS lab-01: official checks, 4096 zero bytes, boundaries, atomic set16, all 256 byte views
~~~
Позитивні сценарії виконані в Debug з ASan+UBSan без діагностики. Окремий навчальний overflow вище очікувано дає UBSan.

## Структура

- src/memory.* — пам'ять і перевірки меж.
- src/dump.* — hex/ASCII та чотири представлення.
- src/main.cpp — наданий диспетчер, доповнений inc і set16.
- checks/, tests/ — сценарій викладача та додаткові перевірки.

[Завдання Lab 01](https://github.com/rmalkevy/Programming-Practice-Projects/blob/main/courses/programming-fundamentals/lab-01-a-box-of-bytes.md).
[ISA машини](https://github.com/rmalkevy/Programming-Practice-Projects/blob/main/courses/programming-fundamentals/ISA.uk.md).

## Lab 02 — Біти не брешуть

CPU, Flags і константи OP взяті з наданого заголовка завдання. Додані alu.cpp/hpp, cpu.cpp/hpp,
команди reg a/b, regs та step. CPU вказує на ту саму Memory, яку змінює set.
PC містить адресу наступної інструкції, усі реалізовані тут опкоди займають один байт.
Після HALT PC уже посунутий; наступні step відмовляють без змін.

### Підтримувані інструкції

| Опкод | Мнемоніка | Байти | Прапорці |
|---|---|---:|---|
| 00 | HALT | 1 | — |
| 01 | NOP | 1 | — |
| 10 | ADD A,B | 1 | Z,N,C |
| 11 | SUB A,B | 1 | Z,N,C; C=позика |
| 12 / 13 / 14 | AND / OR / XOR A,B | 1 | Z,N; C=0 |
| 15 | NOT A | 1 | Z,N; C=0 |
| 16 / 17 | SHL / SHR A | 1 | Z,N,C=біт за краєм |
| 18 / 19 | INC / DEC A | 1 | Z,N; C збережений |

Інші інструкції ще не реалізовані; як дозволено в Lab 02, невідомий байт поки проходить як NOP.
У наступних етапах це стане діагностикою.

### Побітовий ADD і прапорці

Справжній alu_add проходить біти 0–7: s = a_bit XOR b_bit XOR carry.
Наступне перенесення — хоча б дві одиниці серед трьох входів.
Результат збирається OR з 1u << bit. Звичайне додавання лишене тільки в Debug assert як незалежна перевірка.
Z означає результат 0, N — встановлений біт 7. N не є прапорцем signed overflow.

200 + 100 = 300 = 256 + 44: A=44, Z=0, N=0, C=1.
Другий приклад: 3−5 = −2, у байті 254 (0xfe); Z=0, N=1, C=1, бо була позика.

~~~text
halted
PASS: 65536 operand pairs, all unary values, preserved carry, CPU/HALT/bounds
ADD 200 100: result=44 Z=0 N=0 C=1
SHL 128: result=0 Z=1 N=0 C=1
AND 7 1: result=1 Z=0 N=0 C=0
~~~

~~~text
ember 0.2 - 4096 bytes of memory you can see. Type `help`.
PC=1 A=254 B=5 Z=0 N=1 C=1 halted=0
~~~
### Трасування

Перед запуском reg a 64; у пам'яті 16 16 18 00 (SHL, SHL, INC, HALT).
Фактичний вивід regs та get 2818:

~~~text
ember 0.2 - 4096 bytes of memory you can see. Type `help`.
PC=0 A=64 B=0 Z=0 N=0 C=0 halted=0
PC=1 A=128 B=0 Z=0 N=1 C=0 halted=0
PC=2 A=0 B=0 Z=1 N=0 C=1 halted=0
PC=3 A=1 B=0 Z=0 N=0 C=1 halted=0
4  0x04  0b00000100  '.'
PC=4 A=1 B=0 Z=0 N=0 C=1 halted=1
~~~
| PC до | Опкод | PC після | A | Z | N | C |
|---:|---|---:|---:|---:|---:|---:|
| 0 | 16 SHL | 1 | 128 | 0 | 1 | 0 |
| 1 | 16 SHL | 2 | 0 | 1 | 0 | 1 |
| 2 | 18 INC | 3 | 1 | 0 | 0 | 1 |
| 3 | 00 HALT | 4 | 1 | 0 | 0 | 1 |

Друга SHL викинула старший біт; INC змінив Z/N, але зберіг C.

### Розбір і розширення

decode використовує group=(op >> 4) & 0x0f та index=op & 0x0f.
Група керує зовнішнім switch, всередині вибирається конкретна дія.

| Адреса розширення | Розмір | Біти | Поведінка |
|---|---:|---|---|
| 0xB02 FLAG_STATUS | 1 | bit0=Z, bit1=N, bit2=C | після кожного успішного step дзеркалить прапорці |

Це розширення пам'яті, нових опкодів немає. Адреса 0xB02 вибрана, щоб не займати
необов'язкові PORT_OUT=0xB00 і TICKS=0xB01 з ISA. Ручний set у дзеркало не змінює Flags;
наступний step відновлює дзеркало. Гостьове LOAD для читання з'явиться в Lab 03.

### Notes і пріоритет операцій

Реальні досліди §§1–4: hex/біти, доповняльний код та integer promotion, три поля з одного байта,
явне порівняння неправильного й правильного групування:

~~~text
a3
10100011
-5 -6 250
11 2 1
0 1
~~~
Оригінальна помилка x & 1 == 0 з -Werror не компілюється; попередження не вимикалось:

~~~text
/home/psxfnw/pf-private/ember/experiments/lab-02/precedence.cpp: In function ‘int main()’:
/home/psxfnw/pf-private/ember/experiments/lab-02/precedence.cpp:2:40: error: suggest parentheses around comparison in operand of ‘&’ [-Werror=parentheses]
    2 | int main(){int x=2;std::cout << (x & 1 == 0);}
      |                                      ~~^~~~
cc1plus: all warnings being treated as errors
~~~
Для запуску обох значень неправильне групування записано явно як x & (1==0).
Виправлення для перевірки парності — (x & 1)==0. Перевірка побітового додавання Notes §5
використовує той самий справжній ALU, результати наведені вище.

### Перевірки Lab 02

~~~bash
cmake --build build
ctest --test-dir build --output-on-failure
./build/ember < checks/lab-02.txt
python3 tests/check_cli.py build/ember
~~~

~~~text
Test project /home/psxfnw/pf/ember/build
    Start 1: alu-and-cpu
1/1 Test #1: alu-and-cpu ......................   Passed    0.02 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   0.02 sec
~~~

~~~text
ember 0.2 - 4096 bytes of memory you can see. Type `help`.
PC=0 A=200 B=100 Z=0 N=0 C=0 halted=0
PC=1 A=44 B=100 Z=0 N=0 C=1 halted=0
halted
PC=2 A=44 B=100 Z=0 N=0 C=1 halted=1
~~~
Додаткова C++-перевірка охопила всі 65536 пар для ADD/SUB/AND/OR/XOR, всі байти унарних операцій, обидва початкові C для INC/DEC, зупинку HALT та межі. Регресія Lab 01 пройшла. Debug ASan+UBSan без діагностики у готовому коді.

## Lab 03 — Адреси, а не імена

Додано get16/set16 у memory.cpp за наданою формою get16 з перевіркою всього двобайтового діапазону.
Перевірка addr >= MEM_SIZE−1 стоїть до addr+1, тому навіть size_t max не загортається.
Гостьовий порядок — little-endian; set16 лишився сумісним із Lab 01.

### Нові інструкції ISA

| Опкод | Мнемоніка | Байти | Прапорці |
|---|---|---:|---|
| 02 / 03 | OUT / OUTN | 1 | — |
| 20 / 21 | LOADI A / B, imm8 | 2 | — |
| 22 / 23 | LOAD A / B, [addr16] | 3 | — |
| 24 / 25 | STORE [addr16], A / B | 3 | — |
| 26 / 27 | MOV A,B / MOV B,A | 1 | — |
| 28 | LOADH H, imm16 | 3 | — |
| 29 / 2A | LOAD A,[H] / STORE [H],A | 1 | — |
| 2B / 2C | INCH / DECH | 1 | — |

Нові команди: run, get16 addr, dumpcpu; reg дозволяє h/pc (16-бітні), a/b (8-бітні).
run має стелю 100000 кроків і друкує кількість виконаних кроків.
instruction_size задає розмір, step перевіряє всі операнди ДО читання.
Адреса>=4096 зупиняє CPU повідомленням. H — 16-бітний регістр,
INCH/DECH виконують арифметику modulo65536; звертатися поза пам'яттю це не дозволяє.
Жодна інструкція пересилання не змінює прапорці.

### Розділені код і дані

Абсолютна програма 22 00 08 02 00: LOAD A,[0x800]; OUT; HALT. Дані — ABC і нуль:

~~~text
A
steps=3
PC=5 A=65 B=0 H=0 Z=0 N=0 C=0 halted=1
0000  22 00 08 02 00 00 00 00 00 00 00 00 00 00 00 00  |"...............|
0800  41 42 43 00 00 00 00 00 00 00 00 00 00 00 00 00  |ABC.............|
~~~
Офіційна програма через H:

~~~text
ember 0.3 - 4096 bytes of memory you can see. Type `help`.
AB
steps=7
PC=9 A=66 B=0 H=2049 Z=0 N=0 C=0 halted=1
~~~
Покрокове трасування:

~~~text
ember 0.3 - 4096 bytes of memory you can see. Type `help`.
PC=0 A=0 B=0 H=0 Z=0 N=0 C=0 halted=0
PC=3 A=0 B=0 H=2048 Z=0 N=0 C=0 halted=0
PC=4 A=65 B=0 H=2048 Z=0 N=0 C=0 halted=0
APC=5 A=65 B=0 H=2048 Z=0 N=0 C=0 halted=0
PC=6 A=65 B=0 H=2049 Z=0 N=0 C=0 halted=0
PC=7 A=66 B=0 H=2049 Z=0 N=0 C=0 halted=0
BPC=8 A=66 B=0 H=2049 Z=0 N=0 C=0 halted=0
PC=9 A=66 B=0 H=2049 Z=0 N=0 C=0 halted=1
~~~
LOADH займає 3 байти: PC0→3. LOAD A,[H] дає 65; INCH змінює H2048→2049;
друге читання дає 66. HALT завершується на PC9.

### Хост і гість

CPU.mem — Memory* хоста, справжня адреса коробки у процесі C++.
PC/H — uint16_t, адреси гостя. H=0x800 означає mem.data[2048], а не адресу 0x800 ОС.
Збереження вказівника хоста в гостьовому регістрі обрізає адресу й прив'язує програму до запуску.
Byte*+1 — наступний байт, int*+1 — наступний int.
Notes §§1–4 і визначений перегляд float через memcpy:

~~~text
65 65
66
10 20 30
1 4
8 4 8
1
2
3f800000
~~~
Рядки:65/65 і 66 через *p; масив 10/20/30; крок 1 елемент і 4 байти; sizeof8/4/8;
після копії 1, після вказівника 2; біти 1.0f — 3f800000 у цьому середовищі.

### M4: ASan і виправлення

Окрема програма записала за індексом 4096 у масив на 4096 байтів.
Збірка C++17, -Wall -Wextra -Werror -fsanitize=address,undefined -g. Фактичний звіт:

~~~text
/home/psxfnw/pf-private/ember/experiments/lab-03/overflow.cpp:2:129: runtime error: index 4096 out of bounds for type 'unsigned char [4096]'
/home/psxfnw/pf-private/ember/experiments/lab-03/overflow.cpp:2:130: runtime error: store to address 0x7aac10ff1040 with insufficient space for an object of type 'unsigned char'
0x7aac10ff1040: note: pointer points here
 00 00 00 00  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00  00 00 00 00
              ^
=================================================================
==5448==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7aac10ff1040 at pc 0x60717a00b517 bp 0x7fff07051460 sp 0x7fff07051450
WRITE of size 1 at 0x7aac10ff1040 thread T0
    #0 0x60717a00b516 in main /home/psxfnw/pf-private/ember/experiments/lab-03/overflow.cpp:2
    #1 0x7eac12a2a600  (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a600) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #2 0x7eac12a2a717 in __libc_start_main (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a717) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #3 0x60717a00b204 in _start (/home/psxfnw/pf-private/ember/experiments/lab-03/overflow+0x1204) (BuildId: c83b50d52a9a323655d9a8fb77116e2c3c036c33)

Address 0x7aac10ff1040 is located in stack of thread T0 at offset 4160 in frame
    #0 0x60717a00b2d8 in main /home/psxfnw/pf-private/ember/experiments/lab-03/overflow.cpp:2

  This frame has 2 object(s):
    [32, 40) 'index' (line 2)
    [64, 4160) 'data' (line 2) <== Memory access at offset 4160 overflows this variable
HINT: this may be a false positive if your program uses some custom stack unwind mechanism, swapcontext or vfork
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: stack-buffer-overflow /home/psxfnw/pf-private/ember/experiments/lab-03/overflow.cpp:2 in main
Shadow bytes around the buggy address:
  0x7aac10ff0d80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7aac10ff0e00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7aac10ff0e80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7aac10ff0f00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7aac10ff0f80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x7aac10ff1000: 00 00 00 00 00 00 00 00[f3]f3 f3 f3 f3 f3 f3 f3
  0x7aac10ff1080: f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3
  0x7aac10ff1100: f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3
  0x7aac10ff1180: f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3
  0x7aac10ff1200: f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3
  0x7aac10ff1280: f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3 f3
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==5448==ABORTING
~~~
Ключове: stack-buffer-overflow, WRITE of size1, main/рядок експерименту.
У готовому ember перевіряються PC, повний операнд та адреса перед доступом.
Навмисний баг не входить до проєкту.

### Advanced: довільний дамп та копіювання

dump(const void*,size_t) читає представлення через const unsigned char* і друкує лише n байтів,
включно з неповним останнім рядком. dumpcpu викликає dump(&cpu,sizeof(cpu)).
Байтовий перегляд дозволений; це не читання float через int*. Вказівник і padding залежать від запуску:

~~~text
ember 0.3 - 4096 bytes of memory you can see. Type `help`.
0000  e0 05 1f d2 d3 75 00 00 00 00 00 00 00 00 00 00  |.....u..........|
0010  00 00 00 00 00 00 00 00                          |........|
~~~
programs/lab-03-copy.txt копіює ABCD із 0x800–0x803 у 0x810–0x813.
Використані H-адресація, LOADH/LOAD/STORE, MOV та INCH, без прямого адресного LOAD/STORE.
H ставиться на джерело, байт зберігається в B, H — на призначення, MOV повертає байт у A, STORE записує.
Чотири повторення розгорнуті, стрибки будуть у Lab04.

~~~bash
./build/ember < programs/lab-03-copy.txt
~~~

Кожен рядок таблиці отриманий з фактичного regs після step:
| PC до | Опкод | Дія | PC після | A | B | H після |
|---:|---|---|---:|---:|---:|---:|
| 0 | 28 | LOADH src | 3 | 0 | 0 | 2048 |
| 3 | 29 | LOAD A,[H] | 4 | 65 | 0 | 2048 |
| 4 | 27 | MOV B,A | 5 | 65 | 65 | 2048 |
| 5 | 28 | LOADH dst | 8 | 65 | 65 | 2064 |
| 8 | 26 | MOV A,B | 9 | 65 | 65 | 2064 |
| 9 | 2a | STORE [H],A | 10 | 65 | 65 | 2064 |
| 10 | 2b | INCH | 11 | 65 | 65 | 2065 |
| 11 | 28 | LOADH src | 14 | 65 | 65 | 2049 |
| 14 | 29 | LOAD A,[H] | 15 | 66 | 65 | 2049 |
| 15 | 27 | MOV B,A | 16 | 66 | 66 | 2049 |
| 16 | 28 | LOADH dst | 19 | 66 | 66 | 2065 |
| 19 | 26 | MOV A,B | 20 | 66 | 66 | 2065 |
| 20 | 2a | STORE [H],A | 21 | 66 | 66 | 2065 |
| 21 | 2b | INCH | 22 | 66 | 66 | 2066 |
| 22 | 28 | LOADH src | 25 | 66 | 66 | 2050 |
| 25 | 29 | LOAD A,[H] | 26 | 67 | 66 | 2050 |
| 26 | 27 | MOV B,A | 27 | 67 | 67 | 2050 |
| 27 | 28 | LOADH dst | 30 | 67 | 67 | 2066 |
| 30 | 26 | MOV A,B | 31 | 67 | 67 | 2066 |
| 31 | 2a | STORE [H],A | 32 | 67 | 67 | 2066 |
| 32 | 2b | INCH | 33 | 67 | 67 | 2067 |
| 33 | 28 | LOADH src | 36 | 67 | 67 | 2051 |
| 36 | 29 | LOAD A,[H] | 37 | 68 | 67 | 2051 |
| 37 | 27 | MOV B,A | 38 | 68 | 68 | 2051 |
| 38 | 28 | LOADH dst | 41 | 68 | 68 | 2067 |
| 41 | 26 | MOV A,B | 42 | 68 | 68 | 2067 |
| 42 | 2a | STORE [H],A | 43 | 68 | 68 | 2067 |
| 43 | 2b | INCH | 44 | 68 | 68 | 2068 |
| 44 | 00 | HALT | 45 | 68 | 68 | 2068 |

Байти призначення:

~~~text
PC=45 A=68 B=68 H=2068 Z=0 N=0 C=0 halted=1
65  0x41  0b01000001  'A'
66  0x42  0b01000010  'B'
67  0x43  0b01000011  'C'
68  0x44  0b01000100  'D'
~~~
### Перевірки

~~~bash
ctest --test-dir build --output-on-failure
python3 tests/check_machine.py build/ember
python3 tests/check_cli.py build/ember
~~~

~~~text
PASS lab-03: checks 01-03, transfers, sizes, H, truncated operand, invalid address, 16-bit boundary
~~~
C++-тест звіряє всі 65536 значень get16/set16 на останній парі, відмову на size_t max та незмінність Z/N/C при пересиланнях. Регресія Lab01–02 пройшла; позитивні сценарії без діагностики санітайзерів.

## Lab 04 — Форма керування

З цього етапу кожен невідомий опкод зупиняє CPU: повідомлення unknown opcode 0x..,
faulted=true, PC залишається на помилковій інструкції. Тихі NOP зі старого етапу прибрано.
Кожен відомий опкод має case, decode з маскою/зсувом збережено.

| Опкод | Мнемоніка | Байти | Дія |
|---|---|---:|---|
| 1A | CMP A,B | 1 | Z/N/C як A−B, A не змінюється |
| 2D | HLOW | 1 | A=H & 0xff, прапорці незмінні |
| 30 | JMP addr16 | 3 | завжди |
| 31 / 32 | JZ / JNZ addr16 | 3 | Z=1 / Z=0 |
| 33 / 34 | JC / JNC addr16 | 3 | C=1 / C=0 |

Це надані опкоди ISA, власних нових опкодів немає.
Спочатку next_pc=pc+size; взятий стрибок замінює next_pc на операнд.
У кінці PC присвоюється ОДИН раз, без додавання 3 після стрибка.
Ціль взятого стрибка перевіряється на межі пам'яті.

run [limit]: за замовчуванням 100000, допустимий явний ліміт 1...10000000.
При досягненні ліміту run повертає prompt, CPU не пошкоджується, можна продовжити.
Фактичний захист від JMP0:

~~~text
ember 0.4 - 4096 bytes of memory you can see. Type `help`.

steps=100000
step limit reached (100000)
PC=0 A=0 B=0 H=0 Z=0 N=0 C=0 halted=0
~~~
### Зворотний відлік

| Адреса | Байти | Інструкція |
|---|---|---|
| 0000 | 20 03 | LOADI A,3 |
| 0002 | 03 | loop: OUTN |
| 0003 | 19 | DEC A |
| 0004 | 32 02 00 | JNZ loop |
| 0007 | 00 | HALT |

Рівноцінний для початкового A=3 цикл:
~~~cpp
unsigned a = 3;
while (a > 0) {
    std::cout << a << ' ';
    --a;
}
~~~
Фактичний запуск:

~~~text
ember 0.4 - 4096 bytes of memory you can see. Type `help`.
3 2 1
steps=11
PC=8 A=0 B=0 H=0 Z=1 N=0 C=0 halted=1
~~~
| PC до | A до | Z до | PC після | A після | Z після |
|---:|---:|---:|---:|---:|---:|
| 0 | 0 | 0 | 2 | 3 | 0 |
| 2 | 3 | 0 | 3 | 3 | 0 |
| 3 | 3 | 0 | 4 | 2 | 0 |
| 4 | 2 | 0 | 2 | 2 | 0 |
| 2 | 2 | 0 | 3 | 2 | 0 |
| 3 | 2 | 0 | 4 | 1 | 0 |
| 4 | 1 | 0 | 2 | 1 | 0 |
| 2 | 1 | 0 | 3 | 1 | 0 |
| 3 | 1 | 0 | 4 | 0 | 1 |
| 4 | 0 | 1 | 7 | 0 | 1 |
| 7 | 0 | 1 | 8 | 0 | 1 |
### Лінійний пошук двома способами

find lo hi byte перевіряє піввідкритий діапазон [lo,hi), повертає першу абсолютну адресу й індекс.
За промаху друкує not found; порожній діапазон допустимий. Відмовляє, якщо lo>hi, hi>4096 або byte>255.

programs/lab-04-search.txt — той самий пошук восьми байтів на 0x800 гостем.
H — поточна адреса; B — needle65; CMP/JZ знаходять збіг.
Після промаху INCH/HLOW отримують індекс, B тимчасово 8, CMP/JZ перевіряють кінець.
Повернення в loop заново ставить B=65. Успіх друкує 0...7, повний промах —255.
Адреса 0x808 не читається, навіть якщо там лежить needle.

| Адреса | Байти | Інструкція |
|---|---|---|
| 0000 | 28 00 08 | LOADH H,0x800 |
| 0003 | 21 41 | loop: LOADI B,65 |
| 0005 | 29 | LOAD A,[H] |
| 0006 | 1a | CMP A,B |
| 0007 | 31 15 00 | JZ found |
| 000a | 2b | INCH |
| 000b | 2d | HLOW |
| 000c | 21 08 | LOADI B,8 |
| 000e | 1a | CMP A,B |
| 000f | 31 18 00 | JZ miss |
| 0012 | 30 03 00 | JMP loop |
| 0015 | 2d | found: HLOW |
| 0016 | 03 | OUTN |
| 0017 | 00 | HALT |
| 0018 | 20 ff | miss: LOADI A,255 |
| 001a | 03 | OUTN |
| 001b | 00 | HALT |
Успіх та промах, реальні результати обох способів:

~~~text
ember 0.4 - 4096 bytes of memory you can see. Type `help`.
found 2051 index=3
3
steps=38
PC=24 A=3 B=65 H=2051 Z=1 N=0 C=0 halted=1
~~~

~~~text
ember 0.4 - 4096 bytes of memory you can see. Type `help`.
not found
255
steps=83
PC=28 A=255 B=8 H=2056 Z=1 N=0 C=0 halted=1
~~~
### Advanced: точки зупину

b addr задає одну точку зупину; b off вимикає її. run зупиняється ДО виконання інструкції з цим PC,
не виставляючи halted. Щоб пройти через неї, виконайте step або вимкніть точку.

~~~text
ember 0.4 - 4096 bytes of memory you can see. Type `help`.
3 breakpoint at 3

steps=2
step limit reached (100000)
PC=3 A=3 B=0 H=0 Z=0 N=0 C=0 halted=0
2 1
steps=8
PC=8 A=0 B=0 H=0 Z=1 N=0 C=0 halted=1
~~~
### Notes

Присвоєння в умові, коротке замикання, явний fallthrough, затінення, static та while:

~~~text
yes 1
A
AB
12
10 1
1
1 1
2 1
3 1
3 2 1
~~~
Присвоєння для запуску позначене явно if((x=1)); провалювання — [[fallthrough]].
Попередження не вимикались. Оригінальне if(x=1) відхилене з -Werror:

~~~text
/home/psxfnw/pf-private/ember/experiments/lab-04/assignment.cpp: In function ‘int main()’:
/home/psxfnw/pf-private/ember/experiments/lab-04/assignment.cpp:2:24: error: suggest parentheses around assignment used as truth value [-Werror=parentheses]
    2 | int main(){int x=3;if(x=1)std::cout<<x;}
      |                       ~^~
cc1plus: all warnings being treated as errors
~~~
### Перевірки

~~~bash
ctest --test-dir build --output-on-failure
python3 tests/check_control.py build/ember
python3 tests/check_machine.py build/ember
python3 tests/check_cli.py build/ember
~~~

~~~text
PASS lab-04: countdown11, all branch conditions, stale flags, unknown opcodes, bounded run, breakpoint, search all8/miss
~~~
Перевірено всі варіанти взятих/невзятих JZ/JNZ/JC/JNC, збереження прапорців LOADI, невідомі байти в кожній групі, обмеження run, breakpoint, усі 8 позицій пошуку та промах. Регресія 01–03 пройшла, ASan+UBSan чисті у готовій програмі.

## Lab 05 — Масиви, рядки та екран

Екран 64×32, один біт на піксель: 64·32/8=256 байтів 0xA00–0xAFF усередині тієї самої Memory.
Окремого масиву pixels немає. show взятий з наданого коду, plot/clear написані за тією самою формулою:

~~~text
адреса байта = 0xA00 + y * 8 + x / 8
номер біта = 7 - x % 8
маска = 1u << номер біта
~~~

Наприклад (13,2): адреса 0xA11, біт 2, маска 0x04.
plot встановлює OR, unplot скидає AND з інверсією маски.
clear занулює рівно VRAM. Координати перевіряються, включно з від'ємними в CLI.

| Опкод | Мнемоніка | Байти | Прапорці |
|---|---|---:|---|
| 04 | OUTS addr16 | 3 | — |
| 40 | CLS | 1 | — |
| 41 / 42 | PLOT / UNPLOT | 1 | лише C:0 за успіху,1 за межею |
| 43 | SHOW | 1 | — |

OUTS друкує до нульового байта,256 байтів або кінця Memory, не виходячи за масив.
Для рядка гостя використано байти пам'яті, не std::string.
Нові CLI: show,clear,frame,plot x y,rowmin y,sort lo hi,sortshow lo hi.

### Доказ: екран — пам'ять

Офіційний сценарій робить STORE [H],A з A0x80,H0xA00, без PLOT.
Вивід першого рядка SHOW та читання VRAM:

~~~text
#...............................................................
128  0x80  0b10000000  '.'
~~~
Решта 31 рядок порожня; це автоматично перевірено. Програма на три пікселі відтворюється так:
~~~bash
./build/ember < programs/lab-05-pixels.txt
~~~
Пікселі(0,0),(13,2),(63,31); байти 0xA11=4 та 0xAFF=1.
### Рамка з C++ і пошук

frame двома циклами малює верх/низ та лівий/правий край, show читає отриману VRAM.
programs/lab-05-frame.txt друкує всю рамку та дамп. Фрагмент рядків 0,1,31:

~~~text
################################################################
#..............................................................#
################################################################
~~~
Дамп початку й кінця відеопам’яті:

~~~text
0a00  ff ff ff ff ff ff ff ff 80 00 00 00 00 00 00 01  |................|
0af0  80 00 00 00 00 00 00 01 ff ff ff ff ff ff ff ff  |................|
~~~
Байтff — вісім засвічених пікселів;80 — лівий край,01 — правий. find по VRAM та rowmin:

~~~text
found 2560 index=0
rowmin=0
~~~
HELLO та той самий find по даних:

~~~text
ember 0.5 - 4096 bytes of memory you can see. Type `help`.
found 2048 index=0
HELLO
steps=2
PC=4 A=0 B=0 H=0 Z=0 N=0 C=0 halted=1
~~~
### Advanced: сортування й кадри

sort lo hi — бульбашка на місці, діапазон[lo,hi). Після проходу найбільший із невпорядкованих
опиняється справа. За відсутності обмінів завершуємо раніше.
sortshow викликає SHOW після КОЖНОГО проходу; вимагає до 64 байтів у DATA, щоб малювання
не перезаписало дані, які сортуються. Висота стовпчика=min(value,32), x=i−lo, низ — рядок 31.
rowmin y повертає найлівіший піксель або−1; ним виконано альтернативу вимоги про анімацію.

~~~bash
./build/ember < programs/lab-05-sort.txt
~~~

Фактичні рядки пам'яті до/після:

~~~text
0800  08 03 07 01 06 02 05 04 00 00 00 00 00 00 00 00  |................|
0800  01 02 03 04 05 06 07 08 00 00 00 00 00 00 00 00  |................|
~~~
Надруковано 5 повних кадрів. Фрагмент рядків 24–31 першого та останнього:

~~~text
.......#........................................................
.#.....#........................................................
.#.#...#........................................................
.#.#.#.#........................................................
.#.#.###........................................................
##.#.###........................................................
##.#####........................................................
########........................................................
~~~

~~~text
.......#........................................................
......##........................................................
.....###........................................................
....####........................................................
...#####........................................................
..######........................................................
.#######........................................................
########........................................................
~~~
### Додаткові порт і таймер ISA

0xB00 PORT_OUT: кожен STORE гостя друкує символ і зберігає байт.
Повторний запис того самого байта — повторна дія; set хоста не друкує.
0xB01 TICKS: гостьовий LOAD повертає кількість інструкцій разом із поточною modulo256.
Підтримані прямі адреси та H. Дзеркало Flags лишилося на 0xB02, конфлікту немає.
Справжній регістр пристрою в C++ потребував би volatile: звичайні два однакові записи
компілятор може згорнути як зайвий запис, якщо інших спостережуваних дій немає.
У нашому емуляторі side effect — явний виклик виводу в guest_write.
Реальні два записи A та читання таймера через H:

~~~text
ember 0.5 - 4096 bytes of memory you can see. Type `help`.
AA
steps=5
65  0x41  0b01000001  'A'
~~~

~~~text
ember 0.5 - 4096 bytes of memory you can see. Type `help`.
2
steps=4
PC=6 A=2 B=0 H=2817 Z=0 N=0 C=0 halted=1
~~~
### Notes і C-рядок без нуля

Розміри масиву/вказівника, матриця та окремий плоский масив, перший і повний проходи сортування:

~~~text
12 8
6 6
1 3 2 4
1 2 3 4
~~~
У приватному експерименті термінатор AB замінено на X; фактичний ASan-звіт:

~~~text
65 66 0
=================================================================
==473==ERROR: AddressSanitizer: stack-buffer-overflow on address 0x7444378f0023 at pc 0x78443aa89cf8 bp 0x7ffd1d460a80 sp 0x7ffd1d460238
READ of size 4 at 0x7444378f0023 thread T0
    #0 0x78443aa89cf7 in strlen ../../../../src/libsanitizer/sanitizer_common/sanitizer_common_interceptors.inc:425
    #1 0x78443a763170 in std::basic_ostream<char, std::char_traits<char> >& std::operator<< <std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*) (/usr/lib/x86_64-linux-gnu/libstdc++.so.6+0x163170) (BuildId: 3a7a8c6bd922e6314edb7a710fdc0293265c2c42)
    #2 0x5f003f85d965 in main /home/psxfnw/pf-private/ember/experiments/lab-05/string.cpp:2
    #3 0x784439a2a600  (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a600) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #4 0x784439a2a717 in __libc_start_main (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a717) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #5 0x5f003f85d264 in _start (/home/psxfnw/pf-private/ember/experiments/lab-05/string+0x2264) (BuildId: d93ee262931d1e28e6b29dce76c9d799a2082eec)

Address 0x7444378f0023 is located in stack of thread T0 at offset 35 in frame
    #0 0x5f003f85d338 in main /home/psxfnw/pf-private/ember/experiments/lab-05/string.cpp:2

  This frame has 1 object(s):
    [32, 35) 's' (line 2) <== Memory access at offset 35 overflows this variable
HINT: this may be a false positive if your program uses some custom stack unwind mechanism, swapcontext or vfork
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: stack-buffer-overflow (/usr/lib/x86_64-linux-gnu/libstdc++.so.6+0x163170) (BuildId: 3a7a8c6bd922e6314edb7a710fdc0293265c2c42) in std::basic_ostream<char, std::char_traits<char> >& std::operator<< <std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*)
Shadow bytes around the buggy address:
  0x7444378efd80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7444378efe00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7444378efe80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7444378eff00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7444378eff80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x7444378f0000: f1 f1 f1 f1[03]f3 f3 f3 00 00 00 00 00 00 00 00
  0x7444378f0080: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7444378f0100: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7444378f0180: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7444378f0200: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x7444378f0280: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==473==ABORTING
~~~
У готовій VM OUTS має незалежні обмеження; цей баг не входить до застосунку.
### Перевірки
~~~bash
python3 tests/check_display.py build/ember
ctest --test-dir build --output-on-failure
~~~

~~~text
PASS lab-05: exact64x32/stride, bounds/C, OUTS256/end, sort frames, port side effects, timer including current instruction
~~~
C++-перевірка охопила всі 2048 координат, точну маску/байт, unplot, межі clear, збереження Z/N та зміну лише C у PLOT. Пройшли всі попередні сценарії 01–04. У готовій програмі ASan+UBSan без діагностики.

## Lab 06 — Структури, переліки та купа

Усі OP-константи замінені на enum class Op : uint8_t із ТИМИ САМИМИ значеннями ISA.
Сирий байт перетворюється явно; внутрішні switch перемикаються по Op.
Невідомі значення лишилися помилкою. Всі офіційні та додаткові сценарії Lab01–05 пройшли без змін.

### Фактична розкладка

~~~bash
./build/layout
~~~

~~~text
CPU size=48 align=8
mem offset=0 size=8
pc offset=8 size=2
ticks offset=16 size=8
heap_ptr offset=24 size=2
h offset=26 size=2
a offset=28 size=1
b offset=29 size=1
f offset=30 size=3
faulted offset=33 size=1
breakpoint offset=36 size=4
halted offset=40 size=1
Flags size=3
Sprite size=5 align=1
Sprite offsets 0 1 2 3 4
P size=8 fields=5 n-offset=4
Value size=4 kind-offset=0 bits-offset=2 Addr=3072
Op Add=16
~~~
CPU займає 48 байтів, сума розмірів полів 33, вирівнювальних 15:
6 між pc та ticks,2 перед breakpoint,7 у хвості для вирівнювання наступного CPU на 8.
Memory не входить у CPU повністю — всередині лише 8-байтовий вказівник.
Flags має 3 байти. Sprite має 5 без padding, поля на 0,1,2,3,4.
Ці числа виміряні в цій версії; подальші поля можуть змінити sizeof.

### Карта всіх 4096 байтів

| Регіон | Адреси | Байти | Напрям/призначення |
|---|---|---:|---|
| CODE | 000–7FF | 2048 | програма, PC стартує 0 |
| DATA | 800–9FF | 512 | рядки/масиви |
| VRAM | A00–AFF | 256 |64×32 біти |
| запас | B00–BFF | 256 | B00 порт, B01 таймер, B02 Flags |
| HEAP | C00–EFF | 768 | heap_ptr росте вгору |
| STACK | F00–FFF | 256 | зарезервований, у Lab07 ростиме вниз |

### Спрайти

На хості Sprite sprites[8]{} з x/y:uint8_t, vx/vy:int8_t, alive:bool.
sprite i x y vx vy задає слот; tick [n] рухає всі живі, віддзеркалює рух біля країв,
очищує VRAM і малює новий кадр. sprites показує стан.
Координати —0...63/0...31; швидкості−127...127. −128 відхиляється,
бо протилежна швидкість+128 не подається в int8_t.
Для великих швидкостей цикл віддзеркалення обробляє кілька зіткнень за такт.
При досягненні краю швидкість вже дивиться всередину екрана.

~~~bash
./build/ember < programs/lab-06-sprites.txt
~~~

Реальні стани до руху, на краях і після відскоку:

~~~text
sprite 0 x=62 y=1 vx=1 vy=-1
sprite 1 x=1 y=30 vx=-1 vy=1
sprite 0 x=63 y=0 vx=-1 vy=1
sprite 1 x=0 y=31 vx=1 vy=-1
sprite 0 x=62 y=1 vx=-1 vy=1
sprite 1 x=1 y=30 vx=1 vy=-1
~~~
### Advanced: ALLOC

| Опкод | Мнемоніка | Байти | Прапорці | Поведінка |
|---|---|---:|---|---|
| 60 | ALLOC | 1 | лише C | виділити A байтів, H=початок, посунути heap_ptr; за відмови C=1, H/heap_ptr незмінні |

heap_ptr стартує 0xC00, виключена верхня межа 0xF00. Перевіряємо місткість до додавання.
A=0 дозволений: повертає поточну вершину без резервування байтів.
Немає FREE: блок живе до завершення/нового CPU, повторно використати його цей алокатор не вміє.
Він виділяє адресу ГОСТЯ всередині Memory, не виконує new хоста.

programs/lab-06-heap.txt виділяє 4 та 3 байти й записує 11 і 22 відповідно:

~~~text
PC=23 A=34 B=0 H=3078 Z=0 N=0 C=0 halted=1 heap=3079
0c00  11 11 11 11 22 22 22 00 00 00 00 00 00 00 00 00  |....""".........|
~~~
Перший блок C00–C03, другий C04–C06, нова вершинаC07: дірок немає,
бо перед кожним виділенням беремо рівно поточну вершину.

### Тегований Value проти union

Value містить Kind (Byte,Addr,SpriteId) і uint16_t bits; у виміряній розкладці це 4 байти.
Вид записаний поруч зі значенням: Value{Kind::Addr,0xC00} можна відрізнити від Byte з тими самими бітами.
union зберігає альтернативи в одній пам'яті; сам по собі не знає, який член активний.
У тегованій структурі цей факт зберігається явно. Потрібно однаково правильно перевіряти тег перед тлумаченням.
Value обрано як Advanced-альтернативу складнішому FREE зі списком вільних блоків.

### Notes: типізація та масив записів

Спроба присвоїти int в Op відхилена компілятором:

~~~text
/home/psxfnw/pf-private/ember/experiments/lab-06/enum-error.cpp: In function ‘int main()’:
/home/psxfnw/pf-private/ember/experiments/lab-06/enum-error.cpp:1:70: error: cannot convert ‘int’ to ‘Op’ in assignment
    1 | enum class Op:unsigned char{Halt=0,Add=16};int main(){Op o=Op::Add;o=1;return static_cast<int>(o);}
      |                                                                      ^
      |                                                                      |
      |                                                                      int
~~~
Окремий фрагмент Notes §5 після одного руху двох записів:

~~~text
0 1
1 9
~~~
### Три справжні звіти ASan/LSan й виправлення

Досліди виконані окремими програмами в Ubuntu/WSL, Clang21, C++17,
-Wall -Wextra -Werror -fsanitize=address,undefined -g. Для імен файлів використано llvm-symbolizer21.
Linux LeakSanitizer доступний; обмеження Apple Silicon тут не застосовується.

**Читання після delete**

~~~text
42
=================================================================
==464==ERROR: AddressSanitizer: heap-use-after-free on address 0x72e2ef1e0010 at pc 0x5e43767e81ca bp 0x7ffef8d94ff0 sp 0x7ffef8d94fe8
READ of size 4 at 0x72e2ef1e0010 thread T0
    #0 0x5e43767e81c9 in main /home/psxfnw/pf-private/ember/experiments/lab-06/use-after-free.cpp:2:76
    #1 0x76c2f002a600  (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a600) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #2 0x76c2f002a717 in __libc_start_main (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a717) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #3 0x5e43766fd394 in _start (/home/psxfnw/pf-private/ember/experiments/lab-06/use-after-free+0x2d394) (BuildId: 537dee9d5f2e71e339461341cd1fc0b1133570c3)

0x72e2ef1e0010 is located 0 bytes inside of 4-byte region [0x72e2ef1e0010,0x72e2ef1e0014)
freed by thread T0 here:
    #0 0x5e43767e77e6 in operator delete(void*, unsigned long) (/home/psxfnw/pf-private/ember/experiments/lab-06/use-after-free+0x1177e6) (BuildId: 537dee9d5f2e71e339461341cd1fc0b1133570c3)
    #1 0x5e43767e8168 in main /home/psxfnw/pf-private/ember/experiments/lab-06/use-after-free.cpp:2:56
    #2 0x76c2f002a600  (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a600) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #3 0x76c2f002a717 in __libc_start_main (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a717) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #4 0x5e43766fd394 in _start (/home/psxfnw/pf-private/ember/experiments/lab-06/use-after-free+0x2d394) (BuildId: 537dee9d5f2e71e339461341cd1fc0b1133570c3)

previously allocated by thread T0 here:
    #0 0x5e43767e6b61 in operator new(unsigned long) (/home/psxfnw/pf-private/ember/experiments/lab-06/use-after-free+0x116b61) (BuildId: 537dee9d5f2e71e339461341cd1fc0b1133570c3)
    #1 0x5e43767e8000 in main /home/psxfnw/pf-private/ember/experiments/lab-06/use-after-free.cpp:2:19
    #2 0x76c2f002a600  (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a600) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #3 0x76c2f002a717 in __libc_start_main (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a717) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #4 0x5e43766fd394 in _start (/home/psxfnw/pf-private/ember/experiments/lab-06/use-after-free+0x2d394) (BuildId: 537dee9d5f2e71e339461341cd1fc0b1133570c3)

SUMMARY: AddressSanitizer: heap-use-after-free /home/psxfnw/pf-private/ember/experiments/lab-06/use-after-free.cpp:2:76 in main
Shadow bytes around the buggy address:
  0x72e2ef1dfd80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x72e2ef1dfe00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x72e2ef1dfe80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x72e2ef1dff00: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x72e2ef1dff80: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x72e2ef1e0000: fa fa[fd]fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x72e2ef1e0080: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x72e2ef1e0100: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x72e2ef1e0180: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x72e2ef1e0200: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x72e2ef1e0280: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==464==ABORTING
~~~
Об’єкта вже немає, збережена адреса стала завислою; читати через неї не можна.

**Повторне delete**

~~~text
=================================================================
==471==ERROR: AddressSanitizer: attempting double-free on 0x6ffab7fe0010 in thread T0:
    #0 0x5ae4c30ab7b6 in operator delete(void*, unsigned long) (/home/psxfnw/pf-private/ember/experiments/lab-06/double-free+0x1177b6) (BuildId: c4bd28d29d90eb92defd865f9d14ea0da44210bf)
    #1 0x5ae4c30ac0ce in main /home/psxfnw/pf-private/ember/experiments/lab-06/double-free.cpp:1:40
    #2 0x73dab8c2a600  (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a600) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #3 0x73dab8c2a717 in __libc_start_main (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a717) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #4 0x5ae4c2fc1364 in _start (/home/psxfnw/pf-private/ember/experiments/lab-06/double-free+0x2d364) (BuildId: c4bd28d29d90eb92defd865f9d14ea0da44210bf)

0x6ffab7fe0010 is located 0 bytes inside of 4-byte region [0x6ffab7fe0010,0x6ffab7fe0014)
freed by thread T0 here:
    #0 0x5ae4c30ab7b6 in operator delete(void*, unsigned long) (/home/psxfnw/pf-private/ember/experiments/lab-06/double-free+0x1177b6) (BuildId: c4bd28d29d90eb92defd865f9d14ea0da44210bf)
    #1 0x5ae4c30ac086 in main /home/psxfnw/pf-private/ember/experiments/lab-06/double-free.cpp:1:31
    #2 0x73dab8c2a600  (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a600) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #3 0x73dab8c2a717 in __libc_start_main (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a717) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #4 0x5ae4c2fc1364 in _start (/home/psxfnw/pf-private/ember/experiments/lab-06/double-free+0x2d364) (BuildId: c4bd28d29d90eb92defd865f9d14ea0da44210bf)

previously allocated by thread T0 here:
    #0 0x5ae4c30aab31 in operator new(unsigned long) (/home/psxfnw/pf-private/ember/experiments/lab-06/double-free+0x116b31) (BuildId: c4bd28d29d90eb92defd865f9d14ea0da44210bf)
    #1 0x5ae4c30abfd0 in main /home/psxfnw/pf-private/ember/experiments/lab-06/double-free.cpp:1:19
    #2 0x73dab8c2a600  (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a600) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #3 0x73dab8c2a717 in __libc_start_main (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a717) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #4 0x5ae4c2fc1364 in _start (/home/psxfnw/pf-private/ember/experiments/lab-06/double-free+0x2d364) (BuildId: c4bd28d29d90eb92defd865f9d14ea0da44210bf)

SUMMARY: AddressSanitizer: double-free (/home/psxfnw/pf-private/ember/experiments/lab-06/double-free+0x1177b6) (BuildId: c4bd28d29d90eb92defd865f9d14ea0da44210bf) in operator delete(void*, unsigned long)
==471==ABORTING
~~~
Один виділений об’єкт не можна звільняти двічі.

**Витік**

~~~text
=================================================================
==473==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 4 byte(s) in 1 object(s) allocated from:
    #0 0x5b1627208b31 in operator new(unsigned long) (/home/psxfnw/pf-private/ember/experiments/lab-06/leak+0x116b31) (BuildId: 44a41d581c95d81480ae49e82d9f36e448595342)
    #1 0x5b1627209fd0 in main /home/psxfnw/pf-private/ember/experiments/lab-06/leak.cpp:1:12
    #2 0x75714402a600  (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a600) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #3 0x75714402a717 in __libc_start_main (/usr/lib/x86_64-linux-gnu/libc.so.6+0x2a717) (BuildId: 066527e430a32768d82741e00b81eebb1a872294)
    #4 0x5b162711f364 in _start (/home/psxfnw/pf-private/ember/experiments/lab-06/leak+0x2d364) (BuildId: 44a41d581c95d81480ae49e82d9f36e448595342)

SUMMARY: AddressSanitizer: 4 byte(s) leaked in 1 allocation(s).
~~~
new створив об’єкт, останній вказівник не збережений; delete виконати нікому.
Виправлений окремий варіант читає значення ДО delete, зануляє вказівник,
для new[] використовує delete[]. Нульовий вказівник не розіменовують.
З ASAN_OPTIONS=detect_leaks=1 діагностики немає, реальний вивід:

~~~text
42
7
~~~
Навмисно зламані фрагменти не входять до вихідного дерева ember.

### Перевірки

Окремого офіційного checks/lab-06.txt немає. За CHECKS.md виконана регресія всіх попередніх,
C++-тест, структура-layout та власний tests/check_records.py:

~~~text
PASS lab-06: refactor regression, two sprites/both axes, large speeds, heap exact fit and failed ALLOC preserving H
~~~

~~~text
Test project /home/psxfnw/pf/ember/build
    Start 1: alu-and-cpu
1/2 Test #1: alu-and-cpu ......................   Passed    0.03 sec
    Start 2: structure-layout
2/2 Test #2: structure-layout .................   Passed    0.01 sec

100% tests passed, 0 tests failed out of 2

Total Test time (real) =   0.04 sec
~~~
У готовому коді Debug ASan+UBSan/LSan чисті; помилки вище належать лише навмисним приватним дослідам.

## Lab 07 — стек, виклики й рекурсія

Реалізовано Basic, Standard і всі три розширення Advanced. Додані опкоди:
MUL A,B=1B; PUSH A=50; POP A=51; PUSH B=52; POP B=53; CALL addr16=54;
RET=55; PUSH H=56; POP H=57. CALL має три байти, інші нові інструкції — один.
MUL змінює Z/N за молодшим байтом добутку, C означає добуток понад 255.
Стекові інструкції й CALL/RET зберігають прапорці.

### Представлення й конвенція

Stack — окремий ADT у stack.hpp. Представлення приховане за StackState.
CPU викликає push/pop/push16/pop16/peek16, не знає про масив чи вузли.
Порожній спадний стек займає F00–FFF: спочатку SP=FFF, push записує
mem[SP] і зменшує SP; pop спочатку збільшує SP і читає байт.
Після 256 push SP=EFF, але запису в EFF немає. Наступний push відмовляє.
SP показує вільну позицію, а верхній зайнятий байт — SP+1.

CALL зберігає адресу наступної інструкції PC+3: спочатку молодший байт,
потім старший. RET знімає старший, потім молодший. PUSH/POP H мають такий
самий порядок. Двобайтові операції перевіряють наявність двох місць/байтів
до зміни стану. Помилка не лишає половини адреси. RET перевіряє також адресу
повернення до зняття байтів.

Аргумент fact лежить в A, результат повертається в A. Викликана функція може
змінити B і H, тому їх за потреби зберігає викликач. Власне n кладемо PUSH A
до рекурсії: після CALL в A вже fact(n−1), а POP B повертає збережене n.
MUL A,B обчислює результат. Адресу повернення зберігає сам CALL, локальне n —
програма. Збережений H у нашій mul-підпрограмі є додатковою гарантією цієї функції.

### Один виклик: CALL printA

Початково A=65, SP=FFF; байти з адреси 0: 54 04 00 00 02 55.
Після кожного step викликаємо regs і stack:

| Виконано за адресою | Інструкція | Новий PC | SP | Верхні байти |
|---|---|---|---|---|
| 0000 | CALL 0004 | 0004 | FFD | [FFE]=00, [FFF]=03 |
| 0004 | OUT | 0005 | FFD | 00,03; надруковано A |
| 0005 | RET | 0003 | FFF | стек порожній |
| 0003 | HALT | 0004 | FFF | стек порожній |

PC+3 потрібен, щоб продовжити після CALL. Збереження адреси самого CALL
повторно викликало б підпрограму після RET.

### Офіційний факторіал і його зміна

~~~bash
./build/ember < checks/lab-07.txt
./build/ember < examples/fact-add.txt
python3 tests/check_stack.py build/ember
~~~

Офіційний checks/lab-07.txt скопійований без зміни байтів. Реальний вивід:
~~~text
120
steps=60
PC=7 A=120 B=5 H=0 Z=0 N=0 C=0 halted=1 heap=3072 SP=4095
SP=4095 used=0
~~~

Траса кожного CALL/RET для fact(5), адреси й SP у hex:
| PC до | Операція | PC після | SP після | A після |
|---|---|---|---|---|
| 0002 | CALL | 0007 | FFD | 5 |
| 0015 | CALL | 0007 | FFA | 4 |
| 0015 | CALL | 0007 | FF7 | 3 |
| 0015 | CALL | 0007 | FF4 | 2 |
| 0015 | CALL | 0007 | FF1 | 1 |
| 001D | RET | 0018 | FF3 | 1 |
| 001A | RET | 0018 | FF6 | 2 |
| 001A | RET | 0018 | FF9 | 6 |
| 001A | RET | 0018 | FFC | 24 |
| 001A | RET | 0005 | FFF | 120 |

Мінімальний SP=FF1: зайнято 14 байтів. Це п'ять адрес повернення по два
байти плюс чотири збережені n. Після останнього RET стек порожній.

| Аргумент | A на виході | Кроків | C | Пояснення |
|---|---|---|---|---|
| 0 | 1 | 9 | 0 | перший базовий випадок |
| 1 | 1 | 12 | 0 | другий базовий випадок |
| 5 | 120 | 60 | 0 | поміщається в байт |
| 6 | 208 | 72 | 1 | 720 mod 256 = 208 |

Для досліду fact(6) змінено тільки set 1 0x05 на set 1 0x06. Прапорець C
від останнього MUL повідомляє про втрату старших бітів 720. Це не помилка
C++ і не UB: обчислення робиться у ширшому unsigned, потім явно звужується.

Для навмисного переповнення обидва JZ base та їхні операнди за адресами
10–12 і 16–18 замінено NOP=01. Отримано:
~~~text
CPU error: stack overflow

steps=1107
PC=21 A=176 B=1 H=0 Z=0 N=1 C=0 halted=1 heap=3072 SP=3840
SP=3840 used=255
  0f01: b1
  0f02: 00
  0f03: 18
  0f04: b2
  0f05: 00
  0f06: 18
  0f07: b3
  0f08: 00
~~~
PC=21 (hex15) — CALL, якому не вистачає двох байтів. Зайнято 255 байтів;
одне вільне місце не використовується, оскільки push16 атомарний.
Це пояснює SP=F00, а не EFF у цій помилці. Для 256 окремих PUSH
тест окремо перевіряє повний стан SP=EFF. Після помилки CPU зупинений,
процес продовжує приймати команди. ASan/UBSan помилок не повідомили.

### Advanced: множення підпрограмою

examples/fact-add.txt містить готові команди set. У цій версії замість MUL
виконується CALL 0020. Мітки, пораховані під час підготовки байткоду:
fact=0007, base=001D, mul=0020, loop=0033, no_carry=0040, done=0047.

Підпрограма приймає A×B, повертає молодший байт в A; B змінюється.
PUSH H зберігає вказівник, LOADH H,0820 переводить його на робочі дані:
[820] — множене, [821] — лічильник повторень, [822] — накопичене перенесення.
Акумулятор починається з нуля. Цикл додає множене рівно початкове B разів.
PUSH A/POP A захищають суму, поки A використовується для лічильника.
DEC задає Z для JNZ, а POP його не псує.
Якщо ADD дає C=1, [822] стає 1 і більше не скидається. Наприкінці SHR
цього байта відновлює C, INC/DEC результату — Z/N зі збереженням C.
POP H відновлює вказівник, RET завершує виклик. Робочі байти не є
рекурсивними локальними: сама mul не викликає mul, виклики послідовні.

Реальні порівняння:

| fact | Опкод MUL | Підпрограма ADD | Результат |
|---|---|---|---|
| 5 | 60 кроків | 262 кроки | 120, C=0 |
| 6 | 72 кроки | 351 крок | 208, C=1 |

У демонстрації H=2304 перед запуском і після повернення, SP=4095.
Підпрограма повільніша через цикл, роботу з пам'яттю і збереження регістрів.

### Advanced: заміна стека без зміни CPU

~~~bash
cmake -S . -B build-linked -G Ninja -DCMAKE_BUILD_TYPE=Debug -DEMBER_LINKED_STACK=ON
cmake --build build-linked
ctest --test-dir build-linked --output-on-failure
python3 tests/check_stack.py build-linked/ember
~~~

За замовчуванням stack.cpp зберігає значення в масиві гостьової Memory.
stack_linked.cpp додає зв'язані вузли з адресами зайнятих байтів. Значення
читаються з тієї самої Memory, тому гостьовий STORE у стек видимий для POP.
Деструктор звільняє всі вузли навіть після переповнення; копіювання Stack
заборонене, щоб не було подвійного delete.

Перевага масиву — немає виділення вузла на кожний push, менше накладних витрат.
Перевага списку — додавання/видалення вершини без переміщення інших вузлів;
така реалізація може рости динамічно, хоча тут обмежена ISA 256 байтами.
CPU не змінювався між збірками: SHA256 cpu.cpp
4a07f186fa0f85febd0f3f6a4fc1a49e90d55247ae45d917707de0ba3ea1224c.

В обох конфігураціях пройшли 3 тести CTest та всі CLI-перевірки Lab01–07.
Додатково перевірено всі 65536 пар MUL, 256 push/pop, атомарну відмову
двобайтової операції, адреси повернення з ненульовим старшим байтом,
пошкоджений RET, незмінність прапорців і звільнення вузлів санітайзерами.

### Досліди Notes 07

Передача за значенням/посиланням/вказівником дала 1 2 2: лише копія
не змінює змінну викликача. C++ fact(3) надрукував:
~~~text
>3
>2
>1
<2
<3
6
~~~
На вході n=3 кадр чекає 3×fact(2), кадр n=2 чекає 2×fact(1),
кадр n=1 повертає 1. Розгортання дає 2, потім 6.
Два визначення void push() в a.cpp і b.cpp справді дали помилку лінкера:
~~~text
multiple definition of push(); ... first defined here
collect2: error: ld returned 1 exit status
~~~
Заголовок містить оголошення, реалізація — одне визначення у вибраному .cpp.
#pragma once захищає включення в одну одиницю трансляції, а не весь лінкер.

## Lab 08 — мова для ember

Виконано Basic, Standard і Advanced: лексер із каркаса викладача, двопрохідний
асемблер, зв'язний список токенів, hello/search/рекурсивна fib, bounce,
дизасемблер та числові макроси .define. Теги lab-08 і v1.0.0 позначають
одну перевірену версію. Правила ISA взято з
[офіційної специфікації](https://github.com/rmalkevy/Programming-Practice-Projects/blob/main/courses/programming-fundamentals/ISA.uk.md),
завдання — зі знімка курсу a713eac від 07.10.2026.

### Архітектура

~~~mermaid
flowchart LR
    Text["programs/*.asm"] --> Lexer["lexer: символи → Token"]
    Lexer --> List["зв'язний список"]
    List --> Pass1["прохід 1: мітки й розміри"]
    Pass1 --> Pass2["прохід 2: байти LE"]
    Pass2 --> Memory["Memory: 4096 байтів"]
    Memory --> CPU["step / run"]
    CPU --> Screen["VRAM → show"]
    CPU --> Output["OUT / OUTN / OUTS"]
~~~

src/asm/token.* дає push_back, обхід через next і destroy.
lexer.* розпізнає класи лексем, зберігає їхній текст та рядок.
assembler.* зіставляє точні форми операндів і підставляє символи.
frontend.* читає файл, друкує токени/байти/лістинг або запускає CPU.
Розміри інструкцій в обох проходах і дизасемблері беруться з тієї самої
instruction_size, що й у step. Усі 49 форм ISA перевірені повним обертом.

### Команди

~~~bash
./build/ember programs/hello.asm
./build/ember programs/search.asm
./build/ember programs/fib.asm
./build/ember programs/bounce.asm
./build/ember lex programs/hello.asm
./build/ember asm programs/fib.asm
./build/ember disasm programs/fib.asm
~~~

CLI asm друкує hex-байти. CLI disasm приймає .asm (спершу збирає) або
інший файл як сирі двійкові байти. Вихід — канонічний асемблер,
який можна знову зібрати; мітки й коментарі не відновлюються.
У REPL команда asm шлях.asm також завантажує байти з адреси 0:
після успіху скидає регістри, стек, heap_ptr, breakpoint і всю пам'ять.
Помилкова збірка нічого не завантажує. Після asm доступні step/run/regs/stack/dump/show.
Шлях у REPL може містити пробіли, лапки навколо нього не потрібні.
Файловий запуск повертає ненульовий код при помилці читання/збірки,
помилці CPU або вичерпанні ліміту кроків.

### Карта пам'яті фінальної версії

| Адреси hex | Байти | Призначення |
|---|---:|---|
| 000–7FF | 2048 | код; асемблер не дозволяє перевищити регіон |
| 800–9FF | 512 | DATA |
| A00–AFF | 256 | VRAM: 64×32 біти, 8 байтів на рядок |
| B00–BFF | 256 | резерв і регістри пристроїв |
| C00–EFF | 768 | bump-купа, росте вгору; FREE немає |
| F00–FFF | 256 | порожній спадний стек, росте вниз |

Перегородка між heap і stack фіксована. Memory — спільний масив, тому
гостьові STORE можуть змінювати код і дані; асемблер не реалізує захист пам'яті.
B00=PORT_OUT: кожний гостьовий STORE друкує символ.
B01=TICKS: гостьовий LOAD читає кількість інструкцій, включно з поточною, mod256.
B02=FLAG_STATUS: після успішного step тут bit0=Z, bit1=N, bit2=C.
Звичайний set у REPL записує пам'ять без побічних ефектів пристрою.

### Повний набір реалізованих інструкцій

Таблиця відповідає ISA викладача; усі opt у ній також реалізовано.
Нових власних опкодів немає. Власне розширення — дзеркало FLAG_STATUS;
.define є можливістю асемблера, а не інструкцією CPU.

| Опкод | Мнемоніка | Байти | Прапорці | Дія | Лаба |
|---|---|---|---|---|---|
| `0x00` | `HALT` | 1 | — | зупинитись; наступні `step` відмовляють | 2 |
| `0x01` | `NOP` | 1 | — | нічого | 2 |
| `0x02` | `OUT` | 1 | — | надрукувати `A` як **символ** | 3 |
| `0x03` | `OUTN` | 1 | — | надрукувати `A` як **десяткове число** і пробіл | 3 |
| `0x04` | `OUTS addr16` | 3 | — | друкувати байти з `addr`, поки не трапиться `0` або поки не набереться 256 байтів | 5 |
| `0x10` | `ADD A, B` | 1 | Z N C | `A = A + B` | 2 |
| `0x11` | `SUB A, B` | 1 | Z N C | `A = A - B` | 2 |
| `0x12` | `AND A, B` | 1 | Z N, C=0 | `A = A & B` | 2 |
| `0x13` | `OR A, B` | 1 | Z N, C=0 | `A = A \| B` | 2 |
| `0x14` | `XOR A, B` | 1 | Z N, C=0 | `A = A ^ B` | 2 |
| `0x15` | `NOT A` | 1 | Z N, C=0 | `A = ~A` | 2 |
| `0x16` | `SHL A` | 1 | Z N C | `A = A << 1`; у `C` потрапляє біт, що виїхав із сьомої позиції | 2 |
| `0x17` | `SHR A` | 1 | Z N C | `A = A >> 1`; у `C` потрапляє біт, що виїхав із нульової | 2 |
| `0x18` | `INC A` | 1 | Z N | `A = A + 1` | 2 |
| `0x19` | `DEC A` | 1 | Z N | `A = A - 1` | 2 |
| `0x1A` | `CMP A, B` | 1 | Z N C | порахувати `A - B`, виставити прапорці, **результат викинути** | 4 |
| `0x1B` | `MUL A, B` | 1 | Z N C | `A = A * B`, молодші 8 бітів; `C` виставляється, якщо справжній добуток не вліз | 7 |
| `0x20` | `LOADI A, imm8` | 2 | — | `A = imm8` | 3 |
| `0x21` | `LOADI B, imm8` | 2 | — | `B = imm8` | 3 |
| `0x22` | `LOAD A, [addr16]` | 3 | — | `A = mem[addr]` | 3 |
| `0x23` | `LOAD B, [addr16]` | 3 | — | `B = mem[addr]` | 3 |
| `0x24` | `STORE [addr16], A` | 3 | — | `mem[addr] = A` | 3 |
| `0x25` | `STORE [addr16], B` | 3 | — | `mem[addr] = B` | 3 |
| `0x26` | `MOV A, B` | 1 | — | `A = B` | 3 |
| `0x27` | `MOV B, A` | 1 | — | `B = A` | 3 |
| `0x28` | `LOADH H, imm16` | 3 | — | `H = imm16` — покласти адресу в адресний регістр | 3 |
| `0x29` | `LOAD A, [H]` | 1 | — | `A = mem[H]` — піти за вказівником | 3 |
| `0x2A` | `STORE [H], A` | 1 | — | `mem[H] = A` | 3 |
| `0x2B` | `INCH` | 1 | — | `H = H + 1` — крок до наступної комірки | 3 |
| `0x2C` | `DECH` | 1 | — | `H = H - 1` | 3 |
| `0x2D` | `HLOW` | 1 | — | *opt* `A = H & 0xFF` — молодший байт `H`, зручно надрукувати індекс | 4 |
| `0x30` | `JMP addr16` | 3 | — | стрибнути завжди | 4 |
| `0x31` | `JZ addr16` | 3 | — | стрибнути, якщо `Z` виставлений | 4 |
| `0x32` | `JNZ addr16` | 3 | — | стрибнути, якщо `Z` не виставлений | 4 |
| `0x33` | `JC addr16` | 3 | — | *opt* стрибнути, якщо `C` виставлений | 4 |
| `0x34` | `JNC addr16` | 3 | — | *opt* стрибнути, якщо `C` не виставлений | 4 |
| `0x40` | `CLS` | 1 | — | занулити всі 256 байтів відеопам'яті | 5 |
| `0x41` | `PLOT` | 1 | C | засвітити піксель `x = A`, `y = B`. За межами екрана: виставити `C` і нічого не міняти | 5 |
| `0x42` | `UNPLOT` | 1 | C | *opt* погасити той самий піксель | 5 |
| `0x43` | `SHOW` | 1 | — | намалювати відеопам'ять у терміналі | 5 |
| `0x50` | `PUSH A` | 1 | — | `mem[SP] = A; SP = SP - 1` | 7 |
| `0x51` | `POP A` | 1 | — | `SP = SP + 1; A = mem[SP]` | 7 |
| `0x52` | `PUSH B` | 1 | — | те саме з `B` | 7 |
| `0x53` | `POP B` | 1 | — | те саме з `B` | 7 |
| `0x54` | `CALL addr16` | 3 | — | покласти адресу повернення (2 байти), потім `PC = addr` | 7 |
| `0x55` | `RET` | 1 | — | зняти 2 байти в `PC` | 7 |
| `0x56` | `PUSH H` | 1 | — | *opt* покласти `H` (2 байти: молодший, потім старший) | 7 |
| `0x57` | `POP H` | 1 | — | *opt* зняти 2 байти в `H` | 7 |
| `0x60` | `ALLOC` | 1 | C | виділити в купі `A` байтів: `H` стає адресою блока, вказівник купи посувається. Не влізло — виставити `C`, `H` не чіпати | 6 |

Усі числові операнди 16 кодуються LOW,HIGH. Завантаження, пересилання й стекові
операції не змінюють Z/N/C. INC/DEC зберігають C. CMP змінює прапорці,
не змінюючи A. Правила виклику та побайтове трасування наведені в Lab07 вище.

### Граматика й обмеження

Одна інструкція на рядок, необов'язкова мітка перед нею, коментар від ; до \n.
Останній рядок без \n дозволений. Ідентифікатор: літера ASCII або _, потім
літери/цифри/_. Мнемоніки та регістри нечутливі до регістру, імена символів —
чутливі. Числа невід'ємні: десяткові, 0x/0X hex, 0b/0B binary.
Ведучий нуль не вмикає вісімкову систему: 010 означає десять.
Байт 0–255, слово 0–65535. Некоректна лексема спершу відхиляється повністю,
тому 12ab не перетвориться на Number12 та Identab.

.define NAME literal визначає глобальну числову константу:
~~~text
.define N 6
LOADI A, N
~~~
Її можна використати до визначення, бо прохід 1 збирає всі символи.
Повторне ім'я мітки/макросу — помилка. Макрос не може бути виразом,
іншою константою чи блоком інструкцій. Директива пишеться саме .define.
Немає AST, 2+3 в операндах, .org/.byte, рядкових літералів або багаторядкових
інструкцій; рядки hello створюються звичайними STORE у DATA.
Дизасемблер працює з кодом без вбудованих даних: невідомі/обірвані опкоди
відхиляє за байтовою позицією. Лексер/асемблер зупиняються на першій помилці.
Немає автоматичного відновлення після помилки чи позначення колонки.
Ці обмеження свідомо зберігають мову невеликою.

### Реальний вивід програм

~~~text
$ ./build/ember programs/hello.asm
EMBER

steps=23
$ ./build/ember programs/search.asm
3
steps=49
$ ./build/ember programs/fib.asm
8
steps=296
~~~

fib(0)=0, fib(1)=1, fib(2)=1, fib(6)=8 і fib(10)=55 перевірені.
Алгоритм рекурсивний: після бази 0/1 зберігає n, викликає fib(n−1),
зберігає його результат, відновлює n, викликає fib(n−2), додає результати.
B потрібний після CALL, тому значення, яке треба пережити виклик, лежить
на стеку. У запуску fib6 після завершення SP=4095, used=0.

Команди asm programs/fib.asm, run, regs, stack, dump дали, зокрема:
~~~text
PC=7 A=8 B=5 H=0 Z=0 N=0 C=0 halted=1 heap=3072 SP=4095
SP=4095 used=0
0000  20 06 54 07 00 03 00 21 00 1a 31 22 00 21 01 1a  | .T....!..1".!..|
0010  31 22 00 50 19 54 07 00 53 50 26 19 19 54 07 00  |1".P.T..SP&..T..|
0020  53 10 55 00 00 00 00 00 00 00 00 00 00 00 00 00  |S.U.............|
~~~
[Повний фактичний dump на 256 рядків](docs/examples/fib-dump.txt).

bounce виконує шість SHOW, кожний показує один піксель y=16:
x=61,62,63,62,61,60. На x=63 напрям змінюється з+1 на−1
(байт 255 при додаванні за модулем 256). Третій реальний кадр:
~~~text
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
...............................................................#
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
................................................................
~~~
Це вивід гостьових CLS/PLOT/SHOW, а не хостових sprite/tick.

### search: читабельний текст поруч із байтами Lab04

Ядро пошуку Lab04 займало 28 байтів:
~~~text
28 00 08 21 41 29 1a 31 15 00 2b 2d 21 08
1a 31 18 00 30 03 00 2d 03 00 20 ff 03 00
~~~
Воно отримувало вхідні дані через set. Нова програма сама створює ті самі
вісім елементів у DATA, тому має префікс ініціалізації й інші адреси
found/missing. Алгоритм ядра збережений: перевірити елемент, зупинитися
при збігу або після восьмого, інакше INCH. Текст:
~~~asm
; Ті самі вісім елементів і обмежений цикл, що в Lab 04.
.define TARGET 65
LOADI A, 1
STORE [0x800], A
STORE [0x801], A
STORE [0x802], A
LOADI A, 65
STORE [0x803], A
LOADI A, 1
STORE [0x804], A
STORE [0x805], A
STORE [0x806], A
STORE [0x807], A
LOADH H, 0x800
loop:
    LOADI B, TARGET
    LOAD A, [H]
    CMP A, B
    JZ found
    INCH
    HLOW
    LOADI B, 8
    CMP A, B
    JZ missing
    JMP loop
found:
    HLOW
    OUTN
    HALT
missing:
    LOADI A, 255
    OUTN
    HALT
~~~
Мітки прибирають ручний перерахунок адрес при додаванні інструкцій.
Зібрана повна програма:
~~~text
20 01 24 00 08 24 01 08 24 02 08 20 41 24 03 08
20 01 24 04 08 24 05 08 24 06 08 24 07 08 28 00
08 21 41 29 1a 31 33 00 2b 2d 21 08 1a 31 36 00
30 21 00 2d 03 00 20 ff 03 00
~~~
Пошук 65 повертає 3. Зміна лише .define TARGET65 на 66 дає 255 (не знайдено);
дані лишаються ті самі.

### Токени й помилки

Фактичний ./build/ember lex programs/hello.asm:
~~~text
1 Newline "\n"
2 Dot "."
2 Ident "define"
2 Ident "MESSAGE"
2 Number "0x800"
2 Newline "\n"
3 Ident "LOADH"
3 Ident "H"
3 Comma ","
3 Ident "MESSAGE"
3 Newline "\n"
4 Ident "LOADI"
4 Ident "A"
4 Comma ","
4 Number "69"
4 Newline "\n"
5 Ident "STORE"
5 LBracket "["
5 Ident "H"
5 RBracket "]"
5 Comma ","
5 Ident "A"
5 Newline "\n"
6 Ident "INCH"
6 Newline "\n"
7 Ident "LOADI"
7 Ident "A"
7 Comma ","
7 Number "77"
7 Newline "\n"
8 Ident "STORE"
8 LBracket "["
8 Ident "H"
8 RBracket "]"
8 Comma ","
8 Ident "A"
8 Newline "\n"
9 Ident "INCH"
9 Newline "\n"
10 Ident "LOADI"
10 Ident "A"
10 Comma ","
10 Number "66"
10 Newline "\n"
11 Ident "STORE"
11 LBracket "["
11 Ident "H"
11 RBracket "]"
11 Comma ","
11 Ident "A"
11 Newline "\n"
12 Ident "INCH"
12 Newline "\n"
13 Ident "LOADI"
13 Ident "A"
13 Comma ","
13 Number "69"
13 Newline "\n"
14 Ident "STORE"
14 LBracket "["
14 Ident "H"
14 RBracket "]"
14 Comma ","
14 Ident "A"
14 Newline "\n"
15 Ident "INCH"
15 Newline "\n"
16 Ident "LOADI"
16 Ident "A"
16 Comma ","
16 Number "82"
16 Newline "\n"
17 Ident "STORE"
17 LBracket "["
17 Ident "H"
17 RBracket "]"
17 Comma ","
17 Ident "A"
17 Newline "\n"
18 Ident "INCH"
18 Newline "\n"
19 Ident "LOADI"
19 Ident "A"
19 Comma ","
19 Number "10"
19 Newline "\n"
20 Ident "STORE"
20 LBracket "["
20 Ident "H"
20 RBracket "]"
20 Comma ","
20 Ident "A"
20 Newline "\n"
21 Ident "INCH"
21 Newline "\n"
22 Ident "LOADI"
22 Ident "A"
22 Comma ","
22 Number "0"
22 Newline "\n"
23 Ident "STORE"
23 LBracket "["
23 Ident "H"
23 RBracket "]"
23 Comma ","
23 Ident "A"
23 Newline "\n"
24 Ident "OUTS"
24 Ident "MESSAGE"
24 Newline "\n"
25 Ident "HALT"
25 Newline "\n"
26 Eof ""
~~~
Ручна таблиця Notes для ADD A,B: Ident ADD, Ident A, Comma,
Ident B, Newline, Eof. Для loop: JMP loop — Ident loop, Colon,
Ident JMP, Ident loop, Newline, Eof. Пробіли й коментарі не є токенами.
Номер рядка зберігається у кожному вузлі.

Файл з LOADI A,0xGG справді відхилено:
~~~text
line 1: bad number: 0xGG
~~~
Також перевірено 0x,12ab,0b102,неприпустимий @,байт 01,незавершені лапки,
забуту кому, відсутню ціль JMP, невідому мітку й перевищення розміру коду.
Число синтаксично правильне, але більше 65535, відхиляється асемблером.

Notes: JMP done / NOP / done: HALT збирається в 30 04 00 01 00:
прохід 1 рахує done=4, прохід 2 підставляє адресу. Додавання NOP перед done
автоматично змінює ціль на 5.

Дослід Notes зі списком із трьох вузлів дав ADD / A / B, без витоків.
В окремій навмисно пошкодженій копії без destroy отримано реальний
LeakSanitizer-звіт: 80 byte(s) leaked in 2 allocation(s). Кількість видимих
витоків залежить від доступних коренів, тому не підміняємо її очікуваними
трьома вузлами. У готовій реалізації destroy виконується на успіху,
лексичній і синтаксичній помилках; тести багаторазово перевіряють усі шляхи.

Обов'язковий [Scanning — Robert Nystrom](https://craftinginterpreters.com/scanning.html)
прочитано. У нашій мові окремо враховано повне зчитування лексеми, рядок
помилки й явний EOF; на відміну від ширшої мови розділу, немає рядкових
літералів, колонок діагностики та відновлення сканування після помилки.

### Перевірки та підсумкове спостереження

Офіційного checks/lab-08.txt немає; тести Lab08 власні, це зазначено в CHECKS.md.
В обох конфігураціях EMBER_LINKED_STACK=OFF/ON пройшли 11/11 CTest із
ASAN_OPTIONS=detect_leaks=1. Серед перевірок: 49 форм ISA й спільні розміри,
повний оберт усіх .asm через сирий .bin і дизасемблер, мітки вперед/назад,
макроси, malformed input,2048 байтів коду, час життя вузлів, регресія Lab01–07.

Найбільш неочевидне спостереження за розвитком проєкту: правильний байт
ще не гарантує правильного алгоритму. LOAD нуля не встановлює Z; повернення
адреси на один байт раніше змінює потік виконання; POP може повернути
правильне число, але з неправильної рамки стека. Видимі dump/regs/stack
і невеликі відтворювані програми допомогли перевіряти саме ці зв'язки.
Мітки наприкінці курсу прибрали ручний перерахунок адрес і дали зосередитися
на рекурсії, яка раніше губилася за hex-байтами.
