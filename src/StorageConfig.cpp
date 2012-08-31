#include <StorageConfig.h>
#include <Debug.h>
#include <Macros.h>
#include <ExecUtils.h>
#include <aim_types.h>

#include <iniparser.h>
#include <dictionary.h>

#include <sstream>

StorageConfig::StorageConfig() : Service("Storage configuration")
{
    iscsiInitiatorNameFile = ISCSI_DEFAULT_INITIATOR_NAME_FILE;
}

StorageConfig::~StorageConfig()
{
}

bool StorageConfig::initialize(dictionary * configuration)
{
    dictionary* d = iniparser_load(iscsiInitiatorNameFile.c_str());

    bool initialized = (d != NULL);

    iniparser_freedict(d);
    
    return initialized;
}

bool StorageConfig::start()
{
    return true;
}

bool StorageConfig::stop()
{
    return true;
}

bool StorageConfig::cleanup()
{
    return true;
}

void StorageConfig::throwError(const string& message)
{
    StorageException exception;
    exception.description = message;

    throw exception;
}

void StorageConfig::getInitiatorIQN(string& iqn)
{
    dictionary* d = iniparser_load(iscsiInitiatorNameFile.c_str());
    iqn = "";

    if (d == NULL)
    {
        LOG("Unable to load %s. The IQN returned will be empty.", iscsiInitiatorNameFile.c_str());
    }

    if (iniparser_find_entry(d, ":InitiatorName") != 0)
    {
        iqn = getStringProperty(d, ":InitiatorName");
    }

    LOG("Request for node ISCSI initiator iqn = '%s'", iqn.c_str());
}

void StorageConfig::rescanISCSI(const vector<string>& targets)
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
