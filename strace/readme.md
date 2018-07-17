# Reference
https://www.tldp.org/LDP/lkmpg/2.6/lkmpg.pdf

# strace is a handy program that gives you details about what system calls a program is making
```
$ gcc -o hello hello.c

$ sudo yum install strace
$ strace ./hello
........
write(1, "hello world\n", 12hello world
$ man 2 write
```

