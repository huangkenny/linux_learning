# Reference
https://trapsink.com/wiki/Device_Mapper_Mechanics

# First, look at all scsi devices we have on the system
```
$ lsscsi
[2:0:0:0]    disk    ATA      INTEL SSDSCKHB34 EM05  /dev/sda 
[6:0:0:0]    disk    ATA      INTEL SSDSCKHB12 EM05  /dev/sdb 
[7:0:0:0]    disk    SAMSUNG  PA33N960 EMC960  EQL3  /dev/sdc 
[7:0:1:0]    disk    SAMSUNG  PA33N960 EMC960  EQL3  /dev/sdd 
[7:0:2:0]    disk    SAMSUNG  PA33N960 EMC960  EQL3  /dev/sde 
[7:0:3:0]    disk    SAMSUNG  PA33N960 EMC960  EQL3  /dev/sdf 
[7:0:4:0]    disk    SAMSUNG  PA33N960 EMC960  EQL3  /dev/sdg 
[7:0:5:0]    disk    SAMSUNG  PA33N960 EMC960  EQL3  /dev/sdh 
[7:0:6:0]    disk    SAMSUNG  PA33N960 EMC960  EQL3  /dev/sdi 
[7:0:7:0]    disk    SAMSUNG  PA33N960 EMC960  EQL3  /dev/sdj 
[7:0:8:0]    enclosu EMC      ESES Enclosure   0001  -        

$ sudo sgdisk -p /dev/sdc
Creating new GPT entries.
Disk /dev/sdc: 1876951040 sectors, 895.0 GiB
Logical sector size: 512 bytes
Disk identifier (GUID): 3BE5115D-024D-452E-B2B5-345782DDEF35
Partition table holds up to 128 entries
First usable sector is 34, last usable sector is 1876951006
Partitions will be aligned on 2048-sector boundaries
Total free space is 1876950973 sectors (895.0 GiB)

$ sudo sgdisk -p /dev/sdd
Creating new GPT entries.
Disk /dev/sdd: 1876951040 sectors, 895.0 GiB
Logical sector size: 512 bytes
Disk identifier (GUID): 938B38CC-385B-47DB-9E25-A47509FE4CBB
Partition table holds up to 128 entries
First usable sector is 34, last usable sector is 1876951006
Partitions will be aligned on 2048-sector boundaries
Total free space is 1876950973 sectors (895.0 GiB)
```

# Create a virtual device of 100 blooks on disk /dev/sdc (starting block 2048) 
```
$ sudo dmsetup create xyz1 --table "0 100 linear /dev/sdc 2048"
## xyz1 device with marjor 254 and minor 0
$ sudo dmsetup ls 
xyz1    (254:0)
$ sudo dmsetup info
$ sudo dmsetup info xyz1
$ ls /dev/mapper/
$ ls -og /dev/mapper/xyz1 
lrwxrwxrwx 1 7 Jul 13 20:08 /dev/mapper/xyz1 -> ../dm-0

$ sudo dmsetup table
xyz1: 0 100 linear 8:32 2048

# 100 block is 50K. So, 1 block is 512 bytes.
$ lsblk  
sdc         8:32   0   895G  0 disk      
`-xyz1    254:0    0    50K  0 dm  

# Create another virtual device 
$ sudo dmsetup create xyz2 --table "0 200 linear /dev/sdd 2048"

It created a virtual device with marjor 254 adn minor 1
$ sudo dmsetup ls                                              
xyz2    (254:1)
xyz1    (254:0)

$ sudo dmsetup table
xyz2: 0 200 linear 8:48 2048
xyz1: 0 100 linear 8:32 2048

$ lsblk  
sdc         8:32   0   895G  0 disk      
`-xyz1    254:0    0    50K  0 dm        
sdd         8:48   0   895G  0 disk      
`-xyz2    254:1    0   100K  0 dm        

# Write some zero's to xyz1
$ sudo dmstats create /dev/mapper/xyz1
xyz1: Created new region with 1 area(s) as region ID 0

$ sudo dmstats list                                      
Name             RgID RgSta RgSize #Areas ArSize ProgID 
xyz1                0     0 50.00k      1 50.00k dmstats

$ sudo dmstats report
Name             RgID ArID ArStart ArSize RMrg/s WMrg/s R/s  W/s  RSz/s WSz/s AvgRqSz QSize Util% AWait RdAWait WrAWait
xyz1                0    0       0 50.00k   0.00   0.00 0.00 0.00     0     0       0  0.00  0.00  0.00    0.00    0.00

