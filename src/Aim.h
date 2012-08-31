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

#ifndef Aim_H
#define Aim_H

#include <TProcessor.h>
#include "aim_types.h"



class AimIf {
 public:
  virtual ~AimIf() {}
  virtual void checkRimpConfiguration() = 0;
  virtual int64_t getDiskFileSize(const std::string& virtualImageDatastorePath) = 0;
  virtual void getDatastores(std::vector<Datastore> & _return) = 0;
  virtual void getNetInterfaces(std::vector<NetInterface> & _return) = 0;
  virtual void copyFromRepositoryToDatastore(const std::string& virtualImageRepositoryPath, const std::string& datastorePath, const std::string& virtualMachineUUID) = 0;
  virtual void deleteVirtualImageFromDatastore(const std::string& datastorePath, const std::string& virtualMachineUUID) = 0;
  virtual void copyFromDatastoreToRepository(const std::string& virtualMachineUUID, const std::string& snapshot, const std::string& destinationRepositoryPath, const std::string& sourceDatastorePath) = 0;
  virtual void createVLAN(const int32_t vlanTag, const std::string& vlanInterface, const std::string& bridgeInterface) = 0;
  virtual void deleteVLAN(const int32_t vlanTag, const std::string& vlanInterface, const std::string& bridgeInterface) = 0;
  virtual void checkVLANConfiguration() = 0;
  virtual void getInitiatorIQN(std::string& _return) = 0;
  virtual void rescanISCSI(const std::vector<std::string> & targets) = 0;
};

class AimNull : virtual public AimIf {
 public:
  virtual ~AimNull() {}
  void checkRimpConfiguration() {
    return;
  }
  int64_t getDiskFileSize(const std::string& /* virtualImageDatastorePath */) {
    int64_t _return = 0;
    return _return;
  }
  void getDatastores(std::vector<Datastore> & /* _return */) {
    return;
  }
  void getNetInterfaces(std::vector<NetInterface> & /* _return */) {
    return;
  }
  void copyFromRepositoryToDatastore(const std::string& /* virtualImageRepositoryPath */, const std::string& /* datastorePath */, const std::string& /* virtualMachineUUID */) {
    return;
  }
  void deleteVirtualImageFromDatastore(const std::string& /* datastorePath */, const std::string& /* virtualMachineUUID */) {
    return;
  }
  void copyFromDatastoreToRepository(const std::string& /* virtualMachineUUID */, const std::string& /* snapshot */, const std::string& /* destinationRepositoryPath */, const std::string& /* sourceDatastorePath */) {
    return;
  }
  void createVLAN(const int32_t /* vlanTag */, const std::string& /* vlanInterface */, const std::string& /* bridgeInterface */) {
    return;
  }
  void deleteVLAN(const int32_t /* vlanTag */, const std::string& /* vlanInterface */, const std::string& /* bridgeInterface */) {
    return;
  }
  void checkVLANConfiguration() {
    return;
  }
  void getInitiatorIQN(std::string& /* _return */) {
    return;
  }
  void rescanISCSI(const std::vector<std::string> & /* targets */) {
    return;
  }
};

class Aim_checkRimpConfiguration_args {
 public:

  Aim_checkRimpConfiguration_args() {
  }

  virtual ~Aim_checkRimpConfiguration_args() throw() {}


