# Reference
http://securitynik.blogspot.ca/2016/12/docker-networking-internals-container.html
[Docker load balancing](https://www.youtube.com/watch?v=nXaF2d97JnE)
https://neuvector.com/blog/docker-swarm-container-networking/

# Start docker daemon and clean up old swarm
Start docker daemon and find swarm nodes
```
[kenny@host-6 ~]$ sudo systemctl status docker
[kenny@host-6 ~]$ sudo systemctl start docker
[kenny@host-7 ~]$ sudo systemctl start docker
[kenny@host-6 ~]$ docker node ls
```

If you have old swarm, you can wipe it as following  
- On work node (host-7)
```
[kenny@host-7 ~]$ docker swarm leave
```
- On manager ndoel (host-6)
```
[kenny@host-6 ~]$ sudo systemctl stop docker
[kenny@host-6 ~]$ sudo rm -rf /var/lib/docker/swarm
```

# Start the docker swarm and deploy a service
## Create docker swarm manager and worker nodes. A docker swarm node is a host (Linux, Windows, or Mac OS) running docker daemon  
- Start docker daemon
```
[kenny@host-6 ~]$ sudo systemctl start docker
```
- Start a cluster (manager node)
```
[kenny@host-6 ~]$ docker swarm init --advertise-addr 192.168.1.16 
[kenny@host-6 ~]$ docker node ls
ID                            HOSTNAME            STATUS              AVAILABILITY        MANAGER STATUS
edxho5ln23dmn4nxz8nbu8lks *   host-6              Ready               Active              Leader

```
- Find the token to join
```
[kenny@host-6 ~]$ docker swarm join-token worker
To add a worker to this swarm, run the following command:

    docker swarm join --token SWMTKN-1-4z06ojzhuptmtwiql52jt3irccj9vbwz6j0kna0ndg8t8trz4z-a6rvsdtbngzuuk311y23tunds 192.168.1.16:2377

```
- On worker node (host-7)
```
[kenny@host-7 ~]$ docker swarm join --token SWMTKN-1-4z06ojzhuptmtwiql52jt3irccj9vbwz6j0kna0ndg8t8trz4z-a6rvsdtbngzuuk311y23tunds 192.168.1.16:2377
```
- On manager node (host-6)
```
[kenny@host-6 ~]$ docker node ls
ID                            HOSTNAME            STATUS              AVAILABILITY        MANAGER STATUS
edxho5ln23dmn4nxz8nbu8lks *   host-6              Ready               Active              Leader
yhyk2zodb5ezgsf6r0acbchap     host-7              Ready               Active              
```

## Create an overlay network named my-overlay-network. 
```
[kenny@host-6 ~]$ docker network ls
NETWORK ID          NAME                DRIVER              SCOPE
6c95e90fa6d5        bridge              bridge              local
2359aa72c263        docker_gwbridge     bridge              local
6e337de971da        host                host                local
j12a5pxum5o1        ingress             overlay             swarm
383137c38d06        none                null                local

[kenny@host-6 ~]$ docker network create --driver overlay --subnet 10.10.10.0/24 my-overlay-network
[kenny@host-6 ~]$ docker network ls
NETWORK ID          NAME                 DRIVER              SCOPE
6c95e90fa6d5        bridge               bridge              local
2359aa72c263        docker_gwbridge      bridge              local
6e337de971da        host                 host                local
j12a5pxum5o1        ingress              overlay             swarm
ocerd3kl450s        my-overlay-network   overlay             swarm
383137c38d06        none                 null                local
```
my-overlay-network
-  The overlay network we created for east-west communication between container.
docker_gwbridge
- The network created by docker for containers to connect to the host that they are running on.
ingress
- The network created by docker for docker swarm to expose services to the external network and provide routing mesh.


## Launch a service. A service in docker context is defined as a containing and a command to execute. 
Note that we can simulate a typical 3-tier web service with nginx, as a load balancer,    
redirects traffic to node as a web server. Then, the node access the redis database. However, in this
study, we will just deploy a nginx and nvbeta/node. 
```
[kenny@host-6 ~]$ docker service create --name nginx --publish published=8080,target=80 --replicas 1 --network my-overlay-network nvbeta/swarm_nginx
[kenny@host-6 ~]$ docker service create --name node --replicas 1 --network my-overlay-network nvbeta/node
[kenny@host-6 ~]$ docker service ls
ID                  NAME                MODE                REPLICAS            IMAGE                PORTS
i4aewmts2j4e        nginx               replicated          1/1                 nvbeta/swarm_nginx:latest   *:8080->80/tcp
jp4voupz63ni        node                replicated          1/1                 nvbeta/node:latest          
[kenny@host-6 ~]$ docker service ps nginx node
ID                  NAME                IMAGE                       NODE                DESIRED STATE       CURRENT STATE                     ERROR               PORTS
n3iv2jmqd3ri        nginx.1             nvbeta/swarm_nginx:latest   host-6              Running             Running about a minute ago                            
tqohzktruvtb        node.1              nvbeta/node:latest          host-7              Running             Running less than a second ago                      

### my-overlay-network
When we create a service (a container and a command to execute), we passed in `--network my-overlay-network` option. The container  
must have an interface connecting to my-overlay-network.

Container nginx network interfaces
```
[kenny@host-6 ~]$ docker ps
CONTAINER ID        IMAGE               COMMAND                  CREATED             STATUS              PORTS               NAMES
4d39e21ec7f0        nginx:latest        "nginx -g 'daemon of…"   20 minutes ago      Up 20 minutes       80/tcp              nginx.1.s91keui948t1xo2lwt61rwpat
[kenny@host-6 ~]$ docker exec nginx.1.n3iv2jmqd3riynhqefedjt967 ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet 10.255.0.4/32 brd 10.255.0.4 scope global lo
       valid_lft forever preferred_lft forever
    inet 10.10.10.4/32 brd 10.10.10.4 scope global lo
       valid_lft forever preferred_lft forever
17: eth0@if18: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP group default 
    link/ether 02:42:0a:ff:00:05 brd ff:ff:ff:ff:ff:ff
    inet 10.255.0.5/16 brd 10.255.255.255 scope global eth0
       valid_lft forever preferred_lft forever
19: eth1@if20: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:ac:12:00:03 brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.3/16 brd 172.18.255.255 scope global eth1
       valid_lft forever preferred_lft forever
22: eth2@if23: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP group default 
    link/ether 02:42:0a:0a:0a:05 brd ff:ff:ff:ff:ff:ff
    inet 10.10.10.5/24 brd 10.10.10.255 scope global eth2
       valid_lft forever preferred_lft forever
```
Network namespaces created by docker swarm
```
[kenny@host-6 ~]$ sudo ls /var/run/docker/netns
1-j12a5pxum5  1-ocerd3kl45  d0914760c2fe  ingress_sbox
```
Create symbolic link to docker netns dirctory so we can use ip netns command (alternatively, we can use nsenter command without the symbolic links)
[kenny@host-6 ~]$ sudo ln -s /var/run/docker/netns /var/run/netns
[kenny@host-6 ~]$ sudo ip netns
d0914760c2fe (id: 3)
1-ocerd3kl45 (id: 2)
1-j12a5pxum5 (id: 0)
ingress_sbox (id: 1)
[kenny@host-6 ~]$ docker network ls
NETWORK ID          NAME                 DRIVER              SCOPE
b091bfab95dd        bridge               bridge              local
2359aa72c263        docker_gwbridge      bridge              local
6e337de971da        host                 host                local
j12a5pxum5o1        ingress              overlay             swarm
ocerd3kl450s        my-overlay-network   overlay             swarm
383137c38d06        none                 null                local

```
Comparing the namespace names with the docker swarms network IDs, we can guess that 
- namespace 1-ocerd3kl45 is used by my-overlay-network
- namespace 1-j12a5pxum5 is used by ingress

Take a look of the interfaces in namespace d0914760c2fe. It is exactly the same as in   
container nginx. So the namespace d0914760c2fe is created by the nginx container. 
```
[kenny@host-6 ~]$ sudo ip netns exec d0914760c2fe ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet 10.255.0.4/32 brd 10.255.0.4 scope global lo
       valid_lft forever preferred_lft forever
    inet 10.10.10.4/32 brd 10.10.10.4 scope global lo
       valid_lft forever preferred_lft forever
17: eth0@if18: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:ff:00:05 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.255.0.5/16 brd 10.255.255.255 scope global eth0
       valid_lft forever preferred_lft forever
19: eth1@if20: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:ac:12:00:03 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet 172.18.0.3/16 brd 172.18.255.255 scope global eth1
       valid_lft forever preferred_lft forever
22: eth2@if23: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:0a:0a:05 brd ff:ff:ff:ff:ff:ff link-netnsid 2
    inet 10.10.10.5/24 brd 10.10.10.255 scope global eth2
       valid_lft forever preferred_lft forever
```
Or use `nsenter` 
```
[kenny@host-6 ~]$ sudo nsenter --net=/var/run/docker/netns/d0914760c2fe ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet 10.255.0.4/32 brd 10.255.0.4 scope global lo
       valid_lft forever preferred_lft forever
    inet 10.10.10.4/32 brd 10.10.10.4 scope global lo
       valid_lft forever preferred_lft forever
17: eth0@if18: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:ff:00:05 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.255.0.5/16 brd 10.255.255.255 scope global eth0
       valid_lft forever preferred_lft forever
19: eth1@if20: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:ac:12:00:03 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet 172.18.0.3/16 brd 172.18.255.255 scope global eth1
       valid_lft forever preferred_lft forever
22: eth2@if23: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:0a:0a:05 brd ff:ff:ff:ff:ff:ff link-netnsid 2
    inet 10.10.10.5/24 brd 10.10.10.255 scope global eth2
```
Inspect the namespace 1-ocerd3kl45 used by my-overlay-network
- br0 is the Linux Bridge all interfaces in this network namespace are connected to.
- vxlan0 is the VTEP interface for VXLAN overlay network

For each veth pair that docker creates for the container, the device inside the container
always has an ID number which is 1 number smaller than the device ID of the other end. So,

- veth0 (23) in the namespace 1-ocerd3kl45 is connected to eth2 in the nginx container.  

```
[kenny@host-6 ~]$ sudo ip netns exec 1-ocerd3kl45 ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
2: br0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 92:d7:bc:a5:62:0a brd ff:ff:ff:ff:ff:ff
    inet 10.10.10.1/24 brd 10.10.10.255 scope global br0
       valid_lft forever preferred_lft forever
21: vxlan0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue master br0 state UNKNOWN 
    link/ether c2:e2:45:65:3c:4c brd ff:ff:ff:ff:ff:ff link-netnsid 0
23: veth0@if22: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue master br0 state UP 
    link/ether 92:d7:bc:a5:62:0a brd ff:ff:ff:ff:ff:ff link-netnsid 1
```

Here is the network diagram based on we have learned so far 
```

               host-6

  +-----------+     
  |  nginx    |      
  |           |      
  +--------+--+  
           | eth2 (10.10.10.5)
           |
           |
           | veth0
      +----+------------------+-------+ my-overlay-network (br0 10.10.10.1) 

```

###docker_gwbridge
Interface on the host, 
```
[kenny@host-6 ~]$ ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: eno16777736: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:a1:0e:9e brd ff:ff:ff:ff:ff:ff
    inet 192.168.1.16/24 brd 192.168.1.255 scope global dynamic eno16777736
       valid_lft 76994sec preferred_lft 76994sec
    inet6 fe80::20c:29ff:fea1:e9e/64 scope link 
       valid_lft forever preferred_lft forever
3: ens37: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:a1:0e:a8 brd ff:ff:ff:ff:ff:ff
    inet 192.168.1.15/24 brd 192.168.1.255 scope global dynamic ens37
       valid_lft 76994sec preferred_lft 76994sec
    inet6 fe80::15a9:3cab:80a7:57dd/64 scope link 
       valid_lft forever preferred_lft forever
4: docker_gwbridge: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:58:6d:89:e8 brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.1/16 brd 172.18.255.255 scope global docker_gwbridge
       valid_lft forever preferred_lft forever
    inet6 fe80::42:58ff:fe6d:89e8/64 scope link 
       valid_lft forever preferred_lft forever
5: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN 
    link/ether 02:42:0b:94:bf:a9 brd ff:ff:ff:ff:ff:ff
    inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
       valid_lft forever preferred_lft forever
16: veth812d9da@if15: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker_gwbridge state UP 
    link/ether 3e:8d:96:78:46:b6 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet6 fe80::3c8d:96ff:fe78:46b6/64 scope link 
       valid_lft forever preferred_lft forever
20: vethb08ef80@if19: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker_gwbridge state UP 
    link/ether d6:77:10:a9:3d:05 brd ff:ff:ff:ff:ff:ff link-netnsid 3
    inet6 fe80::d477:10ff:fea9:3d05/64 scope link 
       valid_lft forever preferred_lft forever
```

Based on docker veth pair device ID relation,
- eth1 (19) in nginx container is paired with vethb08ef80 (20) on the host
- vethb08ef80 is connected to docker_gwbridge on the host 

```
[kenny@host-6 ~]$ brctl show
bridge name     bridge id               STP enabled     interfaces
docker0         8000.02420b94bfa9       no
docker_gwbridge         8000.0242586d89e8       no              veth812d9da
                                                        vethb08ef80
```

docker_gwbridge on the host is like docker0 bridge in a single-host docker depployment. Each container has a leg  
connecting to it so the host can reach the containers running on it. However, it is not used to connect to the  
external network (for service).

Add dcoker_gwbridge to the network diagram, 
```
               host-6

  172.18.0.4     
 +----+------------------+----------------+ docker_gwbridge (172.18.0.1)   
      |vethb08ef80            
      |                 
      |                 
      |eth1(172.18.0.3)                  
  +-----------+
  |  nginx    |
  |           |
  +--------+--+
           | eth2 (10.10.10.5)
           |
           |
           | veth0
      +----+------------------+-------+ my-overlay-network (br0 10.10.10.1)

```

### ingress
Docker swarm create ingress network for swarm service (where you use -p to publish the ports).
```
[kenny@host-6 ~]$ docker network inspect ingress
......
        "Containers": {
            "e565b2544d4525cdfaf35c90087f4048f7dfe3c316f7f568f065ee91a49582d2": {
                "Name": "nginx.1.n3iv2jmqd3riynhqefedjt967",
                "EndpointID": "b88a36c65d03ffdb44df51aaf94981b3b2ce19e365fb1bca5caff48da78c509d",
                "MacAddress": "02:42:0a:ff:00:05",
                "IPv4Address": "10.255.0.5/16",
                "IPv6Address": ""
            },
            "ingress-sbox": {
                "Name": "ingress-endpoint",
                "EndpointID": "05a27ea7bf135a0099bd2dfd33ce3dac976555e19103080e920dfbacb6b5c069",
                "MacAddress": "02:42:0a:ff:00:02",
                "IPv4Address": "10.255.0.2/16",
                "IPv6Address": ""
            }

......
```
Besides nginx connects to it, there is also a hidden container, ingress-sbox, connects to it.
To explore this container, let's find its namespace:
```
[kenny@host-6 ~]$ sudo ip netns show
d0914760c2fe (id: 3)
1-ocerd3kl45 (id: 2)
1-j12a5pxum5 (id: 0)
ingress_sbox (id: 1)
```
Since we have already figured out the owners of namepaces for 
- d0914760c2fe (nginx)
- 1-ocerd3kl45 (my-overlay-network),
- 1-j12a5pxum5 ingress
ingress_sbox namespace must belong to ingress-sbox container. 
```
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
13: eth0@if14: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:ff:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.255.0.2/16 brd 10.255.255.255 scope global eth0
       valid_lft forever preferred_lft forever
15: eth1@if16: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:ac:12:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet 172.18.0.2/16 brd 172.18.255.255 scope global eth1
       valid_lft forever preferred_lft forever
```
Look inside ingress_sbox namespace,
- eth1's mac address matches 02:42:0a:ff:00:02 listed on ingress-sbox (docker network inspect ingress) 
- eth1(15) in ingress_sbox is connected to veth812d9da(16) on host, which is plugged in dcoker_gwbridge  

The ingress-sbox container is used to implement the routing mesh feature: you can access a published port (8080)
on any node no matter what node the service is running on.

Start with the iptable on host-6
```
[kenny@host-6 ~]$ sudo iptables -t nat -nvL
......
Chain DOCKER-INGRESS (2 references)
 pkts bytes target     prot opt in     out     source               destination         
    0     0 DNAT       tcp  --  *      *       0.0.0.0/0            0.0.0.0/0            tcp dpt:8080 to:172.18.0.2:8080
    1    41 RETURN     all  --  *      *       0.0.0.0/0            0.0.0.0/0           
```
It says for any packets with destination port 8080, changes to destination 172.18.0.2:8080. That is eth1 interface of   
the ingress-sbox container:
```
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox ip addr show
......
15: eth1@if16: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:ac:12:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet 172.18.0.2/16 brd 172.18.255.255 scope global eth1
```
Look at the NAT table of ingress-sbox container,  
``` 
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox iptables -nvL -t nat
Chain PREROUTING (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination         

Chain INPUT (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination         

Chain OUTPUT (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination         
    0     0 DOCKER_OUTPUT  all  --  *      *       0.0.0.0/0            127.0.0.11          
    0     0 DNAT       icmp --  *      *       0.0.0.0/0            10.255.0.4           icmptype 8 to:127.0.0.1

Chain POSTROUTING (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination         
    0     0 DOCKER_POSTROUTING  all  --  *      *       0.0.0.0/0            127.0.0.11          
    0     0 SNAT       all  --  *      *       0.0.0.0/0            10.255.0.0/16        ipvs to:10.255.0.2

Chain DOCKER_OUTPUT (1 references)
 pkts bytes target     prot opt in     out     source               destination         
    0     0 DNAT       tcp  --  *      *       0.0.0.0/0            127.0.0.11           tcp dpt:53 to:127.0.0.11:41935
    0     0 DNAT       udp  --  *      *       0.0.0.0/0            127.0.0.11           udp dpt:53 to:127.0.0.11:38451

Chain DOCKER_POSTROUTING (1 references)
 pkts bytes target     prot opt in     out     source               destination         
    0     0 SNAT       tcp  --  *      *       127.0.0.11           0.0.0.0/0            tcp spt:41935 to::53
    0     0 SNAT       udp  --  *      *       127.0.0.11           0.0.0.0/0            udp spt:38451 to::53
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox ip addr show
13: eth0@if14: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:ff:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.255.0.2/16 brd 10.255.255.255 scope global eth0
       valid_lft forever preferred_lft forever
15: eth1@if16: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:ac:12:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet 172.18.0.2/16 brd 172.18.255.255 scope global eth1
       valid_lft forever preferred_lft forever
``` 

The POSTROUTING chain put the packets on 10.255.0.2, which is the eth0 (13) interface in ingress-sbox container. There is also 
ipvs on the SNAT rule, which is a Linux kernel load balancer (IP virtual server).
```
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox iptables -nvL -t mangle
Chain PREROUTING (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination         
    0     0 MARK       tcp  --  *      *       0.0.0.0/0            0.0.0.0/0            tcp dpt:8080 MARK set 0x100

Chain INPUT (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination         

Chain FORWARD (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination         

Chain OUTPUT (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination         
    0     0 MARK       all  --  *      *       0.0.0.0/0            10.255.0.4           MARK set 0x100

Chain POSTROUTING (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination         
```
The iptables rule marks the flow as 0x100 (256 decimal)
```
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox ipvsadm -ln
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port Scheduler Flags
  -> RemoteAddress:Port           Forward Weight ActiveConn InActConn
FWM  256 rr
  -> 10.255.0.5:0                 Masq    1      0          0         

```
ipvsadm says flow 256 (0x100) should be forward to 10.255.0.5:0
```
[kenny@host-6 ~]$ docker ps
CONTAINER ID        IMAGE                       COMMAND                  CREATED             STATUS              PORTS               NAMES
e565b2544d45        nvbeta/swarm_nginx:latest   "nginx -g 'daemon of…"   4 hours ago         Up 4 hours          80/tcp, 443/tcp     nginx.1.n3iv2jmqd3riynhqefedjt967
[kenny@host-6 ~]$ docker exec e565b2544d45 ip addr show
17: eth0@if18: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP group default 
    link/ether 02:42:0a:ff:00:05 brd ff:ff:ff:ff:ff:ff
    inet 10.255.0.5/16 brd 10.255.255.255 scope global eth0
```
From above, 10.255.0.5 is on eth0 interface of nginx container.

Put everything together:
```
[kenny@host-6 ~]$ docker network ls
NETWORK ID          NAME                 DRIVER              SCOPE
b091bfab95dd        bridge               bridge              local
2359aa72c263        docker_gwbridge      bridge              local
6e337de971da        host                 host                local
j12a5pxum5o1        ingress              overlay             swarm
ocerd3kl450s        my-overlay-network   overlay             swarm
383137c38d06        none                 null                local
```
- Host interface (and docker_gwbridge network) 
```
[kenny@host-6 ~]$ ip addr show
2: eno16777736: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:a1:0e:9e brd ff:ff:ff:ff:ff:ff
    inet 192.168.1.16/24 brd 192.168.1.255 scope global dynamic eno16777736
       valid_lft 70319sec preferred_lft 70319sec
    inet6 fe80::20c:29ff:fea1:e9e/64 scope link 
       valid_lft forever preferred_lft forever
3: ens37: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:a1:0e:a8 brd ff:ff:ff:ff:ff:ff
    inet 192.168.1.15/24 brd 192.168.1.255 scope global dynamic ens37
       valid_lft 70319sec preferred_lft 70319sec
    inet6 fe80::15a9:3cab:80a7:57dd/64 scope link 
       valid_lft forever preferred_lft forever
4: docker_gwbridge: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:58:6d:89:e8 brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.1/16 brd 172.18.255.255 scope global docker_gwbridge
       valid_lft forever preferred_lft forever
    inet6 fe80::42:58ff:fe6d:89e8/64 scope link 
       valid_lft forever preferred_lft forever
5: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN 
    link/ether 02:42:0b:94:bf:a9 brd ff:ff:ff:ff:ff:ff
    inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
       valid_lft forever preferred_lft forever
16: veth812d9da@if15: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker_gwbridge state UP 
    link/ether 3e:8d:96:78:46:b6 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet6 fe80::3c8d:96ff:fe78:46b6/64 scope link 
       valid_lft forever preferred_lft forever
20: vethb08ef80@if19: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker_gwbridge state UP 
    link/ether d6:77:10:a9:3d:05 brd ff:ff:ff:ff:ff:ff link-netnsid 3
    inet6 fe80::d477:10ff:fea9:3d05/64 scope link 
       valid_lft forever preferred_lft forever
[kenny@host-6 ~]$ sudo brctl show
bridge name     bridge id               STP enabled     interfaces
docker0         8000.02420b94bfa9       no
docker_gwbridge         8000.0242586d89e8       no              veth812d9da
                                                        vethb08ef80

[kenny@host-6 ~]$ sudo ip netns show
d0914760c2fe (id: 3)
1-ocerd3kl45 (id: 2)
1-j12a5pxum5 (id: 0)
ingress_sbox (id: 1)
```
- nginx container (namespace d0914760c2fe)
```
[kenny@host-6 ~]$ sudo ip netns exec d0914760c2fe ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet 10.255.0.4/32 brd 10.255.0.4 scope global lo
       valid_lft forever preferred_lft forever
    inet 10.10.10.4/32 brd 10.10.10.4 scope global lo
       valid_lft forever preferred_lft forever
17: eth0@if18: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:ff:00:05 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.255.0.5/16 brd 10.255.255.255 scope global eth0
       valid_lft forever preferred_lft forever
19: eth1@if20: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:ac:12:00:03 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet 172.18.0.3/16 brd 172.18.255.255 scope global eth1
       valid_lft forever preferred_lft forever
22: eth2@if23: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:0a:0a:05 brd ff:ff:ff:ff:ff:ff link-netnsid 2
    inet 10.10.10.5/24 brd 10.10.10.255 scope global eth2
       valid_lft forever preferred_lft forever
```
- my-overlay-network (namespace 1-ocerd3kl45)
```
[kenny@host-6 ~]$ sudo ip netns exec 1-ocerd3kl45 ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
2: br0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 92:d7:bc:a5:62:0a brd ff:ff:ff:ff:ff:ff
    inet 10.10.10.1/24 brd 10.10.10.255 scope global br0
       valid_lft forever preferred_lft forever
21: vxlan0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue master br0 state UNKNOWN 
    link/ether c2:e2:45:65:3c:4c brd ff:ff:ff:ff:ff:ff link-netnsid 0
23: veth0@if22: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue master br0 state UP 
    link/ether 92:d7:bc:a5:62:0a brd ff:ff:ff:ff:ff:ff link-netnsid 1
[kenny@host-6 ~]$ sudo ip netns exec 1-ocerd3kl45 brctl show
RTNETLINK answers: Invalid argument
bridge name     bridge id               STP enabled     interfaces
br0             8000.92d7bca5620a       no              veth0
                                                        vxlan0
```
- ingress network (namespace 1-j12a5pxum5)
```
[kenny@host-6 ~]$ sudo ip netns exec 1-j12a5pxum5 ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
2: br0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 36:c2:92:de:69:ef brd ff:ff:ff:ff:ff:ff
    inet 10.255.0.1/16 brd 10.255.255.255 scope global br0
       valid_lft forever preferred_lft forever
12: vxlan0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue master br0 state UNKNOWN 
    link/ether 36:c2:92:de:69:ef brd ff:ff:ff:ff:ff:ff link-netnsid 0
14: veth0@if13: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue master br0 state UP 
    link/ether f2:d2:38:92:f6:a6 brd ff:ff:ff:ff:ff:ff link-netnsid 1
18: veth1@if17: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue master br0 state UP 
    link/ether ca:46:0b:c4:fa:f8 brd ff:ff:ff:ff:ff:ff link-netnsid 2
[kenny@host-6 ~]$ sudo ip netns exec 1-j12a5pxum5 brctl show
RTNETLINK answers: Invalid argument
bridge name     bridge id               STP enabled     interfaces
br0             8000.36c292de69ef       no              veth0
                                                        vxlan0
```
ingress-sbox container (namespace ingress_sbox)
```

[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
13: eth0@if14: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:ff:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.255.0.2/16 brd 10.255.255.255 scope global eth0
       valid_lft forever preferred_lft forever
15: eth1@if16: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:ac:12:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet 172.18.0.2/16 brd 172.18.255.255 scope global eth1
       valid_lft forever preferred_lft forever
```
On host-7,
- host network
```
[kenny@host-7 ~]$ ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN 
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host 
       valid_lft forever preferred_lft forever
2: eno16777736: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:09:96:36 brd ff:ff:ff:ff:ff:ff
    inet 192.168.1.18/24 brd 192.168.1.255 scope global dynamic eno16777736
       valid_lft 61580sec preferred_lft 61580sec
    inet6 fe80::20c:29ff:fe09:9636/64 scope link 
       valid_lft forever preferred_lft forever
3: docker_gwbridge: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:43:88:b7:8f brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.1/16 brd 172.18.255.255 scope global docker_gwbridge
       valid_lft forever preferred_lft forever
    inet6 fe80::42:43ff:fe88:b78f/64 scope link 
       valid_lft forever preferred_lft forever
4: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN 
    link/ether 02:42:fb:28:a3:eb brd ff:ff:ff:ff:ff:ff
    inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
       valid_lft forever preferred_lft forever
20: veth0e0a2df@if19: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker_gwbridge state UP 
    link/ether 8e:52:bb:23:60:be brd ff:ff:ff:ff:ff:ff link-netnsid 3
    inet6 fe80::8c52:bbff:fe23:60be/64 scope link 
       valid_lft forever preferred_lft forever
[kenny@host-7 ~]$ sudo ip netns show
95d5ef06df0e (id: 3)
2-ocerd3kl45 (id: 2)
```
The node container network
```
[kenny@host-7 ~]$ sudo ip netns exec 95d5ef06df0e ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN 
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet 10.10.10.6/32 brd 10.10.10.6 scope global lo
       valid_lft forever preferred_lft forever
17: eth0@if18: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:0a:0a:0b brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.10.10.11/24 brd 10.10.10.255 scope global eth0
       valid_lft forever preferred_lft forever
19: eth1@if20: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:ac:12:00:03 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet 172.18.0.3/16 brd 172.18.255.255 scope global eth1
       valid_lft forever preferred_lft forever
```
The my-overlay-network network, 
```
[kenny@host-7 ~]$ sudo ip netns exec 2-ocerd3kl45 ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN 
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
2: br0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 0a:ea:cc:d7:71:a5 brd ff:ff:ff:ff:ff:ff
    inet 10.10.10.1/24 brd 10.10.10.255 scope global br0
       valid_lft forever preferred_lft forever
16: vxlan0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue master br0 state UNKNOWN 
    link/ether 0a:ea:cc:d7:71:a5 brd ff:ff:ff:ff:ff:ff link-netnsid 0
18: veth0@if17: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue master br0 state UP 
    link/ether 36:86:fd:95:87:68 brd ff:ff:ff:ff:ff:ff link-netnsid 1

```



 ------------------------+--------------------------------------------- FIOS router 192.168.1.1/24 -----------------------------------------------------------------------------
                         |                                                                                                                      |
                         |                                                                                                          :           | 
                         |2: eno16777736 192.168.1.16                                                                               :           | 2: eno16777736 192.168.1.18 
                   +-----------+                                                                                                    :       +-----------+   
                   |  host-6   |                                                                                                    :       |  host-7   | 
                   +-----------+                                                                                                    :       +-----------+
                                                          |                                                                         :
                                                          |                                                                         : 
                                                          |12: vxlan0                                                               : 
 -----------------------+-------------------------------------------------- ingress 2: br0 10.255.0.1/16 (ns 1-j12a5pxum5)          :
                |18: veth1@if17 |14: veth0@if13                                                                                     :
                |               |                                                                                                   :
                |               |13: eth0@if14 10.255.0.2                                                                           : 
                |           +----------------------+                                                                                :    
                |           | ingress-sbox         |                                                                                :      
                |           +----------------------+                                                                                :     
                |               | 15: eth1@if16 172.18.0.2                                                                          :
                |               |                                                                                                   :
                |               |                                                                                                   :
                |               | 16: veth812d9da@if15                                                                              : 
                |            ---+---------------+--------------------------- 4: docker_gwbridge network 172.18.1.1/16 (default ns)  :   ----------- 3: docker_gwbridge 172.18.0.1/16 
                |                               |20: vethb08ef80@if19                                                               :         |20: veth0e0a2df@if19 
                |                               |                                                                                   :         | 
                |                               |                                                                                   :         | 
                |                               |                                                                                   :         | 
                |17: eth0@if18 10.255.0.5/16    |19: eth1@if20 172.18.0.3                                                           :         |19: eth1@if20 172.18.0.3 
              +-----------------------------------+                                                                                 :      +--------------------------------+ 
              |  nginx                            |                                                                                 :      | node                           | 
              +-----------------------------------+                                                                                 :      +--------------------------------+ 
                       | 22: eth2@if23 10.10.10.5/24                                                                                :         |17: eth0@if18  10.10.10.11
                       |                                                                                                            :         |
                       |                                                                                                            :         |
                       | 23: veth0@if22                                                                                             :         |18: veth0@if17 
 ----------------------+-------------------------------- my-overlay-network 2: br0 10.10.10.1 (ns 1-ocerd3kl45) --------------------:------------------my-overlay-network 2: br0 10.10.10.1
                                          |21: vxlan0                                                                               :               |16: vxlan
                                          |                                                                                         :               |
                                          |                                                                                         :               |
                                          +-----------------------------------------------------------------------------------------:---------------+
```


# Packet routing
- client send http requestt to advertise-addr 192.168.1.16:8080 
- host nat iptable changes the destination to 172.18.0.2:8080
``` 
[kenny@host-6 ~]$ sudo iptables -t nat -nvL
......
Chain DOCKER-INGRESS (2 references)
 pkts bytes target     prot opt in     out     source               destination
    0     0 DNAT       tcp  --  *      *       0.0.0.0/0            0.0.0.0/0            tcp dpt:8080 to:172.18.0.2:8080
```
- The default routing table route to docker_gwbridge (in default namespace)
```
[kenny@host-6 ~]$ ip route get 172.18.0.2
172.18.0.2 dev docker_gwbridge src 172.18.0.1 
```
- The packet arrives on interface 15: eth1@if16 172.18.0.2 of ingress-sbox container 
- The POSTROUTING chain put the packets on 10.255.0.2, which is the eth0 (13) interface in ingress-sbox container.  
```
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox iptables -nvL -t nat
Chain POSTROUTING (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination
    0     0 DOCKER_POSTROUTING  all  --  *      *       0.0.0.0/0            127.0.0.11
    0     0 SNAT       all  --  *      *       0.0.0.0/0            10.255.0.0/16        ipvs to:10.255.0.2
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox iptables -nvL -t mangle
Chain PREROUTING (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination
    0     0 MARK       tcp  --  *      *       0.0.0.0/0            0.0.0.0/0            tcp dpt:8080 MARK set 0x100
Chain OUTPUT (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination
    0     0 MARK       all  --  *      *       0.0.0.0/0            10.255.0.4           MARK set 0x100
```
- The iptables rule marks the flow as 0x100 (256 decimal)
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox ipvsadm -ln
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port Scheduler Flags
  -> RemoteAddress:Port           Forward Weight ActiveConn InActConn
FWM  256 rr
  -> 10.255.0.5:0                 Masq    1      0          0

```
- ipvsadm says flow 256 (0x100) should be forward to 10.255.0.5:0
```
- The route table in ingress_sbox namespace forward the package to eth0@if14 10.255.0.2  which is connected to ingress network 
```
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox ip route get 10.255.0.5
RTNETLINK answers: Invalid argument
10.255.0.5 dev eth0 src 10.255.0.2 
    cache 
[kenny@host-6 ~]$ sudo ip netns exec d0914760c2fe ip route get 10.255.0.5
RTNETLINK answers: Invalid argument
local 10.255.0.5 dev lo src 10.255.0.5 
```
- The packet arrives on eth0@if18 10.255.0.5 interface of nginx
- nginx consumes the IP package and handles the http request



[kenny@host-6 ~]$ ip addr show
16: veth812d9da@if15: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker_gwbridge state UP 
[kenny@host-6 ~]$ brctl show
bridge name     bridge id               STP enabled     interfaces
docker0         8000.02420b94bfa9       no
docker_gwbridge         8000.0242586d89e8       no              veth812d9da
                                                        vethb08ef80
```

So, on host_6, based on subnet, docker container 5f7a2fea6162,
- eth0 is connected to ingress network (10.255.0.0/16)
- eth1 is connected to docker_gwbridge newtork (172.18.0.0/16)
- eth2 is connected to my-overlay-networ (10.10.10.0/24) 


Install brctl
```
$ su -
# yum install bridge-utils -y
$ brctl show
bridge name     bridge id               STP enabled     interfaces
docker0         8000.024246405080       no
docker_gwbridge         8000.0242ca818fd7       no              veth82b324b
                                                        veth8894eae
```
The linux bridge, docker_gwbridge, connects the 172.18.0.0/16 subnet 

# Overlay
The overlay networks creates a subnet that can connect containers across multiple hosts (in a swarm cluster). 

On host-7,
- eth0 connects to the ingress network (10.255.0.0/16), i.e. on the same subnet as eth0 on host-6 
- eth2 connects to my-overlay-networ (10.10.10.0/24), i.e. on the same subnet as eth2 on host-6 
```
$ docker ps
CONTAINER ID        IMAGE                       COMMAND                  CREATED             STATUS              PORTS               NAMES
59dde88e52f3        akittana/dockerwebapp:1.1   "/usr/sbin/apache2ct…"   About an hour ago   Up About an hour    80/tcp              my-web.1.1f1o7k4i8qd5nwh6zgdf85n1t

$ docker exec 59dde88e52f3 ip addr show
76: eth0@if77: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP group default 
    link/ether 02:42:0a:ff:00:1f brd ff:ff:ff:ff:ff:ff
    inet 10.255.0.31/16 brd 10.255.255.255 scope global eth0
       valid_lft forever preferred_lft forever
78: eth1@if79: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:ac:12:00:03 brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.3/16 brd 172.18.255.255 scope global eth1
       valid_lft forever preferred_lft forever
81: eth2@if82: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP group default 
    link/ether 02:42:0a:0a:0a:1f brd ff:ff:ff:ff:ff:ff
    inet 10.10.10.31/24 brd 10.10.10.255 scope global eth2
       valid_lft forever preferred_lft forever

$ ip addr show
2: eno16777736: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:09:96:36 brd ff:ff:ff:ff:ff:ff
    inet 10.238.74.112/23 brd 10.238.75.255 scope global dynamic eno16777736
       valid_lft 151664sec preferred_lft 151664sec
    inet6 fe80::20c:29ff:fe09:9636/64 scope link 
       valid_lft forever preferred_lft forever
3: docker_gwbridge: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:53:4a:94:bb brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.1/16 brd 172.18.255.255 scope global docker_gwbridge
       valid_lft forever preferred_lft forever
    inet6 fe80::42:53ff:fe4a:94bb/64 scope link 
       valid_lft forever preferred_lft forever
4: docker0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN 
    link/ether 02:42:fc:96:58:fc brd ff:ff:ff:ff:ff:ff
    inet 172.17.0.1/16 brd 172.17.255.255 scope global docker0
       valid_lft forever preferred_lft forever
75: veth55e83ff@if74: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker_gwbridge state UP 
    link/ether 42:05:db:5a:2e:b4 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet6 fe80::4005:dbff:fe5a:2eb4/64 scope link 
       valid_lft forever preferred_lft forever
79: vethbeedd1d@if78: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue master docker_gwbridge state UP 
    link/ether d2:7e:19:70:23:4d brd ff:ff:ff:ff:ff:ff link-netnsid 3
    inet6 fe80::d07e:19ff:fe70:234d/64 scope link 
       valid_lft forever preferred_lft forever
```
So, eth1 (ifindex 78) in container and vethbeedd1d@if78 (ifindex 79) on host is a pair, connecting container to docker_gwbridge  

Similar to bridge networks, docker creates a bridge interface for each overlay network, which connect the virtual tunnel interfaces that make the vxlan tunnel connections between the hosts. However, these bridge and vxlan tunnel interfaces are not created directly on the tunnel host, but instead they are in separate containers that docker launches for each overlay network that is created.

Both eth0@if77 and eth2@if82 in container are tunnelled through eno16777736 (vxlan tunnel) 
On host-7, docker_gwbridge is the bridge interface. docker0 is unused docker bridge. veth55e83ff?  
```
$ docker exec 59dde88e52f3 ping 10.255.0.32
PING 10.255.0.32 (10.255.0.32) 56(84) bytes of data.
64 bytes from 10.255.0.32: icmp_seq=1 ttl=64 time=1.39 ms

$ docker exec 59dde88e52f3 ping 10.10.10.32
PING 10.10.10.32 (10.10.10.32) 56(84) bytes of data.
64 bytes from 10.10.10.32: icmp_seq=1 ttl=64 time=0.839 ms

```

Docker’s overlay network uses vxlan technology which encapsulates layer 2 frames into layer 4 packets (UDP/IP). 
This allows docker to create create virtual networks on top of existing connections between hosts that may or
may not be in the same subnet. Any network endpoints participating in this virtual network, see each other as
if they’re connected over a switch, without having to care about the underlying physical network. vxlan uses
udp port 4789.

icmp packet between 10.255.0.31 and 10.255.0.32 is tunnelled through 10.238.74.112.42022 and 10.238.74.108.4789 vxlan (layer 4) tunnel
IP packets between 10.10.10.31 and 10.10.10.32 is tunnelled through 10.238.74.108.51995 and 10.238.74.112.4789 vxlan (layer 4) tunnel
```
$ sudo tcpdump -i eno16777736 -n udp and port 4789
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on eno16777736, link-type EN10MB (Ethernet), capture size 262144 bytes
16:18:08.650919 IP 10.238.74.112.42022 > 10.238.74.108.4789: VXLAN, flags [I] (0x08), vni 4096
IP 10.255.0.31 > 10.255.0.32: ICMP echo request, id 16, seq 398, length 64

16:53:55.062742 IP 10.238.74.108.51995 > 10.238.74.112.4789: VXLAN, flags [I] (0x08), vni 4097
IP 10.10.10.32 > 10.10.10.31: ICMP echo reply, id 22, seq 2185, length 64

```
Docker swarm creates 4 network namespaces
```
$ sudo ls /var/run/docker/netns
1-nu90ifi429  1-vs3fqz7f7f  9d96585064fc  ingress_sbox
```
Create symbolics link to the docker netns directory will allow us see the namespaces iwth ip netns tool  
```
$ sudo ln -s /var/run/docker/netns /var/run/netns
$ sudo ip netns
9d96585064fc (id: 3)
1-nu90ifi429 (id: 2)
1-vs3fqz7f7f (id: 0)
ingress_sbox (id: 1)

```
Guessing from similarity of IDs
/var/run/docker/netns/1-nu90ifi429 must be for my-overlay-network
/var/run/docker/netns/1-vs3fqz7f7f must be for ingress
```
$ docker network ls
NETWORK ID          NAME                 DRIVER              SCOPE
fd366a65bcf8        bridge               bridge              local
5cf8c4af8fe0        docker_gwbridge      bridge              local
6e337de971da        host                 host                local
vs3fqz7f7fsx        ingress              overlay             swarm
nu90ifi429x4        my-overlay-network   overlay             swarm
383137c38d06        none                 null                local
```
By comparing interfaces, we can confirm 9d96585064fc network namespace is created by container process e7073fd5f371. 
```
docker ps
CONTAINER ID        IMAGE                       COMMAND                  CREATED             STATUS              PORTS               NAMES
e7073fd5f371        akittana/dockerwebapp:1.1   "/usr/sbin/apache2ct…"   About an hour ago   Up About an hour    80/tcp              my-web.2.opj5p85xwbj3ftohrkeolhoku
$ docker exec e7073fd5f371 ip addr show
83: eth0@if84: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP group default 
    link/ether 02:42:0a:ff:00:24 brd ff:ff:ff:ff:ff:ff
    inet 10.255.0.36/16 brd 10.255.255.255 scope global eth0
       valid_lft forever preferred_lft forever
85: eth1@if86: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default 
    link/ether 02:42:ac:12:00:03 brd ff:ff:ff:ff:ff:ff
    inet 172.18.0.3/16 brd 172.18.255.255 scope global eth1
       valid_lft forever preferred_lft forever
88: eth2@if89: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP group default 
    link/ether 02:42:0a:0a:0a:24 brd ff:ff:ff:ff:ff:ff
    inet 10.10.10.36/24 brd 10.10.10.255 scope global eth2
       valid_lft forever preferred_lft forever
$ sudo nsenter --net=/var/run/docker/netns/9d96585064fc ip addr show
83: eth0@if84: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:ff:00:24 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.255.0.36/16 brd 10.255.255.255 scope global eth0
       valid_lft forever preferred_lft forever
85: eth1@if86: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:ac:12:00:03 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet 172.18.0.3/16 brd 172.18.255.255 scope global eth1
       valid_lft forever preferred_lft forever
88: eth2@if89: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:0a:0a:24 brd ff:ff:ff:ff:ff:ff link-netnsid 2
    inet 10.10.10.36/24 brd 10.10.10.255 scope global eth2

```
Note that, `sudo ip netns exec 9d96585064fc ip addr show` does the same as nsenter used above.  

Inspect my-overlay-network, it is 10.10.10.0/24 subnet with one container attached to it.  
The container has an interface with IP address 10.10.10.36/24. If also has host interface 
10.238.74.108 attached to it, and the peer endpoint of the vxlan tunnel is host interface  
10.238.74.112 on host-7.
$ docker network inspect my-overlay-network
[
    {
        "Name": "my-overlay-network",
        "Id": "nu90ifi429x4j624lanlhdnwc",
            "Config": [
                {
                    "Subnet": "10.10.10.0/24",
                    "Gateway": "10.10.10.1"
                }
        "Containers": {
            "e7073fd5f371d1d0f1aa8a2230f74b711cd843dec684cae16949da3315ef3d5e": {
                "Name": "my-web.2.opj5p85xwbj3ftohrkeolhoku",
                "EndpointID": "5fc7ec4de02a922b55c7b37d1f11797c2f257791842d230f0790a6303dafafb8",
                "MacAddress": "02:42:0a:0a:0a:24",
                "IPv4Address": "10.10.10.36/24",
        "Options": {
            "com.docker.network.driver.overlay.vxlanid_list": "4097"
        },
        "Labels": {},
        "Peers": [
            {
                "Name": "6a411c148fab",
                "IP": "10.238.74.112"
            },
            {
                "Name": "5134efb03946",
                "IP": "10.238.74.108"
            }
$ docker exec e7073fd5f371d1d0f1aa8a2230f74b711cd843dec684cae16949da3315ef3d5e ip addr show
88: eth2@if89: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP group default 
    link/ether 02:42:0a:0a:0a:24 brd ff:ff:ff:ff:ff:ff
    inet 10.10.10.36/24 brd 10.10.10.255 scope global eth2
[host-6 run]$ ip addr show
3: ens37: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:a1:0e:a8 brd ff:ff:ff:ff:ff:ff
    inet 10.238.74.108/23 brd 10.238.75.255 scope global dynamic ens37
       valid_lft 141004sec preferred_lft 141004sec
    inet6 fe80::15a9:3cab:80a7:57dd/64 scope link 
       valid_lft forever preferred_lft forever
[host-7 ~]$ ip addr show
2: eno16777736: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:09:96:36 brd ff:ff:ff:ff:ff:ff
    inet 10.238.74.112/23 brd 10.238.75.255 scope global dynamic eno16777736

```
Look inside my-overlay-network network namespace, it has
- bridge inteface br0 (10.10.10.1)
- the peer end of veth pair: and eth2@if89 and veth0@if88
- vxlan0 interface
```
$ sudo nsenter --net=//var/run/docker/netns/1-nu90ifi429 ip addr show
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
2: br0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 16:10:a2:63:3d:2a brd ff:ff:ff:ff:ff:ff
    inet 10.10.10.1/24 brd 10.10.10.255 scope global br0
       valid_lft forever preferred_lft forever
87: vxlan0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue master br0 state UNKNOWN 
    link/ether 16:10:a2:63:3d:2a brd ff:ff:ff:ff:ff:ff link-netnsid 0
89: veth0@if88: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue master br0 state UP 
    link/ether b2:96:6b:c5:f0:89 brd ff:ff:ff:ff:ff:ff link-netnsid 1
$ docker ps
CONTAINER ID        IMAGE                       COMMAND                  CREATED             STATUS              PORTS               NAMES
e7073fd5f371        akittana/dockerwebapp:1.1   "/usr/sbin/apache2ct…"   About an hour ago   Up About an hour    80/tcp              my-web.2.opj5p85xwbj3ftohrkeolhoku
$ docker exec e7073fd5f371 ip addr
88: eth2@if89: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP group default 
    link/ether 02:42:0a:0a:0a:24 brd ff:ff:ff:ff:ff:ff
    inet 10.10.10.36/24 brd 10.10.10.255 scope global eth2
       valid_lft forever preferred_lft forever

```
Look at the forwarding table and port of br0 inside my-overlay-network network namespace, it has
vxlan0, vxlan0 dst 10.238.74.112, veth0 attached to it.  
```
$ sudo nsenter --net=/var/run/docker/netns/1-nu90ifi429 bridge show
Object "show" is unknown, try "bridge help".
[kenny@host-6 run]$ sudo nsenter --net=/var/run/docker/netns/1-nu90ifi429 bridge fdb show
33:33:00:00:00:01 dev br0 self permanent
01:00:5e:00:00:01 dev br0 self permanent
16:10:a2:63:3d:2a dev vxlan0 master br0 permanent
02:42:0a:0a:0a:23 dev vxlan0 dst 10.238.74.112 link-netnsid 0 self permanent
b2:96:6b:c5:f0:89 dev veth0 master br0 permanent
33:33:00:00:00:01 dev veth0 self permanent
01:00:5e:00:00:01 dev veth0 self permanent
$ sudo nsenter --net=/var/run/docker/netns/1-nu90ifi429 bridge link show
87: vxlan0 state UNKNOWN : <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 master br0 state forwarding priority 32 cost 100 
89: veth0 state UP @(null): <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 master br0 state forwarding priority 32 cost 2 

```
Capture traffic on veth0
```
$ docker exec -it e7073fd5f371 sh
# ping 10.10.10.35
sudo nsenter --net=/var/run/docker/netns/1-nu90ifi429 tcpdump -i veth0 icmp
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on veth0, link-type EN10MB (Ethernet), capture size 262144 bytes
16:50:20.858902 IP 10.10.10.36 > 10.10.10.35: ICMP echo request, id 41, seq 1, length 64
16:50:20.859514 IP 10.10.10.35 > 10.10.10.36: ICMP echo reply, id 41, seq 1, length 64
$ sudo nsenter --net=/var/run/docker/netns/1-nu90ifi429 tcpdump -i vxlan0 icmp
listening on vxlan0, link-type EN10MB (Ethernet), capture size 262144 bytes
18:00:03.787350 IP 10.10.10.36 > 10.10.10.35: ICMP echo request, id 42, seq 1, length 64
18:00:03.787810 IP 10.10.10.35 > 10.10.10.36: ICMP echo reply, id 42, seq 1, length 64
$ sudo tcpdump -i ens37 -n udp and port 4789
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on ens37, link-type EN10MB (Ethernet), capture size 262144 bytes
18:09:30.778708 IP 10.238.74.108.52903 > 10.238.74.112.4789: VXLAN, flags [I] (0x08), vni 4097
IP 10.10.10.36 > 10.10.10.35: ICMP echo request, id 47, seq 1, length 64
18:09:30.779086 IP 10.238.74.112.42302 > 10.238.74.108.4789: VXLAN, flags [I] (0x08), vni 4097
IP 10.10.10.35 > 10.10.10.36: ICMP echo reply, id 47, seq 1, length 64
[host-6]$ ip addr show
3: ens37: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:a1:0e:a8 brd ff:ff:ff:ff:ff:ff
    inet 10.238.74.108/23 brd 10.238.75.255 scope global dynamic ens37
[host-7 ~]$ ip addr show
2: eno16777736: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:09:96:36 brd ff:ff:ff:ff:ff:ff
    inet 10.238.74.112/23 brd 10.238.75.255 scope global dynamic eno16777736
[kenny@host-6 run]$ ip route get 10.238.74.112
10.238.74.112 dev ens37 src 10.238.74.108 
[kenny@host-6 run]$ sudo nsenter --net=/var/run/docker/netns/1-nu90ifi429 ip route get 10.10.10.35
10.10.10.35 dev br0 src 10.10.10.1 
    cache 
[kenny@host-6 run]$ sudo nsenter --net=/var/run/docker/netns/1-nu90ifi429 ip route get 10.10.10.36
10.10.10.36 dev br0 src 10.10.10.1 
    cache 
```
So, below is the packet flow on the docker overlay network on host-6:

- In my-web container, e7073fd5f371, icmp packet leaves veth interface  eth2@if89 
- The packet reaches the other end of the veth pair, veth0@if88, in network namespace /var/run/docker/netns/1-nu90ifi429. veth0@if88 is attached to bridge br0 in the same namespace.    
- After layer 4 encapsulation (vxlan tunnel between 10.238.74.108.52903 and 10.238.74.112.4789 ), the br0 bridge forwards the packet to vxlan0 interface, in the same network namespace. 
- The routing table route the packet to ens37 interface
- The packet goes out from ens37 interface on the host-6 to eno16777736 interface on host-7      

# ingress
The second network that the containers where connected to was the ingress network. Ingress is an overlay network but it is installed by default once a swarm cluster is initiated. This network is used to provide connectivity when connections are made to containers from the outside world. It is also where the load balancing feature provided by the swarm cluster happens.

The load balancing is handling by IPVS which runs on a container that docker swarm launches by default. We can see this container attached to the ingress network.

Note below, there is an ingress-sbox container connect to ingress network, in addition to my-web container  
```
$ docker network inspect ingress
[
        "Name": "ingress",
        "Id": "vs3fqz7f7fsx1h8vv2crpvyr3",
        "Scope": "swarm",
        "Driver": "overlay",
            "Config": [
                {
                    "Subnet": "10.255.0.0/16",
                    "Gateway": "10.255.0.1"
                }
        "Containers": {
            "e7073fd5f371d1d0f1aa8a2230f74b711cd843dec684cae16949da3315ef3d5e": {
                "Name": "my-web.2.opj5p85xwbj3ftohrkeolhoku",
                "EndpointID": "e2ef491341b94feddb5103e3b1be969246c3a64b4a0baa15bbf5b0ebc4fbb499",
                "MacAddress": "02:42:0a:ff:00:24",
                "IPv4Address": "10.255.0.36/16",
            "ingress-sbox": {
                "Name": "ingress-endpoint",
                "EndpointID": "23e9543cf902033acaf8ba90a4beb4e3f328151b58a5103834939bb7d648a2e1",
                "MacAddress": "02:42:0a:ff:00:02",
                "IPv4Address": "10.255.0.2/16",
                "IPv6Address": ""
            }
        "Options": {
            "com.docker.network.driver.overlay.vxlanid_list": "4096"
        },
        "Labels": {},
        "Peers": [
            {
                "Name": "5134efb03946",
                "IP": "10.238.74.108"
            },
            {
                "Name": "6a411c148fab",
                "IP": "10.238.74.112"
            }
```
The DNAT rule matches traffic destined to port 8080 and forwards it to 172.18.0.2:8080 
```
$ sudo iptables -t nat -n -L
Chain PREROUTING (policy ACCEPT)
target     prot opt source               destination         
DOCKER-INGRESS  all  --  0.0.0.0/0            0.0.0.0/0            ADDRTYPE match dst-type LOCAL
DOCKER     all  --  0.0.0.0/0            0.0.0.0/0            ADDRTYPE match dst-type LOCAL

Chain INPUT (policy ACCEPT)
target     prot opt source               destination         

Chain OUTPUT (policy ACCEPT)
target     prot opt source               destination         
DOCKER-INGRESS  all  --  0.0.0.0/0            0.0.0.0/0            ADDRTYPE match dst-type LOCAL
DOCKER     all  --  0.0.0.0/0           !127.0.0.0/8          ADDRTYPE match dst-type LOCAL

Chain POSTROUTING (policy ACCEPT)
target     prot opt source               destination         
MASQUERADE  all  --  172.17.0.0/16        0.0.0.0/0           
MASQUERADE  all  --  0.0.0.0/0            0.0.0.0/0            ADDRTYPE match src-type LOCAL
MASQUERADE  all  --  172.18.0.0/16        0.0.0.0/0           

Chain DOCKER (2 references)
target     prot opt source               destination         
RETURN     all  --  0.0.0.0/0            0.0.0.0/0           
RETURN     all  --  0.0.0.0/0            0.0.0.0/0           

Chain DOCKER-INGRESS (2 references)
target     prot opt source               destination         
DNAT       tcp  --  0.0.0.0/0            0.0.0.0/0            tcp dpt:8080 to:172.18.0.2:8080
RETURN     all  --  0.0.0.0/0            0.0.0.0/0           
```
Look into ingress_sbox container
```
[kenny@host-6 run]$ sudo ls /var/run/docker/netns
1-nu90ifi429  1-vs3fqz7f7f  9d96585064fc  ingress_sbox
[kenny@host-6 run]$ sudo nsenter --net=/run/docker/netns/ingress_sbox ip addr show
79: eth0@if80: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1450 qdisc noqueue state UP 
    link/ether 02:42:0a:ff:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 0
    inet 10.255.0.2/16 brd 10.255.255.255 scope global eth0
       valid_lft forever preferred_lft forever
81: eth1@if82: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP 
    link/ether 02:42:ac:12:00:02 brd ff:ff:ff:ff:ff:ff link-netnsid 1
    inet 172.18.0.2/16 brd 172.18.255.255 scope global eth1
```
docker then uses iptables mangle rules to mark packets to port 8080 with a certain number, that will then be used by IPVS to load balance to the appropriate containers:
```
[kenny@host-6 run]$ sudo nsenter --net=/var/run/docker/netns/ingress_sbox iptables -t mangle -L -n
Chain PREROUTING (policy ACCEPT)
target     prot opt source               destination         
MARK       tcp  --  0.0.0.0/0            0.0.0.0/0            tcp dpt:8080 MARK set 0x108

Chain INPUT (policy ACCEPT)
target     prot opt source               destination         

Chain FORWARD (policy ACCEPT)
target     prot opt source               destination         

Chain OUTPUT (policy ACCEPT)
target     prot opt source               destination         
MARK       all  --  0.0.0.0/0            10.255.0.34          MARK set 0x108

Chain POSTROUTING (policy ACCEPT)
target     prot opt source               destination         

```

# docker_gwbridge
Finally, there is the docker_gwbridge network. This is a bridge network and has a corresponding interface with the name docker_gwbridge created on each host participating in the swarm cluster. The docker_gwbridge provides connectivity to the outside world for traffic originating on the containers in the swarm cluster (For example, if we do a ping to google, that traffic goes through the docker_gwbridge network).


# Summary
When launching a container in a swarm cluster, the container can be attached to three (or more) networks by default. First there is the docker_gwbridge network which is used to allow containers to communicate with the outside world, then the ingress network which is only used if the containers need to allow inbound connections from the outside world, and finally there are the overlay networks that are user created and can be attached to containers. The overlay networks serve as a shared subnet between containers launched into the same network in which they can communicate directly (even if they are launched on different physical hosts).

We also saw that there separate network spaces that are created by default by docker in a swarm cluster that help manage the vxlan tunnels used for the overlay networks, as well as the load balancing rules for inbound connections to containers.