$ sudo dd if=/dev/zero of=/dev/mapper/xyz1 bs=512 count=100
100+0 records in
100+0 records out
51200 bytes (51 kB, 50 KiB) copied, 0.00845326 s, 6.1 MB/s

$ sudo dmstats report
Name             RgID ArID ArStart ArSize RMrg/s WMrg/s R/s   W/s   RSz/s  WSz/s  AvgRqSz QSize Util% AWait RdAWait WrAWait
xyz1                0    0       0 50.00k   0.00   0.00 32.00 25.00 78.00k 50.00k   2.00k  0.03  0.90  0.58    0.25    1.00

$ sudo dmstats delete /dev/mapper/xyz1 --allregions
$ sudo dmstats report

# Remove the 2 small virtual device we created
$ sudo dmsetup remove /dev/mapper/xyz1
$ sudo dmsetup ls  
xyz2    (254:1)
$  sudo dmsetup ls 
No devices found

$ lsblk
sdc         8:32   0   895G  0 disk      
sdd         8:48   0   895G  0 disk 
```

# Create a filesystem spans two physical block devices
# Because this requires two lines to feed dmsetup (one line for each disk), we create the mapping in a text file:
# Virtual start of the 2nd disk is the same as the ending of the first. 
```
$ cat linear.table 
0 100 linear /dev/sdc 2048
100 200 linear /dev/sdd 2048

$ sudo dmsetup create concat_disks linear.table
$ sudo dmsetup ls
concat_disks    (254:0)

$ sudo dmsetup info /dev/mapper/concat_disks
Name:              concat_disks
State:             ACTIVE
Read Ahead:        256
Tables present:    LIVE
Open count:        0
Event number:      0
Major, minor:      254, 0
Number of targets: 2

# One 50K disk (100 block of 512 bytes) plus one 100K disk, we get a 150K disk.
$ lsblk
sdc              8:32   0   895G  0 disk      
`-concat_disks 254:0    0   150K  0 dm        
sdd              8:48   0   895G  0 disk      
`-concat_disks 254:0    0   150K  0 dm        

# Create a filesystem on concat_disks 
$ sudo mkfs.ext4 -v /dev/mapper/concat_disks 
mke2fs 1.43.3 (04-Sep-2016)
fs_types for mke2fs.conf resolution: 'ext4', 'floppy'

Filesystem too small for a journal
Filesystem label=
OS type: Linux
Block size=1024 (log=0)
Fragment size=1024 (log=0)
Stride=8 blocks, Stripe width=8 blocks
24 inodes, 148 blocks
7 blocks (4.73%) reserved for the super user
First data block=1
Maximum filesystem blocks=262144
1 block group
8192 blocks per group, 8192 fragments per group
24 inodes per group

Allocating group tables: done                            
Writing inode tables: done                            
Writing superblocks and filesystem accounting information: done

$ sudo mkdir /mnt/concat_disks
$ sudo mount /dev/mapper/concat_disks /mnt/concat_disks/
$ ls /mnt/concat_disks/                                 
lost+found

$ sudo chmod go+xrw /mnt/concat_disks/
$ echo "test file 1" > /mnt/concat_disks/test_file1
$ cat /mnt/concat_disks/test_file1
test file 1

$ dd if=/dev/zero of=/mnt/concat_disks/test_file2 bs=1K count=100
100+0 records in
100+0 records out
102400 bytes (102 kB, 100 KiB) copied, 0.000824538 s, 124 MB/s

$ ls -l /mnt/concat_disks/          
total 113
drwx------ 2 root root  12288 Jul 16 15:07 lost+found
-rw-r--r-- 1 core core     12 Jul 16 15:17 test_file1
-rw-r--r-- 1 core core 102400 Jul 16 15:19 test_file2

# The maps are disassembled and recreated as part of the testing to simulate what will happen when the server
# is rebooted - device maps are in memory only so in real use startup/shutdown scripts would be required to
# implement the above correctly. Remove the virtual device doesn't change the backing/target devices. We also
# tested giving the device map a randomly different name the second as a test.

$ sudo umount /mnt/concat_disks
$ sudo dmsetup remove concat_disks

$ sudo dmsetup ls
No devices found

$ lsblk
sdc         8:32   0   895G  0 disk      
sdd         8:48   0   895G  0 disk      

