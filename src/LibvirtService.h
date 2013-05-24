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

using namespace std;

class LibvirtService : public Service
{
    private:
        static string connectionUrl;

        virDomainPtr getDomainByName(virConnectPtr conn, const string& name) throw (LibvirtException);
        DomainInfo getDomainInfo(const virConnectPtr conn, const virDomainPtr domain) throw (LibvirtException);

    public:
        LibvirtService();
        ~LibvirtService();

        void throwError(const string& message);

        virtual bool initialize(dictionary * configuration);
        virtual bool cleanup();
        virtual bool start();
        virtual bool stop();

        // Connection
        virConnectPtr connect() throw (LibvirtException);       // Open a LOCAL connection
        void disconnect(virConnectPtr conn);                    // Closes the given connection

        // Libvirt facade methods
        void getNodeInfo(NodeInfo& _return, virConnectPtr conn) throw (LibvirtException);
        void getDomains(std::vector<DomainInfo> & _return, virConnectPtr conn) throw (LibvirtException);
        void defineDomain(virConnectPtr conn, const std::string& xmlDesc) throw (LibvirtException);
        void undefineDomain(virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        bool existDomain(virConnectPtr conn, const std::string& domainName);
        void getDomainState(std::string& _return, virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void getDomainInfo(DomainInfo& _return, virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void powerOn(virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void powerOff(virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void reset(virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void pause(virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void resume(virConnectPtr conn, const std::string& domainName) throw (LibvirtException);
        void createStoragePool(virConnectPtr conn, const std::string& xmlDesc) throw (LibvirtException);
        void resizeDisk(virConnectPtr conn, const string& domainName, const string& diskPath, const double diskSizeInKb) throw (LibvirtException);
};

#endif
