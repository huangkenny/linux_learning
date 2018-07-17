# Compile
$ make

# Observe
$ dmesg -k -w

# install
```
$ sudo insmod helloworld.ko
$ lsmod | grep helloworld
```

# clean
```
$ sudo rmmod helloworld
$ make clean
```
