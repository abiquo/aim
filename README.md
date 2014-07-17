# Abiquo Infrastructure Management (AIM)

The Abiquo Infrastructure Management (AIM) is a collection of services that manage the deployment of virtual machines, the local event system and the vlan management. AIM must be installed in each node machine that works with the KVM hypervisor.

## Services

There are five three services available:

* Rimp (repository importer) enables the creation of virtual image instances from the shared repository into the local file system datastore, and cloning it from the datastore back to the repository.
* EventsMonitor manages the subscription, unsubscription and notification of the events produced by the virtual machines on the XEN and KVM hypervisors. The plugin uses libvirt to detect the events.
* VLAN service Manages the creation and deletion of VLAN and Bridge on the XEN, KVM and VirtualBox hypervisors. The network 
configuration is persistent using Cent OS network configuration files under _/etc/sysconfig/network-
scripts_.
* Storage service manages the iSCSI storage configuration.
* Libvirt service provides access to the libvirt API.

## Configuration

    [server]
    port = 60606
    
    [rimp]
    repository = /opt/nfs-devel
    autoBackup = false
    autoRestore = false

### Rimp properties

* repository, repository mount point
* autoBackup, on undeploy if autoBackup=true, then disk is backed up rather than deleted from the datastore .
* autoRestore, on deploy if autoRestore=true, then disk is restored from a previous backed up disk rather than copied from repository.

## Command line arguments

    [root@localhost ~]# abiquo-aim --help
    Usage: abiquo-aim options
    -h --help                       Show this help
    -c --config-file=<file>         Alternate configuration file
    -d --daemon                     Run as daemon
    -v --version                    Show AIM server version
    -t --threads                    Maximum threads to handle requests

## Defaults

By default the server is listening at *60606* port, starts a thread pool of *4* threads and loads the configuration file named *aim.ini*

## Logs

AIM writes log messages in the stderr and /var/log/messages

## VLAN configuration examples

*'ifcfg-abiquo_3' bridge configuration file*

    DEVICE=abiquo_3
    TYPE=Bridge
    BOOTPROTO=none
    ONBOOT=yes
    
*'ifcfg-abiquo_eth2.3' VLAN configuration file*

    VLAN=yes
    DEVICE=eth2.3
    BOOTPROTO=none
    ONBOOT=yes
    BRIDGE=abiquo_3
