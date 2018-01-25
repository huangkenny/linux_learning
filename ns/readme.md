# How to find namespaces on the host
- https://www.opencloudblog.com/?p=251
- http://man7.org/linux/man-pages/man7/namespaces.7.html
- execute as root
- You get the list of attached namespaces of PID 1. There are default namespace (the number in the brackets is a inode number
  In order to find other namespaces with attached processes in the system, we use these entries of the PID=1 as a reference.
  Any process or thread in the system, which has not the same namespace ID as PID=1 is not belonging to the DEFAULT namespace.
```
list -l /proc/1/ns/
readlink /proc/$$/ns/uts
```

# clone a process in a new pid namespace
```
[kenny@localhost ns]$ sudo ./a.out 
[sudo] password for kenny: 
pid in parent namespace: 20478
pid child namespace: 1
Type q to exit or nsenter to enter the namespace
```

Default pid namespace
```
[kenny@localhost container]$ sudo readlink /proc/1/ns/pid
pid:[4026531836]
```

process 20478 is in a new pid namespace
```
[kenny@localhost container]$ sudo readlink /proc/20478/ns/pid
pid:[4026532825]
```

Execute /bin/bash (default program to run by nsenter) in the pid namespace of a.out program  
```
[kenny@localhost ns]$ sudo nsenter -p -t 20478

[root@localhost]# ps
   PID TTY          TIME CMD
 21639 pts/7    00:00:00 sudo
 21661 pts/7    00:00:00 nsenter
 21662 pts/7    00:00:00 bash
 21751 pts/7    00:00:00 ps
```
Pid in the new pid namespace
```
[root@localhost kenny]# echo $$
50
```

nsenter is in the default pid namespace
```
[kenny@localhost]$ sudo readlink /proc/21661/ns/pid
pid:[4026531836]
```

the bash forked by nsenter is in the pid namespace of the a.out program 
```
[kenny@localhost]$ sudo readlink /proc/21662/ns/pid
pid:[4026532825]
```

Exit the /bin/bash
```
[root@localhost]# exit
logout
```
