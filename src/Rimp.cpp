#include <Rimp.h>
#include <RimpUtils.h>
#include <Debug.h>
#include <Macros.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <string>

#include <aim_types.h>


Rimp::Rimp() : Service("Rimp")
{
}

Rimp::~Rimp()
{
}

bool Rimp::start()
{
    return true;
}

bool Rimp::stop()
{
    return true;
}

bool Rimp::cleanup()
{
    return true;
}

vector<string> defaultValidTypeVector()
{
    vector<string> types;

    types.push_back(string("ext2"));
    types.push_back(string("ext3"));
    types.push_back(string("ext4"));
    types.push_back(string("nfs"));
    types.push_back(string("nfs4"));
    types.push_back(string("xfs"));
    types.push_back(string("smbfs"));

    return types;
}

bool Rimp::initialize(INIReader configuration)
{
    repository = configuration.Get("rimp", "repository", "");
    autobackup = configuration.GetBoolean("rimp", "autoBackup", false);
    autorestore = configuration.GetBoolean("rimp", "autoRestore", false);
    string dsTypes = configuration.Get("rimp", "datastoreValidTypes", "");

    if (repository.size() < 2)
    {
        LOG("[ERROR] [RIMP] Initialization fails :\n"
                "\tcan not read the ''repository'' configuration element "
                "\tset [rimp]\nrepository = XXXX ");
        return false;
    }

    // check ends with '/'
    if (repository.at(repository.size() - 1) != '/')
    {
        repository = repository.append("/");
    }
    
    if(dsTypes.size() == 0)
    {
        validTypes = defaultValidTypeVector();
    }
    else
    {
        boost::split(validTypes, dsTypes, boost::is_any_of(","));
    }

    // Print config values
    LOG("[DEBUG] Repository: '%s'", repository.c_str());
    LOG("[DEBUG] Auto-backup: %s", autobackup ? "on" : "off");
    LOG("[DEBUG] Auto-restore: %s", autorestore ? "on" : "off");

    // Print datastore valid devices
    ostringstream ss;
    
    for (int i = 0; (i < (int) validTypes.size()-1); i++)
    {
        ss << validTypes[i] << ", ";
    }
    
    ss << validTypes.back();

    LOG("[DEBUG] Valid device types to filter datastores: %s", ss.str().c_str());
    return true;
}

void Rimp::checkRimpConfiguration()
{
    if (!checkRepository(repository))
    {
        string error("Invalid ''repository'' property configuration : ");
        error = error.append(repository);

        RimpException rexception;
        rexception.description = error;
        throw rexception;
    }
}

vector<Datastore> Rimp::getDatastores()
{
    boost::mutex::scoped_lock lock(get_datastores_mutex);

    checkRimpConfiguration();
    
    LOG("[DEBUG] [RIMP] Get Datastores");

    return getDatastoresFromMtab(validTypes);
}

vector<NetInterface> Rimp::getNetInterfaces()
{
    LOG("[DEBUG] [RIMP] Get Network Interfaces");

    return getNetInterfacesFromProcNetDev();
}

int64_t Rimp::getDiskFileSize(const std::string& virtualImageDatastorePath)
{
    checkRimpConfiguration();

    LOG("[DEBUG] [RIMP] Get Disk File Size [%s]", virtualImageDatastorePath.c_str());

    // Check the file exist and can be read
    if (access(virtualImageDatastorePath.c_str(), F_OK | R_OK) == -1)
    {
        RimpException rexception;
        string error ("File does not exist at [");
        error = error.append(virtualImageDatastorePath).append("]");

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexception.description = error;
        throw rexception;
    }

    return getFileSize(virtualImageDatastorePath);
}


