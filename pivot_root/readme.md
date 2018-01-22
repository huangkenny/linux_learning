#pivot_root example
```
mkdir /new-root
mount -n -t tmpfs -o size=500M none /new-root
cd /home/kenny/container/docker/c1/rootfs
find . -depth -xdev -print | cpio -pd --quiet /new-root
cd /new-root
mkdir old-root
unshare -m
pivot_root . old-root
```
