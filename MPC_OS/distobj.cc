
// $Id: distobj.cc,v 1.3 2000/03/08 21:45:22 alex Exp $

#include <stdio.h>
#include <fcntl.h>

#include <new>
#include <cerrno>

// -frtti doesn't work well...
// #include <g++/typeinfo>
#include <stdexcept>
#include <exception>

#include <g++/iostream.h>
#include <g++/fstream.h>

#include <g++/stdiostream.h>

#include <g++/list>
#include <g++/deque>

#include <g++/set>
#include <g++/map>
#include <g++/functional>

#include <g++/pair.h>

#include <string.h>

#define _G_NO_EXTERN_TEMPLATES
#include <g++/string>

#include <pthread.h>

#include <unistd.h>

#include <g++/minmax.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <sys/stat.h>
#include <signal.h>

#include <sys/time.h>

#include "vmd.h"
#include "typeid.h"

#include "distobj.h"

////////////////////////////////////////////////////////////

void *allocator_tab[TYPE_IDS_LAST_TOKEN];

////////////////////////////////////////////////////////////

DistAllocator<class DistObject>
  DistObjectAllocator(DIST_ALLOC_CLASS(DistObject));

DistAllocator<class DistLock>
  DistLockAllocator(DIST_ALLOC_CLASS(DistLock));

DistAllocator<class DistVMSpace>
  DistVMSpaceAllocator(DIST_ALLOC_CLASS(DistVMSpace));

DistAllocator<class DistController>
  DistControllerAllocator(DIST_ALLOC_CLASS(DistController));

////////////////////////////////////////////////////////////

DistObject::DistObject(void) {}
DistObject::~DistObject(void) {}

////////////////////////////////////////////////////////////

DistLock::DistLock(void) {}
DistLock::~DistLock(void) {}

////////////////////////////////////////////////////////////

DistController::DistController(void) {
  VMDAllocator.controller = this;
}
DistController::~DistController(void) {}

////////////////////////////////////////////////////////////

DistVMSpace::DistVMSpace(void) :
  TDistLock<DistVMSpace>(CLASS(DistVMSpace),
			 DIST_ALLOC_CLASS(DistVMSpace)) {}

DistVMSpace::~DistVMSpace(void) {}

////////////////////////////////////////////////////////////

// Check if x < y

// Those definitions must be the same as in modules/data.h
#define PENDING_SEND_SIZE 200
#define PENDING_RECEIVE_SIZE 200

bool SEQ_INF_STRICT(seq_t x, seq_t y)
{
  if (x == y) return false;
  if (x > y) return !SEQ_INF_STRICT(y, x);

  // Here, we can assume that x < y

  if (x <= MAX(PENDING_SEND_SIZE, PENDING_RECEIVE_SIZE) and
      y >= 0x10000L - MAX(PENDING_SEND_SIZE, PENDING_RECEIVE_SIZE)) {
    if (x + 0x10000L - y > 16 + MAX(PENDING_SEND_SIZE, PENDING_RECEIVE_SIZE))
      // Should not happen : delta between sequences too wide.
      throw ExceptFileLine(__FILE__, __LINE__);

    return false;
  }

  if (y - x > 16 + MAX(PENDING_SEND_SIZE, PENDING_RECEIVE_SIZE))
    // Should not happen : delta between sequences too wide.
      throw ExceptFileLine(__FILE__, __LINE__);

  return true;
}

seq_t SEQ_MAX(seq_t x, seq_t y)
{
  return SEQ_INF_STRICT(x, y) ? y : x;
}
