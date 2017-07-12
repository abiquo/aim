#include <MetricCollector.h>

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    for (int i = 0; i < argc; i++) {
        LOG("%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
    }
    return 0;
}

static int execute(sqlite3 *db, const char *sql)
{
    char *zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        LOG("SQL error[%d]: %s", rc, zErrMsg);
        sqlite3_free(zErrMsg);
    }
    return rc;
}

MetricCollector::MetricCollector() { }

MetricCollector::~MetricCollector() { }

int MetricCollector::initialize(int collectFrequencySecs, int refreshFrequencySecs, const char *databaseFile)
{
    database = string(databaseFile);
    collect_frequency = collectFrequencySecs;
    refresh_frequency = refreshFrequencySecs;
    if (collect_frequency < MIN_COLLECT_FREQ_SECS) {
        LOG("Collect frequency must be >= %d seconds", MIN_COLLECT_FREQ_SECS);
        collect_frequency = MIN_COLLECT_FREQ_SECS;
    }

    sqlite3 *db;
    if (sqlite3_open(database.c_str(), &db)) {
        LOG("Cannot open database '%s'", database.c_str());
        return COLLECTOR_CANTOPEN;
    }

    execute(db, SQL_CREATE_STATS);
    sqlite3_close(db);

    LOG("Stats collector config: {collect=%ds, refresh=%ds, database='%s'}", collect_frequency, refresh_frequency, database.c_str());
    return COLLECTOR_OK;
}

void MetricCollector::run()
{
    boost::posix_time::seconds delay(collect_frequency);
    vector<Domain> domains;

    std::time_t now, last_refresh(0);

    while (true)
    {
        std::time(&now);

        // Need to refresh domain list?
        if (last_refresh == 0 || difftime(now, last_refresh) > refresh_frequency) {
            domains.clear();
            refresh(domains);
            last_refresh = now;
        }

        // Read and submit statistics
        read_statistics(domains);
        
        boost::this_thread::sleep(delay);
    }
}

void MetricCollector::parse_xml_dump(char *xml, Domain &domain)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer_inplace(xml, strlen(xml));
 
    if (result) {
        parse_dev_attribute(doc, "/domain/devices/disk/target[@dev]", domain.devices);
        parse_dev_attribute(doc, "/domain/devices/interface/target[@dev]", domain.interfaces);
    }
    else {
        LOG("Error loading XML '%s'. Cause: '%s'", xml, result.description());
    }
}

void MetricCollector::parse_dev_attribute(pugi::xml_document &doc, const char *xpath, vector<string> &collection)
{
    try
    {
        pugi::xpath_node_set nodes = doc.select_nodes(xpath);
        for (pugi::xpath_node_set::const_iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            pugi::xpath_node node = *it;
            string dev = string(node.node().attribute("dev").value());
            collection.push_back(dev);
        }            
    }
    catch (...)
    {
        LOG("Error while parsing xml with '%s' xpath expression", xpath);
    }
}

void MetricCollector::refresh(vector<Domain> &domains)
{
    virConnectPtr conn = virConnectOpenReadOnly(NULL);
    if (conn != NULL) {
        virDomainPtr *domainsPtr;
        int nr_domains = virConnectListAllDomains(conn, &domainsPtr, 0);

        for (int i = 0; i < nr_domains; i++) {
            char uuid[VIR_UUID_STRING_BUFLEN];
            if (virDomainGetUUIDString(domainsPtr[i], uuid) < 0) {
                virDomainFree(domainsPtr[i]);
                continue; 
            }
          
            const char *name = virDomainGetName(domainsPtr[i]);
            if (name == NULL) {
                virDomainFree(domainsPtr[i]);
                continue;
            }

            char *xml = virDomainGetXMLDesc(domainsPtr[i], 0);
            if (xml == NULL) { 
                virDomainFree(domainsPtr[i]);
                continue; 
            }
            
            Domain domain;
            domain.uuid = string(uuid);
            domain.name = string(name);
            parse_xml_dump(xml, domain);
            domains.push_back(domain);

            free((char*) xml); 
            virDomainFree(domainsPtr[i]);
        }
       
        free(domainsPtr);
        virConnectClose(conn);
    }
}

void MetricCollector::read_domain_stats(const string uuid, const string name, 
    const virDomainPtr domain, const virDomainInfo& domainInfo, vector<Stat> &stats)
{
    stats.push_back(stat(uuid, name, "cpu_time", "", "", domainInfo.cpuTime)); // CPU time
    stats.push_back(stat(uuid, name, "used_mem", "", "", domainInfo.memory)); // Used memory
    stats.push_back(stat(uuid, name, "vcpu_number", "", "", domainInfo.nrVirtCpu)); // Number of vcpus
   
    // VCPU time
    virVcpuInfo vinfo[domainInfo.nrVirtCpu];
    int max_vcpus = virDomainGetVcpus(domain, vinfo, domainInfo.nrVirtCpu, NULL, 0);
   
    if (max_vcpus >= 0)
    {
        unsigned long long vcpu = 0;
        for (int j = 0; j < max_vcpus; j++)
        {
            vcpu += vinfo[j].cpuTime;
        }

        stats.push_back(stat(uuid, name, "vcpu_time", "", "", vcpu / domainInfo.nrVirtCpu));
    }
}