$ sudo dmsetup create foobar linear.table
$ sudo dmsetup info foobar     
Name:              foobar
State:             ACTIVE
Read Ahead:        256
Tables present:    LIVE
Open count:        0
Event number:      0
Major, minor:      254, 0
Number of targets: 2
$ lsblk
sdc         8:32   0   895G  0 disk      
`-foobar  254:0    0   150K  0 dm        
sdd         8:48   0   895G  0 disk      
`-foobar  254:0    0   150K  0 dm        

$ sudo mkdir /mnt/foobar
$ sudo mount /dev/mapper/foobar /mnt/foobar
ls -l /mnt/foobar
total 113
drwx------ 2 root root  12288 Jul 16 15:07 lost+found
-rw-r--r-- 1 core core     12 Jul 16 15:17 test_file1
-rw-r--r-- 1 core core 102400 Jul 16 15:19 test_file2

$ cat /mnt/foobar/test_file1
test file 1
```

# Striped Target
```
# syntax
# start length striped #stripes chunk_size device1 offset1 ... deviceN offsetN
  start
    starting block in virtual device
  length
    length of this segment
  #stripes
    number of stripes for the virtual device
  chunk_size
    number of sectors written to each stripe before switching to the next; must be power of 2 at least as big as the kernel page size
  device
    block device, referenced by the device name in the filesystem or by the major and minor numbers in the format major:minor.
  offset
    starting offset of the mapping on the device 
# When using striping technology design each group of data is written in a chunk
# that is a multiple of 2 and typically of a size that is optimized for the data.
# A chunk of 256k is very common for physical RAID controllers, 
# Create a strip of 10 chuncks (2560k) 

$ cat striped.table 
0 2621440 striped 2 256 /dev/sdc 2048 /dev/sdd 2048

$ sudo dmsetup create striped_disk striped.table 
$ sudo dmsetup ls
striped_disk    (254:1)
foobar  (254:0)

$ sudo dmsetup info striped_disk  
Name:              striped_disk
State:             ACTIVE
Read Ahead:        256
Tables present:    LIVE
Open count:        0
Event number:      0
Major, minor:      254, 1
Number of targets: 1


$ lsblk
sdc              8:32   0   895G  0 disk      
|-foobar       254:0    0   150K  0 dm        /mnt/foobar
`-striped_disk 254:1    0   1.3G  0 dm        
sdd              8:48   0   895G  0 disk      
|-foobar       254:0    0   150K  0 dm        /mnt/foobar
`-striped_disk 254:1    0   1.3G  0 dm        

$ sudo dmstats create striped_disk --areas 2
$ sudo dmstats list
Name             RgID RgSta RgSiz #Areas ArSize  ProgID 
striped_disk        0     0 1.25g      2 640.00m dmstats
$ sudo dmstats report
Name             RgID ArID ArStart ArSize  RMrg/s WMrg/s R/s  W/s  RSz/s WSz/s AvgRqSz QSize Util% AWait RdAWait WrAWait
striped_disk        0    0       0 640.00m   0.00   0.00 0.00 0.00     0     0       0  0.00  0.00  0.00    0.00    0.00
striped_disk        0    1 640.00m 640.00m   0.00   0.00 0.00 0.00     0     0       0  0.00  0.00  0.00    0.00    0.00

# Create filesystem
$ sudo mkfs.ext4 -v /dev/mapper/striped_disk 
mke2fs 1.43.3 (04-Sep-2016)
/dev/mapper/striped_disk contains a ext4 file system
        last mounted on /mnt/foobar on Mon Jul 16 15:52:59 2018
Proceed anyway? (y,n) y
fs_types for mke2fs.conf resolution: 'ext4'
Discarding device blocks: done                            
Filesystem label=
OS type: Linux
Block size=4096 (log=2)
Fragment size=4096 (log=2)
Stride=32 blocks, Stripe width=64 blocks
81920 inodes, 327680 blocks
16384 blocks (5.00%) reserved for the super user
First data block=0
Maximum filesystem blocks=335544320
10 block groups
32768 blocks per group, 32768 fragments per group
8192 inodes per group
Filesystem UUID: 68dded92-72f8-4e61-83ca-5ec193aa3401
Superblock backups stored on blocks: 
        32768, 98304, 163840, 229376, 294912

Allocating group tables: done                            
Writing inode tables: done                            
Creating journal (8192 blocks): done
Writing superblocks and filesystem accounting information: done 

