# Connect to target 
target remote 10.0.2.2:1234

# Alphabetical Order
define cat
    add-symbol-file ../user_tests/exe/cat.exe 0x08048094
end
define counter
    add-symbol-file ../user_tests/exe/counter.exe 0x08048094
end
define grep
    add-symbol-file ../user_tests/exe/grep.exe 0x08048094
end
define hello
    add-symbol-file ../user_tests/exe/hello.exe 0x08048094
end
define ls
    add-symbol-file ../user_tests/exe/ls.exe 0x08048094
end
define pingpong
    add-symbol-file ../user_tests/exe/pingpong.exe 0x08048094
end
define shell
    add-symbol-file ../user_tests/exe/shell.exe 0x08048094
end
define sigtest
    add-symbol-file ../user_tests/exe/sigtest.exe 0x08048094
end
define syserr
    add-symbol-file ../user_tests/exe/syserr.exe 0x08048094
end
define testprint
    add-symbol-file ../user_tests/exe/testprint.exe 0x08048094
end
define missile
    add-symbol-file ../missile/missile.exe 0x08048094
end
define write_test
    add-symbol-file ../write_test/write_test.exe 0x08048074
end 
define echo1
    add-symbol-file ../echo/echo1.exe 0x08048074
end 
define echo2
    add-symbol-file ../echo2/echo2.exe 0x08048074
end 
define backspace
    add-symbol-file ../backspace/backspace.exe 0x08048074
define music
    add-symbol-file ../music/music.exe 0x08048094
end 
define clr
    symbol-file
end