void MetricCollector::read_disk_stats(const string uuid, const string name, 
    const virDomainPtr domain, const virDomainInfo& domainInfo, vector<string> devices, vector<Stat> &stats)
{
    for (std::size_t i = 0; i < devices.size(); i++)
    {
        _virDomainBlockStats domainBlockStats;
        if (virDomainBlockStats(domain, devices[i].c_str(), &domainBlockStats, sizeof domainBlockStats) >= 0)
        {
            if (domainBlockStats.rd_req != -1)   { stats.push_back(stat(uuid, name, "disk_rd_requests_total", "device", devices[i], domainBlockStats.rd_req)); }
            if (domainBlockStats.wr_req != -1)   { stats.push_back(stat(uuid, name, "disk_wr_requests_total", "device", devices[i], domainBlockStats.wr_req)); }
            if (domainBlockStats.rd_bytes != -1) { stats.push_back(stat(uuid, name, "disk_rd_bytes_total", "device", devices[i], domainBlockStats.rd_bytes));  }
            if (domainBlockStats.wr_bytes != -1) { stats.push_back(stat(uuid, name, "disk_wr_bytes_total", "device", devices[i], domainBlockStats.wr_bytes));  }
        }
    }
}

void MetricCollector::read_interface_stats(const string uuid, const string name, 
    const virDomainPtr domain, const virDomainInfo& domainInfo, vector<string> interfaces, vector<Stat> &stats)
{
    for (std::size_t i = 0; i < interfaces.size(); i++)
    {
        _virDomainInterfaceStats domainInterfaceStats;
        if (virDomainInterfaceStats(domain, interfaces[i].c_str(), &domainInterfaceStats, sizeof domainInterfaceStats) >= 0)
        {
            if (domainInterfaceStats.rx_bytes != -1)    { stats.push_back(stat(uuid, name, "if_rx_bytes_total", "interface", interfaces[i], domainInterfaceStats.rx_bytes));     }
            if (domainInterfaceStats.rx_packets != -1)  { stats.push_back(stat(uuid, name, "if_rx_packets_total", "interface", interfaces[i], domainInterfaceStats.rx_packets)); }
            if (domainInterfaceStats.rx_errs != -1)     { stats.push_back(stat(uuid, name, "if_rx_errors_total", "interface", interfaces[i], domainInterfaceStats.rx_errs));     }
            if (domainInterfaceStats.rx_drop != -1)     { stats.push_back(stat(uuid, name, "if_rx_drops_total", "interface", interfaces[i], domainInterfaceStats.rx_drop));      }
            if (domainInterfaceStats.tx_bytes != -1)    { stats.push_back(stat(uuid, name, "if_tx_bytes_total", "interface", interfaces[i], domainInterfaceStats.tx_bytes));     }
            if (domainInterfaceStats.tx_packets != -1)  { stats.push_back(stat(uuid, name, "if_tx_packets_total", "interface", interfaces[i], domainInterfaceStats.tx_packets)); }
            if (domainInterfaceStats.tx_errs != -1)     { stats.push_back(stat(uuid, name, "if_tx_errors_total", "interface", interfaces[i], domainInterfaceStats.tx_errs));     }
            if (domainInterfaceStats.tx_drop != -1)     { stats.push_back(stat(uuid, name, "if_tx_drops_total", "interface", interfaces[i], domainInterfaceStats.tx_drop));      }          
        }
    }
}

void MetricCollector::read_statistics(vector<Domain> domains)
{
    boost::mutex::scoped_lock lock(db_mutex);

    virConnectPtr conn = virConnectOpenReadOnly(NULL);
    if (conn != NULL)
    {
        LOG("Collecting domain statistics...");
        for (std::size_t i = 0; i < domains.size(); i++)
        {
            const char *uuid = domains[i].uuid.c_str();
            virDomainPtr domainPtr = virDomainLookupByUUIDString(conn, uuid);
            if (domainPtr == NULL) {
                continue;
            }

            virDomainInfo domainInfo; 
            vector<Stat> domainStats;
            if (virDomainGetInfo(domainPtr, &domainInfo) >= 0)
            {
                std::time_t epoch;
                std::time(&epoch);

                read_domain_stats(domains[i].uuid, domains[i].name, domainPtr, domainInfo, domainStats);
                read_disk_stats(domains[i].uuid, domains[i].name, domainPtr, domainInfo, domains[i].devices, domainStats);
                read_interface_stats(domains[i].uuid, domains[i].name, domainPtr, domainInfo, domains[i].interfaces, domainStats);
            
                insert_stats(epoch, domainStats);
            }

            virDomainFree(domainPtr);
        }

        LOG("Truncation of domain statistics..."); 
        truncate_stats();

        LOG("Recollection of domain statistics done");
        virConnectClose(conn);
    }
    else
    {
        LOG("Unable to connect to libvirt");
    }
}

