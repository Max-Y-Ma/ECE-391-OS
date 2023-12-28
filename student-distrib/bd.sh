#!/bin/bash

GDB_SCRIPT="mp3.gdb"
GDB_EXIT_SCRIPT="mp3_exit.gdb"
PROG="bootimg"

make clean && make dep && sudo make \
&& echo -e "\e[1;32m\n\n\nOpen DEBUG QEMU Machine!\e[0m" \
&& echo -e "\e[1;32mThen Press Enter to GDB into QEMU...\e[0m" && read \
&& gdb -q -x "$GDB_SCRIPT" "$PROG" \
&& gdb -q -x "$GDB_EXIT_SCRIPT" "$PROG" \
&& echo -e "\e[1;32m~ Dead & Locked!\e[0m"