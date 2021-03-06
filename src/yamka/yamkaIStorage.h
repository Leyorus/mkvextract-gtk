// -*- Mode: c++; tab-width: 8; c-basic-offset: 2; indent-tabs-mode: nil -*-
// NOTE: the first line of this file sets up source code indentation rules
// for Emacs; it is also a hint to anyone modifying this file.

// Created   : Sat Apr 10 12:47:08 MDT 2010
// Copyright : Pavel Koshevoy
// License   : MIT -- http://www.opensource.org/licenses/mit-license.php

#ifndef YAMKA_ISTORAGE_H_
#define YAMKA_ISTORAGE_H_

// yamka includes:
#include <yamkaBytes.h>
#include <yamkaCrc32.h>
#include <yamkaStdInt.h>

// boost includes:
#include <boost/shared_ptr.hpp>


namespace Yamka
{
  
  //----------------------------------------------------------------
  // IStorage
  // 
  // NOTE: storage is assumed to be not thread safe, multiple threads
  // should not attempt to access the same storage simultaneously
  // and expect well-defined results.
  // 
  struct IStorage
  {
    virtual ~IStorage() {}
    
    // forward declaration:
    struct IReceipt;
    
    //----------------------------------------------------------------
    // IReceiptPtr
    // 
    typedef boost::shared_ptr<IReceipt> IReceiptPtr;
    
    //----------------------------------------------------------------
    // IReceipt
    // 
    struct IReceipt
    {
      virtual ~IReceipt() {}
      
      // NOTE: position interpretation is implementation specific:
      virtual uint64 position() const = 0;
      
      // return number of stored bytes for this receipt:
      virtual uint64 numBytes() const = 0;
      
      // set number of stored bytes for this receipt:
      virtual IReceipt & setNumBytes(uint64 numBytes) = 0;
      
      // increase number of stored bytes for this receipt:
      virtual IReceipt & add(uint64 numBytes) = 0;
      
      // return false if load/save fails:
      virtual bool save(const Bytes & data) = 0;
      virtual bool load(Bytes & data) = 0;
      
      // compute CRC-32 checksum on data covered by this receipt,
      // skip data in region specied by receiptSkip:
      virtual bool calcCrc32(Crc32 & computeCrc32,
                             const IReceiptPtr & receiptSkip) = 0;
      
      // increase number of stored bytes for this receipt
      // by adding number of stored bytes in a given receipt:
      IReceipt & operator += (const IReceiptPtr & receipt);
    };

    // If a storage implementation does not actually load/save
    // any data it should override this to return true.
    // A NULL storage implementation is useful for file layout optimization.
    virtual bool isNullStorage() const
    { return false; }
    
    // get a receipt for the current storage position:
    virtual IReceiptPtr receipt() const = 0;
    
    // NOTE: IStorage::save always appends at the end of the file:
    virtual IReceiptPtr save(const Bytes & data) = 0;
    
    // NOTE: IStorage::load always reads from current storage position:
    virtual IReceiptPtr load(Bytes & data) = 0;
    
    // NOTE: IStorage::skip always skips from current storage position:
    virtual IReceiptPtr skip(uint64 numBytes) = 0;
  };
  
  //----------------------------------------------------------------
  // NullStorage
  // 
  // Helper for estimating saved element positions without actually
  // saving any element data
  // 
  struct NullStorage : public IStorage
  {
    NullStorage(uint64 currentPostion = 0);
    
    // virtual:
    bool isNullStorage() const;
    
    // virtual:
    IReceiptPtr receipt() const;
    
    // virtual:
    IReceiptPtr save(const Bytes & data);
    
    // virtual: not supported for null-storage:
    IReceiptPtr load(Bytes & data);
    
    // virtual:
    IReceiptPtr skip(uint64 numBytes);
    
    //----------------------------------------------------------------
    // Receipt
    // 
    struct Receipt : public IReceipt
    {
      Receipt(uint64 addr);
      
      // virtual:
      uint64 position() const;
      
      // virtual:
      uint64 numBytes() const;
      
      // virtual:
      Receipt & setNumBytes(uint64 numBytes);
      
      // virtual:
      Receipt & add(uint64 numBytes);
      
      // virtual: not supported for null-storage:
      bool save(const Bytes & data);
      bool load(Bytes & data);
      bool calcCrc32(Crc32 & computeCrc32, const IReceiptPtr & receiptSkip);
      
    protected:
      uint64 addr_;
      uint64 numBytes_;
    };
    
    uint64 currentPosition_;
  };
  
}


#endif // YAMKA_ISTORAGE_H_
