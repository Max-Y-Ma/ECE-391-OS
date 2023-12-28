#!/bin/bash

GDB_SCRIPT="mp3_fs.gdb"
GDB_EXIT_SCRIPT="mp3_exit.gdb"
PROG="bootimg"

make clean

cd ../music && make clean && make all && cp music ../user_tests/fs/music && cd ../student-distrib
cd ../missile && make clean && make all && cp missile ../user_tests/fs/missile && cd ../student-distrib
cd ../write_test && make clean && make all && cp write_test ../user_tests/fs/write_test && cd ../student-distrib
cd ../echo && make clean && make all && cp echo1 ../user_tests/fs/echo1 && cd ../student-distrib
cd ../echo2 && make clean && make all && cp echo2 ../user_tests/fs/echo2 && cd ../student-distrib
cd ../backspace && make clean && make all && cp backspace ../user_tests/fs/backspace && cd ../student-distrib

cd ../user_tests && make clean && make all && cd ../student-distrib/

make dep && sudo make fs_test \
&& echo -e "\e[1;32mYou are debugging user programs!\e[0m" \
&& echo -e "\e[1;32mType name of user program to add symbols!\e[0m" \
&& echo -e "\e[1;32mType 'clr' to remove symbols!\e[0m" \
&& echo -e "\e[1;32mEx: (gdb) testprint\e[0m" \
&& echo -e "\e[1;32mEx: (gdb) b ece391testprint.c:main\e[0m" \
&& echo -e "\e[1;32mEx: (gdb) continue\e[0m" \
&& echo -e "\e[1;32m\n\nOpen DEBUG QEMU Machine!\e[0m" \
&& echo -e "\e[1;32mThen Press Enter to GDB into QEMU...\e[0m" && read \
&& gdb -q -x "$GDB_SCRIPT" "$PROG" \
&& gdb -q -x "$GDB_EXIT_SCRIPT" "$PROG" \
&& echo -e "\e[1;32m~ Dead & Locked!\e[0m"