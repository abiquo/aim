#include <LibvirtService.h>
#include <ConfigConstants.h>
#include <Debug.h>
#include <Macros.h>

#include <iniparser.h>
#include <dictionary.h>

#include <libvirt/virterror.h>
#include <string>
#include <sstream>

#include <ExecUtils.h>

#include <boost/filesystem.hpp>
#include <sys/types.h>
#include <sys/stat.h>

#include <pugixml.hpp>

#define CONNECTION_ERROR_CODE       -1
#define XML_PARSE_ERROR_CODE        -2
#define RESCAN_DEVICE_ERROR_CODE    -3
#define NONE_ERROR_OCCURRED_CODE    -4
#define NFS_MOUNT_POINT_CREATION    -5

using namespace boost::filesystem;

string LibvirtService::connectionUrl = "";

LibvirtService::LibvirtService() : Service("Libvirt")
{
}

LibvirtService::~LibvirtService()
{
}

// Private methods

virDomainPtr LibvirtService::getDomainByName(const virConnectPtr conn, const std::string& name) throw (LibvirtException)
{
    virDomainPtr domain = virDomainLookupByName(conn, name.c_str());
    if (domain == NULL)
    {
        throwLastKnownError(conn);
    }
    return domain;
}

DomainInfo LibvirtService::getDomainInfo(const virConnectPtr conn, const virDomainPtr domain) throw (LibvirtException)
{
    if (domain == NULL)
    {
        throwLastKnownError(conn);
    }

    virDomainInfo info;
    if (virDomainGetInfo(domain, &info) < 0)
    {
        virDomainFree(domain);
        throwLastKnownError(conn);
    }

    const char *name = virDomainGetName(domain);
    if (name == NULL)
    {
        virDomainFree(domain);
        throwLastKnownError(conn);
    }

    const char *xml = virDomainGetXMLDesc(domain, 0);
    if (xml == NULL)
    {
        virDomainFree(domain);
        throwLastKnownError(conn);
    }
    
    char uuid[VIR_UUID_STRING_BUFLEN];
    if (virDomainGetUUIDString(domain, uuid) < 0)
    {
        virDomainFree(domain);
        throwLastKnownError(conn);
    }

    DomainInfo domainInfo;
    domainInfo.name            = string(name);              // the domain name
    domainInfo.uuid            = string(uuid);              // the domain UUID
    domainInfo.state           = toDomainState(info.state); // the running state, one of virDomainState
    domainInfo.numberVirtCpu   = info.nrVirtCpu;            // the number of virtual CPUs for the domain 
    domainInfo.memory          = info.memory;               // the memory in KBytes used by the domain
    domainInfo.xmlDesc         = string(xml);

    return domainInfo;
}

DomainState::type LibvirtService::toDomainState(unsigned char state)
{
    switch (state)
    {
        case VIR_DOMAIN_NOSTATE:
            return DomainState::UNKNOWN;
            break; 
        case VIR_DOMAIN_RUNNING:
            return DomainState::ON;
            break;
        case VIR_DOMAIN_BLOCKED:
            return DomainState::ON;
            break;
        case VIR_DOMAIN_PAUSED:
            return DomainState::PAUSED;
            break;
        case VIR_DOMAIN_SHUTDOWN:
            return DomainState::OFF;
            break;
        case VIR_DOMAIN_SHUTOFF:
            return DomainState::OFF;
            break;
        case VIR_DOMAIN_CRASHED:
            return DomainState::UNKNOWN;
            break;
        case VIR_DOMAIN_PMSUSPENDED:
            return DomainState::PAUSED;
            break;
        default:
            return DomainState::UNKNOWN;
    }
}

string LibvirtService::stringBetween(const std::string& input, const std::string& startPattern, const std::string& endPattern)
{
    size_t startPatternLength = startPattern.length();
    
    size_t start = input.find(startPattern);
    if (start != string::npos)
    {
        start += startPatternLength;
        size_t end = input.find(endPattern, start);
        if (end != string::npos)
        {
            return input.substr(start, end - start);
        }
    }

    return "";
}