  bool operator == (const Aim_checkRimpConfiguration_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Aim_checkRimpConfiguration_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_checkRimpConfiguration_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_checkRimpConfiguration_pargs {
 public:


  virtual ~Aim_checkRimpConfiguration_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_checkRimpConfiguration_result {
 public:

  Aim_checkRimpConfiguration_result() {
  }

  virtual ~Aim_checkRimpConfiguration_result() throw() {}

  RimpException re;

  struct __isset {
    __isset() : re(false) {}
    bool re;
  } __isset;

  bool operator == (const Aim_checkRimpConfiguration_result & rhs) const
  {
    if (!(re == rhs.re))
      return false;
    return true;
  }
  bool operator != (const Aim_checkRimpConfiguration_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_checkRimpConfiguration_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_checkRimpConfiguration_presult {
 public:


  virtual ~Aim_checkRimpConfiguration_presult() throw() {}

  RimpException re;

  struct __isset {
    __isset() : re(false) {}
    bool re;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class Aim_getDiskFileSize_args {
 public:

  Aim_getDiskFileSize_args() : virtualImageDatastorePath("") {
  }

  virtual ~Aim_getDiskFileSize_args() throw() {}

  std::string virtualImageDatastorePath;

  struct __isset {
    __isset() : virtualImageDatastorePath(false) {}
    bool virtualImageDatastorePath;
  } __isset;

  bool operator == (const Aim_getDiskFileSize_args & rhs) const
  {
    if (!(virtualImageDatastorePath == rhs.virtualImageDatastorePath))
      return false;
    return true;
  }
  bool operator != (const Aim_getDiskFileSize_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_getDiskFileSize_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getDiskFileSize_pargs {
 public:


  virtual ~Aim_getDiskFileSize_pargs() throw() {}

  const std::string* virtualImageDatastorePath;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getDiskFileSize_result {
 public:

  Aim_getDiskFileSize_result() : success(0) {
  }

  virtual ~Aim_getDiskFileSize_result() throw() {}

  int64_t success;
  RimpException re;

  struct __isset {
    __isset() : success(false), re(false) {}
    bool success;
    bool re;
  } __isset;

  bool operator == (const Aim_getDiskFileSize_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(re == rhs.re))
      return false;
    return true;
  }
  bool operator != (const Aim_getDiskFileSize_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_getDiskFileSize_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getDiskFileSize_presult {
 public:


  virtual ~Aim_getDiskFileSize_presult() throw() {}

  int64_t* success;
  RimpException re;

  struct __isset {
    __isset() : success(false), re(false) {}
    bool success;
    bool re;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class Aim_getDatastores_args {
 public:

  Aim_getDatastores_args() {
  }

  virtual ~Aim_getDatastores_args() throw() {}


  bool operator == (const Aim_getDatastores_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Aim_getDatastores_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_getDatastores_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getDatastores_pargs {
 public:


  virtual ~Aim_getDatastores_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getDatastores_result {
 public:

  Aim_getDatastores_result() {
  }

  virtual ~Aim_getDatastores_result() throw() {}

  std::vector<Datastore>  success;
  RimpException re;

  struct __isset {
    __isset() : success(false), re(false) {}
    bool success;
    bool re;
  } __isset;

  bool operator == (const Aim_getDatastores_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(re == rhs.re))
      return false;
    return true;
  }
  bool operator != (const Aim_getDatastores_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_getDatastores_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getDatastores_presult {
 public:


  virtual ~Aim_getDatastores_presult() throw() {}

  std::vector<Datastore> * success;
  RimpException re;

  struct __isset {
    __isset() : success(false), re(false) {}
    bool success;
    bool re;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class Aim_getNetInterfaces_args {
 public:

  Aim_getNetInterfaces_args() {
  }

  virtual ~Aim_getNetInterfaces_args() throw() {}


  bool operator == (const Aim_getNetInterfaces_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Aim_getNetInterfaces_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_getNetInterfaces_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getNetInterfaces_pargs {
 public:


  virtual ~Aim_getNetInterfaces_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getNetInterfaces_result {
 public:

  Aim_getNetInterfaces_result() {
  }

  virtual ~Aim_getNetInterfaces_result() throw() {}

  std::vector<NetInterface>  success;
  RimpException re;

  struct __isset {
    __isset() : success(false), re(false) {}
    bool success;
    bool re;
  } __isset;

  bool operator == (const Aim_getNetInterfaces_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(re == rhs.re))
      return false;
    return true;
  }
  bool operator != (const Aim_getNetInterfaces_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_getNetInterfaces_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getNetInterfaces_presult {
 public:


  virtual ~Aim_getNetInterfaces_presult() throw() {}

  std::vector<NetInterface> * success;
  RimpException re;

  struct __isset {
    __isset() : success(false), re(false) {}
    bool success;
    bool re;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class Aim_copyFromRepositoryToDatastore_args {
 public:

  Aim_copyFromRepositoryToDatastore_args() : virtualImageRepositoryPath(""), datastorePath(""), virtualMachineUUID("") {
  }

  virtual ~Aim_copyFromRepositoryToDatastore_args() throw() {}

  std::string virtualImageRepositoryPath;
  std::string datastorePath;
  std::string virtualMachineUUID;

  struct __isset {
    __isset() : virtualImageRepositoryPath(false), datastorePath(false), virtualMachineUUID(false) {}
    bool virtualImageRepositoryPath;
    bool datastorePath;
    bool virtualMachineUUID;
  } __isset;

  bool operator == (const Aim_copyFromRepositoryToDatastore_args & rhs) const
  {
    if (!(virtualImageRepositoryPath == rhs.virtualImageRepositoryPath))
      return false;
    if (!(datastorePath == rhs.datastorePath))
      return false;
    if (!(virtualMachineUUID == rhs.virtualMachineUUID))
      return false;
    return true;
  }
  bool operator != (const Aim_copyFromRepositoryToDatastore_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_copyFromRepositoryToDatastore_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_copyFromRepositoryToDatastore_pargs {
 public:


  virtual ~Aim_copyFromRepositoryToDatastore_pargs() throw() {}

  const std::string* virtualImageRepositoryPath;
  const std::string* datastorePath;
  const std::string* virtualMachineUUID;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_copyFromRepositoryToDatastore_result {
 public:

  Aim_copyFromRepositoryToDatastore_result() {
  }

  virtual ~Aim_copyFromRepositoryToDatastore_result() throw() {}

  RimpException re;

  struct __isset {
    __isset() : re(false) {}
    bool re;
  } __isset;

  bool operator == (const Aim_copyFromRepositoryToDatastore_result & rhs) const
  {
    if (!(re == rhs.re))
      return false;
    return true;
  }
  bool operator != (const Aim_copyFromRepositoryToDatastore_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_copyFromRepositoryToDatastore_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_copyFromRepositoryToDatastore_presult {
 public:


  virtual ~Aim_copyFromRepositoryToDatastore_presult() throw() {}

  RimpException re;

  struct __isset {
    __isset() : re(false) {}
    bool re;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class Aim_deleteVirtualImageFromDatastore_args {
 public:

  Aim_deleteVirtualImageFromDatastore_args() : datastorePath(""), virtualMachineUUID("") {
  }

  virtual ~Aim_deleteVirtualImageFromDatastore_args() throw() {}

  std::string datastorePath;
  std::string virtualMachineUUID;

  struct __isset {
    __isset() : datastorePath(false), virtualMachineUUID(false) {}
    bool datastorePath;
    bool virtualMachineUUID;
  } __isset;

  bool operator == (const Aim_deleteVirtualImageFromDatastore_args & rhs) const
  {
    if (!(datastorePath == rhs.datastorePath))
      return false;
    if (!(virtualMachineUUID == rhs.virtualMachineUUID))
      return false;
    return true;
  }
  bool operator != (const Aim_deleteVirtualImageFromDatastore_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_deleteVirtualImageFromDatastore_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_deleteVirtualImageFromDatastore_pargs {
 public:


  virtual ~Aim_deleteVirtualImageFromDatastore_pargs() throw() {}

  const std::string* datastorePath;
  const std::string* virtualMachineUUID;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_deleteVirtualImageFromDatastore_result {
 public:

  Aim_deleteVirtualImageFromDatastore_result() {
  }

  virtual ~Aim_deleteVirtualImageFromDatastore_result() throw() {}

  RimpException re;

  struct __isset {
    __isset() : re(false) {}
    bool re;
  } __isset;

  bool operator == (const Aim_deleteVirtualImageFromDatastore_result & rhs) const
  {
    if (!(re == rhs.re))
      return false;
    return true;
  }
  bool operator != (const Aim_deleteVirtualImageFromDatastore_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_deleteVirtualImageFromDatastore_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_deleteVirtualImageFromDatastore_presult {
 public:


  virtual ~Aim_deleteVirtualImageFromDatastore_presult() throw() {}

  RimpException re;

  struct __isset {
    __isset() : re(false) {}
    bool re;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class Aim_copyFromDatastoreToRepository_args {
 public:

  Aim_copyFromDatastoreToRepository_args() : virtualMachineUUID(""), snapshot(""), destinationRepositoryPath(""), sourceDatastorePath("") {
  }

  virtual ~Aim_copyFromDatastoreToRepository_args() throw() {}

  std::string virtualMachineUUID;
  std::string snapshot;
  std::string destinationRepositoryPath;
  std::string sourceDatastorePath;

  struct __isset {
    __isset() : virtualMachineUUID(false), snapshot(false), destinationRepositoryPath(false), sourceDatastorePath(false) {}
    bool virtualMachineUUID;
    bool snapshot;
    bool destinationRepositoryPath;
    bool sourceDatastorePath;
  } __isset;

  bool operator == (const Aim_copyFromDatastoreToRepository_args & rhs) const
  {
    if (!(virtualMachineUUID == rhs.virtualMachineUUID))
      return false;
    if (!(snapshot == rhs.snapshot))
      return false;
    if (!(destinationRepositoryPath == rhs.destinationRepositoryPath))
      return false;
    if (!(sourceDatastorePath == rhs.sourceDatastorePath))
      return false;
    return true;
  }
  bool operator != (const Aim_copyFromDatastoreToRepository_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_copyFromDatastoreToRepository_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_copyFromDatastoreToRepository_pargs {
 public:


  virtual ~Aim_copyFromDatastoreToRepository_pargs() throw() {}

  const std::string* virtualMachineUUID;
  const std::string* snapshot;
  const std::string* destinationRepositoryPath;
  const std::string* sourceDatastorePath;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_copyFromDatastoreToRepository_result {
 public:

  Aim_copyFromDatastoreToRepository_result() {
  }

  virtual ~Aim_copyFromDatastoreToRepository_result() throw() {}

  RimpException re;

  struct __isset {
    __isset() : re(false) {}
    bool re;
  } __isset;

  bool operator == (const Aim_copyFromDatastoreToRepository_result & rhs) const
  {
    if (!(re == rhs.re))
      return false;
    return true;
  }
  bool operator != (const Aim_copyFromDatastoreToRepository_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_copyFromDatastoreToRepository_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_copyFromDatastoreToRepository_presult {
 public:


  virtual ~Aim_copyFromDatastoreToRepository_presult() throw() {}

  RimpException re;

  struct __isset {
    __isset() : re(false) {}
    bool re;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class Aim_createVLAN_args {
 public:

  Aim_createVLAN_args() : vlanTag(0), vlanInterface(""), bridgeInterface("") {
  }

  virtual ~Aim_createVLAN_args() throw() {}

  int32_t vlanTag;
  std::string vlanInterface;
  std::string bridgeInterface;

  struct __isset {
    __isset() : vlanTag(false), vlanInterface(false), bridgeInterface(false) {}
    bool vlanTag;
    bool vlanInterface;
    bool bridgeInterface;
  } __isset;

  bool operator == (const Aim_createVLAN_args & rhs) const
  {
    if (!(vlanTag == rhs.vlanTag))
      return false;
    if (!(vlanInterface == rhs.vlanInterface))
      return false;
    if (!(bridgeInterface == rhs.bridgeInterface))
      return false;
    return true;
  }
  bool operator != (const Aim_createVLAN_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_createVLAN_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_createVLAN_pargs {
 public:


  virtual ~Aim_createVLAN_pargs() throw() {}

  const int32_t* vlanTag;
  const std::string* vlanInterface;
  const std::string* bridgeInterface;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_createVLAN_result {
 public:

  Aim_createVLAN_result() {
  }

  virtual ~Aim_createVLAN_result() throw() {}

  VLanException ve;

  struct __isset {
    __isset() : ve(false) {}
    bool ve;
  } __isset;

  bool operator == (const Aim_createVLAN_result & rhs) const
  {
    if (!(ve == rhs.ve))
      return false;
    return true;
  }
  bool operator != (const Aim_createVLAN_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_createVLAN_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_createVLAN_presult {
 public:


  virtual ~Aim_createVLAN_presult() throw() {}

  VLanException ve;

  struct __isset {
    __isset() : ve(false) {}
    bool ve;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class Aim_deleteVLAN_args {
 public:

  Aim_deleteVLAN_args() : vlanTag(0), vlanInterface(""), bridgeInterface("") {
  }

  virtual ~Aim_deleteVLAN_args() throw() {}

  int32_t vlanTag;
  std::string vlanInterface;
  std::string bridgeInterface;

  struct __isset {
    __isset() : vlanTag(false), vlanInterface(false), bridgeInterface(false) {}
    bool vlanTag;
    bool vlanInterface;
    bool bridgeInterface;
  } __isset;

  bool operator == (const Aim_deleteVLAN_args & rhs) const
  {
    if (!(vlanTag == rhs.vlanTag))
      return false;
    if (!(vlanInterface == rhs.vlanInterface))
      return false;
    if (!(bridgeInterface == rhs.bridgeInterface))
      return false;
    return true;
  }
  bool operator != (const Aim_deleteVLAN_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_deleteVLAN_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_deleteVLAN_pargs {
 public:


  virtual ~Aim_deleteVLAN_pargs() throw() {}

  const int32_t* vlanTag;
  const std::string* vlanInterface;
  const std::string* bridgeInterface;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_deleteVLAN_result {
 public:

  Aim_deleteVLAN_result() {
  }

  virtual ~Aim_deleteVLAN_result() throw() {}

  VLanException ve;

  struct __isset {
    __isset() : ve(false) {}
    bool ve;
  } __isset;

  bool operator == (const Aim_deleteVLAN_result & rhs) const
  {
    if (!(ve == rhs.ve))
      return false;
    return true;
  }
  bool operator != (const Aim_deleteVLAN_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_deleteVLAN_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_deleteVLAN_presult {
 public:


  virtual ~Aim_deleteVLAN_presult() throw() {}

  VLanException ve;

  struct __isset {
    __isset() : ve(false) {}
    bool ve;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class Aim_checkVLANConfiguration_args {
 public:

  Aim_checkVLANConfiguration_args() {
  }

  virtual ~Aim_checkVLANConfiguration_args() throw() {}


  bool operator == (const Aim_checkVLANConfiguration_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Aim_checkVLANConfiguration_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_checkVLANConfiguration_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_checkVLANConfiguration_pargs {
 public:


  virtual ~Aim_checkVLANConfiguration_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_checkVLANConfiguration_result {
 public:

  Aim_checkVLANConfiguration_result() {
  }

  virtual ~Aim_checkVLANConfiguration_result() throw() {}

  VLanException ve;

  struct __isset {
    __isset() : ve(false) {}
    bool ve;
  } __isset;

  bool operator == (const Aim_checkVLANConfiguration_result & rhs) const
  {
    if (!(ve == rhs.ve))
      return false;
    return true;
  }
  bool operator != (const Aim_checkVLANConfiguration_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_checkVLANConfiguration_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_checkVLANConfiguration_presult {
 public:


  virtual ~Aim_checkVLANConfiguration_presult() throw() {}

  VLanException ve;

  struct __isset {
    __isset() : ve(false) {}
    bool ve;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class Aim_getInitiatorIQN_args {
 public:

  Aim_getInitiatorIQN_args() {
  }

  virtual ~Aim_getInitiatorIQN_args() throw() {}


  bool operator == (const Aim_getInitiatorIQN_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const Aim_getInitiatorIQN_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_getInitiatorIQN_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getInitiatorIQN_pargs {
 public:


  virtual ~Aim_getInitiatorIQN_pargs() throw() {}


  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getInitiatorIQN_result {
 public:

  Aim_getInitiatorIQN_result() : success("") {
  }

  virtual ~Aim_getInitiatorIQN_result() throw() {}

  std::string success;
  StorageException se;

  struct __isset {
    __isset() : success(false), se(false) {}
    bool success;
    bool se;
  } __isset;

  bool operator == (const Aim_getInitiatorIQN_result & rhs) const
  {
    if (!(success == rhs.success))
      return false;
    if (!(se == rhs.se))
      return false;
    return true;
  }
  bool operator != (const Aim_getInitiatorIQN_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_getInitiatorIQN_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_getInitiatorIQN_presult {
 public:


  virtual ~Aim_getInitiatorIQN_presult() throw() {}

  std::string* success;
  StorageException se;

  struct __isset {
    __isset() : success(false), se(false) {}
    bool success;
    bool se;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class Aim_rescanISCSI_args {
 public:

  Aim_rescanISCSI_args() {
  }

  virtual ~Aim_rescanISCSI_args() throw() {}

  std::vector<std::string>  targets;

  struct __isset {
    __isset() : targets(false) {}
    bool targets;
  } __isset;

  bool operator == (const Aim_rescanISCSI_args & rhs) const
  {
    if (!(targets == rhs.targets))
      return false;
    return true;
  }
  bool operator != (const Aim_rescanISCSI_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_rescanISCSI_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_rescanISCSI_pargs {
 public:


  virtual ~Aim_rescanISCSI_pargs() throw() {}

  const std::vector<std::string> * targets;

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_rescanISCSI_result {
 public:

  Aim_rescanISCSI_result() {
  }

  virtual ~Aim_rescanISCSI_result() throw() {}

  StorageException se;

  struct __isset {
    __isset() : se(false) {}
    bool se;
  } __isset;

  bool operator == (const Aim_rescanISCSI_result & rhs) const
  {
    if (!(se == rhs.se))
      return false;
    return true;
  }
  bool operator != (const Aim_rescanISCSI_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Aim_rescanISCSI_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class Aim_rescanISCSI_presult {
 public:


  virtual ~Aim_rescanISCSI_presult() throw() {}

  StorageException se;

  struct __isset {
    __isset() : se(false) {}
    bool se;
  } __isset;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

};

class AimClient : virtual public AimIf {
 public:
  AimClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) :
    piprot_(prot),
    poprot_(prot) {
    iprot_ = prot.get();
    oprot_ = prot.get();
  }
  AimClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) :
    piprot_(iprot),
    poprot_(oprot) {
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void checkRimpConfiguration();
  void send_checkRimpConfiguration();
  void recv_checkRimpConfiguration();
  int64_t getDiskFileSize(const std::string& virtualImageDatastorePath);
  void send_getDiskFileSize(const std::string& virtualImageDatastorePath);
  int64_t recv_getDiskFileSize();
  void getDatastores(std::vector<Datastore> & _return);
  void send_getDatastores();
  void recv_getDatastores(std::vector<Datastore> & _return);
  void getNetInterfaces(std::vector<NetInterface> & _return);
  void send_getNetInterfaces();
  void recv_getNetInterfaces(std::vector<NetInterface> & _return);
  void copyFromRepositoryToDatastore(const std::string& virtualImageRepositoryPath, const std::string& datastorePath, const std::string& virtualMachineUUID);
  void send_copyFromRepositoryToDatastore(const std::string& virtualImageRepositoryPath, const std::string& datastorePath, const std::string& virtualMachineUUID);
  void recv_copyFromRepositoryToDatastore();
  void deleteVirtualImageFromDatastore(const std::string& datastorePath, const std::string& virtualMachineUUID);
  void send_deleteVirtualImageFromDatastore(const std::string& datastorePath, const std::string& virtualMachineUUID);
  void recv_deleteVirtualImageFromDatastore();
  void copyFromDatastoreToRepository(const std::string& virtualMachineUUID, const std::string& snapshot, const std::string& destinationRepositoryPath, const std::string& sourceDatastorePath);
  void send_copyFromDatastoreToRepository(const std::string& virtualMachineUUID, const std::string& snapshot, const std::string& destinationRepositoryPath, const std::string& sourceDatastorePath);
  void recv_copyFromDatastoreToRepository();
  void createVLAN(const int32_t vlanTag, const std::string& vlanInterface, const std::string& bridgeInterface);
  void send_createVLAN(const int32_t vlanTag, const std::string& vlanInterface, const std::string& bridgeInterface);
  void recv_createVLAN();
  void deleteVLAN(const int32_t vlanTag, const std::string& vlanInterface, const std::string& bridgeInterface);
  void send_deleteVLAN(const int32_t vlanTag, const std::string& vlanInterface, const std::string& bridgeInterface);
  void recv_deleteVLAN();
  void checkVLANConfiguration();
  void send_checkVLANConfiguration();
  void recv_checkVLANConfiguration();
  void getInitiatorIQN(std::string& _return);
  void send_getInitiatorIQN();
  void recv_getInitiatorIQN(std::string& _return);
  void rescanISCSI(const std::vector<std::string> & targets);
  void send_rescanISCSI(const std::vector<std::string> & targets);
  void recv_rescanISCSI();
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class AimProcessor : virtual public ::apache::thrift::TProcessor {
 protected:
  boost::shared_ptr<AimIf> iface_;
  virtual bool process_fn(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, std::string& fname, int32_t seqid);
 private:
  std::map<std::string, void (AimProcessor::*)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*)> processMap_;
  void process_checkRimpConfiguration(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
  void process_getDiskFileSize(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
  void process_getDatastores(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
  void process_getNetInterfaces(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
  void process_copyFromRepositoryToDatastore(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
  void process_deleteVirtualImageFromDatastore(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
  void process_copyFromDatastoreToRepository(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
  void process_createVLAN(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
  void process_deleteVLAN(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
  void process_checkVLANConfiguration(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
  void process_getInitiatorIQN(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
  void process_rescanISCSI(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot);
 public:
  AimProcessor(boost::shared_ptr<AimIf> iface) :
    iface_(iface) {
    processMap_["checkRimpConfiguration"] = &AimProcessor::process_checkRimpConfiguration;
    processMap_["getDiskFileSize"] = &AimProcessor::process_getDiskFileSize;
    processMap_["getDatastores"] = &AimProcessor::process_getDatastores;
    processMap_["getNetInterfaces"] = &AimProcessor::process_getNetInterfaces;
    processMap_["copyFromRepositoryToDatastore"] = &AimProcessor::process_copyFromRepositoryToDatastore;
    processMap_["deleteVirtualImageFromDatastore"] = &AimProcessor::process_deleteVirtualImageFromDatastore;
    processMap_["copyFromDatastoreToRepository"] = &AimProcessor::process_copyFromDatastoreToRepository;
    processMap_["createVLAN"] = &AimProcessor::process_createVLAN;
    processMap_["deleteVLAN"] = &AimProcessor::process_deleteVLAN;
    processMap_["checkVLANConfiguration"] = &AimProcessor::process_checkVLANConfiguration;
    processMap_["getInitiatorIQN"] = &AimProcessor::process_getInitiatorIQN;
    processMap_["rescanISCSI"] = &AimProcessor::process_rescanISCSI;
  }

  virtual bool process(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot);
  virtual ~AimProcessor() {}
};

class AimMultiface : virtual public AimIf {
 public:
  AimMultiface(std::vector<boost::shared_ptr<AimIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~AimMultiface() {}
 protected:
  std::vector<boost::shared_ptr<AimIf> > ifaces_;
  AimMultiface() {}
  void add(boost::shared_ptr<AimIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void checkRimpConfiguration() {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      ifaces_[i]->checkRimpConfiguration();
    }
  }

  int64_t getDiskFileSize(const std::string& virtualImageDatastorePath) {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      if (i == sz - 1) {
        return ifaces_[i]->getDiskFileSize(virtualImageDatastorePath);
      } else {
        ifaces_[i]->getDiskFileSize(virtualImageDatastorePath);
      }
    }
  }

  void getDatastores(std::vector<Datastore> & _return) {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      if (i == sz - 1) {
        ifaces_[i]->getDatastores(_return);
        return;
      } else {
        ifaces_[i]->getDatastores(_return);
      }
    }
  }

  void getNetInterfaces(std::vector<NetInterface> & _return) {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      if (i == sz - 1) {
        ifaces_[i]->getNetInterfaces(_return);
        return;
      } else {
        ifaces_[i]->getNetInterfaces(_return);
      }
    }
  }

  void copyFromRepositoryToDatastore(const std::string& virtualImageRepositoryPath, const std::string& datastorePath, const std::string& virtualMachineUUID) {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      ifaces_[i]->copyFromRepositoryToDatastore(virtualImageRepositoryPath, datastorePath, virtualMachineUUID);
    }
  }

  void deleteVirtualImageFromDatastore(const std::string& datastorePath, const std::string& virtualMachineUUID) {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      ifaces_[i]->deleteVirtualImageFromDatastore(datastorePath, virtualMachineUUID);
    }
  }

  void copyFromDatastoreToRepository(const std::string& virtualMachineUUID, const std::string& snapshot, const std::string& destinationRepositoryPath, const std::string& sourceDatastorePath) {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      ifaces_[i]->copyFromDatastoreToRepository(virtualMachineUUID, snapshot, destinationRepositoryPath, sourceDatastorePath);
    }
  }

  void createVLAN(const int32_t vlanTag, const std::string& vlanInterface, const std::string& bridgeInterface) {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      ifaces_[i]->createVLAN(vlanTag, vlanInterface, bridgeInterface);
    }
  }

  void deleteVLAN(const int32_t vlanTag, const std::string& vlanInterface, const std::string& bridgeInterface) {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      ifaces_[i]->deleteVLAN(vlanTag, vlanInterface, bridgeInterface);
    }
  }

  void checkVLANConfiguration() {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      ifaces_[i]->checkVLANConfiguration();
    }
  }

  void getInitiatorIQN(std::string& _return) {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      if (i == sz - 1) {
        ifaces_[i]->getInitiatorIQN(_return);
        return;
      } else {
        ifaces_[i]->getInitiatorIQN(_return);
      }
    }
  }

  void rescanISCSI(const std::vector<std::string> & targets) {
    uint32_t sz = ifaces_.size();
    for (uint32_t i = 0; i < sz; ++i) {
      ifaces_[i]->rescanISCSI(targets);
    }
  }

};



#endif
