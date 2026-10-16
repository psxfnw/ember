# Перевірки ember

Офіційне джерело: Programming-Practice-Projects, знімок a713eac8354ef3fc3d6fad1c8c281046e04873f2.
checks/lab-01.txt … lab-05.txt і lab-07.txt скопійовано з курсу.
Офіційних lab-06.txt та lab-08.txt у цьому знімку немає.
Їх не підмінено файлами з вигаданим статусом «офіційні».

~~~bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir build --output-on-failure
~~~

Для Python CLI-перевірок потрібен Python3 (стандартна бібліотека).
Якщо Python3 відсутній, CMake повідомляє це і запускає лише 4 C++ тести;
повна перевірка тут містить 11 тестів. У перевіреному WSL Python3 присутній.

| Перевірка | Статус і призначення |
|---|---|
| checks/lab-01…05,07 | офіційні REPL-сценарії |
| alu-and-cpu | власний C++ тест усіх 65536 пар ALU, пам'яті, VRAM, ALLOC |
| stack-and-calls | власний C++ тест 256 байтів, LE16, CALL/RET/H, відмов і прапорців |
| assembler-and-lexer | власний C++ тест 49 форм, міток, макросів, лексики та часу життя |
| structure-layout | власне вимірювання розкладки об'єктів |
| check_cli | Lab01, нулі 4096 байтів, межі, усі 256 подань |
| check_machine | офіційні 01–03 плюс адресація і обрізані операнди |
| check_control | офіційний 04 плюс усі умови, пошук 8/відсутність, breakpoint |
| check_display | офіційний 05 плюс VRAM/OUTS/сортування/MMIO |
| check_records | власні Lab06: спрайти, швидкості, межі купи |
| check_stack | офіційний 07 плюс 0/1/6, рекурсія, ADD-mul і H |
| check_assembly | власні Lab08: програми, помилки рядків, bounce, двійковий оберт |

Альтернативний стек перевіряється тими самими тестами:

~~~bash
cmake -S . -B build-linked -DCMAKE_BUILD_TYPE=Debug -DEMBER_LINKED_STACK=ON
cmake --build build-linked
ASAN_OPTIONS=detect_leaks=1 ctest --test-dir build-linked --output-on-failure
~~~

Негативні сценарії CPU повинні дати власне повідомлення, а не помилку
санітайзера. Навмисні UB/витоки з Notes запускалися в окремих дослідах;
їхнього небезпечного коду немає у src або готових програмах.
Результат фінальної перевірки:11/11 для кожного стека, без повідомлень
санітайзерів у коректних сценаріях. Межі й відмови перевірені окремо.
