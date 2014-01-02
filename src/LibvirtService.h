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

#ifndef LIBVIRT_SERVICE_H
#define LIBVIRT_SERVICE_H

#include <string>
#include <Service.h>
#include <aim_types.h>
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>

using namespace std;

class LibvirtService : public Service
{
    private:
        static string connectionUrl;

        virDomainPtr getDomainByName(virConnectPtr conn, const string& name) throw (LibvirtException);
        DomainInfo getDomainInfo(const virConnectPtr conn, const virDomainPtr domain) throw (LibvirtException);
        DomainState::type toDomainState(unsigned char state);
        void defineStoragePool(const virConnectPtr conn, const std::string& xmlDesc) throw (LibvirtException);

        bool existPrimaryDisk(const DomainInfo& domainInfo);

        string parseDevicePath(const std::string& xmlDesc);
        string parseTargetPath(const std::string& xmlDesc);
        void parseSourceHostAndDir(const std::string& xmlDesc, std::string& host, std::string& dir);
        string stringBetween(const std::string& input, const std::string& startPattern, const std::string& endPattern);
        string to_string(const double value);

    public:
        LibvirtService();
        ~LibvirtService();

        void throwLastKnownError(const virConnectPtr conn);

        virtual bool initialize(dictionary * configuration);
        virtual bool cleanup();
        virtual bool start();
        virtual bool stop();

        // Connection
        virConnectPtr connect() throw (LibvirtException);       // Open a LOCAL connection
        void disconnect(const virConnectPtr conn);                    // Closes the given connection

        // Utils
        bool comparePaths(const std::string& one, const std::string& other);
        bool endsWith(const std::string& value, const std::string& end);

        // Libvirt facade methods
        void getNodeInfo(NodeInfo& _return, const virConnectPtr conn) throw (LibvirtException);
        void getDomains(std::vector<DomainInfo> & _return, const virConnectPtr conn) throw (LibvirtException);
        void defineDomain(const virConnectPtr conn, const std::string& xmlDesc) throw (LibvirtException);
        void undefineDomain(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        bool existDomain(const virConnectPtr conn, const std::string& domainName);
        DomainState::type getDomainState(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void getDomainInfo(DomainInfo& _return, virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void powerOn(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void powerOff(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void reset(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void pause(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void resume(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void createISCSIStoragePool(const virConnectPtr conn, const std::string& name, const std::string& host, const std::string& iqn, const std::string& targetPath) throw (LibvirtException);
        void createNFSStoragePool(const virConnectPtr conn, const std::string& name, const std::string& host, const std::string& dir, const std::string& targetPath) throw (LibvirtException);
        void createDirStoragePool(const virConnectPtr conn, const std::string& name, const std::string& targetPath) throw (LibvirtException);
        void createDisk(const virConnectPtr conn, const string& poolName, const string& name, const double capacityInKb, const double allocationInKb, const string& format) throw (LibvirtException);
        void deleteDisk(const virConnectPtr conn, const string& poolName, const string& name) throw (LibvirtException);
        void resizeVol(const virConnectPtr conn, const string& poolName, const string& name, const double capacityInKb) throw (LibvirtException);
        void resizeDisk(const virConnectPtr conn, const string& domainName, const string& diskPath, const double diskSizeInKb) throw (LibvirtException);
};

#endif