void MetricCollector::insert_stats(std::time_t &timestamp, const vector<Stat> &domainStats)
{
    sqlite3 *db;
    if (sqlite3_open(database.c_str(), &db)) {
        LOG("Insert stats error, cannot open database '%s'", database.c_str());
        return;
    }

    for (std::size_t i = 0; i < domainStats.size(); i++)
    {
        Stat stat = domainStats[i];

        std::ostringstream ss;
        ss << "insert into stats values('" << stat.uuid << "','" << stat.name << "','" << stat.metric << "',";
        ss << timestamp << ",";
        if (stat.value_type == 0)       { ss << stat.ull << ","; }
        else if (stat.value_type == 1)  { ss << stat.ul << ",";  }
        else if (stat.value_type == 2)  { ss << stat.us << ",";  }
        else if (stat.value_type == 3)  { ss << stat.ll << ",";  }
        ss << "'" << stat.dimension_name << "','" << stat.dimension_value << "');";

        execute(db, ss.str().c_str());
    }

    sqlite3_close(db);
}

void MetricCollector::truncate_stats()
{
    std::time_t now;
    std::time(&now);

    std::ostringstream ss;
    ss << "delete from stats where timestamp < " << (now - 3600);

    sqlite3 *db;
    if (sqlite3_open(database.c_str(), &db)) {
        LOG("Trancate stats error, cannot open database '%s'", database.c_str());
        return;
    }

    execute(db, ss.str().c_str());
    sqlite3_close(db);
}

MetricCollector::Stat MetricCollector::stat(const string &uuid, const string &name, const string &metric, const string &dname, const string &dvalue,
                unsigned long long value)
{
    Stat stat;
    stat.uuid = uuid;
    stat.name = name;
    stat.metric = metric;
    stat.dimension_name = dname;
    stat.dimension_value = dvalue;
    stat.ull = value;
    stat.value_type = 0;
    return stat;
}

MetricCollector::Stat MetricCollector::stat(const string &uuid, const string &name, const string &metric, const string &dname, const string &dvalue,
                unsigned long value)
{
    Stat stat;
    stat.uuid = uuid;
    stat.name = name;
    stat.metric = metric;
    stat.dimension_name = dname;
    stat.dimension_value = dvalue;
    stat.ul = value;
    stat.value_type=1;
    return stat;
}

MetricCollector::Stat MetricCollector::stat(const string &uuid, const string &name, const string &metric, const string &dname, const string &dvalue,
                unsigned short value)
{
    Stat stat;
    stat.uuid = uuid;
    stat.name = name;
    stat.metric = metric;
    stat.dimension_name = dname;
    stat.dimension_value = dvalue;
    stat.us = value;
    stat.value_type = 2;
    return stat;
}

MetricCollector::Stat MetricCollector::stat(const string &uuid, const string &name, const string &metric, const string &dname, const string &dvalue,
                long long value)
{
    Stat stat;
    stat.uuid = uuid;
    stat.name = name;
    stat.metric = metric;
    stat.dimension_name = dname;
    stat.dimension_value = dvalue;
    stat.ll = value;
    stat.value_type = 3;
    return stat;
}

Measure MetricCollector::create_measure(string name)
{
    Measure measure;
    measure.metric = name;
    return measure;
}

Datapoint MetricCollector::create_datapoint(int timestamp, long value)
{
    Datapoint datapoint;
    datapoint.timestamp = timestamp;
    datapoint.value = value;
    return datapoint;
}

void MetricCollector::get_datapoints(string& name, int start, vector<Measure> &_return)
{
    boost::mutex::scoped_lock lock(db_mutex);

    LOG("Getting datapoints for domain %s from start %d...", name.c_str(), start);

    sqlite3 *db;
    if (sqlite3_open_v2(database.c_str(), &db, SQLITE_OPEN_READONLY, NULL)) {
        LOG("Unable to get datapoints, cannot open database '%s'", database.c_str());
        return;
    }
    
    sqlite3_stmt *stmt;
    const char *zTail;

    int rc = sqlite3_prepare(db,
            "select * from stats where name=? and timestamp >= ?;", 
            -1, &stmt, &zTail);
    
    if (rc != SQLITE_OK) {
        LOG("Unable to prepare statement, SQL error code: %d", rc);
        sqlite3_close(db);
        return;
    }

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        LOG("Unable to bind domain name, SQL error code: %d", rc);
        sqlite3_finalize(stmt);
        sqlite3_close(db);   
        return;
    }
    
    rc = sqlite3_bind_int(stmt, 2, start);
    if (rc != SQLITE_OK) {
        LOG("Unable to bind start timestamp, SQL error code: %d", rc);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string metric = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        long timestamp = sqlite3_column_int(stmt, 3);
        long value = sqlite3_column_int64(stmt, 4);
        string dn = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        string dv = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));

        Measure measure = create_measure(metric);
        measure.dimensions[dn] = dv;
        measure.datapoints.push_back(create_datapoint(timestamp, value));
        _return.push_back(measure);
    } 

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    LOG("%zu datapoints returned for domain %s", _return.size(), name.c_str());
} 