string LibvirtService::parseDevicePath(const std::string& xmlDesc)
{
    string path = stringBetween(xmlDesc, "<device path=\"", "\"/>");
    if (path.empty())
    {
        path = stringBetween(xmlDesc, "<device path='", "'/>");
    }

    return path;
}

string LibvirtService::parseSourceHostAndDir(const std::string& xmlDesc)
{
    string name = stringBetween(xmlDesc, "<host name=\"", "\"/>");
    if (name.empty())
    {
        name = stringBetween(xmlDesc, "<host name='", "'/>");
    }

    string path = stringBetween(xmlDesc, "<dir path=\"", "\"/>");
    if (path.empty())
    {
        path = stringBetween(xmlDesc, "dir path='", "'/>");
    }

    return name + path;
}

string LibvirtService::parseMountPoint(const std::string& xmlDesc)
{
    return stringBetween(xmlDesc, "<path>", "</path>");
}

void LibvirtService::defineStoragePool(const virConnectPtr conn, const std::string& xmlDesc) throw (LibvirtException)
{
    LOG("Define storage pool XML: %s", xmlDesc.c_str());
    virStoragePoolPtr storagePool = virStoragePoolDefineXML(conn, xmlDesc.c_str(), 0);
    if (storagePool == NULL)
    {
        throwLastKnownError(conn);
    }

    LOG("Set storage pool autostart");
    if (virStoragePoolSetAutostart(storagePool, 1) < 0)
    {
        virStoragePoolUndefine(storagePool);
        virStoragePoolFree(storagePool);
        throwLastKnownError(conn);
    }

    LOG("Activate storage pool");
    if (virStoragePoolCreate(storagePool, 0) < 0)
    {
        virStoragePoolUndefine(storagePool);
        virStoragePoolFree(storagePool);
        throwLastKnownError(conn);
    }

    LOG("Storage pool defined and created");
    virStoragePoolFree(storagePool);
}

bool LibvirtService::existPrimaryDisk(const DomainInfo& domainInfo)
{
    // Build writable char buffer
    char * writable = new char[domainInfo.xmlDesc.size() + 1];
    std::copy(domainInfo.xmlDesc.begin(), domainInfo.xmlDesc.end(), writable);
    writable[domainInfo.xmlDesc.size()] = '\0';

    // Load XML description
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_buffer_inplace(writable, domainInfo.xmlDesc.size());

    if (!result)
    {
        LOG("Error loading XML '%s'. Cause: '%s'", domainInfo.xmlDesc.c_str(), result.description());
        return false;
    }

    bool exist = false;

    try
    {
        // Parse the primary disk file or dev
        pugi::xpath_node primary = doc.select_single_node("//disk[target[@dev='hda']]/source");
        const char * file = primary.node().attribute("file").value();
        const char * dev = primary.node().attribute("dev").value();
        
        if (strlen(file) > 0)
        {
            LOG("Checking if file '%s' exists", file);
            exist = boost::filesystem::exists(file);
        }
        else if (strlen(dev) > 0)
        {
            LOG("Checking if dev '%s' exists", dev);
            exist = boost::filesystem::exists(dev);
        }
    }    
    catch (const pugi::xpath_exception& e)
    {
        LOG("Error parsing primary disk info from domain '%s'. Cause: '%s'", domainInfo.name.c_str(), e.what());
    }

    LOG("Primary disk of domain '%s' %s", domainInfo.name.c_str(), exist ? "exist" : "does not exist");
    delete[] writable;
    return exist;
}

// Public methods

