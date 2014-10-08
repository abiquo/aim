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

#ifndef METRIC_COLLECTOR_H
#define METRIC_COLLECTOR_H

#include <string>
#include <vector>

#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#include <pugixml.hpp>

using namespace std;

class MetricCollector
{
    private:
        int collect_frequency;
        int refresh_frequency;

        struct Domain
        {
            string uuid;
            vector<string> devices;
            vector<string> interfaces;
        };

        void parse_xml_dump(char *xml, Domain &domain);
        void parse_dev_attribute(pugi::xml_document &doc, const char *xpath, vector<string> &collection);
        void refresh(vector<Domain> &domains);
        void read_domain_stats(const virDomainPtr domain, const virDomainInfo& domainInfo);
        void read_disk_stats(const virDomainPtr domain, const virDomainInfo& domainInfo, vector<string> devices);
        void read_interface_stats(const virDomainPtr domain, const virDomainInfo& domainInfo, vector<string> interfaces);
        void read_statistics(vector<Domain> domains);
    
    public:
        MetricCollector(int collectFrequency, int refreshFrequency);
        ~MetricCollector();

        void operator()();
};

#endif
