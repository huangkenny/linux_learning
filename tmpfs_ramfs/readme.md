# Reference
[Overview](https://www.thegeekstuff.com/2008/11/overview-of-ramfs-and-tmpfs-on-linux/)
https://www.kernel.org/doc/Documentation/filesystems/ramfs-rootfs-initramfs.txt

# Examples
```
sudo mkdir -p /mnt/tmp
sudo mount -t tmpfs -o size=20m tmpfs /mnt/tmp
cd /mnt/tmp
mkdir src
echo "hello world" > file1
df
sudo umount /mnt/tmp
ls /mnt/tmp
```

# Comparison of ramfs and tmpfs
Experimentation	                        | Tmpfs              | Ramfs
--------------------------------------- | ------------------ | -----
Fill maximum space and continue writing	| Will display error | Will continue writing
Fixed Size                              | Yes                | No
Uses Swap                               | Yes                | No
Volatile Storage                        | Yes                | Yes
