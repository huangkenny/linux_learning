# Reference
[Docker Networking Internals](http://securitynik.blogspot.com/2016/12/docker-networking-internals-how-docker_16.html)

# Experience docker network
Create two docker networks. The docker daemon will create two linux bridges. 
Then, create a busybox container, c1, attaching it to net1 network. The docker deamon will create the container with
a veth pair: one end attaches to the container c1, and the peer end attaches to the bridge (created by net1).
Create another container, c2, and attaching it to net2.
```
$ docker network create net1
$ docker network create net2
$ docker run -it --name c1 --network net1 busybox
$ docker run -it --name c2 --network net2 busybox
$ brctl show
bridge name     bridge id               STP enabled     interfaces
br-340fda49f12b         8000.02423d4989e3       no              vethc5e11ad
br-576fd0522c4e         8000.024278be8124       no              vethed2c534
docker0         8000.02422d4b4f62       no
$ docker inspect net1
[
    {
       "Name": "net1",
       "Driver": "bridge",

            "Config": [
                {
                    "Subnet": "172.18.0.0/16",
                    "Gateway": "172.18.0.1"
                }
            ]

        "Containers": {
            "10f3b9d2c2f8b73ac0e78a7c1bee3501b7377ebc33e7730c4f65598ae2f11915": {
                "Name": "c1",
                "EndpointID": "3620d8dc8822eb1b2f338e1e92bdbf8f67d1a9f8161729d9d07bd37dd0db0278",
                "MacAddress": "02:42:ac:12:00:02",
                "IPv4Address": "172.18.0.2/16",
                "IPv6Address": ""
            }
        },
]
```
In c1,
```
# ip addr show
7: eth0@if8: <BROADCAST,MULTICAST,UP,LOWER_UP,M-DOWN> mtu 1500 qdisc noqueue 
    link/ether 02:42:ac:12:00:02 brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.2/16 scope global eth0
       valid_lft forever preferred_lft forever
```
In c2, ping won't reach c1 since docker daemon inserted rule to drop packets between containers on different networks. 
```
# ip addr show 
9: eth0@if10: <BROADCAST,MULTICAST,UP,LOWER_UP,M-DOWN> mtu 1500 qdisc noqueue 
    link/ether 02:42:ac:13:00:02 brd ff:ff:ff:ff:ff:ff
    inet 172.19.0.2/16 scope global eth0
       valid_lft forever preferred_lft forever
# ping 172.18.0.2
PING 172.18.0.2 (172.18.0.2): 56 data bytes
```
In host, the icmp package from c2 to c1 was dropped by chain DOCKER-ISOLATION, rule num 2. Notice the pkts column increases as we execute the ping.  
```
# iptables -t filter -L -v --line-numbers
Chain FORWARD (policy DROP 2 packets, 100 bytes)
num   pkts bytes target     prot opt in     out     source               destination         
1        8   604 DOCKER-ISOLATION  all  --  any    any     anywhere             anywhere            

Chain DOCKER-ISOLATION (1 references)
num   pkts bytes target     prot opt in     out     source               destination         
1        0     0 DROP       all  --  br-340fda49f12b br-576fd0522c4e  anywhere             anywhere            
2        6   504 DROP       all  --  br-576fd0522c4e br-340fda49f12b  anywhere             anywhere            
```
tcpdump sees the icmp packets reaches br-576fd0522c4e  
```
[root@localhost ~]# tcpdump -n -i br-576fd0522c4e
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on br-576fd0522c4e, link-type EN10MB (Ethernet), capture size 262144 bytes
07:39:17.389793 IP 172.19.0.2 > 172.18.0.2: ICMP echo request, id 2560, seq 0, length 64
07:39:22.875673 ARP, Request who-has 172.19.0.1 tell 172.19.0.2, length 28
07:39:22.875800 ARP, Reply 172.19.0.1 is-at 02:42:78:be:81:24, length 28
```
As an experiment, insert two rules to accept all packets from c1 172.18.0.2/16 network and c2 172.19.0.2/16 network on the host 
```
iptables -t filter -I DOCKER-ISOLATION 1 -s 172.19.0.2/16 -j ACCEPT
iptables -t filter -I DOCKER-ISOLATION 1 -s 172.18.0.2/16 -j ACCEPT
# iptables -t filter -nL -v --line-numbers
Chain DOCKER-ISOLATION (1 references)
num   pkts bytes target     prot opt in     out     source               destination         
1        4   336 ACCEPT     all  --  *      *       172.18.0.0/16        0.0.0.0/0           
2        5   420 ACCEPT     all  --  *      *       172.19.0.0/16        0.0.0.0/0           
3        1    84 DROP       all  --  br-340fda49f12b br-576fd0522c4e  0.0.0.0/0            0.0.0.0/0           
4        6   504 DROP       all  --  br-576fd0522c4e br-340fda49f12b  0.0.0.0/0            0.0.0.0/0           
```
Now, we can ping between the c1 and c2 containers. 
```
# ping 172.18.0.2 
PING 172.18.0.2 (172.18.0.2): 56 data bytes
64 bytes from 172.18.0.2: seq=0 ttl=63 time=0.153 ms
64 bytes from 172.18.0.2: seq=1 ttl=63 time=0.254 ms
64 bytes from 172.18.0.2: seq=2 ttl=63 time=0.194 ms
```
On host, tcpdump captures the package flow [refer to netfilter packet-flow](https://www.garron.me/images/2012-04/Netfilter-packet-flow.svg): 
- The icmp package leave c2 eth0 interface: src IP 172.19.0.2 dest IP 172.18.0.2. 
- eth0@if8 tunnel the packet to br-576fd0522c4e (172.19.0.1) bridge interface. The bridge interface sends the packet to the host router since the packet not destined to the local network.  
- In theory, the packet is first filered by host iptables INPUT chain. But, the is no rule configured in this chain. 
- The packet is routed towards br-340fda49f12b (172.18.0.1) interface. 
- The packet is filtered by the forward chain DOCKER-ISOLATION rule, which is now configured to accept all packet with source 172.19.0.0/16. 
- Let's skip the nat filter for now.
- The packet comes out br-340fda49f12b (172.18.0.1) interface and tunnelled to eth0@if10 in c2.  
- c2 sends icmp echo reply to 172.19.0.2
- the icmp echo reply traverse the reverse path (suggesting to similar natting, filtering, and routing. 
```
tcpdump -n -i br-576fd0522c4e
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on br-576fd0522c4e, link-type EN10MB (Ethernet), capture size 262144 bytes
07:51:16.994936 IP 172.19.0.2 > 172.18.0.2: ICMP echo request, id 3328, seq 0, length 64
07:51:22.234849 ARP, Request who-has 172.19.0.1 tell 172.19.0.2, length 28
07:51:22.235027 ARP, Reply 172.19.0.1 is-at 02:42:78:be:81:24, length 28
07:52:10.418126 IP 172.19.0.2 > 172.18.0.2: ICMP echo request, id 3584, seq 0, length 64
07:52:10.418166 IP 172.18.0.2 > 172.19.0.2: ICMP echo reply, id 3584, seq 0, length 64
07:52:15.483569 ARP, Request who-has 172.19.0.2 tell 172.19.0.1, length 28
07:52:15.483812 ARP, Request who-has 172.19.0.1 tell 172.19.0.2, length 28
07:52:15.483886 ARP, Reply 172.19.0.1 is-at 02:42:78:be:81:24, length 28
07:52:15.483862 ARP, Reply 172.19.0.2 is-at 02:42:ac:13:00:02, length 28
07:52:18.978730 IP 172.19.0.2 > 172.18.0.2: ICMP echo request, id 3840, seq 0, length 64
07:52:18.978825 IP 172.18.0.2 > 172.19.0.2: ICMP echo reply, id 3840, seq 0, length 64
07:52:19.979870 IP 172.19.0.2 > 172.18.0.2: ICMP echo request, id 3840, seq 1, length 64
07:52:19.979982 IP 172.18.0.2 > 172.19.0.2: ICMP echo reply, id 3840, seq 1, length 64
07:52:20.980619 IP 172.19.0.2 > 172.18.0.2: ICMP echo request, id 3840, seq 2, length 64
07:52:20.980698 IP 172.18.0.2 > 172.19.0.2: ICMP echo reply, id 3840, seq 2, length 64
```

