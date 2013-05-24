#include <LibvirtService.h>
#include <ConfigConstants.h>
#include <Debug.h>
#include <Macros.h>

#include <iniparser.h>
#include <dictionary.h>

#include <libvirt/virterror.h>
#include <string>
#include <sstream>

string LibvirtService::connectionUrl = "";

LibvirtService::LibvirtService() : Service("Libvirt service")
{
}

LibvirtService::~LibvirtService()
{
}

// Private methods

virDomainPtr LibvirtService::getDomainByName(virConnectPtr conn, const string& name) throw (LibvirtException)
{
    virDomainPtr dom = virDomainLookupByName(conn, name.c_str());
    if (dom == NULL)
    {
        throwError("Domain not found: " + name);
    }
    return dom;
}

DomainInfo LibvirtService::getDomainInfo(const virConnectPtr conn, const virDomainPtr domain) throw (LibvirtException)
{
    if (domain == NULL)
    {
        throwError(""); // TODO
    }

    virDomainInfo info;
    if (virDomainGetInfo(domain, &info) < 0)
    {
        throwError(""); // TODO
    }

    const char *name = virDomainGetName(domain);
    if (name == NULL)
    {
        throwError(""); // TODO
    }

    DomainInfo domainInfo;
    domainInfo.name            = string(name);
    domainInfo.uuid            = "";                // TODO
    domainInfo.state           = info.state;        // the running state, one of virDomainState
    domainInfo.numberVirtCpu   = info.nrVirtCpu;    // the number of virtual CPUs for the domain 
    domainInfo.memory          = info.memory;       // the memory in KBytes used by the domain
    return domainInfo;
}

// Public methods

bool LibvirtService::initialize(dictionary* configuration)
{
    if (LibvirtService::connectionUrl.empty())
    {
        LibvirtService::connectionUrl = getStringProperty(configuration, monitorUri);
    }

    return !LibvirtService::connectionUrl.empty();
}

bool LibvirtService::start()
{
    return true;
}

bool LibvirtService::stop()
{
    return true;
}

bool LibvirtService::cleanup()
{
    return true;
}

void LibvirtService::throwError(const string& message)
{
    LibvirtException exception;
    exception.description = message;
    throw exception;
}

virConnectPtr LibvirtService::connect() throw (LibvirtException)
{
    virConnectPtr conn = virConnectOpen(LibvirtService::connectionUrl.c_str());
    if (conn == NULL)
    {
        throwError("Could not connect to " + LibvirtService::connectionUrl);
    }
    return conn;
}

void LibvirtService::disconnect(virConnectPtr conn)
{
    if (virConnectClose(conn) < 0)
    {
        LOG(("Error closing connection: " + LibvirtService::connectionUrl).c_str());
    }
}

void LibvirtService::getNodeInfo(NodeInfo& _return, virConnectPtr conn) throw (LibvirtException)
{
    virNodeInfo info;

    if (virNodeGetInfo(conn, &info) < 0)
    {
        throwError(""); // TODO
    }

    _return.cores   = info.cores;    // number of cores per socket
    _return.sockets = info.sockets;  // number of CPU sockets per node
    _return.memory  = info.memory;   // memory size in kilobytes
}

// Adapted from http://builder.virt-tools.org/artifacts/libvirt-appdev-guide/html/Application_Development_Guide-Guest_Domains-Listing.html
void LibvirtService::getDomains(std::vector<DomainInfo> & _return, virConnectPtr conn) throw (LibvirtException)
{
    int numActiveDomains = virConnectNumOfDomains(conn);
    if (numActiveDomains > 0)
    {
        int *activeDomains = (int*) malloc(sizeof(int) * numActiveDomains);
        numActiveDomains = virConnectListDomains(conn, activeDomains, numActiveDomains);

        for (int i = 0; i < numActiveDomains; i++)
        {
            virDomainPtr domain = virDomainLookupByID(conn, activeDomains[i]);
                
            if (domain != NULL)
            {
                try
                {
                    _return.push_back(getDomainInfo(conn, domain));
                    virDomainFree(domain);
                }
                catch (...)
                {
                    virDomainFree(domain);
                }
            }
        }

        free(activeDomains);
    }
    // TODO else => throwError?

    int numInactiveDomains = virConnectNumOfDefinedDomains(conn);
    if (numInactiveDomains > 0)
    {
        char **inactiveDomains = (char**) malloc(sizeof(char *) * numInactiveDomains);
        numInactiveDomains = virConnectListDefinedDomains(conn, inactiveDomains, numInactiveDomains);

        for (int i = 0; i < numInactiveDomains; i++)
        {
            virDomainPtr domain = virDomainLookupByName(conn, inactiveDomains[i]);
        
            if (domain != NULL)
            {
                try
                {
                    _return.push_back(getDomainInfo(conn, domain));
                    virDomainFree(domain);
                }
                catch (...)
                {
                    virDomainFree(domain);
                }
            }

            free(inactiveDomains[i]);
        }

        free(inactiveDomains);
    }
    // TODO else => throwError?
}

