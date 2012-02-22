#include <Rimp.h>
#include <RimpUtils.h>
#include <ConfigConstants.h>
#include <Debug.h>
#include <Macros.h>

#include <aim_types.h>

#define TRUE_CHAR "TRUE\0"
#define holdsTrueValue(propChar) (propChar != NULL && strcasecmp(propChar, TRUE_CHAR) == 0)

Rimp::Rimp() : Service("Rimp")
{
}

Rimp::~Rimp()
{
}

bool Rimp::start()
{
    LOG("[DEBUG] [RIMP] start");
    return true;
}

bool Rimp::stop()
{
    LOG("[DEBUG] [RIMP] stop");
    return true;
}

bool Rimp::cleanup()
{
    LOG("[DEBUG] [RIMP] cleanup");
    return true;
}

bool Rimp::initialize(dictionary * configuration)
{
    const char* repository_c = getStringProperty(configuration, rimpRepository);
    const char* datastore_c = getStringProperty(configuration, rimpDatastore);
    const char* autobackup_c = getStringProperty(configuration, rimpAutoBackup);
    const char* autorestore_c = getStringProperty(configuration, rimpAutoRestore);

    if (repository_c == NULL || strlen(repository_c) < 2)
    {
        LOG("[ERROR] [RIMP] Initialization fails :\n"
                "\tcan not read the ''repository'' configuration element "
                "\tset [rimp]\nrepository = XXXX ");

        return false;
    }

    if (datastore_c == NULL || strlen(datastore_c) < 2)
    {
        LOG("[ERROR] [RIMP] Initialization fails :\n"
                "\tcan not read the ''datastore'' configuration element "
                "\tset [rimp]\ndatastore = XXXX ");

        return false;
    }

    autobackup = holdsTrueValue(autobackup_c);
    autorestore = holdsTrueValue(autorestore_c);
    repository = string(repository_c);
    datastore_default = string(datastore_c);

    // check ends with '/'
    if (repository.at(repository.size() - 1) != '/')
    {
        repository = repository.append("/");
    }
    if (datastore_default.at(datastore_default.size() - 1) != '/')
    {
        datastore_default = datastore_default.append("/");
    }

    bool initDatastore = checkDatastore(datastore_default);
    bool initRepository = checkRepository(repository);

    return initDatastore && initRepository;
}

void Rimp::checkRimpConfiguration()
{
    string error("Invalid RIMP configuration:");
    bool initDatastore = checkDatastore(datastore_default);
    bool initRepository = checkRepository(repository);

    if (!initDatastore)
    {
        error = error.append("\n''datastore'' :").append(datastore_default);
    }

    if (!initRepository)
    {
        error = error.append("\n''repository'' :").append(repository);
    }

    if (!initDatastore || !initRepository)
    {
        RimpException rexecption;
        rexecption.description = error;
        throw rexecption;
    }
}

int64_t Rimp::getDatastoreSize()
{
    unsigned long int space = getFreeSpaceOn(datastore_default);

    return space;
}

vector<Datastore> Rimp::getDatastores()
{
    boost::mutex::scoped_lock lock(get_datastores_mutex);

    LOG("[DEBUG] [RIMP] Get Datastores");
    return getDatastoresFromMtab();
}

vector<NetInterface> Rimp::getNetInterfaces()
{
    LOG("[DEBUG] [RIMP] Get Network Interfaces");

    return getNetInterfacesFromXXX();
}

int64_t Rimp::getDiskFileSize(const std::string& virtualImageDatastorePath)
{
    checkRimpConfiguration();

    LOG("[DEBUG] [RIMP] Get Disk File Size [%s]", virtualImageDatastorePath.c_str());

    // Check the file exist and can be read
    if (access(virtualImageDatastorePath.c_str(), F_OK | R_OK) == -1)
    {
        RimpException rexecption;
        string error ("File does not exist at [");
        error = error.append(virtualImageDatastorePath).append("]");

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexecption.description = error;
        throw rexecption;
    }

    return getFileSize(virtualImageDatastorePath);
}


