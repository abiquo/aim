#include <MetricService.h>
#include <MetricCollector.h>

#include <Debug.h>
#include <Macros.h>

MetricService::MetricService() : Service("Metrics") { }

MetricService::~MetricService() { }

bool MetricService::initialize(INIReader configuration)
{
    return true;
}

bool MetricService::start()
{
    MetricCollector collector(/* collect every seconds */ 60, /* refresh domains each seconds */ 30);
    if (collector.initialize(/* domain stats database */ "/opt/test.db") != COLLECTOR_OK) {
        return false;
    }
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
