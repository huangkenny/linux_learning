# Network Namespace
# Reference
[Intro Linux Network Namespace](https://blog.scottlowe.org/2013/09/04/introducing-linux-network-namespaces/)
```
man ip netns
lsns
ip netns
```
# Example
# Create a network namespace
```
[root@new-host-6 ~]# ip netns list
[root@new-host-6 ~]# ip netns add blue
[root@new-host-6 ~]# ip netns list
blue
```

# Create a pair of virtual interfaces in default namespace
# Note that veth always come in pairs - whatever comes in one veth interface will come out the other peer veth interface
```
ip link add veth0 type veth peer name veth1
ip link list
15: veth1@veth0: <BROADCAST,MULTICAST,M-DOWN> mtu 1500 qdisc noop state DOWN mode DEFAULT qlen 1000
    link/ether 12:40:29:49:03:50 brd ff:ff:ff:ff:ff:ff
16: veth0@veth1: <BROADCAST,MULTICAST,M-DOWN> mtu 1500 qdisc noop state DOWN mode DEFAULT qlen 1000
    link/ether 56:73:ed:92:26:91 brd ff:ff:ff:ff:ff:ff
```
# Move one of the veth interfaces to the blue namespace
```
ip link set veth1 netns blue
ip link list
16: veth0@if15: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN mode DEFAULT qlen 1000
    link/ether 56:73:ed:92:26:91 brd ff:ff:ff:ff:ff:ff link-netnsid 0
ip netns exec blue ip link list
1: lo: <LOOPBACK> mtu 65536 qdisc noop state DOWN mode DEFAULT qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
15: veth1@if16: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN mode DEFAULT qlen 1000
    link/ether 12:40:29:49:03:50 brd ff:ff:ff:ff:ff:ff link-netnsid 0
```

- Create a bridge (br1) in default network namespace 
- Add a physical NIC to the bridge 
- Add the veth0 to the bridge 
- Add the an IP interface to veth1
- Bring all links up 
```
brctl addbr br1
brctl addif br1 ens37
brctl addif br1 veth0
ip netns exec blue ip addr add 192.168.1.20/24 dev veth1
ip netns exec blue ip link set dev veth1 up
ip l set dev veth0 up
ifconfig br1 up
```
- Now you and ping 192.168.1.20 from outside world
```
PS C:\Users\huangk5> ping 192.168.1.20
Pinging 192.168.1.20 with 32 bytes of data:
Request timed out.
Reply from 192.168.1.20: bytes=32 time=3ms TTL=64
Reply from 192.168.1.20: bytes=32 time=3ms TTL=64
Reply from 192.168.1.20: bytes=32 time=1ms TTL=64

```
