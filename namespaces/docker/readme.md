# Run a docker container creates mnt, uts, ipc, pid, net namespaces
# When the container (process) exit, the namespaces are gone
```
[kenny@new-host-6 ~]$ docker run -it busybox
[root@new-host-6 ~]# lsns
4026532581 mnt        1 34865 root sh
4026532582 uts        1 34865 root sh
4026532583 ipc        1 34865 root sh
4026532584 pid        1 34865 root sh
4026532586 net        1 34865 root sh
```
