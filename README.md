Abiquo Infrastructure Management (AIM)
======================================

The Abiquo Infrastructure Management (AIM) is a collection of services that manage the deployment of virtual machines, the local event system and the vlan management. AIM must be installed in each node machine that works with the following hypervisors:

* KVM
* XEN

Building
--------

Go [here](http://wiki.abiquo.com/display/ABI17/Building+thrift+based+AIM+on+CentOS5)

Architecture
------------

There are three services available:

* Rimp (repository importer) enables the creation of virtual image instances from the shared repository into the local file system datastore, and cloning it from the datastore back to the repository.
* EventsMonitor manages the subscription, unsubscription and notification of the events produced by the virtual machines on the XEN and KVM hypervisors. The plugin uses libvirt to detect the events.
* VLAN service Manages the creation and deletion of VLAN and Bridge on the XEN, KVM and VirtualBox hypervisors. The network 
configuration is persistent using Cent OS network configuration files under _/etc/sysconfig/network-
scripts_. 

'ifcfg-abiquo_3' bridge configuration file
------------------------------------------

    DEVICE=abiquo_3
    TYPE=Bridge
    BOOTPROTO=none
    ONBOOT=yes
    
'ifcfg-abiquo_eth2.3' VLAN configuration file
---------------------------------------------

    VLAN=yes
    DEVICE=eth2.3
    BOOTPROTO=none
    ONBOOT=yes
    BRIDGE=abiquo_3

Configuration
-------------

    [server]
    port = 60606
    
    [monitor]
    uri = "qemu+tcp:///system"
    redisHost = 10.60.1.203
    redisPort = 6379
    
    [rimp]
    repository = /opt/nfs-devel
    datastore = /var/lib/virt
    autoBackup = false
    autoRestore = false
    
    [vlan]
    ifconfigCmd = /sbin/ifconfig
    vconfigCmd = /sbin/vconfig
    brctlCmd = /usr/sbin/brctl

EventsMonitor properties
------------------------

* uri, the URI of the hypervisor to monitorize
* redisHost and redisPort, where is redis listening

Rimp properties
---------------

* repository, repository mount point
* datastore, local file system path
* autoBackup, on undeploy if autoBackup=true, then disk is backed up rather than deleted from the datastore .
* autoRestore, on deploy if autoRestore=true, then disk is restored from a previous backed up disk rather than copied from repository.

Vlan properties
---------------

The correct path in the system for ifconfig, vconfig and brctl commands.

* ifconfigCmd
* vconfigCmd
* brctlCmd

Command line arguments
----------------------

    [root@localhost ]# ./aim --help
    Usage: ./aim options
    -h --help                       Show this help
    -c --config-file=<file>         Alternate configuration file
    -p --port=<port>                Port to bind
    -d --daemon                     Run as daemon
    -u --uri=<uri>                  Hypervisor URI
    -r --repository=<repository>    Repository mount point
    -s --datastore=<datastore>      Local file system path
    -v --version                    Show AIM server version

Defaults
--------

By default the server is listening at *60606* port and loads a configuration file named *aim.ini*

Logs
----

AIM writes log messages in the stderr and /var/log/messages
