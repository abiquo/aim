#include <MetricService.h>

MetricService::MetricService() : Service("Metrics") { }

MetricService::~MetricService() { }

bool MetricService::initialize(INIReader configuration)
{
    int collectFreq = configuration.GetInteger("stats", "collectFreqSeconds", 60);
    int refreshFreq = configuration.GetInteger("stats", "refreshFreqSeconds", 30);
    string database = configuration.Get("stats", "database", "/var/lib/abiquo-aim.db");

    if (collector.initialize(collectFreq, refreshFreq, database.c_str()) != COLLECTOR_OK) {
        return false;
    }

    return true;
}

bool MetricService::start()
{
    collectorThread = boost::thread(&MetricCollector::run, &collector);
    return true;
}

bool MetricService::stop()
{
    collectorThread.interrupt();
    collectorThread.join();
    return true;
}

bool MetricService::cleanup()
{
    return true;
}

void MetricService::getDatapoints(vector<Measure> &_result, string domainName, int from)
{
    collector.get_datapoints(domainName, from, _result);
}
