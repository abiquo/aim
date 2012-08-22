/**
 * Abiquo community edition
 * cloud management application for hybrid clouds
 * Copyright (C) 2008-2010 - Abiquo Holdings S.L.
 *
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU LESSER GENERAL PUBLIC
 * LICENSE as published by the Free Software Foundation under
 * version 3 of the License
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * LESSER GENERAL PUBLIC LICENSE v.3 for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef RIMP_H
#define RIMP_H

#include <Service.h>
#include <vector>
#include <aim_types.h>
#include <boost/thread/mutex.hpp>

class Rimp: public Service
{
protected:

    /** Mount point of the shared repository used to distribute the virtual images along all the cloud nodes on a datacenter. */
    string repository;

    /** Wether to automatically backup VirtualImages on delete **/
    bool autobackup;

    /** Wether to automatically restore VirtualImages on create **/
    bool autorestore;

    /** list of valid devices (checking agaings /etc/mtab/#mnt_type) used in ''getDatastore'' to filter /etc/mtab devices. **/
    vector<string> validTypes;

    /** Mutex for sync the getDatastores method access **/
    boost::mutex get_datastores_mutex;

public:
    Rimp();
    ~Rimp();

    virtual bool initialize(dictionary * configuration);
    virtual bool cleanup();
    virtual bool start();
    virtual bool stop();

    /** ******************************* */
    /** RIMP specific functionalities.  */
    /** ******************************* */

    /**
     * Check for configuration consistency:
     * 1) the ''repository'' is actually mounted (present on /etc/mtab).
     *
     * @throws RimpException with the description cause if some of the preconditions fails.
     * */
    virtual void checkRimpConfiguration();

    /**
     * Returns the size of the virtual disk file located on the specified path
     *
     * @param virtualImageDatastorePath, absolute path to a virtual disk file
     *
     * @return size in Kb.
     * */
    virtual int64_t getDiskFileSize(const std::string& virtualImageDatastorePath);

    /**
    * Get all the disk devices
    * 
    * @return the list fo configured disk devices (mount point, size, uuid)
    */
    virtual vector<Datastore> getDatastores();

    /**
    * Get all the network device
    *
    * @return the list of configured network interfaces (ip and mac address)
    */
    virtual vector<NetInterface> getNetInterfaces();

    /**
     * Copy from the repository to the datastore in order to start a single insance of a virtual machine.
     *
     * @param virtualImageRepositoryPath, the path of the virtual image disk file relative to the ''repository'' mount point.
     * @param virtualMachineUUID, the virtual machine UUID using the virtual image. The destination path will be ''datastore'' + ''virtualMachineUUID''.
     *
     * @throws RimpException, if the source ''virtualImageRepositoryPath'' do not exist or can not be copied (not enough free space on the ''datastore'')
     * */
    virtual void copyFromRepositoryToDatastore(const std::string& virtualImageRepositoryPath,
            std::string& datastorePath, const std::string& virtualMachineUUID);

    /**
     * Deletes a virtual image on the datastored used on a undeployed virtual machine.
     *
     * @param virtualMachineUUID, identify the path to delete: ''datastore'' + ''virtualMachineUUID''.
     *
     * @throws RimpException, if the target path do not exist or can not be deleted.
     * */
    virtual void deleteVirtualImageFromDatastore(std::string& datastorePath, const std::string& virtualMachineUUID);

    /**
     * Creates an instance of a virtual machine's virtual image into the ''repository''.
     *
     * @param sourceDatastorePath, path to the source ''datastore'' (for imported virtual machines it will not be the same as the configured ''datastore'').
     * @param virtualMachineUUID, identify the source virtual image on the ''sourceDatastorePath''. Source path = ''sourceDatastorePath'' + ''virtualImageUUID''.
     * @param destinationRepositoryPath, path on the ''repository'' used as destination of the copy.
     * @param snapshot, identify the target virtual image on the ''destinationRepositoryPath''. Destination path = ''destinationRepositoryPath'' + ''snaphot''.
     *
     * @throws RimpException, if the source ''virtualImageRepositoryPath'' do not exist or can not be copied (not enough free space on the ''datastore'')
     * */
    virtual void copyFromDatastoreToRepository(const std::string& virtualMachineUUID, const std::string& snapshot,
            const std::string& destinationRepositoryPath, const std::string& sourceDatastorePath);
};

#endif
