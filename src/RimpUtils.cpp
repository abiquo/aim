#include <RimpUtils.h>
#include <ExecUtils.h>
#include <iostream>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <iterator> 

bool checkDatastore(const string& datastore)
{
  bool success = true;

  // check ''datastore'' exist and can be r/w
  if (access(datastore.c_str(), F_OK | R_OK | W_OK) == -1)
  {
    LOG("[ERROR] [RIMP] Initialization fails :\n"
        "''datastore'' [%s] does not exist or is not r/w", datastore.c_str());

    success = false;
  } // ''datastore'' exist

  return success;
}

const string MTAB_FILE("/etc/mtab");

/**
 * @param rootMountPoint, top level datastore folder (is accessible).
 * */
string getDatastoreUuidFolderMark(const string& rootMountPoint)
{
  // http://www.boost.org/doc/libs/1_33_1/libs/filesystem/doc/index.htm
  const char* markPrefix = DATASTORE_MARK.c_str();
  path p = rootMountPoint;
  string uuid;

  // iterate over the datastore root folder
  for (directory_iterator itr(p); itr!=directory_iterator() && uuid.empty(); ++itr)
  {
    if (is_directory(*itr))
    {
      string lea = itr->leaf();

      if (lea.at(lea.size() - 1) == '/')
      {
        LOG("[RIMP] ERROR - folder mark ends with / [%s]", lea.c_str());   
        lea = lea.substr(0, lea.length() -2);
      }

      if (lea.at(0) == '/')
      {
        LOG("[RIMP] ERROR - folder mark starts with / [%s]", lea.c_str());
        lea = lea.substr(1, lea.length() -1);
      }

      const char* str = lea.c_str(); //itr->leaf().c_str();

      if(strncmp(str, markPrefix, strlen(markPrefix))==0)
      {
        string filestr = str;
        uuid = filestr.substr(strlen(markPrefix), strlen(str)-1);
        LOG("[RIMP] [DEBUG] Datastore UUID found: %s", uuid.c_str());
      }
    }
  }

  if(!uuid.empty())
  {
    // folder mark found
    return uuid;
  }
  else
  {
    // create the folder mark

    //XXX Boost 1.42 uuid uuid = random_generator(); string uuidstr = to_string(uuid);

    char uuidBuff[36];
    uuid_t uuidGenerated;
    uuid_generate_random(uuidGenerated);
    uuid_unparse(uuidGenerated, uuidBuff);

    string uuidstr = string(uuidBuff);

    string datastoreFolderMark = string(rootMountPoint.c_str());
    datastoreFolderMark.append(DATASTORE_MARK).append(uuidstr);

    create_directory(datastoreFolderMark);

    LOG("[RIMP] [DEBUG] Created Datastore [%s] UUID folder mark [%s]", rootMountPoint.c_str(), datastoreFolderMark.c_str());

    return uuidstr;
  }
}


vector<Datastore> getDatastoresFromMtab(const vector<string> validTypes)
{
  vector<Datastore> datastores;

  struct mntent *ent = NULL;
  FILE *mounts = setmntent(MTAB_FILE.c_str(), "r");

  while ((ent = getmntent(mounts)) != NULL)
  {
    string type(ent->mnt_type);
    string directory(ent->mnt_dir);

    // check if the file system type should be considered
    bool found = false;
    for (int i = 0; (i < (int) validTypes.size()) && !found; i++)
    {
      int com = validTypes[i].compare(type);
      found = (com == 0);
    }

    if (found && directory.compare("/boot"))
    {
      Datastore datastore;

      char* device = ent->mnt_fsname;
      char* path = ent->mnt_dir;
      unsigned long int totalSize = getTotalSpaceOn(path);
      unsigned long int usableSize = getFreeSpaceOn(path);


      // its an accessible directory (can r/w)
      if(is_directory(path))
      {
        // check ''repository'' exist and can be r/w
        if (access(path, F_OK | R_OK | W_OK) == -1)
        {
          LOG("[RIMP] [WARN] the current datastore mount point [%s] "
              "is not accessible", path);
        }
        else
        {
          string strPath = string(path);
          if (strPath.at(strPath.size() - 1) != '/')
          {
            strPath = strPath.append("/");
          }

          string dsUuid = getDatastoreUuidFolderMark(strPath.c_str());

          datastore.device = string(device);
          datastore.path = string(strPath.c_str());
          // XXX unused type
          datastore.type = dsUuid;

          datastore.totalSize = totalSize;
          datastore.usableSize = usableSize;

          datastores.push_back(datastore);
        }
      }
      else
      {
        LOG("[RIMP] [ERROR] the current mount point is not a directory [%s]", path);
      }

    }
  }

  endmntent(mounts);
  //free( mountP);

  return datastores;
}

string getHardwareAddress(const char* devname)
{
  int sockfd;
  int io;
  struct ifreq ifr;
  RimpException rimpexc;

  sprintf(ifr.ifr_name, "%s", devname);

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    rimpexc.description = "Can not open socket stream";
    throw rimpexc;
  }

  io = ioctl(sockfd, SIOCGIFHWADDR, (char *) &ifr);
  if (io < 0)
  {
    rimpexc.description = string("Can not obtain hardware address for device").append(devname);
    throw rimpexc;
  }

  char macBuffer[17];

  sprintf(macBuffer, "%02x:%02x:%02x:%02x:%02x:%02x", (unsigned char) ifr.ifr_hwaddr.sa_data[0],
      (unsigned char) ifr.ifr_hwaddr.sa_data[1], (unsigned char) ifr.ifr_hwaddr.sa_data[2],
      (unsigned char) ifr.ifr_hwaddr.sa_data[3], (unsigned char) ifr.ifr_hwaddr.sa_data[4],
      (unsigned char) ifr.ifr_hwaddr.sa_data[5]);


  close(sockfd);

  return string(macBuffer);
}

