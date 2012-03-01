#include <VLan.h>
#include <ConfigConstants.h>
#include <Debug.h>
#include <Macros.h>
#include <sstream>
#include <sys/wait.h>
#include <aim_types.h>

VLan::VLan() : Service("VLAN")
{
}

VLan::~VLan()
{
}

bool VLan::initialize(dictionary * configuration)
{
    ifconfig = getStringProperty(configuration, vlanIfConfigCmd);
    vconfig = getStringProperty(configuration, vlanVconfigCmd);
    brctl = getStringProperty(configuration, vlanBrctlCmd);

    bool ok = true;

    ok &= commandExist(ifconfig);
    ok &= commandExist(vconfig);
    ok &= commandExist(brctl);

    if (!ok)
    {
        LOG("Some required command is missing, check:\n\t%s\n\t%s\n\t%s", 
                ifconfig.c_str(), vconfig.c_str(), brctl.c_str());
    }

    return ok;
}

bool VLan::start()
{
    return true;
}

bool VLan::stop()
{
    return true;
}

bool VLan::cleanup()
{
    return true;
}

void VLan::throwError(const string& message)
{
    VLanException exception;
    exception.description = message;

    throw exception;
}

void VLan::createVLAN(int vlan, const string& vlanInterface, const string& bridgeInterface)
{
    if (!createBridgeInterface(bridgeInterface))
    {
        ostringstream error;
        error << "Error creating bridge interface " << bridgeInterface;
        error.flush();

        LOG("%s", error.str().c_str());
        throwError(error.str());
    }

    if (!createVLANInterface(vlan, vlanInterface, bridgeInterface))
    {
        ostringstream error;
        error << "Error creating VLAN with tag " << vlan << " and interface " << vlanInterface;
        error.flush();

        LOG("%s", error.str().c_str());
        throwError(error.str());
    }

    LOG("VLan created, tag=%d, interface=%s, bridge=%s", vlan, vlanInterface.c_str(), bridgeInterface.c_str());
}

bool VLan::createVLANInterface(int vlan, const string& vlanIf, const string& bridgeIf)
{
    if (!existsVlan(vlan, vlanIf))
    {
        string filename = buildVLANFilename(vlan, vlanIf);

        if (!writeVLANConfiguration(vlanIf, vlan, bridgeIf, NETWORK_SCRIPTS_FOLDER, filename))
        {
            return false;
        }

        if (!ifUp(filename))
        {
            LOG("Unable to tear up the VLAN interface %s", vlanIf.c_str());
            return false;
        }
    }
    else
    {
        LOG("VLAN with tag %d and interface %s already exists.", vlan, vlanIf.c_str());
    }

    return true;
}

bool VLan::createBridgeInterface(const string& bridgeIf)
{
    if (!existsBridge(bridgeIf))
    {
        string filename = buildBridgeFilename(bridgeIf);

        if (!writeBridgeConfiguration(bridgeIf, NETWORK_SCRIPTS_FOLDER, filename))
        {
            return false;
        }

        if (!ifUp(filename))
        {
            LOG("Unable to tear up the bridge %s", bridgeIf.c_str());
            return false;
        }
    }
    else
    {
        LOG("Bridge interface %s already exists.", bridgeIf.c_str());
    }

    return true;
}

bool VLan::ifUp(string& filename)
{
    ostringstream command;
    command << "ifup " << filename;
    command.flush();

    return (executeCommand(command.str()) == 0);
}

bool VLan::ifDown(string& filename)
{
    ostringstream command;
    command << "ifdown " << filename;
    command.flush();

    return (executeCommand(command.str()) == 0);
}

string VLan::buildVLANFilename(int vlan, const string& vlanIf)
{
    ostringstream oss;
    oss << "ifcfg-abiquo_" << vlanIf << "." << vlan;
    oss.flush();

    return oss.str();
}

string VLan::buildBridgeFilename(const string& bridgeIf)
{
    ostringstream oss;
    oss << "ifcfg-" << bridgeIf;
    oss.flush();

    return oss.str();
}

void VLan::deleteVLAN(int vlan, const string& vlanInterface, const string& bridgeInterface)
{
    boost::mutex::scoped_lock lock(delete_vlan_mutex);

    if(!deleteBridgeInterface(bridgeInterface))
    {
        ostringstream error;
        error << "Error deleting bridge interface " << bridgeInterface;
        error.flush();

        LOG("%s", error.str().c_str());
        throwError(error.str());
    }

    if (!deleteVLANInterface(vlan, vlanInterface))
    {   
        ostringstream error;
        error << "Error deleting VLAN interface " << vlanInterface;
        error.flush();

        LOG("%s", error.str().c_str());
        throwError(error.str());
    }

    LOG("VLan deleted, tag=%d, interface=%s, bridge=%s", vlan, vlanInterface.c_str(), bridgeInterface.c_str());
}

bool VLan::deleteBridgeInterface(const string& bridgeIf)
{
    if (existsBridge(bridgeIf))
    {
        string filename = buildBridgeFilename(bridgeIf);

        if (!ifDown(filename))
        {
            LOG("Unable to tear down the bridge %s", bridgeIf.c_str());
            return false;
        }

        if (!removeFile(NETWORK_SCRIPTS_FOLDER, filename))
        {
            // TODO try to do an up?
            ifUp(filename);
            return false;
        }
    }
    else
    {
        LOG("Bridge interface %s does not exist.", bridgeIf.c_str());
    }

    return true;
}