bool LibvirtService::initialize(dictionary* configuration)
{
    if (LibvirtService::connectionUrl.empty())
    {
        LibvirtService::connectionUrl = getStringProperty(configuration, monitorUri);
    }

    LOG("Libvirt connection url (empty is bad): '%s'", LibvirtService::connectionUrl.c_str());
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

void LibvirtService::throwLastKnownError(const virConnectPtr conn)
{
    virErrorPtr error = virConnGetLastError(conn);

    if (error == NULL)
    {
        LibvirtException exception;
        exception.code = NONE_ERROR_OCCURRED_CODE;
        exception.msg = "None error occurred";
        LOG(exception.msg.c_str());
        throw exception;
    }

    LibvirtException exception;
    exception.code   = error->code;
    exception.domain = error->domain;
    exception.msg    = error->message == NULL ? "" : string(error->message);
    exception.level  = error->level;
    exception.str1   = error->str1 == NULL ? "" : string(error->str1);
    exception.str2   = error->str2 == NULL ? "" : string(error->str2);
    exception.str3   = error->str3 == NULL ? "" : string(error->str3);
    exception.int1   = error->int1;
    exception.int2   = error->int2;
    LOG("LibvirtError '%d' level: '%d' message: '%s'", exception.code, exception.level, exception.msg.c_str());

    throw exception;
}

virConnectPtr LibvirtService::connect() throw (LibvirtException)
{
    virConnectPtr conn = virConnectOpen(LibvirtService::connectionUrl.c_str());
    if (conn == NULL)
    {
        LibvirtException exception;
        exception.code = CONNECTION_ERROR_CODE;
        exception.msg = "Could not connect to " + LibvirtService::connectionUrl;
        LOG(exception.msg.c_str());
        throw exception;
    }
    return conn;
}

void LibvirtService::disconnect(const virConnectPtr conn)
{
    if (virConnectClose(conn) < 0)
    {
        LOG(("Error closing connection: " + LibvirtService::connectionUrl).c_str());
    }
}

void LibvirtService::getNodeInfo(NodeInfo& _return, const virConnectPtr conn) throw (LibvirtException)
{
    LOG("Get node info");
    virNodeInfo info;
    if (virNodeGetInfo(conn, &info) < 0)
    {
        throwLastKnownError(conn);
    }
    
    char *name = virConnectGetHostname(conn);
    if (name == NULL)
    {
        throwLastKnownError(conn);
    }

    unsigned long version;
    if (virConnectGetLibVersion(conn, &version) < 0)
    {
        throwLastKnownError(conn);
    }

    _return.name    = string(name); // system hostname
    _return.version = version;      // libvirt version, have the format major * 1,000,000 + minor * 1,000 + release
    _return.cores   = info.cores;   // number of cores per socket
    _return.sockets = info.sockets; // number of CPU sockets per node
    _return.memory  = info.memory;  // memory size in kilobytes

    free(name);
}

void LibvirtService::getDomains(std::vector<DomainInfo> & _return, const virConnectPtr conn) throw (LibvirtException)
{
    LOG("Get all domains");
    virDomainPtr *domains;

    int ret = virConnectListAllDomains(conn, &domains, 0);
    if (ret < 0)
    {
        throwLastKnownError(conn);
    }

    for (int i = 0; i < ret; i++)
    {
        try
        {
            DomainInfo domainInfo = getDomainInfo(conn, domains[i]);
            if (existPrimaryDisk(domainInfo))
            {
                _return.push_back(domainInfo);
            }
            virDomainFree(domains[i]);
        }
        catch(...)
        {
            // Nothing to do, pass error and continue
            // See getDomainInfo(conn, domain)
        }
    }

    LOG("%zu domains returned", _return.size());
    free(domains);
}

void LibvirtService::defineDomain(const virConnectPtr conn, const std::string& xmlDesc) throw (LibvirtException)
{
    LOG("Define domain");
    virDomainPtr domain = virDomainDefineXML(conn, xmlDesc.c_str());
    if (domain == NULL)
    {
        throwLastKnownError(conn);
    }

    virDomainFree(domain);
}

void LibvirtService::undefineDomain(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    LOG("Undefine domain '%s'", domainName.c_str());
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainUndefine(domain);
    virDomainFree(domain);

    if (ret < 0)
    {
        throwLastKnownError(conn);
    }
}

bool LibvirtService::existDomain(const virConnectPtr conn, const std::string& domainName)
{
    try
    {
        LOG("Check if domain '%s' exists", domainName.c_str());
        virDomainPtr domain = getDomainByName(conn, domainName);
        virDomainFree(domain);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

DomainState::type LibvirtService::getDomainState(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    LOG("Get domain '%s' state", domainName.c_str());
    virDomainPtr domain = getDomainByName(conn, domainName);
    virDomainInfo info;

    if (virDomainGetInfo(domain, &info) < 0)
    {
        virDomainFree(domain);
        throwLastKnownError(conn);
    }

    DomainState::type result = toDomainState(info.state);

    virDomainFree(domain);

    return result;
}

void LibvirtService::getDomainInfo(DomainInfo& _return, const virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    LOG("Get domain '%s' info", domainName.c_str());
    virDomainPtr domain = getDomainByName(conn, domainName);
    _return = getDomainInfo(conn, domain);
    virDomainFree(domain);
}

void LibvirtService::powerOn(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    LOG("Power on domain '%s'", domainName.c_str());
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainCreate(domain);
    virDomainFree(domain);
    
    if (ret < 0)
    {
        throwLastKnownError(conn);
    }   
}

void LibvirtService::powerOff(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    LOG("Power off domain '%s'", domainName.c_str());
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainDestroy(domain);
    virDomainFree(domain);
    
    if (ret < 0)
    {
        throwLastKnownError(conn);
    }
}

void LibvirtService::reset(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    LOG("Reset domain '%s'", domainName.c_str());
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainReboot(domain, 0);
    virDomainFree(domain);
    
    if (ret < 0)
    {
        throwLastKnownError(conn);
    }
}

void LibvirtService::pause(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    LOG("Pause domain '%s'", domainName.c_str());
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainSuspend(domain);
    virDomainFree(domain);
    
    if (ret < 0)
    {
        throwLastKnownError(conn);
    }
}

void LibvirtService::resume(const virConnectPtr conn, const std::string& domainName) throw (LibvirtException)
{
    LOG("Resume domain '%s'", domainName.c_str());
    virDomainPtr domain = getDomainByName(conn, domainName);

    int ret = virDomainResume(domain);
    virDomainFree(domain);
    
    if (ret < 0)
    {
        throwLastKnownError(conn);
    }
}

void LibvirtService::createISCSIStoragePool(const virConnectPtr conn, const std::string& xmlDesc) throw (LibvirtException)
{
    LOG("Creating ISCSI storage pool...");
    string path = parseDevicePath(xmlDesc);
    if (path.empty())
    {
        LibvirtException exception;
        exception.code = XML_PARSE_ERROR_CODE;
        exception.msg = "Unable to parse device path from the xml description: " + xmlDesc;
        LOG(exception.msg.c_str());
        throw exception;
    }

    virStoragePoolPtr *pools;
    int ret = virConnectListAllStoragePools(conn, &pools, VIR_CONNECT_LIST_STORAGE_POOLS_ISCSI);
    if (ret < 0)
    {
        throwLastKnownError(conn);
    }

    LOG("Creck if the ISCSI storage pool device path '%s' is already defined", path.c_str());
    bool defined = false;
    for (int i = 0; i < ret && !defined; i++)
    {
        LOG("Checking ISCSI storage pool %d of %d...", i, ret);
        char *xml = virStoragePoolGetXMLDesc(pools[i], 0);
        if (xml != NULL)
        {
            string definedPath = parseDevicePath(string(xml));
            LOG("Found device path: '%s'", definedPath.c_str());
            defined = (path.compare(definedPath) == 0);
        }
        virStoragePoolFree(pools[i]);
    }

    free(pools);

    if (!defined)
    {
        virStoragePoolPtr storagePool = virStoragePoolCreateXML(conn, xmlDesc.c_str(), 0);
        if (storagePool == NULL)
        {
            throwLastKnownError(conn);
        }

        LOG("Storage pool defined and created");
        virStoragePoolFree(storagePool);
    }
    else
    {
        LOG("Storage pool already defined, rescanning '%s'...", path.c_str());
        ostringstream command;
        command << "/sbin/iscsiadm -m node -T " << path << " -R";
        command.flush();

        if (executeCommand(command.str()) != 0)
        {
            LibvirtException exception;
            exception.code = RESCAN_DEVICE_ERROR_CODE;
            exception.msg = "Unable to rescan device path " + path;
            LOG(exception.msg.c_str());
            throw exception;
        }
    }
}

void LibvirtService::createNFSStoragePool(const virConnectPtr conn, const std::string& xmlDesc) throw (LibvirtException)
{
    LOG("Creating NFS storage pool...");

    string device = parseSourceHostAndDir(xmlDesc);
    if (device.empty())
    {
        LibvirtException exception;
        exception.code = XML_PARSE_ERROR_CODE;
        exception.msg = "Unable to parse source host and dir from the xml description: " + xmlDesc;
        LOG(exception.msg.c_str());
        throw exception;
    }
    
    string mountPoint = parseMountPoint(xmlDesc);
    if (mountPoint.empty())
    {
        LibvirtException exception;
        exception.code = XML_PARSE_ERROR_CODE;
        exception.msg = "Unable to parse mount point from the xml description: " + xmlDesc;
        LOG(exception.msg.c_str());
        throw exception;
    }

    virStoragePoolPtr *pools;
    int ret = virConnectListAllStoragePools(conn, &pools, VIR_CONNECT_LIST_STORAGE_POOLS_NETFS);
    if (ret < 0)
    {
        throwLastKnownError(conn);
    }

    LOG("Creck if the NFS device ('%s') is already defined", device.c_str());
    bool defined = false;
    for (int i = 0; i < ret && !defined; i++)
    {
        LOG("Checking NFS storage pool %d of %d...", i, ret);
        char *xml = virStoragePoolGetXMLDesc(pools[i], 0);
        if (xml != NULL)
        {
            string definedDevice = parseSourceHostAndDir(string(xml));
            LOG("Found device: '%s'", definedDevice.c_str());
            defined = (device.compare(definedDevice) == 0);
        }
        virStoragePoolFree(pools[i]);
    }

    free(pools);

    LOG("Check if mount point '%s' exists", mountPoint.c_str());
    if (!exists(mountPoint))
    {
        LOG("Creating mount point directory '%s'", mountPoint.c_str());
        if (!create_directories(mountPoint))
        {
            LibvirtException exception;
            exception.code = NFS_MOUNT_POINT_CREATION;
            exception.msg = "Unable to create mount point at " + mountPoint;
            LOG(exception.msg.c_str());
            throw exception; 
        }
    }

    if (!defined)
    {
        defineStoragePool(conn, xmlDesc);
    }
    else
    {
        LOG("Storage pool already defined");
    }
}

void LibvirtService::resizeDisk(const virConnectPtr conn, const std::string& domainName, const std::string& diskPath, const double diskSizeInKb) throw (LibvirtException)
{
    LOG("Resize disk '%s' of domain '%s' to %f Kb", diskPath.c_str(), domainName.c_str(), diskSizeInKb);

    virDomainInfo info;
    virDomainPtr domain = getDomainByName(conn, domainName);

    // [ABICLOUDPREMIUM-5486] In the CentOS 6 libvirt version (0.10.2-18)
    // it seems that disks can not be resized if the domain is not running.
    // Just make sure the domain is running before resizing and restore it afterwards.
    if (virDomainGetInfo(domain, &info) < 0)
    {
        virDomainFree(domain);
        throwLastKnownError(conn);
    }

    DomainState::type state = toDomainState(info.state);
    bool running = (state == DomainState::ON);

    if (!running)
    {
        LOG("Domain '%s' is not running. Powering on to resize the disk...", domainName.c_str());
        if (virDomainCreate(domain) < 0)
        {
            virDomainFree(domain);
            throwLastKnownError(conn);
        }
    }

    int result = virDomainBlockResize(domain, diskPath.c_str(), diskSizeInKb, 0);

    // Even if the resize fails, we need to restore the domain to its original state
    if (!running)
    {
        LOG("Restoring domain '%s' to its original state...", domainName.c_str());
        if (virDomainDestroy(domain) < 0)
        {
            virDomainFree(domain);
            throwLastKnownError(conn);
        }
    }

    virDomainFree(domain);

    if (result < 0)
    {
        throwLastKnownError(conn);
    }
}