/**
 * Get all the network device names from the ''/pro/net/dev''
 * Based on the code from  http://www.doctort.org/adam/ by Adam Pierce <adam@doctort.org>
 * */
vector<NetInterface> getNetInterfacesFromProcNetDev()
{
    vector<NetInterface> interfaces;

    std::string results = exec("ip link show | awk '/ UP/ && $2 ~ /^[^@]+$/ {print substr($2, 0, length($2)-1)}'");
    if (results.empty())
    {
        RimpException exception;
        exception.description = "Unable to get the network interfaces.";
        throw exception;
    }

    std::stringstream ss(results);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> nets(begin, end);

    for(std::vector<std::string>::size_type i = 0; i != nets.size(); i++) 
    {
        NetInterface netInterface;
        netInterface.name = nets[i];
        netInterface.physicalAddress = getHardwareAddress(nets[i].c_str());
        interfaces.push_back(netInterface);
    }
    return interfaces;
}

bool checkRepository(const string& repository)
{
  // check ''repository'' exist and can be r/w
  if (access(repository.c_str(), F_OK | R_OK | W_OK) == -1)
  {
    LOG("[ERROR] [RIMP] Initialization fails :\n"
        "''repository'' [%s] does not exist or is not r/w", repository.c_str());

    return false;
  } // ''repository'' exist


  // check for the ''REPOSITORY_MARK'' file on the ''repository''
  string repositoryFileMark(repository);
  repositoryFileMark = repositoryFileMark.append(REPOSITORY_MARK);

  if (access(repositoryFileMark.c_str(), F_OK) == -1)
  {
    LOG("[ERROR] [RIMP] Initialization fails :\n"
        "''repository'' [%s] does not contain the abiquo repository marker file [%s]",
        repository.c_str(), REPOSITORY_MARK.c_str());

    return false;
  }// ''repository'' file mark exist

  return true;
}

extern int alphasort();
int getNumItemsOnDir(string dir)
{
  // using : http://www.programatium.com/manuales/c/19.htm
  struct dirent **files;
  return scandir(dir.c_str(), &files, 0, alphasort);
}

/******************************************************************************
 * ***************************************************************************/

unsigned long int getFreeSpaceOn(const string& dir)
{
  struct statvfs sbuf;

  if (statvfs(dir.c_str(), &sbuf) < 0)
  {
    //error("[RIMP] Can not get the free space on [%s]", dir);
    return -1;
  }

  unsigned long int s = sbuf.f_bavail;
  long bs = sbuf.f_bsize;
  unsigned long int div = s / 1024l;
  unsigned int mod = s % 1024l;

  unsigned long int freeSpace = (div * bs) + ((mod * bs) / 1024l);

  return freeSpace;
}

unsigned long int getTotalSpaceOn(const string& dir)
{
  struct statvfs sbuf;

  if (statvfs(dir.c_str(), &sbuf) < 0)
  {
    //error("[RIMP] Can not get the total space on [%s]", dir);
    return -1;
  }

  unsigned long int s = sbuf.f_blocks;
  long bs = sbuf.f_bsize;
  unsigned long int div = s / 1024l;
  unsigned int mod = s % 1024l;

  unsigned long int totalSpace = (div * bs) + ((mod * bs) / 1024l);

  return totalSpace;
}

unsigned long int getFileSize(const string& filename)
{
  path p (filename, native);

  return file_size(p) / 1024;
}
//    struct stat results;
//    unsigned long int size;
//    if (stat(filename.c_str(), &results) == 0)
//    {
//        size = results.st_size / 1024l;
//    } else
//    {
//error("[RIMP] Can not get the size of [%s]", filename);
//        size = -1;
//    }
//    return size;


// target always ends with '/'
string createTargetDirectory(const string& target)
{
  string error = string();

  if (target.empty() || access(target.c_str(), F_OK) == 0)
  {
    return error;
  } else
  {
    int iPath = target.substr(0, target.length() - 1).find_last_of('/');
    string parentDir = target.substr(0, iPath + 1);

    error = createTargetDirectory(parentDir);
    if (!error.empty())
    {
      return error;
    }

    int err = mkdir(target.c_str(), S_IRWXU | S_IRWXG);
    if (err == -1)
    {
      error = error.append("Can not create folder at :").append(target);
      error = error.append("\nCaused by: ").append(strerror(errno));
      return error;
    }
  }

  return error;
}

string fileCopy(const string& source, const string& target)
{
  string targetCopy(target);

  int iPath = targetCopy.find_last_of('/');
  string parentDir = targetCopy.substr(0, iPath + 1);
  string error = createTargetDirectory(parentDir);

  if (!error.empty())
  {
    return error;
  }

  try
  {
    path psource (source.c_str(), native);
    path ptarget (target.c_str(), native);

    copy_file(psource, ptarget);

  } catch (exception& e)
  {
    string error(e.what());
    return error;
  }
  //std::ifstream ifs("input.txt", std::ios::binary);
  //std::ofstream ofs("output.txt", std::ios::binary);
  //ofs << ifs.rdbuf();

  return string();
}

string fileRename(const string& source, const string& target)
{
  string targetCopy(target);

  int iPath = targetCopy.find_last_of('/');
  string parentDir = targetCopy.substr(0, iPath + 1);
  string error = createTargetDirectory(parentDir);

  if (!error.empty())
  {
    return error;
  }

  try
  {
    path psource (source.c_str(), native);
    path ptarget (target.c_str(), native);

    rename(psource, ptarget);

  } catch (exception& e)
  {
    string error(e.what());
    return error;
  }

  return string();
}