void Rimp::copyFromRepositoryToDatastore(const std::string& virtualImageRepositoryPath, std::string& datastore,
        const std::string& virtualMachineUUID)
{
    string error("");
    RimpException rexception;

    // check datastore path end with '/'
    if (datastore.at(datastore.size() - 1) != '/')
    {
        datastore = datastore.append("/");
    }
    
    checkRimpConfiguration();
    if (!checkDatastore(datastore))
    {
        error = error.append("Provided ''datastore'' :").append(datastore).append(" can not be used");
        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexception.description = error;
        throw rexception;
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
        rexception.description = error;
        throw rexception;
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
            // Then perform a recovery rather then a fullDestination deploy.
            string renameError2 = fileRename(viDatastorePathBackup, viDatastorePath);

            if (!renameError2.empty())
            {
                error = error.append("Can not move :").append(viDatastorePathBackup).append(" to :").append(viDatastorePath).append("\nCaused by :").append(renameError2);

                LOG("[ERROR] [RIMP] %s", error.c_str());
                rexception.description = error;
                throw rexception;
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
        rexception.description = error;
        throw rexception;
    }

    string copyError2 = fileCopy(viRepositoryPath, viDatastorePath);

    if (!copyError2.empty())
    {
        error = error.append("Can not copy to :").append(viDatastorePath).append("\nCaused by :").append(copyError2);

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexception.description = error;
        throw rexception;
    }

    LOG("[INFO] [RIMP] Created virtual image instance for virtual machine [%s]", virtualMachineUUID.c_str());
}

void Rimp::deleteVirtualImageFromDatastore(std::string& datastore, const std::string& virtualMachineUUID)
{
    string error("");
    RimpException rexception;

    // check datastore path end with '/'
    if (datastore.at(datastore.size() - 1) != '/')
    {
        datastore = datastore.append("/");
    }
    
    checkRimpConfiguration();
    if (!checkDatastore(datastore))
    {
        error = error.append("Provided ''datastore'' :").append(datastore).append(" can not be used");
        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexception.description = error;
        throw rexception;
    }

    LOG("[DEBUG] [RIMP] Deleting virtual machine [%s]", virtualMachineUUID.c_str());

    string viDatastorePath(datastore);
    viDatastorePath = viDatastorePath.append(virtualMachineUUID);

    // check the file exist on the datastore
    if (access(viDatastorePath.c_str(), F_OK) == -1)
    {
        error = error.append("Virtual image file does not exist on the ''datastore'' :");
        error = error.append(viDatastorePath);

        LOG("[WARNING] [RIMP] %s", error.c_str());
        return;
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

        // Then perform a recovery rather then a fullDestination deploy.
        string renameError2 = fileRename( viDatastorePath, viDatastorePathBackup);

        if (!renameError2.empty())
        {
            error = error.append("Can not move :").append(viDatastorePath).append(" to :").append(viDatastorePathBackup).append("\nCaused by :").append(renameError2);

            LOG("[ERROR] [RIMP] %s", error.c_str());
            rexception.description = error;
            throw rexception;
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
    RimpException rexception;

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
            rexception.description = error;
            throw rexception;
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
        rexception.description = error;
        throw rexception;
    }

    string viRepositoryDestination(destinationRepositoryPath);
    viRepositoryDestination = viRepositoryDestination.append(snapshot);

    // Check target path do not exist
    if (access(viRepositoryDestination.c_str(), F_OK) == 0)
    {
        error = error.append("Snapshot already exists on the repository: ");
        error = error.append(viRepositoryDestination);

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexception.description = error;
        throw rexception;
    }

    // Checking there are enough free space to copy
    unsigned long int viSize = getFileSize(viDatastoreSource);
    unsigned long int repositoryFreeSize = getFreeSpaceOn(destinationRepositoryPath);

    if (repositoryFreeSize < viSize)
    {
        error = error.append("There is no enough space left to copy the file :");
        error = error.append(viDatastoreSource).append(" to :").append(viRepositoryDestination);

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexception.description = error;
        throw rexception;
    }

    string errorCopy = fileCopy(viDatastoreSource, viRepositoryDestination);
    if (!errorCopy.empty())
    {
        error = error.append("Can not copy to :");
        error = error.append(viRepositoryDestination).append("\nCaused by :").append(errorCopy);

        LOG("[ERROR] [RIMP] %s", error.c_str());
        rexception.description = error;
        throw rexception;
    }

    LOG("[INFO] [RIMP] Created snapshot [%s] from virtual machine [%s]", snapshot.c_str(), virtualMachineUUID.c_str());
}

void Rimp::copy(const std::string& source, const std::string& destination)
{
    checkRimpConfiguration();

    // Get directory from destination filename
    string fullDestination = string(repository).append(destination);
    size_t found = fullDestination.find_last_of("/\\");
    string destinationPath = fullDestination.substr(0, found);

    LOG("[RIMP] Copying '%s' to '%s'", source.c_str(), fullDestination.c_str());

    // Check destination repository folder exist and can be written
    if (access(destinationPath.c_str(), F_OK | W_OK) == -1)
    {
        int err = mkdir(destinationPath.c_str(), S_IRWXU | S_IRWXG);

        if (err == -1)
        {
            string error("");
            error = error.append("Can not create destination folder at :");
            error = error.append(destinationPath).append(" Caused by: ").append(strerror(errno));

            LOG("[ERROR] [RIMP] %s", error.c_str());

            RimpException rexception;
            rexception.description = error;
            throw rexception;
        }
    }

    // Check source file exist and can be read
    if (access(source.c_str(), F_OK | R_OK) == -1)
    {
        string error("");
        error = error.append("Source file does not exist or can not be read: ");
        error = error.append(source);

        LOG("[ERROR] [RIMP] %s", error.c_str());

        RimpException rexception;
        rexception.description = error;
        throw rexception;
    }

    // Check target path do not exist
    if (access(fullDestination.c_str(), F_OK) == 0)
    {
        string error("");
        error = error.append("Destination already exists: ");
        error = error.append(fullDestination);

        LOG("[ERROR] [RIMP] %s", error.c_str());

        RimpException rexception;
        rexception.description = error;
        throw rexception;
    }

    // Checking there are enough free space to copy
    unsigned long int viSize = getFileSize(source);
    unsigned long int repositoryFreeSize = getFreeSpaceOn(destinationPath);

    if (repositoryFreeSize < viSize)
    {
        string error("");
        error = error.append("There is no enough space left to copy the file: ");
        error = error.append(source).append(" to :").append(destinationPath);

        LOG("[ERROR] [RIMP] %s", error.c_str());

        RimpException rexception;
        rexception.description = error;
        throw rexception;
    }

    string errorCopy = fileCopy(source, fullDestination);
    if (!errorCopy.empty())
    {
        string error("");
        error = error.append("Can not copy to: ");
        error = error.append(fullDestination).append("\nCaused by :").append(errorCopy);

        LOG("[ERROR] [RIMP] %s", error.c_str());

        RimpException rexception;
        rexception.description = error;
        throw rexception;
    }

     LOG("[RIMP] File '%s' copied", fullDestination.c_str());
}

void Rimp::rename(const std::string& oldPath, const std::string& newPath)
{
    LOG("[RIMP] Moving '%s' to '%s'", oldPath.c_str(), newPath.c_str());

    // Check source file exist and can be read
    if (access(oldPath.c_str(), F_OK | R_OK) == -1)
    {
        string error("Source file does not exist or can not be read: ");
        error = error.append(oldPath);
        LOG("[ERROR] [RIMP] %s", error.c_str());

        RimpException rexception;
        rexception.description = error;
        throw rexception;
    }

    size_t lastSlash = newPath.find_last_of("/\\");
    string destinationFolder = newPath.substr(0, lastSlash == 0 ? 1 : lastSlash);

    // Check destination folder exist and can be written
    if (access(destinationFolder.c_str(), F_OK | W_OK) == -1)
    {
        int err = mkdir(destinationFolder.c_str(), S_IRWXU | S_IRWXG);

        if (err == -1)
        {
            string error("Unable to create destination folder ");
            error = error.append(destinationFolder).append(": ").append(strerror(errno));
            LOG("[ERROR] [RIMP] %s", error.c_str());

            RimpException rexception;
            rexception.description = error;
            throw rexception;
        }
    }

    // Check that new path does not exist    
    if (access(newPath.c_str(), F_OK) == 0)
    {
        string error("Destination already exists: ");
        error = error.append(newPath);
        LOG("[ERROR] [RIMP] %s", error.c_str());

        RimpException rexception;
        rexception.description = error;
        throw rexception;
    }

    // Rename file
    string error = fileRename(oldPath, newPath);
    if (!error.empty())
    {
        string error("Unable to move to: ");
        error = error.append(newPath).append(error);
        LOG("[ERROR] [RIMP] %s", error.c_str());

        RimpException rexception;
        rexception.description = error;
        throw rexception;
    }

    LOG("[RIMP] File '%s' moved to '%s'", oldPath.c_str(), newPath.c_str());
}