# We have just destroied the filesystem we created on /dev/mapper/foobar
$ ls /mnt/foobar/

$ sudo mkdir /mnt/stripped_disk        
core@FNM00174900369-A ~ $ sudo mount /dev/mapper/striped_disk /mnt/stripped_disk/
mount: /mnt/stripped_disk: mount(2) system call failed: Structure needs cleaning.

$ sudo fsck.ext4 /dev/mapper/striped_disk -a
/dev/mapper/striped_disk contains a file system with errors, check forced.
/dev/mapper/striped_disk: 11/24 files (0.0% non-contiguous), 12/148 blocks

$ sudo mount /dev/mapper/striped_disk /mnt/stripped_disk/
$ ls /mnt/stripped_disk/
lost+found
$ sudo chmod go+xrw /mnt/stripped_disk/
$ df /mnt/stripped_disk/
Filesystem               1K-blocks  Used Available Use% Mounted on
/dev/mapper/striped_disk       140     4       127   4% /mnt/stripped_disk

# So, we have space for 127 1K blocks or 254 512-byte blocks 

$ dd if=/dev/zero of=/mnt/stripped_disk/testfile1 bs=512 count=254
254+0 records in
254+0 records out
130048 bytes (130 kB, 127 KiB) copied, 0.000964791 s, 135 MB/s

$ sudo dmstats report
Name             RgID ArID ArStart ArSize  RMrg/s WMrg/s R/s    W/s     RSz/s WSz/s   AvgRqSz QSize Util% AWait RdAWait WrAWait
striped_disk        0    0       0 640.00m   0.00   0.00 144.00 8438.00 4.95m 673.51m  80.50k  7.37 12.40  0.86    0.36    0.87
striped_disk        0    1 640.00m 640.00m   0.00   0.00 264.00   23.00 4.55m 640.09m   2.25m  0.16  6.20  0.55    0.33    3.13

# Make the chunk size 4 blocks, write read and write performance increased
$ cat striped.table 
0 2621440 striped 2 8 /dev/sdc 2048 /dev/sdd 2048
......
$ dd if=/dev/zero of=/mnt/stripped_disk/testfile1 bs=512 count=254
254+0 records in
254+0 records out
130048 bytes (130 kB, 127 KiB) copied, 0.000671879 s, 194 MB/s
$ sudo dmstats report
Name             RgID ArID ArStart ArSize  RMrg/s WMrg/s R/s    W/s     RSz/s WSz/s   AvgRqSz QSize Util%  AWait RdAWait WrAWait
striped_disk        0    0       0 640.00m   0.00   0.00  92.00 8458.00 2.68m 689.02m  82.50k 43.44 100.00  5.08    0.33    5.13
striped_disk        0    1 640.00m 640.00m   0.00   0.00 148.00   23.00 2.53m 640.09m   3.76m  0.10   4.40  0.60    0.39    1.96

```
# Mirror
```
// Mirror requires a log device, a space to record metadata about writes.
// This example constructs disk partitions and use a technique generally known as "mirroring the mirror".
// First we'll prep out 2 partition on each physical block device, one to store log data to disk - so that
// when rebooting/recreating the mirror has it's data on each leg, otherwise a bootstrap would be required
// again - and one to store data. They will be exactly the same on both disks as this is a mirror configuration.

// Wipe out any existing partition table on disk /dev/sdc
$ sudo sgdisk -Z /dev/sdc
Creating new GPT entries.
GPT data structures destroyed! You may now partition the disk using fdisk or
other utilities.

// Make 8M (16384 sectors) partiton for log and 80M for data 
$ sudo parted /dev/sdc mktable gpt
$ sudo parted /dev/sdc mkpart primary ext3 2048s 18432s
$ sudo parted /dev/sdc mkpart primary ext3 20480s 184320s

$ sudo parted /dev/sdc print       
Model: SAMSUNG PA33N960 EMC960 (scsi)
Disk /dev/sdc: 961GB
Sector size (logical/physical): 512B/4096B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name     Flags
 1      1049kB  9438kB  8389kB  ext4         primary
 2      10.5MB  94.4MB  83.9MB               primary

 $ sudo parted /dev/sdc unit s print
Model: SAMSUNG PA33N960 EMC960 (scsi)
Disk /dev/sdc: 1876951040s
Sector size (logical/physical): 512B/4096B
Partition Table: gpt
Disk Flags: 

