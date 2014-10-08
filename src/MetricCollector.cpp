#include <MetricCollector.h>
#include <Debug.h>
#include <Macros.h>

#include <ctime>

#include <boost/thread.hpp>
#include <sqlite3.h>

/*
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
*/
MetricCollector::MetricCollector(int collectFrequency, int refreshFrequency) 
: collect_frequency(collectFrequency) , refresh_frequency(refreshFrequency)
{ 
}

MetricCollector::~MetricCollector() { }

void MetricCollector::operator()()
{
    boost::posix_time::seconds delay(collect_frequency);
    vector<Domain> domains;
    LOG("Entering statistics collection loop...");

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
    // TODO result?
   
    parse_dev_attribute(doc, "/domain/devices/disk/target[@dev]", domain.devices);
    parse_dev_attribute(doc, "/domain/devices/interface/target[@dev]", domain.interfaces);
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
        // TODO
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
           
            char *xml = virDomainGetXMLDesc(domainsPtr[i], 0);
            if (xml == NULL) { 
                virDomainFree(domainsPtr[i]);
                continue; 
            }
            
            Domain domain;
            domain.uuid = string(uuid);
            parse_xml_dump(xml, domain);
            domains.push_back(domain);

            virDomainFree(domainsPtr[i]);
        }
        
        free(domainsPtr);
        virConnectClose(conn);
    }
}

void MetricCollector::read_domain_stats(const virDomainPtr domain, const virDomainInfo& domainInfo)
{
    // CPU
    LOG("\tcpu_time = %lu", /* unsigned long long */ domainInfo.cpuTime);

    // Memory
    LOG("\tused_mem (KBytes) = %lu", /* unsigned long */ domainInfo.memory);
    
    // VCPU
    virVcpuInfoPtr vinfo = static_cast<virVcpuInfoPtr>(malloc(domainInfo.nrVirtCpu * sizeof(virVcpuInfoPtr)));

    if (virDomainGetVcpus(domain, vinfo, domainInfo.nrVirtCpu, NULL, 0) >= 0)
    {
        unsigned long long vcpu = 0;
        for (int j = 0; j < domainInfo.nrVirtCpu; j++)
        {
            vcpu += vinfo[j].cpuTime;
        }

        LOG("\tvcpu_time = %lu", vcpu / domainInfo.nrVirtCpu); 
    }

    LOG("\tvcpu_number = %d", domainInfo.nrVirtCpu);
}

void MetricCollector::read_disk_stats(const virDomainPtr domain, const virDomainInfo& domainInfo, vector<string> devices)
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

    LOG("\tdisk_rd_requests = %d", rd_req);
    LOG("\tdisk_rd_bytes = %d", rd_bytes);
    LOG("\tdisk_wr_requests = %d", wr_req);
    LOG("\tdisk_wr_bytes = %d", wr_bytes);
}

void MetricCollector::read_interface_stats(const virDomainPtr domain, const virDomainInfo& domainInfo, vector<string> interfaces)
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

    LOG("\tif_rx_bytes = %d", rx_bytes);
    LOG("\tif_rx_packets = %d", rx_packets);
    LOG("\tif_rx_errors = %d", rx_errs);
    LOG("\tif_rx_drops = %d", rx_drop);
    LOG("\tif_tx_bytes = %d", tx_bytes);
    LOG("\tif_tx_packets = %d", tx_packets);
    LOG("\tif_tx_errors = %d", tx_errs);
    LOG("\tif_tx_drops = %d", tx_drop);
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

            LOG("Statistics for '%s' domain", uuid);

            virDomainInfo domainInfo; 
            if (virDomainGetInfo(domainPtr, &domainInfo) >= 0)
            {
                read_domain_stats(domainPtr, domainInfo);
                read_disk_stats(domainPtr, domainInfo, domains[i].devices);
                read_interface_stats(domainPtr, domainInfo, domains[i].interfaces);
            }

            virDomainFree(domainPtr);
        }

        virConnectClose(conn);

        sqlite3 *db;
        int rc = sqlite3_open("/opt/test.db", &db);
    }
    else
    {
        LOG("Unable to connect to libvirt");
    }
}
