```
docker pull busybox
mkdir -p busybox/rootfs
tmpcontainer=$(docker create busybox)
echo $tmpcontainer
docker export $tmpcontainer > busybox.tar
tar -C busybox/rootfs/ -xf busybox.tar
docker rm $tmpcontainer 
sudo systemd-nspawn -bD rootfs/
sudo systemd-nspawn -D rootfs/
```
