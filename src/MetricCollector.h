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

#include <Debug.h>

#include <string>
#include <vector>
#include <ctime>

#include <boost/thread.hpp>

#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>
#include <pugixml.hpp>
#include <sqlite3.h>

#define MIN_COLLECT_FREQ_SECS 60

/* beginning of return codes */
#define COLLECTOR_OK        0
#define COLLECTOR_CANTOPEN  1
/* end of return codes */

using namespace std;

const static char * SQL_CREATE_STATS =  "CREATE TABLE stats(" \
                                        "uuid               TEXT    NOT NULL," \
                                        "name               TEXT    NOT NULL," \
                                        "metric             TEXT    NOT NULL," \
                                        "timestamp          INTEGER NOT NULL," \
                                        "value              INTEGER NOT NULL," \
                                        "dimension_name     TEXT," \
                                        "dimension_value    TEXT);";

class MetricCollector
{
    private:
        int collect_frequency;
        int refresh_frequency;
        string database;

        struct Domain
        {
            string uuid;
            string name;
            vector<string> devices;
            vector<string> interfaces;
        };

        struct Stat
        {
            string uuid;
            string name;
            string metric;
            string dimension_name;
            string dimension_value;
            int value_type;
            unsigned long long ull; // value_type=0
            unsigned long ul;       // value_type=1
            unsigned short us;      // value_type=2
            long long ll;           // value_type=3
        };

        void parse_xml_dump(char *xml, Domain &domain);
        void parse_dev_attribute(pugi::xml_document &doc, const char *xpath, vector<string> &collection);
        void refresh(vector<Domain> &domains);
        void read_domain_stats(const string uuid, const string name, const virDomainPtr domain, const virDomainInfo& domainInfo, 
                vector<Stat> &stats);
        void read_disk_stats(const string uuid, const string name, const virDomainPtr domain, const virDomainInfo& domainInfo, 
                vector<string> devices, vector<Stat> &stat);
        void read_interface_stats(const string uuid, const string name, const virDomainPtr domain, const virDomainInfo& domainInfo, 
                vector<string> interfaces, vector<Stat> &stats);
        void read_statistics(vector<Domain> domains);
        void insert_stats(std::time_t &timestamp, const vector<Stat> &domainStats);
        void truncate_stats();
        Stat stat(const string &uuid, const string &name, const string &metric, const string &dname, const string &dvalue,
                unsigned long long value);
        Stat stat(const string &uuid, const string &name, const string &metric, const string &dname, const string &dvalue,
                unsigned long value);
        Stat stat(const string &uuid, const string &name, const string &metric, const string &dname, const string &dvalue,
                unsigned short value);
        Stat stat(const string &uuid, const string &name, const string &metric, const string &dname, const string &dvalue,
                long long value);
    public:
        MetricCollector();
        ~MetricCollector();
        
        int initialize(int collectFrequencySecs, int refreshFrequencySecs, const char *databaseFile);
        void operator()();
};

#endif
