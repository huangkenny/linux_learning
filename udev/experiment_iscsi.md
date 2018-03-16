# Reference
man udev

# Monitor uevent
```
$ udevadm monitor --environment --udev
```
# Discover iscsi targets

```
sudo iscsiadm -m discovery -t st -p 10.240.65.24
10.240.65.25:3260,1 iqn.2015-10.com.dell:dellemc-cyclone-8dcc9d9494bf4996ad296944b51a3136
10.240.65.24:3260,1 iqn.2015-10.com.dell:dellemc-cyclone-49506fa0476e4db49c1832e5234e8e9a

udevadm monitor --environment --udev
monitor will print the received events for:
UDEV - the event which udev sends out after rule processing

UDEV  [19301.854397] add      /class/iscsi_endpoint (class)
ACTION=add
DEVPATH=/class/iscsi_endpoint
SEQNUM=12996
SUBSYSTEM=class
USEC_INITIALIZED=301848426

UDEV  [19301.854429] add      /class/iscsi_iface (class)
ACTION=add
DEVPATH=/class/iscsi_iface
SEQNUM=12997
SUBSYSTEM=class
USEC_INITIALIZED=301848722

UDEV  [19301.854441] add      /module/scsi_transport_iscsi (module)
ACTION=add
DEVPATH=/module/scsi_transport_iscsi
SEQNUM=12994
SUBSYSTEM=module
USEC_INITIALIZED=301847229

UDEV  [19301.854478] add      /class/iscsi_transport (class)
ACTION=add
DEVPATH=/class/iscsi_transport
SEQNUM=12995
SUBSYSTEM=class
USEC_INITIALIZED=301848191

UDEV  [19301.854503] add      /class/iscsi_host (class)
ACTION=add
DEVPATH=/class/iscsi_host
SEQNUM=12998
SUBSYSTEM=class
USEC_INITIALIZED=301848941

UDEV  [19301.854521] add      /bus/iscsi_flashnode (bus)
ACTION=add
DEVPATH=/bus/iscsi_flashnode
SEQNUM=13001
SUBSYSTEM=bus
USEC_INITIALIZED=301850362

UDEV  [19301.854541] add      /class/iscsi_connection (class)
ACTION=add
DEVPATH=/class/iscsi_connection
SEQNUM=12999
SUBSYSTEM=class
USEC_INITIALIZED=301849955

UDEV  [19301.854552] add      /class/iscsi_session (class)
ACTION=add
DEVPATH=/class/iscsi_session
SEQNUM=13000
SUBSYSTEM=class
USEC_INITIALIZED=301850136


UDEV  [19301.880691] add      /module/libiscsi (module)
ACTION=add
DEVPATH=/module/libiscsi
SEQNUM=13002
SUBSYSTEM=module
USEC_INITIALIZED=1880025

UDEV  [19301.884295] add      /module/libiscsi_tcp (module)
ACTION=add
DEVPATH=/module/libiscsi_tcp
SEQNUM=13003
SUBSYSTEM=module
USEC_INITIALIZED=1883977

UDEV  [19301.889200] add      /devices/virtual/iscsi_transport/tcp (iscsi_transport)
ACTION=add
DEVPATH=/devices/virtual/iscsi_transport/tcp
SEQNUM=13005
SUBSYSTEM=iscsi_transport
USEC_INITIALIZED=1888768

UDEV  [19301.889226] add      /module/iscsi_tcp (module)
ACTION=add
DEVPATH=/module/iscsi_tcp
SEQNUM=13004
SUBSYSTEM=module
USEC_INITIALIZED=1888626

```

