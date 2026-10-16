# ember — віртуальний комп'ютер на C++17

Навчальний проєкт курсу «Основи програмування»: від пам'яті на 4096 байтів до процесора й асемблера.
Поточний етап — **Lab 03, Advanced**: адресація, H, LOAD/STORE, run, дамп об’єкта. Попередній етап: CPU, побітовий ALU, прапорці, step/regs і дзеркало стану. Lab 01: перевірений доступ до пам'яті, hex/ASCII-дамп, чотири представлення байта, inc та set16.

## Збірка й запуск

Перевірено в Ubuntu 26.04 / WSL2, x86_64, GCC 15.2.0, CMake 4.2.3.
На Windows відкрийте проєкт через VS Code WSL; команди виконуйте в Ubuntu.

~~~bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/ember
~~~

C++17, -Wall -Wextra -Werror; у Debug увімкнені -fsanitize=address,undefined, -fno-omit-frame-pointer і -g.
Попередження та санітайзери не вимкнені.

~~~text
ember 0.3 - 4096 bytes of memory you can see. Type `help`.
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
run має стелю100000 кроків і друкує кількість виконаних кроків.
instruction_size задає розмір, step перевіряє всі операнди ДО читання.
Адреса>=4096 зупиняє CPU повідомленням. H — 16-бітний регістр,
INCH/DECH виконують арифметику modulo65536; звертатися поза пам'яттю це не дозволяє.
Жодна інструкція пересилання не змінює прапорці.

### Розділені код і дані

Абсолютна програма22 00 08 02 00: LOAD A,[0x800]; OUT; HALT. Дані — ABC і нуль:

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
LOADH займає3 байти: PC0→3. LOAD A,[H] дає65; INCH змінює H2048→2049;
друге читання дає66. HALT завершується на PC9.

### Хост і гість

CPU.mem — Memory* хоста, справжня адреса коробки у процесі C++.
PC/H — uint16_t, адреси гостя. H=0x800 означає mem.data[2048], а не адресу0x800 ОС.
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
Рядки:65/65 і66 через *p; масив10/20/30; крок1 елемент і4 байти; sizeof8/4/8;
після копії1, після вказівника2; біти1.0f — 3f800000 у цьому середовищі.

### M4: ASan і виправлення

Окрема програма записала за індексом4096 у масив на4096 байтів.
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
programs/lab-03-copy.txt копіює ABCD із0x800–0x803 у0x810–0x813.
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
C++-тест звіряє всі65536 значень get16/set16 на останній парі, відмову на size_t max та незмінність Z/N/C при пересиланнях. Регресія Lab01–02 пройшла; позитивні сценарії без діагностики санітайзерів.
