# Reference
[md syntax](https://help.github.com/articles/basic-writing-and-formatting-syntax/)
[Namespace overview](http://man7.org/linux/man-pages/man7/namespaces.7.html)

# Update to new kernel (that supports user namespace) 
(https://wiki.mikejung.biz/CentOS_7#Upgrade_CentOS_7_Kernel_to_3.17)

# Check run-time environment
```
cat /proc/version
uname -rs
id  -u
id -g 
```

# Build the example code 
```
wget http://man7.org/tlpi/code/online/dist/namespaces/userns_child_exec.c
gcc userns_child_exec.c -o userns_child_exec
```

# Experiment with namespaces
```
./userns_child_exec -v -p -m -U -M '0 1000 1' -G '0 1000 1' bash
```

# From parent namespace
## uid 0 is mapped to 1000
```
[kenny@localhost ~]$ id -u
1000
[kenny@localhost ~]$ id -g
1000
[kenny@localhost ~]$ cat /proc/7971/uid_map 
         0       1000          1
```

## /proc/$$/ns has sybolic link to a processes namespace
## Below says the child process is in a new mnt, pid, and user namespaces from a bash in the default namespaces  
```
[kenny@localhost namespaces]$ sudo readlink /proc/$$/ns/*
cgroup:[4026531835]
ipc:[4026531839]
mnt:[4026531840]
net:[4026531993]
pid:[4026531836]
pid:[4026531836]
user:[4026531837]
uts:[4026531838]
[kenny@localhost namespaces]$ sudo readlink /proc/7971/ns/*
cgroup:[4026531835]
ipc:[4026531839]
mnt:[4026532579]
net:[4026531993]
pid:[4026532580]
pid:[4026532580]
user:[4026532578]
uts:[4026531838]
```
# From chile namespace
```
[root@localhost namespaces]# id -u
0
[root@localhost namespaces]# id -g
0
[root@localhost namespaces]# whoami
root
[root@localhost namespaces]# echo $$
1
```
## Mount the proc filesystem and list all processes visible in the child PID namespace
```
[root@localhost namespaces]# mount -t proc proc /proc
[root@localhost namespaces]# ps ax
   PID TTY      STAT   TIME COMMAND
     1 pts/3    S      0:00 bash
    19 pts/3    R+     0:00 ps ax
[root@localhost namespaces]# ls /proc/
1          cgroups   diskstats    fs          kcore        kpageflags  modules       partitions   softirqs       thread-self  vmstat
20         cmdline   dma          interrupts  keys         loadavg     mounts        sched_debug  stat           timer_list   zoneinfo
........
[root@localhost namespaces]# cat /proc/$$/status
Name:   bash
Pid:    1
PPid:   0
Uid:    0       0       0       0
Gid:    0       0       0       0

```
## Create a new process in the child namespaces
### The pid is 19,  

```
[root@localhost namespaces]# sleep 300&
[root@localhost namespaces]# pstree -p
bash(1)─┬─pstree(20)
        └─sleep(19)

```
### In parent PID namespaces, pid is 99925. the sleep process is in the same namespaces as the child process
```
[kenny@localhost ~]$ ps -elf | grep sleep
0 S kenny     99925  98648  0  80   0 - 26979 hrtime 18:52 pts/2    00:00:00 sleep 300
[kenny@localhost ~]$ readlink /proc/99925/ns/*
cgroup:[4026531835]
ipc:[4026531839]
mnt:[4026532579]
net:[4026531993]
pid:[4026532580]
pid:[4026532580]
user:[4026532578]
uts:[4026531838]
/proc/101263/uid_map
[kenny@localhost ~]$ cat /proc/99925/uid_map 
         0       1000          1
[kenny@localhost ~]$ cat /proc/99925/gid_map 
         0       1000          1
```

## Bind mount a directory in the child namespaces is not visiable in the parent namespace
## [Good article on bind mount use cases](https://unix.stackexchange.com/questions/198590/what-is-a-bind-mount) 
### In child namespace
```
[root@localhost home]# mount --bind /home/kenny/ /root
[root@localhost home]# ls /root/
container  data_science  Desktop  ex_protobuf  github  go  linux  protobuf  rpm  rpmbuild  websocket
```
### In parent namespace
```
[kenny@localhost home]$ sudo ls /root/
anaconda-ks.cfg  cmake_3_6_1
```
