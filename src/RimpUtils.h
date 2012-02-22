/**
 * Abiquo community edition
 * cloud management application for hybrid clouds
 * Copyright (C) 2008-2010 - Abiquo Holdings S.L.
 *
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU LESSER GENERAL PUBLIC
 * LICENSE as published by the Free Software Foundation under
 * version 3 of the License
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * LESSER GENERAL PUBLIC LICENSE v.3 for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef RIMP_UTILS_H
#define RIMP_UTILS_H

//#include <nfs/nfs.h>
//#include <linux/nfs_mount.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include <sys/vfs.h>
#include <pwd.h>
#include <fts.h>
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <fstab.h>
#include <mntent.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <string.h>

#include <Debug.h>

#include <aim_types.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

//#include <boost/uuid/uuid.hpp>
//#include <boost/uuid/uuid_serialize.hpp>
//#include <boost/uuid/uuid_generators.hpp>

#include <uuid/uuid.h>


// net interfaces
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <sys/socket.h>
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned char u8;
#include <linux/cciss_ioctl.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>

using namespace boost::filesystem;

//using namespace boost::uuids;

using namespace std;

/** Identify a file present on the ''repository'' in order to validate as an ''abiquo'' repository. */
const string REPOSITORY_MARK = ".abiquo_repository\0";

/** Datastore folder mark perfix*/
const string DATASTORE_MARK = "datastoreuuid.\0";

/**
 * @return true if the configured ''datastore'' path exist and can be read/write.
 */
bool checkDatastore(const string& datastore);

/**
 * @return true if the configured ''repository'' path is mounted on the host and can be read/write and exist the ''REPOSITORY_MARK''.
 */
bool checkRepository(const string& repository);

/******************************************************************************
 *                              COLLECTING INFORMATION
 * Functions related to obtain information about the system state.
 * ***************************************************************************/

/**
 * @return the available free space (in KiloBytes) on the provided directory or -1 if do not exist.
 */
unsigned long int getFreeSpaceOn(const string& dir);

/**
 * @return the total size of the partition (in KiloBytes) of the provided directory or -1 if do not exist.
 */
unsigned long int getTotalSpaceOn(const string& dir);

/**
 * @return the size of the provided file (in KiloBytes) or -1 if do not exist.
 */
unsigned long int getFileSize(const string& filename);

/**
 * Copy ''source'' file into the ''target'' path.
 * If ''target'' path exist it returns.
 *
 * @return error message or NULL on success.
 */
string fileCopy(const string& source, const string& target);

/**
 * Rename (move) ''source'' file into the ''target'' path.
 * If ''target'' path directory doesn't exist it will be created.
 *
 * @return error message or NULL on success.
 */
string fileRename(const string& source, const string& target);

/**
 * TODO TBD
 */
vector<Datastore> getDatastoresFromMtab();


/**
 * find the folder mark ''datastoreuuid.XXXX'' or create it. On the top of the datastore filesystem (the mount point)
 *
 * @return the datastore UUID
 * */
string getDatastoreUuidFolderMark(const string& rootMountPoint);



/**
 * TODO TBD
 */
vector<NetInterface> getNetInterfacesFromXXX();

#endif
