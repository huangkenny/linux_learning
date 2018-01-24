# Network namespace
## Reference
- [Intro Linux Network Namespace](https://blog.scottlowe.org/2013/09/04/introducing-linux-network-namespaces/)
- [TUM/TAP](http://www.naturalborncoder.com/virtualization/2014/10/17/understanding-tun-tap-interfaces/)
Man pages and commands
```
man ip netns
lsns
ip netns
```
## Example and use cases
### Example 1: 
Create a network namespace
```
# su -
# ip netns list
# ip netns add blue
# ip netns list
blue
```
Create a pair of virtual nic (veth) in default namespace.
Note that veth always come in pairs - whatever comes in one veth will come out the other peer veth interface
```
# ip link add veth_in type veth peer name veth_out
# ip link list
5: veth_out@veth_in: <BROADCAST,MULTICAST,M-DOWN> mtu 1500 qdisc noop state DOWN mode DEFAULT qlen 1000
    link/ether 52:59:fb:ec:82:78 brd ff:ff:ff:ff:ff:ff
6: veth_in@veth_out: <BROADCAST,MULTICAST,M-DOWN> mtu 1500 qdisc noop state DOWN mode DEFAULT qlen 1000
    link/ether 8a:52:92:99:10:81 brd ff:ff:ff:ff:ff:ff
```
Move one of the veth interfaces to the blue namespace
```
# ip link set veth_in netns blue
# ip link list
5: veth_out@if6: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN mode DEFAULT qlen 1000
    link/ether 52:59:fb:ec:82:78 brd ff:ff:ff:ff:ff:ff link-netnsid 0
# ip netns exec blue ip link list
1: lo: <LOOPBACK> mtu 65536 qdisc noop state DOWN mode DEFAULT qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
6: veth_in@if5: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN mode DEFAULT qlen 1000
    link/ether 8a:52:92:99:10:81 brd ff:ff:ff:ff:ff:ff link-netnsid 0
```
Connect the blue namespace to the outside world
- Create a bridge (br0) in default network namespace 
- Add the veth_out to the bridge 
- Add a physical NIC to the bridge 
- Bring all links up 
```
# ip link add name br0 type bridge
# ip link set dev veth_out master br0
# ip link set dev veth_out up
# ip link set dev ens37 master br0
# ip link set dev br0 up
# ip netns exec blue ip link set dev veth_in up
# ip link show
3: ens37: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast master br0 state UP mode DEFAULT qlen 1000
    link/ether 00:0c:29:a1:0e:a8 brd ff:ff:ff:ff:ff:ff
5: veth_out@if6: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master br0 state UP mode DEFAULT qlen 1000
    link/ether 52:59:fb:ec:82:78 brd ff:ff:ff:ff:ff:ff link-netnsid 0
7: br0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP mode DEFAULT qlen 1000
    link/ether 00:0c:29:a1:0e:a8 brd ff:ff:ff:ff:ff:ff
# ip netns exec blue ip link show
6: veth_in@if5: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP mode DEFAULT qlen 1000
    link/ether 8a:52:92:99:10:81 brd ff:ff:ff:ff:ff:ff link-netnsid 0
```
Alterntiely, you can use the following command to create a bridge
```
# brctl addbr br0
# brctl addif br0 veth_out
# brctl addif br0 ens37
# brctl show br0
bridge name     bridge id               STP enabled     interfaces
br0             8000.000c29a10ea8       no              ens37
                                                        veth_out
```
DHCP assign IP address to veth_in
```
# dhclient --no-pid br0
# ip netns exec blue dhclient --no-pid veth_in
# ip netns exec blue ip addr show
6: veth_in@if5: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP qlen 1000
    link/ether 8a:52:92:99:10:81 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.238.74.133/23 brd 10.238.75.255 scope global dynamic veth_in
       valid_lft 172797sec preferred_lft 172797sec
    inet6 fe80::8852:92ff:fe99:1081/64 scope link 
       valid_lft forever preferred_lft forever
# ip addr show
3: ens37: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast master br0 state UP qlen 1000
    link/ether 00:0c:29:a1:0e:a8 brd ff:ff:ff:ff:ff:ff
    inet 10.238.74.94/23 brd 10.238.75.255 scope global dynamic ens37
       valid_lft 171327sec preferred_lft 171327sec
    inet6 fe80::15a9:3cab:80a7:57dd/64 scope link 
       valid_lft forever preferred_lft forever
5: veth_out@if6: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master br0 state UP qlen 1000
    link/ether 52:59:fb:ec:82:78 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet6 fe80::5059:fbff:feec:8278/64 scope link 
       valid_lft forever preferred_lft forever
7: br0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP qlen 1000
    link/ether 00:0c:29:a1:0e:a8 brd ff:ff:ff:ff:ff:ff
    inet 10.238.74.132/23 brd 10.238.75.255 scope global dynamic br0
       valid_lft 172288sec preferred_lft 172288sec
    inet6 fe80::20c:29ff:fea1:ea8/64 scope link 
       valid_lft forever preferred_lft forever
```
Now you and ping from blue network namespace to outside world. 
```
[root@localhost ~]# ip netns exec blue ping 10.244.91.162
PING 10.244.91.162 (10.244.91.162) 56(84) bytes of data.
64 bytes from 10.244.91.162: icmp_seq=1 ttl=57 time=1.27 ms
```
Ping from PC to blue network namespace
```
PS C:\Users\huangk5> ping 10.238.74.133

Pinging 10.238.74.133 with 32 bytes of data:
Reply from 10.238.74.133: bytes=32 time=1ms TTL=64
```
Route table in the blue network namespace (v.s. route in the default namespace)
```
# ip netns exec blue route 
Kernel IP routing table
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
default         gateway         0.0.0.0         UG    0      0        0 veth_in
10.238.74.0     0.0.0.0         255.255.254.0   U     0      0        0 veth_in
# ip route
default via 10.238.74.1 dev br0 
default via 10.238.74.1 dev ens37 proto static metric 100 
default via 10.238.74.1 dev eno16777736 proto static metric 101 
10.238.74.0/23 dev br0 proto kernel scope link src 10.238.74.132 
10.238.74.0/23 dev ens37 proto kernel scope link src 10.238.74.94 metric 100 
10.238.74.0/23 dev eno16777736 proto kernel scope link src 10.238.74.93 metric 101 
```
Create TAP interface in the blue network space
```
# ip netns exec blue ip tuntap add name tap0 mode tap
# ip netns exec blue ip addr add 192.168.50.5 dev tap0
# ip netns exec blue ip addr show
2: tap0: <BROADCAST,MULTICAST> mtu 1500 qdisc noop state DOWN qlen 1000
    link/ether ea:e8:55:3b:a8:70 brd ff:ff:ff:ff:ff:ff
    inet 192.168.50.5/32 scope global tap0
       valid_lft forever preferred_lft forever
6: veth_in@if5: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP qlen 1000
    link/ether 8a:52:92:99:10:81 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.238.74.133/23 brd 10.238.75.255 scope global dynamic veth_in
       valid_lft 167926sec preferred_lft 167926sec
    inet6 fe80::8852:92ff:fe99:1081/64 scope link 
       valid_lft forever preferred_lft forever
```

