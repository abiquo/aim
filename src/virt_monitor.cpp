#include <virt_monitor.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <libvirt/libvirt.h>
#include <Debug.h>

# ifndef ATTRIBUTE_UNUSED
#  define ATTRIBUTE_UNUSED __attribute__((__unused__))
# endif

const char* eventToString(int event, int detail);
void (*notify_routine)(const char*, const char*) = NULL;

/* Callback functions */

const char *eventToString(int event, int detail) {
    const char *ret = "\0";
    switch(event)
    {
        case VIR_DOMAIN_EVENT_DEFINED:
            ret = CREATED;
            break;

        case VIR_DOMAIN_EVENT_UNDEFINED:
            ret = DESTROYED;
            break;

        case VIR_DOMAIN_EVENT_STARTED:
            ret = POWEREDON;
            break;

        case VIR_DOMAIN_EVENT_SUSPENDED:
            ret = SUSPENDED;
            break;

        case VIR_DOMAIN_EVENT_RESUMED:
            ret = RESUMED;
            break;

        case VIR_DOMAIN_EVENT_STOPPED:
            if (detail == VIR_DOMAIN_EVENT_STOPPED_SAVED)
            {
                ret = SAVED;
            }
            else
            {
                ret = POWEREDOFF;
            }

            break;
    }
    return ret;
}

static int domainEventCallback(virConnectPtr conn ATTRIBUTE_UNUSED, virDomainPtr dom, int event, int detail, void *opaque ATTRIBUTE_UNUSED)
{
    char* domainName = (char*) virDomainGetName(dom);
    int domainId = virDomainGetID(dom);
    const char* eventString = eventToString(event, detail);

    if (strlen(eventString) > 0)
    {
        LOG("%s EVENT: Domain %s(%d) %s", __func__, domainName, domainId, eventString);
        notify_routine(domainName, eventString);
    }

    return 0;
}

static void freeFunc(void *opaque)
{
    char *str = (char*)opaque;
    LOG("%s: Freeing [%s]", __func__, str);
    free(str);
}

virConnectPtr conn = NULL;
int run = 0;

int connect(const char * url, void (*callback_routine)(const char*, const char*))
{
    // Registers a default event implementation based on the poll() system call
    virEventRegisterDefaultImpl();

    // Connect to libvirt
    conn = virConnectOpenReadOnly(url);
    if (!conn)
    {
        LOG("Unable to connect poller to %s", url);
        return 0;
    }

    // Register the callback
    if (virConnectDomainEventRegister(  conn, 
                                        domainEventCallback, 
                                        strdup("CALLBACK"), freeFunc) < 0)
    {
        LOG("Unable to register domain event callback");
        return 0;
    }

    // EventsMonitor callback
    notify_routine = callback_routine;

    // The poller can start
    run = 1;
    return run;
}

void * listen(void* opaque)
{
    while(run)
    {
        // Run one iteration of the event loop.
        if (virEventRunDefaultImpl() < 0)
        {
            LOG("Failed while iterating event loop");
            return 0;
        }
    }

    LOG("Deregistering event handlers");
    virConnectDomainEventDeregister(conn, domainEventCallback);
    LOG("Closing connection");
    
    if (conn && virConnectClose(conn) < 0)
    {
        LOG("error closing");
    }

    return 0;
}

void cancel()
{
    run = 0;
}