void Rimp::copyFromRepositoryToDatastore(const std::string& virtualImageRepositoryPath, std::string& datastore,
        const std::string& virtualMachineUUID)
{
    string error("");
    RimpException rexecption;

    if (datastore.empty())
    {
        datastore = datastore_default;
    }
    else
    {
        // check datastore path end with '/'
        if (datastore.at(datastore.size() - 1) != '/')
        {
            datastore = datastore.append("/");
        }
    }

    checkRimpConfiguration();
    if (!checkDatastore(datastore))
    {
        error = error.append("Provided ''datastore'' :").append(datastore).append(" can not be used");
        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexecption.description = error;
        throw rexecption;
    }

    LOG("[DEBUG] [RIMP] Instantiating virtual image [%s] for virtual machine [%s]",
            virtualImageRepositoryPath.c_str(), virtualMachineUUID.c_str());

    string viRepositoryPath(repository);
    viRepositoryPath = viRepositoryPath.append(virtualImageRepositoryPath);

    // Check the source file (on the repository) exist and can be read
    if (access(viRepositoryPath.c_str(), F_OK | R_OK) == -1)
    {
        error = error.append("Source file does not exist at [").append(viRepositoryPath).append("]");

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexecption.description = error;
        throw rexecption;
    }

    unsigned long int viSize = getFileSize(viRepositoryPath);

    /** Copy from ''Local Repository'' to ''Datastore''. */
    string viDatastorePath(datastore);
    viDatastorePath = viDatastorePath.append(virtualMachineUUID);

    // if the file exist on the datastore delete it
    if (access(viDatastorePath.c_str(), F_OK) == 0)
    {
        if (autorestore && autobackup)
        {
            // Sometimes an undeploy fails and the origional image does not get re/moved so we don't want to delete it and we don't need to try to recover so there is nothing to do.
            // NOTE: You really do want this, don't over think it.

            LOG("[INFO] [RIMP] Not checking for backup; existing virtual image instance found for virtual machine [%s]", virtualMachineUUID.c_str());
            return;
        }

        LOG("[WARNING] [RIMP] File with the same UUID already present on the ''datastore'' [%s], removing it.",
                viDatastorePath.c_str());

        remove(viDatastorePath.c_str());
    }

    if (autorestore)
    {
        string viDatastorePathBackup(datastore);
        viDatastorePathBackup = viDatastorePathBackup.append("backup/");
        viDatastorePathBackup = viDatastorePathBackup.append(virtualMachineUUID);
        if (access(viDatastorePathBackup.c_str(), F_OK | R_OK) == 0)
        {
            // Then perform a recovery rather then a full deploy.
            string renameError2 = fileRename(viDatastorePathBackup, viDatastorePath);

            if (!renameError2.empty())
            {
                error = error.append("Can not move :").append(viDatastorePathBackup).append(" to :").append(viDatastorePath).append("\nCaused by :").append(renameError2);

                LOG("[ERROR] [RIMP] %s", error.c_str());
                rexecption.description = error;
                throw rexecption;
            }

            LOG("[INFO] [RIMP] Recovered virtual image instance for virtual machine [%s]", virtualMachineUUID.c_str());
            return;
        }
    }

    // XXX viSize is the same on the repository and on the local repository
    unsigned long int datastoreFreeSize = getFreeSpaceOn(datastore);

    if (datastoreFreeSize < viSize)
    {
        error = error.append("There is no enough space left to copy the file :");
        error = error.append(viRepositoryPath).append(" to :").append(datastore);

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexecption.description = error;
        throw rexecption;
    }

    string copyError2 = fileCopy(viRepositoryPath, viDatastorePath);

    if (!copyError2.empty())
    {
        error = error.append("Can not copy to :").append(viDatastorePath).append("\nCaused by :").append(copyError2);

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexecption.description = error;
        throw rexecption;
    }

    LOG("[INFO] [RIMP] Created virtual image instance for virtual machine [%s]", virtualMachineUUID.c_str());
}

void Rimp::deleteVirtualImageFromDatastore(std::string& datastore, const std::string& virtualMachineUUID)
{
    string error("");
    RimpException rexecption;

    if (datastore.empty())
    {
        datastore = datastore_default;
    }
    else
    {
        // check datastore path end with '/'
        if (datastore.at(datastore.size() - 1) != '/')
        {
            datastore = datastore.append("/");
        }
    }

    checkRimpConfiguration();
    if (!checkDatastore(datastore))
    {
        error = error.append("Provided ''datastore'' :").append(datastore).append(" can not be used");
        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexecption.description = error;
        throw rexecption;
    }

    LOG("[DEBUG] [RIMP] Deleting virtual machine [%s]", virtualMachineUUID.c_str());

    string viDatastorePath(datastore);
    viDatastorePath = viDatastorePath.append(virtualMachineUUID);

    // check the file exist on the datastore
    if (access(viDatastorePath.c_str(), F_OK) == -1)
    {
        error = error.append("Virtual image file does not exist on the ''datastore'' :");
        error = error.append(viDatastorePath);

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexecption.description = error;
        throw rexecption;
    }

    if (autobackup)
    {
        // Then backup the file rather than deleting it.

        string viDatastorePathBackup(datastore);
        viDatastorePathBackup = viDatastorePathBackup.append("backup/");
        viDatastorePathBackup = viDatastorePathBackup.append(virtualMachineUUID);

        if (access(viDatastorePathBackup.c_str(), F_OK) == 0)
        {
            // Then a backup already exists, delete it.
            remove(viDatastorePathBackup.c_str());
        }

        // Then perform a recovery rather then a full deploy.
        string renameError2 = fileRename( viDatastorePath, viDatastorePathBackup);

        if (!renameError2.empty())
        {
            error = error.append("Can not move :").append(viDatastorePath).append(" to :").append(viDatastorePathBackup).append("\nCaused by :").append(renameError2);

            LOG("[ERROR] [RIMP] %s", error.c_str());
            rexecption.description = error;
            throw rexecption;
        }

        LOG("[INFO] [RIMP] Backedup virtual machine [%s]", virtualMachineUUID.c_str());
        return;
    }

    remove(viDatastorePath.c_str());

    LOG("[INFO] [RIMP] Deleted virtual machine [%s]", virtualMachineUUID.c_str());
}