# iscsi login
```
$ sudo iscsiadm -m node -p 10.240.65.24 --login
Logging in to [iface: default, target: iqn.2015-10.com.dell:dellemc-cyclone-49506fa0476e4db49c1832e5234e8e9a, portal: 10.240.65.24,3260] (multiple)
Login to [iface: default, target: iqn.2015-10.com.dell:dellemc-cyclone-49506fa0476e4db49c1832e5234e8e9a, portal: 10.240.65.24,3260] successful.

$ udevadm monitor --environment --udev
monitor will print the received events for:
UDEV - the event which udev sends out after rule processing

UDEV  [20221.999243] add      /devices/platform/host3 (scsi)
ACTION=add
DEVPATH=/devices/platform/host3
DEVTYPE=scsi_host
SEQNUM=13012
SUBSYSTEM=scsi
USEC_INITIALIZED=221965523

UDEV  [20221.999327] add      /devices/platform/host3/scsi_host/host3 (scsi_host)
ACTION=add
DEVPATH=/devices/platform/host3/scsi_host/host3
SEQNUM=13013
SUBSYSTEM=scsi_host
USEC_INITIALIZED=1969897

UDEV  [20221.999370] add      /devices/platform/host3/session1/iscsi_session/session1 (iscsi_session)
ACTION=add
DEVPATH=/devices/platform/host3/session1/iscsi_session/session1
SEQNUM=13015
SUBSYSTEM=iscsi_session
USEC_INITIALIZED=221970032

UDEV  [20221.999412] add      /devices/platform/host3/session1/connection1:0/iscsi_connection/connection1:0 (iscsi_connection)
ACTION=add
DEVPATH=/devices/platform/host3/session1/connection1:0/iscsi_connection/connection1:0
SEQNUM=13016
SUBSYSTEM=iscsi_connection
USEC_INITIALIZED=221970078

UDEV  [20221.999453] add      /devices/platform/host3/iscsi_host/host3 (iscsi_host)
ACTION=add
DEVPATH=/devices/platform/host3/iscsi_host/host3
SEQNUM=13014
SUBSYSTEM=iscsi_host
USEC_INITIALIZED=221969973

UDEV  [20222.050462] add      /devices/platform/host3/session1/target3:0:0 (scsi)
ACTION=add
DEVPATH=/devices/platform/host3/session1/target3:0:0
DEVTYPE=scsi_target
SEQNUM=13017
SUBSYSTEM=scsi
USEC_INITIALIZED=2041030

UDEV  [20222.125925] add      /devices/virtual/bdi/8:16 (bdi)
ACTION=add
DEVPATH=/devices/virtual/bdi/8:16
SEQNUM=13028
SUBSYSTEM=bdi
USEC_INITIALIZED=2125058

UDEV  [20222.174163] add      /devices/platform/host3/session1/target3:0:0/3:0:0:0 (scsi)
ACTION=add
DEVPATH=/devices/platform/host3/session1/target3:0:0/3:0:0:0
DEVTYPE=scsi_device
MODALIAS=scsi:t-0x0c
SEQNUM=13018
SUBSYSTEM=scsi
USEC_INITIALIZED=2041324

UDEV  [20222.194346] add      /devices/platform/host3/session1/target3:0:0/3:0:0:0/scsi_generic/sg2 (scsi_generic)
ACTION=add
DEVNAME=/dev/sg2
DEVPATH=/devices/platform/host3/session1/target3:0:0/3:0:0:0/scsi_generic/sg2
MAJOR=21
MINOR=2
SEQNUM=13020
SUBSYSTEM=scsi_generic
USEC_INITIALIZED=2041407

UDEV  [20222.194440] add      /devices/platform/host3/session1/target3:0:0/3:0:0:0/scsi_device/3:0:0:0 (scsi_device)
ACTION=add
DEVPATH=/devices/platform/host3/session1/target3:0:0/3:0:0:0/scsi_device/3:0:0:0
SEQNUM=13019
SUBSYSTEM=scsi_device
USEC_INITIALIZED=2041363

UDEV  [20222.194488] add      /devices/platform/host3/session1/target3:0:0/3:0:0:8 (scsi)
ACTION=add
DEVPATH=/devices/platform/host3/session1/target3:0:0/3:0:0:8
DEVTYPE=scsi_device
MODALIAS=scsi:t-0x00
SEQNUM=13022
SUBSYSTEM=scsi
USEC_INITIALIZED=2077212

UDEV  [20222.194540] add      /devices/platform/host3/session1/target3:0:0/3:0:0:0/bsg/3:0:0:0 (bsg)
ACTION=add
DEVNAME=/dev/bsg/3:0:0:0
DEVPATH=/devices/platform/host3/session1/target3:0:0/3:0:0:0/bsg/3:0:0:0
MAJOR=248
MINOR=2
SEQNUM=13021
SUBSYSTEM=bsg
USEC_INITIALIZED=2041452

UDEV  [20222.194590] add      /devices/platform/host3/session1/target3:0:0/3:0:0:8/scsi_disk/3:0:0:8 (scsi_disk)
ACTION=add
DEVPATH=/devices/platform/host3/session1/target3:0:0/3:0:0:8/scsi_disk/3:0:0:8
SEQNUM=13023
SUBSYSTEM=scsi_disk
USEC_INITIALIZED=2077389

UDEV  [20222.194636] bind     /devices/platform/host3/session1/target3:0:0/3:0:0:8 (scsi)
ACTION=bind
DEVPATH=/devices/platform/host3/session1/target3:0:0/3:0:0:8
DEVTYPE=scsi_device
DRIVER=sd
MODALIAS=scsi:t-0x00
SEQNUM=13024
SUBSYSTEM=scsi
USEC_INITIALIZED=2077446

UDEV  [20222.217883] add      /devices/platform/host3/session1/target3:0:0/3:0:0:8/scsi_device/3:0:0:8 (scsi_device)
ACTION=add
DEVPATH=/devices/platform/host3/session1/target3:0:0/3:0:0:8/scsi_device/3:0:0:8
SEQNUM=13025
SUBSYSTEM=scsi_device
USEC_INITIALIZED=2077660

UDEV  [20222.217970] add      /devices/platform/host3/session1/target3:0:0/3:0:0:8/bsg/3:0:0:8 (bsg)
ACTION=add
DEVNAME=/dev/bsg/3:0:0:8
DEVPATH=/devices/platform/host3/session1/target3:0:0/3:0:0:8/bsg/3:0:0:8
MAJOR=248
MINOR=3
SEQNUM=13027
SUBSYSTEM=bsg
USEC_INITIALIZED=2077789

UDEV  [20222.218028] add      /devices/platform/host3/session1/target3:0:0/3:0:0:8/scsi_generic/sg3 (scsi_generic)
ACTION=add
DEVNAME=/dev/sg3
DEVPATH=/devices/platform/host3/session1/target3:0:0/3:0:0:8/scsi_generic/sg3
MAJOR=21
MINOR=3
SEQNUM=13026
SUBSYSTEM=scsi_generic
USEC_INITIALIZED=2077731

UDEV  [20222.526658] add      /devices/platform/host3/session1/target3:0:0/3:0:0:8/block/sdb (block)
ACTION=add
DEVLINKS=/dev/disk/by-id/scsi-368ccf09800744a14b26d09fd41b7e9b5 /dev/disk/by-id/wwn-0x68ccf09800744a14b26d09fd41b7e9b5 /dev/disk/by-path/ip-10.240.65.24:3260-iscsi-iqn.2015-10.com.dell:dellemc-cyclone-49506fa0476e4db49c1832e5234e8e9a-lun-8
DEVNAME=/dev/sdb
DEVPATH=/devices/platform/host3/session1/target3:0:0/3:0:0:8/block/sdb
DEVTYPE=disk
ID_BUS=scsi
ID_MODEL=HADA2016
ID_MODEL_ENC=HADA2016\x20\x20\x20\x20\x20\x20\x20\x20
ID_PATH=ip-10.240.65.24:3260-iscsi-iqn.2015-10.com.dell:dellemc-cyclone-49506fa0476e4db49c1832e5234e8e9a-lun-8
ID_PATH_TAG=ip-10_240_65_24_3260-iscsi-iqn_2015-10_com_dell_dellemc-cyclone-49506fa0476e4db49c1832e5234e8e9a-lun-8
ID_REVISION=SCSI
ID_SCSI=1
ID_SCSI_SERIAL=FNM00175101182
ID_SERIAL=368ccf09800744a14b26d09fd41b7e9b5
ID_SERIAL_SHORT=68ccf09800744a14b26d09fd41b7e9b5
ID_TARGET_PORT=1
ID_TYPE=disk
ID_VENDOR=DellEMC
ID_VENDOR_ENC=DellEMC\x20
ID_WWN=0x68ccf09800744a14
ID_WWN_VENDOR_EXTENSION=0xb26d09fd41b7e9b5
ID_WWN_WITH_EXTENSION=0x68ccf09800744a14b26d09fd41b7e9b5
MAJOR=8
MINOR=16
SEQNUM=13029
SUBSYSTEM=block
TAGS=:systemd:
USEC_INITIALIZED=2204916

$ lsscsi -s
[3:0:0:0]    storage DellEMC  HADA2016         SCSI  -               -
[3:0:0:8]    disk    DellEMC  HADA2016         SCSI  /dev/sdb   70.3TB
```

# udve rules location and priority
man udev
