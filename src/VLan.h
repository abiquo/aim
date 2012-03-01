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

#ifndef VLAN_H
#define VLAN_H

#include <Service.h>
#include <string>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <boost/thread/mutex.hpp>

#define NETWORK_SCRIPTS_FOLDER "/etc/sysconfig/network-scripts\0"

using namespace std;

class VLan: public Service
{
    protected:
        string ifconfig;
        string vconfig;
        string brctl;

        bool commandExist(string& command);
        
        void throwError(const string& message);

        void checkVlanRange(const int vlan);
        
        bool existsVlan(const int vlan, const string& vlanInterface);
        
        bool existsInterface(const string& interface);

        bool existsBridge(const string& interface);

        /** VLAN related methods */
        bool createVLANInterface(int vlan, const string& vlanIf, const string& bridgeIf);
        bool deleteVLANInterface(int vlan, const string& vlanIf);
        bool writeVLANConfiguration(const string& device, int vlan, const string& bridgeName, const string& folder, const string& filename);
        string buildVLANFilename(int vlan, const string& vlanIf);

        /** Bridge related methods */
        bool createBridgeInterface(const string& bridgeIf);
        bool deleteBridgeInterface(const string& bridgeIf);
        bool writeBridgeConfiguration(const string& device, const string& folder, const string& filename);
        string buildBridgeFilename(const string& bridgeIf);

        /** Filesystem helper methods */
        bool isAccessible(const string& path);
        bool removeFile(const string& folder, const string& filename);

        /** Execute command related */
        int executeCommand(string command, bool redirect = false);
        bool ifUp(string& filename);
        bool ifDown(string& filename);

        boost::mutex delete_vlan_mutex;

    public:
        VLan();
        ~VLan();

        virtual bool initialize(dictionary * configuration);
        virtual bool cleanup();
        virtual bool start();
        virtual bool stop();

        void createVLAN(int vlan, const string& vlanInterface, const string& bridgeInterface);
        void deleteVLAN(int vlan, const string& vlanInterface, const string& bridgeInterface);

        void checkVLANConfiguration();
};

#endif
