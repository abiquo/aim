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

const static char * SQL_CREATE_STATS =  "CREATE TABLE domain_stats(" \
                                        "uuid               TEXT    NOT NULL," \
                                        "timestamp          INTEGER NOT NULL," \
                                        "cpu_time           INTEGER," \
                                        "used_mem           INTEGER," \
                                        "vcpu_time          INTEGER," \
                                        "vcpu_number        INTEGER," \
                                        "disk_rd_requests   INTEGER," \
                                        "disk_rd_bytes      INTEGER," \
                                        "disk_wr_requests   INTEGER," \
                                        "disk_wr_bytes      INTEGER," \
                                        "if_rx_bytes        INTEGER," \
                                        "if_rx_packets      INTEGER," \
                                        "if_rx_errors       INTEGER," \
                                        "if_rx_drops        INTEGER," \
                                        "if_tx_bytes        INTEGER," \
                                        "if_tx_packets      INTEGER," \
                                        "if_tx_errors       INTEGER," \
                                        "if_tx_drops        INTEGER);";
class MetricCollector
{
    private:
        int collect_frequency;
        int refresh_frequency;
        int rows;
        string database;

        struct Domain
        {
            string uuid;
            vector<string> devices;
            vector<string> interfaces;
        };

        struct Stats
        {
            string uuid;
            unsigned long long cpu_time;
            unsigned long used_mem;
            unsigned long long vcpu_time;
            unsigned short vcpu_number; 
            long long disk_rd_requests;
            long long disk_rd_bytes;
            long long disk_wr_requests;
            long long disk_wr_bytes;
            long long if_rx_bytes;
            long long if_rx_packets; 
            long long if_rx_errors;
            long long if_rx_drops;
            long long if_tx_bytes;
            long long if_tx_packets;
            long long if_tx_errors;
            long long if_tx_drops;
        };

        void parse_xml_dump(char *xml, Domain &domain);
        void parse_dev_attribute(pugi::xml_document &doc, const char *xpath, vector<string> &collection);
        void refresh(vector<Domain> &domains);
        void read_domain_stats(const virDomainPtr domain, const virDomainInfo& domainInfo, Stats& stats);
        void read_disk_stats(const virDomainPtr domain, const virDomainInfo& domainInfo, vector<string> devices, Stats& stats);
        void read_interface_stats(const virDomainPtr domain, const virDomainInfo& domainInfo, vector<string> interfaces, Stats& stats);
        void read_statistics(vector<Domain> domains);
        void insert(std::time_t &timestamp, const Stats &stats);

    public:
        MetricCollector();
        ~MetricCollector();
        
        int initialize(int collectFrequencySecs, int refreshFrequencySecs, const char *databaseFile);
        void operator()();
};

#endif