void LibvirtService::defineDomain(virConnectPtr conn, const std::string& xmlDesc) throw (LibvirtException)
{
    virDomainPtr domain = virDomainDefineXML(conn, xmlDesc.c_str());

    if (domain == NULL)
    {
        throwError(""); // TODO
    }

    virDomainFree(domain);
}

void LibvirtService::undefineDomain(virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainUndefine(domain);
    virDomainFree(domain);

    if (ret < 0)
    {
        throwError(""); // TODO
    }
}

bool LibvirtService::existDomain(virConnectPtr conn, const std::string& domainName)
{
    try
    {
        virDomainPtr domain = getDomainByName(conn, domainName);
        virDomainFree(domain);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void LibvirtService::getDomainState(std::string& _return, virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    virDomainPtr domain = getDomainByName(conn, domainName);
    virDomainInfo info;

    if (virDomainGetInfo(domain, &info) < 0)
    {
        virDomainFree(domain);
        throwError(""); // TODO
    }

    _return = info.state; // the running state, one of virDomainState

    virDomainFree(domain);
}

void LibvirtService::getDomainInfo(DomainInfo& _return, virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    virDomainPtr domain = getDomainByName(conn, domainName);
    
    try
    {
        _return = getDomainInfo(conn, domain);
        virDomainFree(domain);
    }
    catch (LibvirtException exception)
    {
        virDomainFree(domain);
        throw exception;
    }
}

void LibvirtService::powerOn(virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainCreate(domain);
    virDomainFree(domain);
    
    if (ret < 0)
    {
        throwError(""); // TODO
    }   
}

void LibvirtService::powerOff(virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainDestroy(domain);
    virDomainFree(domain);
    
    if (ret < 0)
    {
        throwError(""); // TODO
    }
}

void LibvirtService::reset(virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainReboot(domain, 0);
    virDomainFree(domain);
    
    if (ret < 0)
    {
        throwError(""); // TODO
    }
}

void LibvirtService::pause(virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainSuspend(domain);
    virDomainFree(domain);
    
    if (ret < 0)
    {
        throwError(""); // TODO
    }
}

void LibvirtService::resume(virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainResume(domain);
    virDomainFree(domain);
    
    if (ret < 0)
    {
        throwError(""); // TODO
    }
}

void LibvirtService::createStoragePool(virConnectPtr conn, const std::string& xmlDesc) throw (LibvirtException)
{
    virStoragePoolPtr storagePool = virStoragePoolCreateXML(conn, xmlDesc.c_str(), 0);
    if (storagePool == NULL)
    {
        throwError(""); // TODO
    }

    virStoragePoolFree(storagePool);
}

void LibvirtService::resizeDisk(virConnectPtr conn, const string& domainName, const string& diskPath, const double diskSizeInKb) throw (LibvirtException)
{
    virDomainPtr dom = getDomainByName(conn, domainName);

    int result = virDomainBlockResize(dom, diskPath.c_str(), diskSizeInKb, 0);
    if (result < 0)
    {
        stringstream msg;
        virErrorPtr error = virConnGetLastError(conn);
        msg << "Resize operation failed. Error code [" << error->code << "] Message [" << error->message << "]";
        throwError(msg.str());
    }

    virDomainFree(dom);
}

