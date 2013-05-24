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

