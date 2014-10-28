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

void MetricCollector::operator()()
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

            virDomainFree(domainsPtr[i]);
        }
        
        free(domainsPtr);
        virConnectClose(conn);
    }
}

void MetricCollector::read_domain_stats(const virDomainPtr domain, const virDomainInfo& domainInfo, Stats& stats)
{
    stats.cpu_time = domainInfo.cpuTime; // CPU time
    stats.used_mem = domainInfo.memory; // Used memory
    stats.vcpu_number = domainInfo.nrVirtCpu; // Number of vcpus
   
    // VCPU time
    virVcpuInfo vinfo[stats.vcpu_number];
    int max_vcpus = virDomainGetVcpus(domain, vinfo, stats.vcpu_number, NULL, 0);
   
    if (max_vcpus >= 0)
    {
        unsigned long long vcpu = 0;
        for (int j = 0; j < max_vcpus; j++)
        {
            vcpu += vinfo[j].cpuTime;
        }

        stats.vcpu_time = vcpu / domainInfo.nrVirtCpu; 
    }
}

void MetricCollector::read_disk_stats(const virDomainPtr domain, const virDomainInfo& domainInfo, vector<string> devices, Stats& stats)
{
    long long rd_req=0, wr_req=0, rd_bytes=0, wr_bytes=0;

    for (std::size_t i = 0; i < devices.size(); i++)
    {
        _virDomainBlockStats stats;
        if (virDomainBlockStats(domain, devices[i].c_str(), &stats, sizeof stats) >= 0)
        {
            if (stats.rd_req != -1) { rd_req += stats.rd_req; }
            if (stats.wr_req != -1) { wr_req += stats.wr_req; }
            if (stats.rd_bytes != -1) { rd_bytes += stats.rd_bytes; }
            if (stats.wr_bytes != -1) { wr_bytes += stats.wr_bytes; }
        }
    }

    if (!devices.empty())
    {
        rd_req /= devices.size();
        wr_req /= devices.size();
        rd_bytes /= devices.size();
        wr_bytes /= devices.size();
    }

    stats.disk_rd_requests = rd_req;
    stats.disk_rd_bytes = rd_bytes;
    stats.disk_wr_requests = wr_req;
    stats.disk_wr_bytes = wr_bytes;
}

void MetricCollector::read_interface_stats(const virDomainPtr domain, const virDomainInfo& domainInfo, vector<string> interfaces, Stats& stats)
{
    long long rx_bytes=0, rx_packets=0, rx_errs=0, rx_drop=0, tx_bytes=0, tx_packets=0, tx_errs=0, tx_drop=0;

    for (std::size_t i = 0; i < interfaces.size(); i++)
    {
        _virDomainInterfaceStats stats;
        if (virDomainInterfaceStats(domain, interfaces[i].c_str(), &stats, sizeof stats) >= 0)
        {
            if (stats.rx_bytes != -1) { rx_bytes += stats.rx_bytes; }
            if (stats.rx_packets != -1) { rx_packets += stats.rx_packets; }
            if (stats.rx_errs != -1) { rx_errs += stats.rx_errs; }
            if (stats.rx_drop != -1) { rx_drop += stats.rx_drop; }
            if (stats.tx_bytes != -1) { tx_bytes += stats.tx_bytes; }
            if (stats.tx_packets != -1) { tx_packets += stats.tx_packets; }
            if (stats.tx_errs != -1) { tx_errs += stats.tx_errs; }
            if (stats.tx_drop != -1) { tx_drop += stats.tx_drop; }          
        }
    }

    if (!interfaces.empty())
    {
        rx_bytes /= interfaces.size();
        rx_packets /= interfaces.size();
        rx_errs /= interfaces.size();
        rx_drop /= interfaces.size();
        tx_bytes /= interfaces.size();
        tx_packets /= interfaces.size();
        tx_errs /= interfaces.size();
        tx_drop /= interfaces.size();
    }

    stats.if_rx_bytes = rx_bytes;
    stats.if_rx_packets = rx_packets;
    stats.if_rx_errors = rx_errs;
    stats.if_rx_drops = rx_drop;
    stats.if_tx_bytes = tx_bytes;
    stats.if_tx_packets = tx_packets;
    stats.if_tx_errors = tx_errs;
    stats.if_tx_drops = tx_drop;
}

void MetricCollector::read_statistics(vector<Domain> domains)
{
    virConnectPtr conn = virConnectOpenReadOnly(NULL);
    if (conn != NULL)
    {
        for (std::size_t i = 0; i < domains.size(); i++)
        {
            const char *uuid = domains[i].uuid.c_str();
            virDomainPtr domainPtr = virDomainLookupByUUIDString(conn, uuid);
            if (domainPtr == NULL) {
                continue;
            }

            virDomainInfo domainInfo; 
            if (virDomainGetInfo(domainPtr, &domainInfo) >= 0)
            {
                std::time_t epoch;
                std::time(&epoch);

                Stats stats;
                stats.uuid = domains[i].uuid;
                stats.name = domains[i].name;
                read_domain_stats(domainPtr, domainInfo, stats);
                read_disk_stats(domainPtr, domainInfo, domains[i].devices, stats);
                read_interface_stats(domainPtr, domainInfo, domains[i].interfaces, stats);
            
                insert_stats(epoch, stats);
            }

            virDomainFree(domainPtr);
        }

        truncate_stats();
        virConnectClose(conn);
    }
    else
    {
        LOG("Unable to connect to libvirt");
    }
}

void MetricCollector::insert_stats(std::time_t &timestamp, const Stats &stats)
{
    sqlite3 *db;
    if (sqlite3_open(database.c_str(), &db)) {
        LOG("Insert stats error, cannot open database '%s'", database.c_str());
        return;
    }

    std::ostringstream ss;
    ss << "insert into domain_stats values('" << stats.uuid << "','" << stats.name << "',";
    ss << timestamp << "," << stats.cpu_time << "," << stats.used_mem << ",";
    ss << stats.vcpu_time << "," << stats.vcpu_number << ",";
    ss << stats.disk_rd_requests << "," << stats.disk_rd_bytes << ",";
    ss << stats.disk_wr_requests << "," << stats.disk_wr_bytes << ",";
    ss << stats.if_rx_bytes << "," << stats.if_rx_packets << ",";
    ss << stats.if_rx_errors << "," << stats.if_rx_drops << ",";
    ss << stats.if_tx_bytes << "," << stats.if_tx_packets << ",";
    ss << stats.if_tx_errors << "," << stats.if_tx_drops << ");";

    execute(db, ss.str().c_str());
    sqlite3_close(db);
}

void MetricCollector::truncate_stats()
{
    std::time_t now;
    std::time(&now);

    std::ostringstream ss;
    ss << "delete from domain_stats where timestamp < " << (now - 3600);

    sqlite3 *db;
    if (sqlite3_open(database.c_str(), &db)) {
        LOG("Trancate stats error, cannot open database '%s'", database.c_str());
        return;
    }

    execute(db, ss.str().c_str());
    sqlite3_close(db);
}
