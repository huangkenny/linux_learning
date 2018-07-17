# Compile
$ make

# Observe
$ dmesg -k -w

# install
```
$ sudo insmod chardev.ko
[  382.789848] I was assigned major number 243. To talk to
[  382.789849] the driver, create a dev file with
[  382.789850] 'mknod /dev/chardev c 243 0'.
[  382.789850] Try various minor numbers. Try to cat and echo to
[  382.789850] the device file.
[  382.789850] Remove the device file and module when done.

$ lsmod | grep chardev
```
# chardev is registered with linux: major 243 
$ cat /proc/devices | grep chardev
243 chardev

# create a device node
$ sudo mknod /dev/chardev c 243 0
$ ls -l /dev/chardev 
crw-r--r--. 1 root root 243, 0 Apr 13 07:16 /dev/chardev

# cat /dev/chardev innvoke fops open, read and release functions 
# static struct file_operations fops = {
#  .read = device_read,
#  .open = device_open,
#  .release = device_release
$ cat /dev/chardev
I already told you 0 times Hello world!

[ 2044.573183] device_open
[ 2044.573191] device_read
[ 2044.573198] device_read
[ 2044.573201] device_release




# clean
```
$ sudo rmmod chardev
[ 5363.300960] unregister_chrdev

$ cat /dev/chardev
cat: /dev/chardev: No such device or address

$ make clean
```
