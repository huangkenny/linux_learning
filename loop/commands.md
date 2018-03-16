```
dd if=/dev/zero of=foobar count=1024 bs=1024
sudo losetup /dev/loop0 foobar 
losetup -v /dev/loop0
sudo mkfs -v -t ext2 /dev/loop0 1024
mkdir filesys0
sudo mount -t ext2 /dev/loop0 ~/filesys0
cd filesys0/
```
