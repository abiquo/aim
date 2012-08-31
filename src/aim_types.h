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

#ifndef aim_TYPES_H
#define aim_TYPES_H

#include <Thrift.h>
#include <protocol/TProtocol.h>
#include <transport/TTransport.h>





class Datastore {
 public:

  static const char* ascii_fingerprint; // = "E2396C406CD75CE894E195C727905C26";
  static const uint8_t binary_fingerprint[16]; // = {0xE2,0x39,0x6C,0x40,0x6C,0xD7,0x5C,0xE8,0x94,0xE1,0x95,0xC7,0x27,0x90,0x5C,0x26};

  Datastore() : device(""), path(""), type(""), totalSize(0), usableSize(0) {
  }

  virtual ~Datastore() throw() {}

  std::string device;
  std::string path;
  std::string type;
  int64_t totalSize;
  int64_t usableSize;

  struct __isset {
    __isset() : device(false), path(false), type(false), totalSize(false), usableSize(false) {}
    bool device;
    bool path;
    bool type;
    bool totalSize;
    bool usableSize;
  } __isset;

  bool operator == (const Datastore & rhs) const
  {
    if (!(device == rhs.device))
      return false;
    if (!(path == rhs.path))
      return false;
    if (!(type == rhs.type))
      return false;
    if (!(totalSize == rhs.totalSize))
      return false;
    if (!(usableSize == rhs.usableSize))
      return false;
    return true;
  }
  bool operator != (const Datastore &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const Datastore & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class NetInterface {
 public:

  static const char* ascii_fingerprint; // = "AB879940BD15B6B25691265F7384B271";
  static const uint8_t binary_fingerprint[16]; // = {0xAB,0x87,0x99,0x40,0xBD,0x15,0xB6,0xB2,0x56,0x91,0x26,0x5F,0x73,0x84,0xB2,0x71};

  NetInterface() : name(""), address(""), physicalAddress("") {
  }

  virtual ~NetInterface() throw() {}

  std::string name;
  std::string address;
  std::string physicalAddress;

  struct __isset {
    __isset() : name(false), address(false), physicalAddress(false) {}
    bool name;
    bool address;
    bool physicalAddress;
  } __isset;

  bool operator == (const NetInterface & rhs) const
  {
    if (!(name == rhs.name))
      return false;
    if (!(address == rhs.address))
      return false;
    if (!(physicalAddress == rhs.physicalAddress))
      return false;
    return true;
  }
  bool operator != (const NetInterface &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const NetInterface & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class RimpException : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "EFB929595D312AC8F305D5A794CFEDA1";
  static const uint8_t binary_fingerprint[16]; // = {0xEF,0xB9,0x29,0x59,0x5D,0x31,0x2A,0xC8,0xF3,0x05,0xD5,0xA7,0x94,0xCF,0xED,0xA1};

  RimpException() : description("") {
  }

  virtual ~RimpException() throw() {}

  std::string description;

  struct __isset {
    __isset() : description(false) {}
    bool description;
  } __isset;

  bool operator == (const RimpException & rhs) const
  {
    if (!(description == rhs.description))
      return false;
    return true;
  }
  bool operator != (const RimpException &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const RimpException & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class VLanException : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "EFB929595D312AC8F305D5A794CFEDA1";
  static const uint8_t binary_fingerprint[16]; // = {0xEF,0xB9,0x29,0x59,0x5D,0x31,0x2A,0xC8,0xF3,0x05,0xD5,0xA7,0x94,0xCF,0xED,0xA1};

  VLanException() : description("") {
  }

  virtual ~VLanException() throw() {}

  std::string description;

  struct __isset {
    __isset() : description(false) {}
    bool description;
  } __isset;

  bool operator == (const VLanException & rhs) const
  {
    if (!(description == rhs.description))
      return false;
    return true;
  }
  bool operator != (const VLanException &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const VLanException & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};

class StorageException : public ::apache::thrift::TException {
 public:

  static const char* ascii_fingerprint; // = "EFB929595D312AC8F305D5A794CFEDA1";
  static const uint8_t binary_fingerprint[16]; // = {0xEF,0xB9,0x29,0x59,0x5D,0x31,0x2A,0xC8,0xF3,0x05,0xD5,0xA7,0x94,0xCF,0xED,0xA1};

  StorageException() : description("") {
  }

  virtual ~StorageException() throw() {}

  std::string description;

  struct __isset {
    __isset() : description(false) {}
    bool description;
  } __isset;

  bool operator == (const StorageException & rhs) const
  {
    if (!(description == rhs.description))
      return false;
    return true;
  }
  bool operator != (const StorageException &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const StorageException & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

};



#endif
