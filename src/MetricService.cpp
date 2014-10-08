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
    MetricCollector collector(5, 30);
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