Number  Start   End      Size     File system  Name     Flags
 1      2048s   18432s   16385s   ext4         primary
 2      20480s  184320s  163841s               primary

// Do the same for /dev/sdd
$ sudo sgdisk -Z /dev/sdd
$ sudo parted /dev/sdd mktable gpt
$ sudo parted /dev/sdd mkpart primary ext3 2048s 18432s
$ sudo parted /dev/sdd mkpart primary ext3 20480s 184320s

$ lsblk
sdc         8:32   0   895G  0 disk      
|-sdc1      8:33   0     8M  0 part      
`-sdc2      8:34   0    80M  0 part      
sdd         8:48   0   895G  0 disk      
|-sdd1      8:49   0     8M  0 part      
`-sdd2      8:50   0    80M  0 part      

# build the maps to the mirror devices
// map table syntax
// start length mirror log_type #logargs logarg1 ... logargN #devs device1 offset1 ... deviceN offsetN
// start
//   starting block in virtual device
// length
//   length of this segment
// log_type
//    The possible log types and their arguments are as follows:
//    core
//      The mirror is local and the mirror log is kept in core memory. This log type takes 1 - 3 arguments:
//      regionsize [[no]sync] [block_on_error]
//    disk
//      The mirror is local and the mirror log is kept on disk. This log type takes 2 - 4 arguments:
//      logdevice regionsize [[no]sync] [block_on_error]
//    clustered_core
//      The mirror is clustered and the mirror log is kept in core memory. This log type takes 2 - 4 arguments:
//      regionsize UUID [[no]sync] [block_on_error]
//    clustered_disk
//      The mirror is clustered and the mirror log is kept on disk. This log type takes 3 - 5 arguments:
//      logdevice regionsize UUID [[no]sync] [block_on_error]
//      LVM maintains a small log which it uses to keep track of which regions are in sync with the mirror or mirrors.
//      The regionsize argument specifies the size of these regions.
//      In a clustered environment, the UUID argument is a unique identifier associated with the mirror log device so
//      that the log state can be maintained throughout the cluster.
//      The optional [no]sync argument can be used to specify the mirror as "in-sync" or "out-of-sync". 
//      The block_on_error argument is used to tell the mirror to respond to errors rather than ignoring them.
//  #log_args
//    number of log arguments that will be specified in the mapping
//  logargs
//    the log arguments for the mirror; the number of log arguments provided is specified by the #log-args
//    parameter and the valid log arguments are determined by the log_type parameter.
//  #devs
//    the number of legs in the mirror; a device and an offset is specified for each leg.
//  device
//    block device for each mirror leg, referenced by the device name in the filesystem or by the major and minor
//    numbers in the format major:minor. A block device and offset is specified for each mirror leg, as indicated
//    by the #devs parameter.
//  offset
//    starting offset of the mapping on the device. A block device and offset is specified for each mirror leg,
//    as indicated by the #devs parameter.

// Log table syntax
// <virtual start> <virtual size> core <params = 1> <param1 = size> <devs = 2> <dev1> <offset1> <dev2> <offset 2> <features = 1> <feature = handle_errors>
$ cat mirror_log.table 
0 8192 mirror core 1 1024 2 /dev/sdc1 0 /dev/sdd1 0 1 handle_errors

// Data table syntax
// <virtual start> <virtual size> disk <params = 2> >param1 = log dev> <param2 = size> <devs = 2> <dev1> <offset1> <dev2> <offset 2> <features = 1> <feature = handle_errors>

// A mirror device of 163840 sectors (80M) using /dev/mapper/mirror-log as log device, and /dev/sdc2 and /dev/sdd2 and backing(taget) disks 
$ cat mirror_data.table
0 163840 mirror disk 2 /dev/mapper/mirror_log 1024 2 /dev/sdc2 0 /dev/sdd2 0 1 handle_errors

// Create the log and data mirror devices
$ sudo dmsetup create mirror_log mirror_log.table
$ sudo dmsetup create mirror_data mirror_data.table 

$ sudo dmsetup info 
Name:              mirror_data
State:             ACTIVE
Read Ahead:        256
Tables present:    LIVE
Open count:        0
Event number:      1
Major, minor:      254, 1
Number of targets: 1

Name:              mirror_log
State:             ACTIVE
Read Ahead:        256
Tables present:    LIVE
Open count:        1
Event number:      1
Major, minor:      254, 0
Number of targets: 1

