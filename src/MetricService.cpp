#include <MetricService.h>

#include <Debug.h>
#include <Macros.h>

MetricService::MetricService() : Service("Metrics") { }

MetricService::~MetricService() { }

bool MetricService::initialize(INIReader configuration)
{
    int collectFreq = configuration.GetInteger("stats", "collectFreqSeconds", 60);
    int refreshFreq = configuration.GetInteger("stats", "refreshFreqSeconds", 30);
    string database = configuration.Get("stats", "database", "/var/lib/abiquo-aim");

    if (collector.initialize(collectFreq, refreshFreq, database.c_str()) != COLLECTOR_OK) {
        return false;
    }

    return true;
}

bool MetricService::start()
{
    collectorThread = boost::thread(collector);
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