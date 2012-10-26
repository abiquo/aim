#include <EventsMonitor.h>
#include <virt_monitor.h>
#include <iniparser.h>
#include <Macros.h>
#include <ConfigConstants.h>
#include <Debug.h>
#include <sstream>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>

EventsMonitor::EventsMonitor() : Service("EventsMonitor")
{
}

EventsMonitor::~EventsMonitor()
{
}

EventsMonitor* EventsMonitor::instance = NULL;
string EventsMonitor::host = "";
string EventsMonitor::machineAddress = "";
int EventsMonitor::port = -1;
string EventsMonitor::machinePort = "";

EventsMonitor* EventsMonitor::getInstance()
{
    if (instance == NULL)
    {
        instance = new EventsMonitor();
    }

    return instance;
}

void EventsMonitor::callback(const char* uuid, const char* event)
{
    redisContext *ctx = redisConnect(EventsMonitor::host.c_str(), EventsMonitor::port);

    if (ctx->err)
    {
        LOG("Unable to connect to redis %s:%d. %s", EventsMonitor::host.c_str(), EventsMonitor::port, ctx->errstr);
        return;
    }   

    // Build hypervisor address
    ostringstream uri;
    uri << "http://" << EventsMonitor::machineAddress << ":" << EventsMonitor::machinePort << "/";
    uri.flush();

    string address = uri.str();

    // Retrieve subscription and monitor id
    string subscriptionId = getSubscriptionId(ctx, uuid);
    string monitorId = getMonitorId(ctx, address.c_str());

    if (subscriptionId.empty())
    {
        LOG("There is currently no subscription for %s. Ignoring %s event.", uuid, event);
        redisFree(ctx);
        return;
    }

    if (monitorId.empty())
    {
        LOG("This machine (%s) is not monitored. Ignoring %s event.", address.c_str(), event);
        redisFree(ctx);
        return;
    }

    // Check if the subscription for this virtual machine points to this monitor
    bool moved = subscribedToOtherMonitor(ctx, subscriptionId.c_str(), monitorId.c_str());

    if (!moved)
    {
        publish(ctx, uuid, event, address.c_str());
    }
    else if (strcmp(POWEREDON, event) == 0 || strcmp(RESUMED, event) == 0)
    {
        if (updateSubscriptionMonitor(ctx, subscriptionId.c_str(), monitorId.c_str()))
        {
            publish(ctx, uuid, MOVED, address.c_str());
            publish(ctx, uuid, POWEREDON, address.c_str());
        }
    }

    redisFree(ctx);
}

bool EventsMonitor::updateSubscriptionMonitor(redisContext* ctx, const char* subscriptionId, const char* monitorId)
{
    redisReply* reply = (redisReply*)redisCommand(ctx, "HSET VirtualMachine:%s physicalMachine_id %s", subscriptionId, monitorId);
    bool updated = false;

    if (reply != NULL)
    {
        LOG("Subscription %s has been updated, monitor id is now %s", subscriptionId, monitorId);
        updated = true;
    }
    else
    {
        LOG("Unable to update monitor of subscription %s. %s", subscriptionId, ctx->errstr);
    }

    freeReplyObject(reply);
    return updated;
}

bool EventsMonitor::subscribedToOtherMonitor(redisContext* ctx, const char* subscriptionId, const char* monitorId)
{
    redisReply* reply = (redisReply*)redisCommand(ctx, "HGET VirtualMachine:%s physicalMachine_id", subscriptionId);
    string otherId = string(reply->str);
    freeReplyObject(reply);

    return strcmp(otherId.c_str(), monitorId) != 0;
}

string EventsMonitor::getSubscriptionId(redisContext* ctx, const char* uuid)
{
    redisReply *reply = (redisReply*)redisCommand(ctx, "SMEMBERS VirtualMachine:name:%s", uuid);

    if (reply != NULL)
    {
        if (reply->type == REDIS_REPLY_ARRAY)
        {
            if (reply->elements == 0)
            {
                return string("");
            }

            string id = string(reply->element[0]->str);
            freeReplyObject(reply);
            return id;
        }
    }

    return string("");
}

string EventsMonitor::getMonitorId(redisContext* ctx, const char* address)
{   
    redisReply *reply = (redisReply*)redisCommand(ctx, "SMEMBERS PhysicalMachine:address:%s", address);
           
    if (reply != NULL)
    {
        if (reply->type == REDIS_REPLY_ARRAY)
        {
            if (reply->elements == 0)
            {   
                return string("");
            }

            string id = string(reply->element[0]->str);
            freeReplyObject(reply);
            return id;
        }
    }
                                                                                                        
    return string();
}

void EventsMonitor::publish(redisContext* ctx, const char* uuid, const char* event, const char* address)
{
    LOG("Publish '%s' event on machine '%s' to redis.", event, uuid);

    void* reply = redisCommand(ctx, "PUBLISH EventingChannel %s|%s|%s", uuid, event, address);

    if (reply != NULL)
    {
        freeReplyObject(reply);
    }
    else
    {
        LOG("Unable to notify event. %s", ctx->errstr);
    }
}

bool EventsMonitor::initialize(dictionary * configuration)
{
    const char* uri = getStringProperty(configuration, monitorUri);
    EventsMonitor::host = getStringProperty(configuration, redisHost);
    EventsMonitor::port = getIntProperty(configuration, redisPort);
    EventsMonitor::machinePort = getStringProperty(configuration, serverPort);

    bool initialized = true;

    // connect to libvirt
    if (!connect(uri, callback))
    {
        LOG("Unable to connect to hypervisor uri '%s'", uri);
        initialized = false;
    }

    if (initialized)
    {
        EventsMonitor::machineAddress = getIP(EventsMonitor::host, EventsMonitor::port);
        initialized = !EventsMonitor::machineAddress.empty();
    }

    if (initialized)
    {
        LOG("Physical machine address is http://%s:%s", EventsMonitor::machineAddress.c_str(), EventsMonitor::machinePort.c_str());
    }

    return initialized;
}

string EventsMonitor::getIP(string& address, int port)
{
    struct sockaddr_in to, my;
    int sd, rc;

    struct hostent *hp = gethostbyname(address.c_str());

    if (hp == NULL)
    {
        LOG("Unable to resolve hostname %s", address.c_str());
        return string("");
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);

    to.sin_family = AF_INET;
    bcopy ( hp->h_addr, &(to.sin_addr.s_addr), hp->h_length);
    to.sin_port = htons(port);
    memset(&(to.sin_zero), 0, sizeof(to.sin_zero));

    rc = connect(sd, (struct sockaddr *)&to, sizeof(struct sockaddr_in));

    if (rc < 0)
    {
        LOG("Unable to connect to %s:%d", address.c_str(), port);
        return string("");
    }

    socklen_t len = sizeof(struct sockaddr);
    rc = getsockname(sd, (struct sockaddr *)&my, &len);

    if (rc < 0)
    {
        LOG("Unable to get the sock name %s:%d", address.c_str(), port);
        return string("");
    }

    string ip = string(inet_ntoa(my.sin_addr));

    close(sd);

    return ip;
}

bool EventsMonitor::start()
{
    pthread_attr_t attrJoinable;

    pthread_attr_init(&attrJoinable);
    pthread_attr_setdetachstate(&attrJoinable, PTHREAD_CREATE_JOINABLE);

    int ret = pthread_create(&threadId, &attrJoinable, listen, NULL);
    return (ret != 1);
}

bool EventsMonitor::stop()
{
    cancel();
    pthread_join(threadId, NULL); 

    return true;
}

bool EventsMonitor::cleanup()
{
    return true;
}

