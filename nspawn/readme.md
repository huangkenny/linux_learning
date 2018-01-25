Create a rootfs
```
wget http://mirror.centos.org/centos-7/7.4.1708/os/x86_64/Packages/centos-release-7-4.1708.el7.centos.x86_64.rpm
mkdir centos_chroot
sudo rpm -i --root=/home/kenny/container/nspawn/centos_chroot --nodeps centos-release-7-4.1708.el7.centos.x86_64.rpm
sudo yum --installroot=/home/kenny/container/nspawn/centos_chroot groups install -y -q "Minimal Install"
replace the root entry in /home/kenny/container/nspawn/centos_chroot/etc/shadow with root entry in /etc/shadow 
wget http://mirror.centos.org/centos-7/7.4.1708/os/x86_64/Packages/centos-release-7-4.1708.el7.centos.x86_64.rpm
```

Use nspawn to boot for the rootfs (in a new namespace) 
```
sudo systemd-nspawn -bD centos_chroot -M mycontainer
change password to Actonma#1
```

On host, monitor and manage the virtual machine (created by nspawn)
```
machinectl list
```
Find the leader process id of the virtual machine 
```
machinectl status centos_chroot
```
The process tree on the host
```
pstree -pl
```
Show processes trees in individual namespaces
```
pstree -pl -N pid
machinectl poweroff centos-chroot
```
