```
$ sudo losetup /dev/loop0 foobar 
$ udevadm monitor --env
monitor will print the received events for:
UDEV - the event which udev sends out after rule processing
KERNEL - the kernel uevent

KERNEL[10032.322051] change   /devices/virtual/block/loop0 (block)
ACTION=change
DEVNAME=/dev/loop0
DEVPATH=/devices/virtual/block/loop0
DEVTYPE=disk
MAJOR=7
MINOR=0
SEQNUM=4745
SUBSYSTEM=block

UDEV  [10032.332910] change   /devices/virtual/block/loop0 (block)
ACTION=change
DEVNAME=/dev/loop0
DEVPATH=/devices/virtual/block/loop0
DEVTYPE=disk
MAJOR=7
MINOR=0
SEQNUM=4745
SUBSYSTEM=block
TAGS=:systemd:
USEC_INITIALIZED=84527892

KERNEL[10032.341751] change   /devices/virtual/block/loop0 (block)
ACTION=change
DEVNAME=/dev/loop0
DEVPATH=/devices/virtual/block/loop0
DEVTYPE=disk
MAJOR=7
MINOR=0
SEQNUM=4746
SUBSYSTEM=block

UDEV  [10032.347926] change   /devices/virtual/block/loop0 (block)
ACTION=change
DEVLINKS=/dev/disk/by-uuid/d3351e34-0ead-48b3-ae73-02e6a6be59b1
DEVNAME=/dev/loop0
DEVPATH=/devices/virtual/block/loop0
DEVTYPE=disk
ID_FS_TYPE=ext2
ID_FS_USAGE=filesystem
ID_FS_UUID=d3351e34-0ead-48b3-ae73-02e6a6be59b1
ID_FS_UUID_ENC=d3351e34-0ead-48b3-ae73-02e6a6be59b1
ID_FS_VERSION=1.0
MAJOR=7
MINOR=0
SEQNUM=4746
SUBSYSTEM=block
TAGS=:systemd:
USEC_INITIALIZED=84527892
```
# Add a farboo link 
```
$ sudo vi /etc/udev/rules.d/60-foobar.rules
ENV{DEVNAME}=="/dev/loop0", SYMLINK+="disk/by-label/foobar"

$ udevadm test /dev/loop0
Reading rules file: /etc/udev/rules.d/60-foobar.rules
........


$ sudo losetup /dev/loop0 foobar
$ udevadm monitor --env
UDEV  [24073.586168] change   /devices/virtual/block/loop0 (block)
ACTION=change
DEVLINKS=/dev/disk/by-label/foobar /dev/disk/by-uuid/d3351e34-0ead-48b3-ae73-02e6a6be59b1
DEVNAME=/dev/loop0
DEVPATH=/devices/virtual/block/loop0
DEVTYPE=disk
ID_FS_TYPE=ext2
ID_FS_USAGE=filesystem
ID_FS_UUID=d3351e34-0ead-48b3-ae73-02e6a6be59b1
ID_FS_UUID_ENC=d3351e34-0ead-48b3-ae73-02e6a6be59b1
ID_FS_VERSION=1.0
MAJOR=7
MINOR=0
SEQNUM=4790
SUBSYSTEM=block
TAGS=:systemd:
USEC_INITIALIZED=84527892

$ ls -l /dev/disk/by-label/foobar 
lrwxrwxrwx. 1 root root 11 Mar 16 13:56 /dev/disk/by-label/foobar -> ../../loop0

```