bool VLan::deleteVLANInterface(int vlan, const string& vlanIf)
{
    if (existsVlan(vlan, vlanIf))
    {
        string filename = buildVLANFilename(vlan, vlanIf);

        if (!ifDown(filename))
        {
            LOG("Unable to tear down the VLAN interface %s", vlanIf.c_str());
            return false;
        }

        if (!removeFile(NETWORK_SCRIPTS_FOLDER, filename))
        {
            // TODO try to do an up?
            ifUp(filename);

            return false;
        }
    }
    else
    {
        LOG("VLAN with tag %d and interface %s does not exist.", vlan, vlanIf.c_str());
    }

    return true;
}

void VLan::checkVlanRange(const int vlan)
{
    if (vlan < 1 || vlan > 4094)
    {
        ostringstream oss;

        oss << "VLAN tag out of range (" << vlan << ").";
        oss.flush();

        LOG("%s", oss.str().c_str());
        throwError(oss.str());
    }
}

bool VLan::existsVlan(const int vlan, const string& vlanInterface)
{
    checkVlanRange(vlan);

    ostringstream oss;

    oss << vlanInterface << "." << vlan;
    oss.flush();

    return existsInterface(oss.str());
}

bool VLan::existsInterface(const string& interface)
{
    ostringstream oss;

    oss << ifconfig << " " << interface << " > /dev/null 2>/dev/null";
    oss.flush();

    return (executeCommand(oss.str()) == 0);
}

bool VLan::existsBridge(const string& interface)
{
    return existsInterface(interface);
}

int VLan::executeCommand(string command, bool redirect)
{
    if (redirect)
    {
        command.append(" > /dev/null");
    }

    LOG("Executing '%s'", command.c_str());

    int status = system(command.c_str());

    return WEXITSTATUS(status);
}

bool VLan::commandExist(string& command)
{
    return (executeCommand(command, true) != 127);
}

void VLan::checkVLANConfiguration()
{
    string error = "Failed to check the command/s: ";
    bool ok = true;

    if (executeCommand(ifconfig) == 127)
    {
        ok = false;
        error.append(ifconfig).append(" ");
    }

    if (executeCommand(vconfig) == 127)
    {
        ok = false;
        error.append(vconfig).append(" ");
    }   

    if (executeCommand(brctl) == 127)
    {
        ok = false;
        error.append(brctl).append(" ");
    }   

    if (!ok)
    {
        LOG("%s", error.c_str());
        throwError(error);
    }
}

bool VLan::writeVLANConfiguration(const string& device, int vlan, const string& bridgeName, const string& folder, const string& filename)
{
    if (isAccessible(folder))
    {
        // Compose filename
        ostringstream filepath; 
        filepath << folder << "/" << filename;

        // Write config file
        ofstream config;

        config.open(filepath.str().c_str(), ios_base::trunc);
        config << "# Autogenerated by Abiquo AIM" << endl << endl;
        config << "VLAN=yes" << endl;
        config << "DEVICE=" << device << "." << vlan << endl;
        config << "BOOTPROTO=none" << endl;
        config << "ONBOOT=yes" << endl;
        config << "BRIDGE=" << bridgeName << endl;
        config.close();

        LOG("The VLAN configuration has been written in '%s'", filepath.str().c_str());
        return true;
    }

    LOG("Unable to write the VLAN configuration '%s' is not accessible", folder.c_str());
    return false;
}

bool VLan::writeBridgeConfiguration(const string& device, const string& folder, const string& filename)
{
    if (isAccessible(folder))
    {
        // Compose filename
        ostringstream filepath;
        filepath << folder << "/" << filename;

        // Write config file
        ofstream config;

        config.open(filepath.str().c_str(), ios_base::trunc);
        config << "# Autogenerated by Abiquo AIM" << endl << endl;
        config << "DEVICE=" << device << endl;
        config << "TYPE=Bridge" << endl;
        config << "BOOTPROTO=none" << endl;
        config << "ONBOOT=yes" << endl;
        config.close();

        LOG("The bridge configuration configuration has been written in '%s'", filepath.str().c_str());
        return true;
    }

    LOG("Unable to write the bridge configuration '%s' is not accessible", folder.c_str());
    return false;
}

bool VLan::isAccessible(const string& path)
{
    if (access(path.c_str(), F_OK | R_OK | W_OK) == -1)
    {
        return false;
    }

    return true;
}

bool VLan::removeFile(const string& folder, const string& filename)
{
    ostringstream oss;
    oss << folder << "/" << filename;

    string filepath = oss.str();

    if (!isAccessible(folder))
    {
        LOG("Unable to remove %s, folder %s is no accessible", filepath.c_str(), folder.c_str());
        return false;
    }

    if (unlink(filepath.c_str()) != 0)
    {
        LOG("Unable to remove %s", filepath.c_str());
        return false;
    }

    return true;
}