void Rimp::copyFromDatastoreToRepository(const std::string& virtualMachineUUID, const std::string& snapshot,
        const std::string& destinationRepositoryPathIn, const std::string& sourceDatastorePathIn)
{
    string error("");
    RimpException rexecption;

    checkRimpConfiguration();

    string destinationRepositoryPath(repository);
    destinationRepositoryPath = destinationRepositoryPath.append(destinationRepositoryPathIn);

    string sourceDatastorePath(sourceDatastorePathIn);

    if (destinationRepositoryPath.at(destinationRepositoryPath.size() - 1) != '/')
    {
        destinationRepositoryPath = destinationRepositoryPath.append("/");
    }
    if (sourceDatastorePath.at(sourceDatastorePath.size() - 1) != '/')
    {
        sourceDatastorePath = sourceDatastorePath.append("/");
    }

    /**
     * TODO destination repository concat configured ''repository''
     * */

    LOG("[DEBUG] [RIMP] Creating virtual machine [%s] instance [%s] from [%s] to [%s]",
            virtualMachineUUID.c_str(), snapshot.c_str(), sourceDatastorePath.c_str(),
            destinationRepositoryPath.c_str());

    // Check destination repository folder exist and can be written
    if (access(destinationRepositoryPath.c_str(), F_OK | W_OK) == -1)
    {
        int err = mkdir(destinationRepositoryPath.c_str(), S_IRWXU | S_IRWXG);

        if (err == -1)
        {
            error = error.append("Can not create destination ''repository'' folder at :");
            error = error.append(destinationRepositoryPath).append(" Caused by: ").append(strerror(errno));

            LOG("[ERROR] [RIMP] %s", error.c_str());
            rexecption.description = error;
            throw rexecption;
        }
    }

    string viDatastoreSource(sourceDatastorePath);
    viDatastoreSource = viDatastoreSource.append(virtualMachineUUID);

    // Check source file exist and can be read
    if (access(viDatastoreSource.c_str(), F_OK | R_OK) == -1)
    {
        error = error.append("Source file does not exist or can not be read: ");
        error = error.append(viDatastoreSource);

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexecption.description = error;
        throw rexecption;
    }

    string viRepositoryDestination(destinationRepositoryPath);
    viRepositoryDestination = viRepositoryDestination.append(snapshot);

    // Check target path do not exist
    if (access(viRepositoryDestination.c_str(), F_OK) == 0)
    {
        error = error.append("Snapshot already exists on the repository: ");
        error = error.append(viRepositoryDestination);

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexecption.description = error;
        throw rexecption;
    }

    // Checking there are enough free space to copy
    unsigned long int viSize = getFileSize(viDatastoreSource);
    unsigned long int repositoryFreeSize = getFreeSpaceOn(destinationRepositoryPath);

    if (repositoryFreeSize < viSize)
    {
        error = error.append("There is no enough space left to copy the file :");
        error = error.append(viDatastoreSource).append(" to :").append(viRepositoryDestination);

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexecption.description = error;
        throw rexecption;
    }

    string errorCopy = fileCopy(viDatastoreSource, viRepositoryDestination);
    if (!errorCopy.empty())
    {
        error = error.append("Can not copy to :");
        error = error.append(viRepositoryDestination).append("\nCaused by :").append(errorCopy);

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexecption.description = error;
        throw rexecption;
    }

    LOG("[INFO] [RIMP] Created snapshot [%s] from virtual machine [%s]", snapshot.c_str(), virtualMachineUUID.c_str());
}
