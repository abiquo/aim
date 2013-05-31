/**
 * abiCloud  community version
 * cloud management application for hybrid clouds
 * Copyright (C) 2008-2010 - Soluciones Grid SL
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

#ifndef AIM_HANDLER_H
#define AIM_HANDLER_H

#include <Service.h>
#include <EventsMonitor.h>
#include <Rimp.h>
#include <VLan.h>
#include <StorageService.h>
#include <LibvirtService.h>

#include <vector>

#include <aim_types.h>

using namespace std;

class AimHandler: virtual public AimIf
{
    protected:
        EventsMonitor* monitor;
        Rimp* rimp;
        VLan* vlan;
        StorageService* storage;
        LibvirtService* libvirt;

    public:
        AimHandler()
        {
            monitor = EventsMonitor::getInstance();
            rimp = new Rimp();
            vlan = new VLan();
            storage = new StorageService();
            libvirt = new LibvirtService();
        }

        vector<Service*> getServices()
        {
            vector<Service*> services;

            services.push_back(monitor);
            services.push_back(rimp);
            services.push_back(vlan);
            services.push_back(storage);
            services.push_back(libvirt);

            return services;
        }

        void checkRimpConfiguration()
        {
            rimp->checkRimpConfiguration();
        }

        int64_t getDiskFileSize(const std::string& virtualImageDatastorePath)
        {
            return rimp->getDiskFileSize(virtualImageDatastorePath);
        }

        void getDatastores(std::vector<Datastore> & _return)
        {
            _return = rimp->getDatastores();
        }

        void getNetInterfaces(std::vector<NetInterface> & _return)
        {
            _return = rimp->getNetInterfaces();
        }

        void copyFromRepositoryToDatastore(const std::string& virtualImageRepositoryPath, const std::string& datastorePath,
                const std::string& virtualMachineUUID)
        {
            string datastoreIn = datastorePath;
            rimp->copyFromRepositoryToDatastore(virtualImageRepositoryPath, datastoreIn, virtualMachineUUID);
        }

        void deleteVirtualImageFromDatastore(const std::string& datastorePath, const std::string& virtualMachineUUID)
        {
            string datastoreIn = datastorePath;
            rimp->deleteVirtualImageFromDatastore(datastoreIn, virtualMachineUUID);
        }

        void copyFromDatastoreToRepository(const std::string& virtualMachineUUID, const std::string& snapshot,
                const std::string& destinationRepositoryPath, const std::string& sourceDatastorePath)
        {
            rimp->copyFromDatastoreToRepository(virtualMachineUUID, snapshot, destinationRepositoryPath,
                    sourceDatastorePath);
        }

        void createVLAN(const int32_t vlanTag, const std::string& vlanInterface, const std::string& bridgeInterface)
        {
            vlan->createVLAN(vlanTag, vlanInterface, bridgeInterface);
        }

        void deleteVLAN(const int32_t vlanTag, const std::string& vlanInterface, const std::string& bridgeInterface)
        {
            vlan->deleteVLAN(vlanTag, vlanInterface, bridgeInterface);
        }

        void checkVLANConfiguration()
        {
            vlan->checkVLANConfiguration();
        }

        void getInitiatorIQN(std::string& _return)
        {
            storage->getInitiatorIQN(_return);
        }

        void rescanISCSI(const std::vector<std::string>& targets)
        {
            storage->rescanISCSI(targets);
        }

        void getNodeInfo(NodeInfo& _return)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->getNodeInfo(_return, conn);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        void getDomains(std::vector<DomainInfo> & _return)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->getDomains(_return, conn);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        void defineDomain(const std::string& xmlDesc)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->defineDomain(conn, xmlDesc);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        void undefineDomain(const std::string& domainName)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->undefineDomain(conn, domainName);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        bool existDomain(const std::string& domainName)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                bool exist = libvirt->existDomain(conn, domainName);
                libvirt->disconnect(conn);
                return exist;
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                return false;
            }
        }

        DomainState::type getDomainState(const std::string& domainName)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                DomainState::type state = libvirt->getDomainState(conn, domainName);
                libvirt->disconnect(conn);
                return state;
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        void getDomainInfo(DomainInfo& _return, const std::string& domainName)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->getDomainInfo(_return, conn, domainName);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        void powerOn(const std::string& domainName)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->powerOn(conn, domainName);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        void powerOff(const std::string& domainName)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->powerOff(conn, domainName);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        void reset(const std::string& domainName)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->reset(conn, domainName);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        void pause(const std::string& domainName)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->pause(conn, domainName);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        void resume(const std::string& domainName)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->resume(conn, domainName);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        void createStoragePool(const std::string& xmlDesc)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->createStoragePool(conn, xmlDesc);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        bool isStoragePoolAlreadyCreated(const std::string& poolName)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                bool created = libvirt->isStoragePoolAlreadyCreated(conn, poolName);
                libvirt->disconnect(conn);
                return created;
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }

        void resizeDisk(const std::string& domainName, const std::string& diskPath, const double diskSizeInKb)
        {
            virConnectPtr conn = libvirt->connect();
            try
            {
                libvirt->resizeDisk(conn, domainName, diskPath, diskSizeInKb);
                libvirt->disconnect(conn);
            }
            catch (...)
            {
                libvirt->disconnect(conn);
                throw;
            }
        }
};

#endif
