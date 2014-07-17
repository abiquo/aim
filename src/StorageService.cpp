#include <StorageService.h>
#include <Debug.h>
#include <Macros.h>
#include <ExecUtils.h>
#include <aim_types.h>

#include <sstream>

StorageService::StorageService() : Service("Storage")
{
}

StorageService::~StorageService()
{
}

bool StorageService::initialize(INIReader configuration)
{
    INIReader reader(ISCSI_DEFAULT_INITIATOR_NAME_FILE);    
    bool initialized = (reader.ParseError() >= 0);

    LOG("Reading ISCSI initiator IQN from '%s' file", ISCSI_DEFAULT_INITIATOR_NAME_FILE);

    if (!initialized)
    {
        LOG("Unable to load ISCSI initiator IQN");
        return false;
    }

    iqnValue = reader.Get("", "InitiatorName", "");
    LOG("ISCSI initiator IQN: '%s'", iqnValue.c_str());
    return initialized;
}

bool StorageService::start()
{
    return true;
}

bool StorageService::stop()
{
    return true;
}

bool StorageService::cleanup()
{
    return true;
}

void StorageService::throwError(const string& message)
{
    StorageException exception;
    exception.description = message;

    throw exception;
}

void StorageService::getInitiatorIQN(string& iqn)
{
    iqn = iqnValue;
    LOG("Request for node ISCSI initiator IQN ('%s')", iqn.c_str());
}

void StorageService::rescanISCSI(const vector<string>& targets)
{
    // Each given target is a volume attached to the virtual machine
    for(vector<string>::const_iterator it = targets.begin(); it < targets.end(); it++)
    {
        ostringstream command;
        command << "/sbin/iscsiadm -m node -T " << *it << " -R";
        command.flush();
        executeCommand(command.str());
    }
}