$ lsblk
sdc                 8:32   0   895G  0 disk      
|-sdc1              8:33   0     8M  0 part      
| `-mirror_log    254:0    0     4M  0 dm        
|   `-mirror_data 254:1    0    80M  0 dm        
`-sdc2              8:34   0    80M  0 part      
  `-mirror_data   254:1    0    80M  0 dm        
sdd                 8:48   0   895G  0 disk      
|-sdd1              8:49   0     8M  0 part      
| `-mirror_log    254:0    0     4M  0 dm        
|   `-mirror_data 254:1    0    80M  0 dm        
`-sdd2              8:50   0    80M  0 part      
  `-mirror_data   254:1    0    80M  0 dm        

// Create a file system on mirror_data and mount it on /mnt/mirror_data
$ sudo mkfs.ext4 -v /dev/mapper/mirror_data 
mke2fs 1.43.3 (04-Sep-2016)
fs_types for mke2fs.conf resolution: 'ext4', 'small'
Discarding device blocks: done                            
Filesystem label=
OS type: Linux
Block size=1024 (log=0)
Fragment size=1024 (log=0)
Stride=8 blocks, Stripe width=8 blocks
20480 inodes, 81920 blocks
4096 blocks (5.00%) reserved for the super user
First data block=1
Maximum filesystem blocks=33685504
10 block groups
8192 blocks per group, 8192 fragments per group
2048 inodes per group
Filesystem UUID: 06f463e3-b334-455d-9c0a-c3fd80b4b2d3
Superblock backups stored on blocks: 
        8193, 24577, 40961, 57345, 73729

Allocating group tables: done                            
Writing inode tables: done                            
Creating journal (4096 blocks): done
Writing superblocks and filesystem accounting information: done 

$ sudo mkdir /mnt/mirror_data
$ sudo mount /dev/mapper/mirror_data /mnt/mirror_data/
$ ls /mnt/mirror_data/
lost+found
$ sudo chmod go+xrw /mnt/mirror_data
$ echo "test file 1" > /mnt/mirror_data/test_file1
$ dd if=/dev/zero of=/mnt/mirror_data/test_file2 bs=512 count=100
ls -l /mnt/mirror_data/
total 63
drwx------ 2 root root 12288 Jul 18 01:55 lost+found
-rw-r--r-- 1 core core    12 Jul 18 02:01 test_file1
-rw-r--r-- 1 core core 51200 Jul 18 02:02 test_file2

// Remove the mirror devives and recreate. simulate reboot
$ sudo umount /mnt/mirror_data 
$ sudo dmsetup remove mirror_data       
$ sudo dmsetup remove mirror_log       
$ sudo dmsetup create mirror_log mirror_log.table 
$ sudo dmsetup create mirror_data mirror_data.table 
$ sudo mount /dev/mapper/mirror_data /mnt/mirror_data/
$ ls -l /mnt/mirror_data/
total 63
drwx------ 2 root root 12288 Jul 18 01:55 lost+found
-rw-r--r-- 1 core core    12 Jul 18 02:01 test_file1
-rw-r--r-- 1 core core 51200 Jul 18 02:02 test_file2

// This /dev/sdc2 and /dev/sdd2 are raw disks both containing the data we saved in the mirror  
// device. In a real situation where one member of the mirror goes offline, we can destroy the
// device maps and manually mount raw block device. Let's verify it.
$ sudo umount /mnt/mirror_data/
$ sudo dmsetup remove mirror_data
$ sudo dmsetup remove mirror_log
$ ls -l /mnt/mirror_data/
total 0

$ sudo mount /dev/sdc2 /mnt/mirror_data/
$ ls -l /mnt/mirror_data/
total 63
drwx------ 2 root root 12288 Jul 18 01:55 lost+found
-rw-r--r-- 1 core core    12 Jul 18 02:01 test_file1
-rw-r--r-- 1 core core 51200 Jul 18 02:02 test_file2
$ cat /mnt/mirror_data/test_file1
test file 1

$ sudo umount /mnt/mirror_data/
$ ls -l /mnt/mirror_data/
total 0
$ sudo mount /dev/sdd2 /mnt/mirror_data/
$ ls -l /mnt/mirror_data/
total 63
drwx------ 2 root root 12288 Jul 18 01:55 lost+found
-rw-r--r-- 1 core core    12 Jul 18 02:01 test_file1
-rw-r--r-- 1 core core 51200 Jul 18 02:02 test_file2

$ cat /mnt/mirror_data/test_file1
test file 1

```
