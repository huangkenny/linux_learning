# Reference
http://securitynik.blogspot.ca/2016/12/docker-networking-internals-container.html
[Docker load balancing](https://www.youtube.com/watch?v=nXaF2d97JnE)
https://neuvector.com/blog/docker-swarm-container-networking/
https://github.com/docker/libnetwork

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

                                                                                                                                    .
                                                                                                                                    .
 ------------------------+--------------------------------------------- FIOS router 192.168.1.1/24 -----------------------------------------------------------------------------
                         |                                                                                                          .           |
                         |                                                                                                          .           | 
                         |2: eno16777736 192.168.1.16                                                                               .           | 2: eno16777736 192.168.1.18 
                   +------------------------+                                                                                       .       +-----------+   
                   |  host-6 (ns default)   |                                                                                       .       |  host-7   | 
                   +------------------------+                                                                                       .       +-----------+
                                                                                                                                    .
                                                                                                                                    . 
                                                 |12: vxlan0                                                                        . 
 ---------------+--------------------------------+--------+--------------- 2: br0 ingress (ns 1-j12a5pxum5 10.255.0.1/16)           .
                |18: veth1@if17                           |14: veth0@if13                                                           .
                |                                         |                                                                         .
                |                                         |13: eth0@if14 10.255.0.2                                                 . 
                |           +-----------------------------+---+                                                                     .    
                |           | ingress-sbox (ns ingress_sbox)  |                                                                     .      
                |           +---+-----------------------------+                                                                     .     
                |               | 15: eth1@if16 172.18.0.2                                                                          .
                |               |                                                                                                   .
                |               |                                                                                                   .
                |               | 16: veth812d9da@if15                                                                              . 
                |            ---+--------------+--------------------------- 4: docker_gwbridge network (ns default 172.18.1.1/16)   .   ------+------ 3: docker_gwbridge network 172.18.0.1/16 
                |                              |20: vethb08ef80@if19                                                                .         |20: veth0e0a2df@if19 
                |                              |                                                                                    .         | 
                |                              |                                                                                    .         | 
                |                              |                                                                                    .         | 
                |17: eth0@if18 10.255.0.5      |19: eth1@if20 172.18.0.3                                                            .         |19: eth1@if20 172.18.0.3 
              +-+------------------------------+-+                                                                                  .      +--+-----------------------------+ 
              |  nginx (ns d0914760c2fe)         |                                                                                  .      | node                           | 
              +--------+-------------------------+                                                                                  .      +--+-----------------------------+ 
                       | 22: eth2@if23 10.10.10.5                                                                                   .         |17: eth0@if18  10.10.10.11
                       |                                                                                                            .         |
                       |                                                                                                            .         |
                       | 23: veth0@if22                                                                                             .         |18: veth0@if17 
 ----------------------+-------------------------------- 2: br0 my-overlay-network (ns 1-ocerd3kl45 10.10.10.1/24)                  .  ------------------ 2: br0 my-overlay-network 10.10.10.1/24
                                          |21: vxlan0                                                                               .     |16: vxlan
                                          |                                                                                         .     |
                                          |                                                                                         .     |
                                          +-----------------------------------------------------------------------------------------------+
                                                                                                                                    .
                                                                                                                                    .
```


# Packet routing
- client sends http request to the advertise-addr and published port: 192.168.1.16:8080 
- IP packeti is  received on eno16777736 interface on host-6
- host NAT iptable DOCKER-INGRESS chain changes the IP header of any packets with destination port 8080 to destination IP address 172.18.0.2 port 8080
``` 
[kenny@host-6 ~]$ sudo iptables -t nat -nvL
......
Chain DOCKER-INGRESS (2 references)
 pkts bytes target     prot opt in     out     source               destination
    0     0 DNAT       tcp  --  *      *       0.0.0.0/0            0.0.0.0/0            tcp dpt:8080 to:172.18.0.2:8080
```
- The routing table (in default namesapace) routes to the packet to docker_gwbridge interface (with detination address 172.18.0.2)
```
[kenny@host-6 ~]$ ip route get 172.18.0.2
172.18.0.2 dev docker_gwbridge src 172.18.0.1 
```
- the docker_gwbridge linux bridge puts the packet on 16: veth812d9da@if15 interface
- The packet arrives on the other end of veth pair 15: eth1@if16 172.18.0.2 of ingress-sbox container
- For incoming packet (destined to the host), the iptables flow is 
  - raw PREROUTING (empty in ingress_sbox) 
  - mangle PREROUTING
  - nat PREROUTING
  - mangle INPUT 
  - filter INPUT 
  - Local processing
- mangle PREROUTING marks any packet destined to port 8080 to flow 0x100.
```
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox iptables -nvL -t mangle
Chain PREROUTING (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination
    0     0 MARK       tcp  --  *      *       0.0.0.0/0            0.0.0.0/0            tcp dpt:8080 MARK set 0x100
```
- There are not other rules configured for incoming. So, the package is send to local processing (by IPVS).
- ipvsadm changes the destination of flow 256 (0x100) to 10.255.0.5, i.e. the nginx
```
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox ipvsadm -ln
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port Scheduler Flags
  -> RemoteAddress:Port           Forward Weight ActiveConn InActConn
FWM  256 rr
  -> 10.255.0.5:0                 Masq    1      0          0
```
- The locally generated packets (by IPVS) flow as following: 
  - routing decision 
  - raw OUTPUT
  - mangle OUTPUT 
  - nat OUTPUT
  - routing decision
  - filter OUTPUT
  - mangle POSTROUTING
  - nat POSTROUTING   
  - goes out on the outgoing interface 
- The routing table in ingress_sbox namespace route the packet to interface eth0.
```
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox ip route get 10.255.0.5
10.255.0.5 dev eth0 src 10.255.0.2 
```

- The POSTROUTING chain put the packet on 10.255.0.2, which is the eth0 (device 13, with source IP 10.255.0.2) interface in ingress-sbox container.  

- The mangle OUTPUT chain only has a rule for destination 10.255.0.4. Our destination is 10.255.0.5, so the following rule does not apply. 
```
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox iptables -nvL -t mangle
Chain OUTPUT (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination
    0     0 MARK       all  --  *      *       0.0.0.0/0            10.255.0.4           MARK set 0x100
```
- The only rule applies is nat POSTROUTING chain the SNAT rule. It changes source from ipvs to 10.255.0.2.
```
[kenny@host-6 ~]$ sudo ip netns exec ingress_sbox iptables -nvL -t nat
Chain POSTROUTING (policy ACCEPT 0 packets, 0 bytes)
 pkts bytes target     prot opt in     out     source               destination
    0     0 DOCKER_POSTROUTING  all  --  *      *       0.0.0.0/0            127.0.0.11
    0     0 SNAT       all  --  *      *       0.0.0.0/0            10.255.0.0/16        ipvs to:10.255.0.2
```

- The package to eth0@if14 with source IP 10.255.0.2 and destination IP 10.255.0.5. The package goes on ingress network 
- the br0 forward the packet to veth1@if17 (device 18) which is connected to veth eth0@if18 10.255.0.5 interface of nginx.
- nginx application consumes the IP package and handles the http request


# Note on how to install brctl
```
$ su -
# yum install bridge-utils -y
$ brctl show
```
