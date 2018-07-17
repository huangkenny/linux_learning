# Old, but good doc to understand how block device works
https://yannik520.github.io/blkdevarch.html

# Build
$ make

# Install 
```
$ sudo insmod my_brd.ko

$ dmesg -k -w
[ 2102.355392] my_brd: loading out-of-tree module taints kernel.
[ 2102.357113] Device name my_ramdisk major 252
[ 2102.361050] brd: module loaded

$ lsmod | grep my_brd
my_brd                 16384  0 

```
# Unload
$ sudo rmmod my_brd

