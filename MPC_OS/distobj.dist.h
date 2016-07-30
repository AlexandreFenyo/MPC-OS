
// $Id: distobj.dist.h,v 1.1.1.1 1998/10/28 21:07:32 alex Exp $

#ifndef _DISTOBJ_H_
#define _DISTOBJ_H_

#include <signal.h>
#include <unistd.h>

extern "C" {
#include "mpcshare.h"
#include "../modules/driver.h"
}

////////////////////////////////////////////////////////////

extern void *allocator_tab[TYPE_IDS_LAST_TOKEN];

////////////////////////////////////////////////////////////

extern bool SEQ_INF_STRICT(seq_t, seq_t);
extern seq_t SEQ_MAX(seq_t, seq_t);

////////////////////////////////////////////////////////////

template <class T>
struct dist_obj {
  class PNode *pnode;
  class T *obj;

  dist_obj() : pnode(NULL), obj(NULL) {}
  dist_obj(pair<PNode *, T *> p) : pnode(p.first), obj(p.second) {}
  dist_obj(PNode *p) : pnode(p), obj(NULL) {}

  ~dist_obj() {}

  // The following declarations are not necessary because the operators
  // do not access private members of the structure (but the operators
  // MUST be defined).
  friend bool operator <  /*with egcs: <>*/ (const dist_obj<class T> &,
					     const dist_obj<class T> &);
  friend bool operator == /*with egcs: <>*/ (const dist_obj<class T> &,
					     const dist_obj<class T> &);

  struct dist_obj<T> & operator = (pair<PNode *, T *> rhs) {
    pnode = rhs.first;
    obj   = rhs.second;
    return *this;
  }

  struct dist_obj<T> & operator = (PNode *rhs) {
    pnode = rhs;
    obj   = NULL;
    return *this;
  }
};

// We make use of a lexicographic comparator.
template <class T>
bool operator < (const dist_obj<T> &X,
		 const dist_obj<T> &Y) {
  return ((X.pnode < Y.pnode) or
	  ((X.pnode == Y.pnode) and (X.obj < Y.obj)));
};

template <class T>
bool operator == (const dist_obj<T> &X,
		  const dist_obj<T> &Y) {
  return ((X.pnode == Y.pnode) and (X.obj == Y.obj));
};

////////////////////////////////////////////////////////////

template <class T>
class DistAllocator : public virtual MutexLock {
  list<class T *> elts_list; // Instances created locally.

public:
  u_int ref_cnt;

  DistAllocator(enum type_ids allocator_type) : ref_cnt(0) {
    allocator_tab[allocator_type] = (void *) this;
  };

  ~DistAllocator(void);

  // Declaration should be: class T *New(void);
  int New(void);

  // Declaration should be: void AsyncDelete(class T *);
  void AsyncDelete(int);

  // Declaration should be: int SyncDelete(class T *);
  int SyncDelete(int elt) {
    AsyncDelete(elt);
    return 0;
  }

  void Dump(void);
};

template <class T>
DistAllocator<T>::~DistAllocator(void) {
  Acquire();
  for (list<class T *>::iterator it = elts_list.begin();
       it != elts_list.end();
       it++) delete *it;
  Release();
};

// Declaration should be: class T *New(void);
template <class T>
int DistAllocator<T>::New(void) {
  class T *elt = new T;

  cerr << "DistAllocator<T>::New()\n";

  Acquire();
  elts_list.push_back(elt);
  Release();

  cerr << "OBJECT CREATED at location " << elt << "\n";
  return (int) elt;
};

// Declaration should be: void AsyncDelete(class T *);
template <class T>
void DistAllocator<T>::AsyncDelete(int elt) {
  list<class T *>::iterator it;
  class T *pelt = (class T *) elt;

  cerr << "DistAllocator<T>::AsyncDelete()\n";

  Acquire();
  for (it = elts_list.begin(); it != elts_list.end(); it++)
    if (*it == pelt) {
      delete *it;
      elts_list.erase(it);
      break;
    }
  if (it == elts_list.end()) throw ExceptFatal(__FILE__, __LINE__);
  Release();
};

template <class T>
void DistAllocator<T>::Dump(void) {
  cerr << "Dumping allocator :\n";

  Acquire();
  for (list<class T *>::iterator it = elts_list.begin();
       it != elts_list.end();
       it++)
    cerr << "Local instance at location " << *it << "\n";
  Release();

  cerr << "Dumping done.\n";
};

////////////////////////////////////////////////////////////

// Local or remote messages to an instance of DistAllocator<T>.
#define DECL_XREMOTE_MSG(AA, BB)                                     \
  void remoteMSG(PNode *, T *, void (DistAllocator<T>::*)(AA) BB);

#define XREMOTE_MSG(AA, BB, CC, DD)                                  \
  template <class T>                                                 \
  void TDistObject<T>::remoteMSG(PNode *pnode, T *p_t,               \
		 void (DistAllocator<T>::*xyz)(AA) BB) {             \
    if (pnode == NULL) {                                             \
      (((DistAllocator<T> *)                                         \
	allocator_tab[allocator_type])->*xyz)(DD);                   \
      return;                                                        \
    }                                                                \
    long long int lxyz;                                              \
    memcpy(&lxyz, &xyz, sizeof lxyz);                                \
    VMDAllocator.CurrentCommunicatorThread()->                       \
      RemoteCallMsg(pnode, allocator_type, (caddr_t) p_t, lxyz CC);  \
  };

// Local or remote RPC to an instance of DistAllocator<T>.
#define DECL_XREMOTE_RPC(AA, BB)                                     \
  int remoteRPC(PNode *, T *, int (DistAllocator<T>::*)(AA) BB);

#define XREMOTE_RPC(AA, BB, CC, DD)                                  \
  template <class T>                                                 \
  int TDistObject<T>::remoteRPC(PNode *pnode, T *p_t,                \
		int (DistAllocator<T>::*xyz)(AA) BB) {               \
    if (pnode == NULL)                                               \
      return (((DistAllocator<T> *)                                  \
	       allocator_tab[allocator_type])->*xyz)(DD);            \
    long long int lxyz;                                              \
    memcpy(&lxyz, &xyz, sizeof lxyz);                                \
    return VMDAllocator.CurrentCommunicatorThread()->                \
      RemoteCallRPC(pnode, allocator_type, (caddr_t) p_t, lxyz CC);  \
  };

// Local or remote messages to an instance of T.
#define DECL_YREMOTE_MSG(AA, BB)                                     \
  void remoteMSG(PNode *, T *, void (T::*)(AA) BB);

#define YREMOTE_MSG(AA, BB, CC, DD)                                  \
  template <class T>                                                 \
  void TDistObject<T>::remoteMSG(PNode *pnode, T *p_t,               \
		 void (T::*xyz)(AA) BB) {                            \
    if (pnode == NULL) {                                             \
      (p_t->*xyz)(DD);                                               \
      return;                                                        \
    }                                                                \
    long long int lxyz;                                              \
    memcpy(&lxyz, &xyz, sizeof lxyz);                                \
    VMDAllocator.CurrentCommunicatorThread()->                       \
      RemoteCallMsg(pnode, object_type, (caddr_t) p_t, lxyz CC);     \
  };

// Local or remote RPC to an instance of T.
#define DECL_YREMOTE_RPC(AA, BB)                                     \
  int remoteRPC(PNode *, T *, int (T::*)(AA) BB);

#define YREMOTE_RPC(AA, BB, CC, DD)                                  \
  template <class T>                                                 \
  int TDistObject<T>::remoteRPC(PNode *pnode, T *p_t,                \
		int (T::*xyz)(AA) BB) {                              \
    if (pnode == NULL) return (p_t->*xyz)(DD);                       \
    long long int lxyz;                                              \
    memcpy(&lxyz, &xyz, sizeof lxyz);                                \
    return VMDAllocator.CurrentCommunicatorThread()->                \
      RemoteCallRPC(pnode, object_type, (caddr_t) p_t, lxyz CC);     \
  };

template <class T>
struct reply_tag {
  int cluster;
  pnode_t node;
  class T *obj;
  caddr_t data;
  size_t size;
};

typedef struct _reply_msg {
  caddr_t addr;
  size_t size;
  char data[0];
} reply_msg_t;

template <class T>
class TDistObject : public virtual MutexLock {
  enum type_ids object_type, allocator_type;

public:
  u_int ref_cnt;

  // Constructor used to instanciate DistObject class.
  TDistObject(void);
  // Constructor used to instanciate derivated classes.
  TDistObject(enum type_ids obj, enum type_ids alloc);
  ~TDistObject(void);

// Remote messages to an instance of DistAllocator<T>.
  DECL_XREMOTE_MSG(void,);
  DECL_XREMOTE_MSG(int, SEPARATOR int p1);
  DECL_XREMOTE_MSG(int SEPARATOR int, SEPARATOR int p1 SEPARATOR int p2);
  DECL_XREMOTE_MSG(int SEPARATOR int SEPARATOR int,
		   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3);
  DECL_XREMOTE_MSG(int SEPARATOR int SEPARATOR int SEPARATOR int,
		   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3 SEPARATOR int p4);
  void remoteMSG(PNode *, T *,
		 void (DistAllocator<T>::*)(long_msg_t *), long_msg_t *);

// Remote RPC to an instance of DistAllocator<T>.
  DECL_XREMOTE_RPC(void,);
  DECL_XREMOTE_RPC(int, SEPARATOR int p1);
  DECL_XREMOTE_RPC(int SEPARATOR int, SEPARATOR int p1 SEPARATOR int p2);
  DECL_XREMOTE_RPC(int SEPARATOR int SEPARATOR int,
		   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3);
  DECL_XREMOTE_RPC(int SEPARATOR int SEPARATOR int SEPARATOR int,
		   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3 SEPARATOR int p4);
  int remoteRPC(PNode *, T *,
		int (DistAllocator<T>::*)(long_msg_t *), long_msg_t *);

// Remote messages to an instance of TDistObject<T>.
  DECL_YREMOTE_MSG(void,);
  DECL_YREMOTE_MSG(int, SEPARATOR int p1);
  DECL_YREMOTE_MSG(int SEPARATOR int, SEPARATOR int p1 SEPARATOR int p2);
  DECL_YREMOTE_MSG(int SEPARATOR int SEPARATOR int,
		   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3);
  DECL_YREMOTE_MSG(int SEPARATOR int SEPARATOR int SEPARATOR int,
		   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3 SEPARATOR int p4);
  void remoteMSG(PNode *, T *, void (T::*)(long_msg_t *), long_msg_t *);

// Remote RPC to an instance of TDistObject<T>.
  DECL_YREMOTE_RPC(void,);
  DECL_YREMOTE_RPC(int, SEPARATOR int p1);
  DECL_YREMOTE_RPC(int SEPARATOR int, SEPARATOR int p1 SEPARATOR int p2);
  DECL_YREMOTE_RPC(int SEPARATOR int SEPARATOR int,
		   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3);
  DECL_YREMOTE_RPC(int SEPARATOR int SEPARATOR int SEPARATOR int,
		   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3 SEPARATOR int p4);
  int remoteRPC(PNode *, T *, int (T::*)(long_msg_t *), long_msg_t *);

  list<dist_obj<T> > dist_obj_list; // Remote instances.
  class MutexLock dist_obj_list_m; // Protect access to dist_obj_list.

  // Method used by remote nodes to reply a long message (longer than an integer).
  reply_tag<T> MakeReplyTag(caddr_t, size_t);
  size_t GetReplySize(reply_tag<T>);
  void Reply(reply_tag<T>, caddr_t, size_t);
  int CommandReply(long_msg_t *);

private:
  dist_obj<T> exclusion_owner; // Central server managing the topology
                               // of this object.
  class MutexLock exclusion_owner_mutex; // Protect access to exclusion_owner.
  class ConditionalLockExpire exclusion_lock; // Sequencialize operations.

  bool IAmOwner(void);

  class MutexLock topology_mutex;

  list<dist_obj<T> > ActivateLockTopology(void);
  void ActivateUnLockTopology(list<dist_obj<T> > &);

public:
  int LockTopology(void);
  int UnLockTopology(void);

  ////////////////////////////////////////////////////////////
  // Creation of an object.

  // Ask the exclusion owner to create a new local or remote object.
  class T *RemoteNew(class PNode *);

  // Create a new local or remote object.
  // Declaration should be: class T *CommandRemoteNew(int, pnode_t);
  int CommandRemoteNew(int, int);

  // Method called at the end of the creation, for use by derivated classes.
  virtual void CommandRemoteNewEnd(int, pnode_t,  pair<PNode *, T *>);

  // Declaration should be: int InformNew(int, pnode_t, class T *);
  int InformNew(int, int, int);

  ////////////////////////////////////////////////////////////
  // Deletion of an object.

  // Ask the exclusion owner to delete a local or remote object.
  // WE MUST NOT delete neither the exclusion owner, nor us.
  void RemoteDelete(class PNode *, T *);

  // Delete a local or remote object.
  // Declaration should be:
  // bool CommandRemoteDelete(int cluster, pnode_t node, class T *elt);
  int CommandRemoteDelete(int, int, int);

  // Method called at the end of the deletion, for use by derivated classes.
  virtual void CommandRemoteDeleteEnd(int, pnode_t,  pair<PNode *, T *>);

  // Declaration should be: int InformDelete(int, pnode_t, class T *);
  virtual int InformDelete(int, int, int);

  ////////////////////////////////////////////////////////////
  // Migration of the exclusion owner.

  // Ask the exclusion owner to migrate.
  void Migrate(class PNode *, T *);

  // Migrate the exclusion owner.
  // Declaration should be:
  // bool CommandMigrate(int cluster, pnode_t node, class T *elt);
  int CommandMigrate(int, int, int);

  // Declaration should be:
  // int InformExclusionOwner(int, pnode_t, class T *);
  int InformExclusionOwner(int, int, int);

  ////////////////////////////////////////////////////////////

  virtual void Dump(void);
};

template <class T>
TDistObject<T>::TDistObject(void) :
  object_type(CLASS(DistObject)),
  allocator_type(DIST_ALLOC_CLASS(DistObject)), ref_cnt(0) {
    // The two following lines mean we are the exclusion owner.
    exclusion_owner.pnode = NULL;
    exclusion_owner.obj = (T *) this;
};

template <class T>
TDistObject<T>::TDistObject(enum type_ids obj, enum type_ids alloc) :
  object_type(obj), allocator_type(alloc), ref_cnt(0) {
    // The two following lines mean we are the exclusion owner.
    exclusion_owner.pnode = NULL;
    exclusion_owner.obj = (T *) this;
};

template <class T>
TDistObject<T>::~TDistObject(void) {
  // We must not use the network when finishing (the event
  // sources may have already been destroyed).
};

// Remote messages to an instance of DistAllocator<T>.
XREMOTE_MSG(void,,,);
XREMOTE_MSG(int, SEPARATOR int p1, SEPARATOR 1 SEPARATOR p1, p1);
XREMOTE_MSG(int SEPARATOR int, SEPARATOR int p1 SEPARATOR int p2,
	    SEPARATOR 2 SEPARATOR p1 SEPARATOR p2, p1 SEPARATOR p2);
XREMOTE_MSG(int SEPARATOR int SEPARATOR int,
	    SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3,
	    SEPARATOR 3 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3,
	    p1 SEPARATOR p2 SEPARATOR p3);
XREMOTE_MSG(int SEPARATOR int SEPARATOR int SEPARATOR int,
	    SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3 SEPARATOR int p4,
	    SEPARATOR 4 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3 SEPARATOR p4,
	    p1 SEPARATOR p2 SEPARATOR p3 SEPARATOR p4);

template <class T>
void TDistObject<T>::remoteMSG(PNode *pnode, T *p_t,
			       void (DistAllocator<T>::*xyz)(long_msg_t *),
			       long_msg_t *p1) {
  if (pnode == NULL) {
    (((DistAllocator<T> *)
      allocator_tab[allocator_type])->*xyz)(p1);
    return;
  }
  long long int lxyz;
  memcpy(&lxyz, &xyz, sizeof lxyz);
  VMDAllocator.CurrentCommunicatorThread()->
    RemoteCallMsg(pnode, allocator_type, (caddr_t) p_t, lxyz,
		  VARARG_VIRTLEN, (int) p1);
};

// Remote RPC to an instance of DistAllocator<T>.
XREMOTE_RPC(void,,,);
XREMOTE_RPC(int, SEPARATOR int p1, SEPARATOR 1 SEPARATOR p1, p1);
XREMOTE_RPC(int SEPARATOR int, SEPARATOR int p1 SEPARATOR int p2,
	    SEPARATOR 2 SEPARATOR p1 SEPARATOR p2, p1 SEPARATOR p2);
XREMOTE_RPC(int SEPARATOR int SEPARATOR int,
	    SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3,
	    SEPARATOR 3 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3,
	    p1 SEPARATOR p2 SEPARATOR p3);
XREMOTE_RPC(int SEPARATOR int SEPARATOR int SEPARATOR int,
	    SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3 SEPARATOR int p4,
	    SEPARATOR 4 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3 SEPARATOR p4,
	    p1 SEPARATOR p2 SEPARATOR p3 SEPARATOR p4);

template <class T>
int TDistObject<T>::remoteRPC(PNode *pnode, T *p_t,
			      int (DistAllocator<T>::*xyz)(long_msg_t *),
			      long_msg_t *p1) {
  if (pnode == NULL)
    return (((DistAllocator<T> *)
	     allocator_tab[allocator_type])->*xyz)(p1);
  long long int lxyz;
  memcpy(&lxyz, &xyz, sizeof lxyz);
  return VMDAllocator.CurrentCommunicatorThread()->
    RemoteCallRPC(pnode, allocator_type, (caddr_t) p_t, lxyz,
		  VARARG_VIRTLEN, (int) p1);
};

// Remote messages to an instance of TDistObject<T>.
YREMOTE_MSG(void,,,);
YREMOTE_MSG(int, SEPARATOR int p1, SEPARATOR 1 SEPARATOR p1, p1);
YREMOTE_MSG(int SEPARATOR int, SEPARATOR int p1 SEPARATOR int p2,
	    SEPARATOR 2 SEPARATOR p1 SEPARATOR p2, p1 SEPARATOR p2);
YREMOTE_MSG(int SEPARATOR int SEPARATOR int,
	    SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3,
	    SEPARATOR 3 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3,
	    p1 SEPARATOR p2 SEPARATOR p3);
YREMOTE_MSG(int SEPARATOR int SEPARATOR int SEPARATOR int,
	    SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3 SEPARATOR int p4,
	    SEPARATOR 4 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3 SEPARATOR p4,
	    p1 SEPARATOR p2 SEPARATOR p3 SEPARATOR p4);

template <class T>
void TDistObject<T>::remoteMSG(PNode *pnode, T *p_t,
			       void (T::*xyz)(long_msg_t *), long_msg_t *p1) {
  if (pnode == NULL) {
    (p_t->*xyz)(p1);
    return;
  }
  long long int lxyz;
  memcpy(&lxyz, &xyz, sizeof lxyz);
  VMDAllocator.CurrentCommunicatorThread()->
    RemoteCallMsg(pnode, object_type, (caddr_t) p_t, lxyz,
		  VARARG_VIRTLEN, (int) p1);
};

// Remote RPC to an instance of TDistObject<T>.
YREMOTE_RPC(void,,,);
YREMOTE_RPC(int, SEPARATOR int p1, SEPARATOR 1 SEPARATOR p1, p1);
YREMOTE_RPC(int SEPARATOR int, SEPARATOR int p1 SEPARATOR int p2,
	    SEPARATOR 2 SEPARATOR p1 SEPARATOR p2, p1 SEPARATOR p2);
YREMOTE_RPC(int SEPARATOR int SEPARATOR int,
	    SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3,
	    SEPARATOR 3 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3,
	    p1 SEPARATOR p2 SEPARATOR p3);
YREMOTE_RPC(int SEPARATOR int SEPARATOR int SEPARATOR int,
	    SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3 SEPARATOR int p4,
	    SEPARATOR 4 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3 SEPARATOR p4,
	    p1 SEPARATOR p2 SEPARATOR p3 SEPARATOR p4);

template <class T>
int TDistObject<T>::remoteRPC(PNode *pnode, T *p_t,
			      int (T::*xyz)(long_msg_t *), long_msg_t *p1) {
  if (pnode == NULL) return (p_t->*xyz)(p1);
  long long int lxyz;
  memcpy(&lxyz, &xyz, sizeof lxyz);
  return VMDAllocator.CurrentCommunicatorThread()->
    RemoteCallRPC(pnode, object_type, (caddr_t) p_t, lxyz,
		  VARARG_VIRTLEN, (int) p1);
};

template <class T>
reply_tag<T> TDistObject<T>::MakeReplyTag(caddr_t data, size_t size) {
  reply_tag<T> rt;

  rt.cluster = VMDAllocator.MyConfEntry()->cluster;
  rt.node    = VMDAllocator.MyConfEntry()->pnode;
  rt.obj     = (class T *) this;
  rt.data    = data;
  rt.size    = size;

  return rt;
};

template <class T>
size_t TDistObject<T>::GetReplySize(reply_tag<T> rt) {
  return rt.size;
};

template <class T>
void TDistObject<T>::Reply(reply_tag<T> rt, caddr_t data, size_t size) {
  dist_obj<T> d_obj;
  long_msg_t lmsg;
  reply_msg_t *rmsg;

  d_obj.pnode = VMDAllocator.FindPNode(rt.cluster, rt.node);
  d_obj.obj   = rt.obj;

  rmsg = (reply_msg_t *) new char[sizeof(reply_msg_t) + size];
  rmsg->addr = rt.data;
  rmsg->size = size;
  bcopy(data, rmsg->data, size);

  lmsg.data = (caddr_t) rmsg;
  lmsg.size = sizeof(reply_msg_t) + size;

  @d_obj,reply,<T>:CommandReply(&lmsg);

  delete (caddr_t) rmsg;
};

template <class T>
int TDistObject<T>::CommandReply(long_msg_t *lmsg) {
  reply_msg_t *rmsg;

  rmsg = (reply_msg_t *) lmsg->data;
  fprintf(stderr, "lmsg=%p, rsmg=%p rsmg->addr=%p rmsg->size=%d\n",
	  lmsg, rmsg, rmsg->addr, rmsg->size);
  bcopy(rmsg->data, rmsg->addr, rmsg->size);
  return 0;
};

template <class T>
bool TDistObject<T>::IAmOwner(void) {
  bool ret;

  exclusion_owner_mutex.Acquire();
  ret = (exclusion_owner.pnode == NULL) and (exclusion_owner.obj == this);
  exclusion_owner_mutex.Release();

  return ret;
};

template <class T>
list<dist_obj<T> > TDistObject<T>::ActivateLockTopology(void) {
  dist_obj_list_m.Acquire();
  for (list<dist_obj<T> >::iterator it = dist_obj_list.begin();
       it != dist_obj_list.end();
       it++) {
    pair<PNode *, T *> p((*it).pnode, (*it).obj);
    dist_obj_list_m.Release();
    // remoteRPC(p.first, p.second, &T::LockTopology);
    @p,reply,<T>:LockTopology();
    dist_obj_list_m.Acquire();
  }
  dist_obj_list_m.Release();

  // We are not in dist_obj_list.
  LockTopology();

  return dist_obj_list;
};

template <class T>
void TDistObject<T>::ActivateUnLockTopology(list<dist_obj<T> > &param) {
  // Only one thread at a time may use param, then we do not need to protect
  // against concurrency.
  // @*it:UnlockTopology();
  for (list<struct dist_obj<T> >::iterator it = param.begin();
       it != param.end();
       it++) {
    // remoteRPC((*it).pnode, (*it).obj, &T::UnLockTopology);
    @*it,reply,<T>:UnLockTopology();
  }

  // We are not in dist_obj_list.
  UnLockTopology();
};

template <class T>
int TDistObject<T>::LockTopology(void) {
  topology_mutex.Acquire();
  return 0;
};

template <class T>
int TDistObject<T>::UnLockTopology(void) {
  topology_mutex.Release();
  return 0;
};

////////////////////////////////////////////////////////////
// Creation of an object.

// Ask the exclusion owner to create a new local or remote object.

template <class T>
T *TDistObject<T>::RemoteNew(class PNode *pnode) {
  dist_obj<T> d;
  class T *ret;

  cerr << "RemoteNew()\n";

  do {
    exclusion_owner_mutex.Acquire();
    d = exclusion_owner;
    exclusion_owner_mutex.Release();

    // Ask the owner to do the job (we may be the owner).
    if (pnode != NULL) {
      // ret = remoteRPC(d.pnode, d.obj, &T::CommandRemoteNew,
      //       pnode->GetConfEntry().cluster,
      //       (int) pnode->GetConfEntry().pnode);
      ret = (class T *) @d,reply,<T>:CommandRemoteNew(pnode->GetConfEntry().cluster,
						      (int) pnode->GetConfEntry().pnode);
    }
    else {
      // ret = (class T *) remoteRPC(d.pnode, d.obj, &T::CommandRemoteNew,
      // VMDAllocator.MyConfEntry()->cluster,
      // (int) VMDAllocator.MyConfEntry()->pnode);
      ret = (class T *) @d,reply,<T>:CommandRemoteNew(VMDAllocator.MyConfEntry()->cluster, (int) VMDAllocator.MyConfEntry()->pnode);
    }
  } while (ret == NULL);

  return ret;
};

// Create a new local or remote object.
// Declaration should be: class T *CommandRemoteNew(int, pnode_t);

template <class T>
int TDistObject<T>::CommandRemoteNew(int cluster, int node) {
  dist_obj<T> d_obj;

  cerr << "CommandRemoteNew()\n";

  if (exclusion_lock.Acquire() == false) return NULL;

  list<dist_obj<T> > llock = ActivateLockTopology();

  d_obj.pnode = VMDAllocator.FindPNode(cluster, (pnode_t) node);

  // Create a new object.
  // d_obj.obj = (class T *)
  //   remoteRPC(d_obj.pnode, (T *) NULL, &DistAllocator<T>::New);
  d_obj.obj = (class T *) @d_obj.pnode,reply,<T>:New();

  // Inform the new object of exclusion owner.
  cerr << "Inform the new object of exclusion owner\n";

  if (IAmOwner()) {
    // remoteRPC(d_obj.pnode, d_obj.obj, &T::InformExclusionOwner,
    //           VMDAllocator.MyConfEntry()->cluster,
    //           (int) VMDAllocator.MyConfEntry()->pnode,
    //           (int) this);
    @d_obj,reply,<T>:InformExclusionOwner(VMDAllocator.MyConfEntry()->cluster,
					 (int) VMDAllocator.MyConfEntry()->pnode,
                                         (int) this);
  }
  else {
    dist_obj<T> d;

    exclusion_owner_mutex.Acquire();
    d = exclusion_owner;
    exclusion_owner_mutex.Release();

    if (d.pnode != NULL) {
      // remoteRPC(d_obj.pnode, d_obj.obj, &T::InformExclusionOwner,
      // d.pnode->GetConfEntry().cluster,
      // (int) d.pnode->GetConfEntry().pnode,
      // (int) d.obj);
      @d_obj,reply,<T>:InformExclusionOwner(d.pnode->GetConfEntry().cluster,
					    (int) d.pnode->GetConfEntry().pnode,
					    (int) d.obj);
    }
    else {
      // remoteRPC(d_obj.pnode, d_obj.obj, &T::InformExclusionOwner,
      // VMDAllocator.MyConfEntry()->cluster,
      // (int) VMDAllocator.MyConfEntry()->pnode,
      // (int) d.obj);
      @d_obj,reply,<T>:InformExclusionOwner(VMDAllocator.MyConfEntry()->cluster,
					    (int) VMDAllocator.MyConfEntry()->pnode,
					    (int) d.obj);
    }
  }

  dist_obj_list_m.Acquire();

  // Inform other objects about this new one.
  cerr << "inform other objects about this new one\n";

  for (list<dist_obj<T> >::iterator it = dist_obj_list.begin();
       it != dist_obj_list.end();
       it++) {
    pair<PNode *, T *> p((*it).pnode, (*it).obj);
    dist_obj_list_m.Release();
    // remoteRPC(p.first, p.second, &T::InformNew, cluster, (int) node,
    // (int) d_obj.obj);
    @p,reply,<T>:InformNew(cluster, (int) node, (int) d_obj.obj);
    dist_obj_list_m.Acquire();
  }

  // Inform the new object about old ones.
  cerr << "inform the new object about old ones\n";

  for (list<dist_obj<T> >::iterator it = dist_obj_list.begin();
       it != dist_obj_list.end();
       it++) {
    pair<PNode *, T *> p((*it).pnode, (*it).obj);
    dist_obj_list_m.Release();
    if (p.first != NULL) {
      // remoteRPC(d_obj.pnode, d_obj.obj, &T::InformNew,
      // p.first->GetConfEntry().cluster,
      // (int) p.first->GetConfEntry().pnode,
      // (int) p.second);
      @d_obj,reply,<T>:InformNew(p.first->GetConfEntry().cluster,
				 (int) p.first->GetConfEntry().pnode,
				 (int) p.second);
    }
    else {
      // remoteRPC(d_obj.pnode, d_obj.obj, &T::InformNew,
      // VMDAllocator.MyConfEntry()->cluster,
      // (int) VMDAllocator.MyConfEntry()->pnode,
      // (int) p.second);
      @d_obj,reply,<T>:InformNew(VMDAllocator.MyConfEntry()->cluster,
				 (int) VMDAllocator.MyConfEntry()->pnode,
				 (int) p.second);
    }
    dist_obj_list_m.Acquire();
  }

  // Inform the new object about us.
  cerr << "inform the new object about us\n";

  dist_obj_list_m.Release();
  // remoteRPC(d_obj.pnode, d_obj.obj, &T::InformNew,
  // VMDAllocator.MyConfEntry()->cluster,
  // (int) VMDAllocator.MyConfEntry()->pnode, (int) this);
  @d_obj,reply,<T>:InformNew(VMDAllocator.MyConfEntry()->cluster,
			     (int) VMDAllocator.MyConfEntry()->pnode, (int) this);
  dist_obj_list_m.Acquire();

  // Add the new object in the local list.
  dist_obj_list.push_back(d_obj);

  dist_obj_list_m.Release();

  // Allow a derivated class to take an action just after the creation.
  pair<class PNode *, class T *> p(d_obj.pnode, d_obj.obj);
  CommandRemoteNewEnd(cluster, node, p);

  ActivateUnLockTopology(llock);

  exclusion_lock.Release();

  return (int) d_obj.obj;
};

// Method called at the end of the creation, for use by derivated classes.
template <class T>
void TDistObject<T>::CommandRemoteNewEnd(int, pnode_t,  pair<PNode *, T *>) {};

// Declaration should be: int InformNew(int, pnode_t, class T *);
template <class T>
int TDistObject<T>::InformNew(int cluster, int node, int obj) {
  dist_obj<T> d_obj;

  cerr << "InformNew(cluster=" << cluster << ",node=" << node << ",obj=" << (class T *) obj << ")\n";

  d_obj.pnode = VMDAllocator.FindPNode(cluster, (pnode_t) node);
  d_obj.obj = (class T *) obj;

  dist_obj_list_m.Acquire();
  dist_obj_list.push_back(d_obj);
  dist_obj_list_m.Release();

  return 0; // Return something to be synchronous.
};

////////////////////////////////////////////////////////////
// Deletion of an object.

// Ask the exclusion owner to delete a local or remote object.
// WE MUST NOT delete neither the exclusion owner, nor us.

template <class T>
void TDistObject<T>::RemoteDelete(class PNode *pnode, T *elt) {
  dist_obj<T> d;
  bool ret;

  cerr << "TDistObject<T>::RemoteDelete()\n";

  do {
    exclusion_owner_mutex.Acquire();
    d = exclusion_owner;
    exclusion_owner_mutex.Release();

    // Check for the exclusion owner.
    if ((exclusion_owner.pnode == pnode) and (exclusion_owner.obj == elt))
      throw ExceptFatal(__FILE__, __LINE__);

    // Check for ourselves.
    if ((pnode == NULL) and (elt == this))
      throw ExceptFatal(__FILE__, __LINE__);

    if (pnode != NULL) {
      // ret = (bool) remoteRPC(d.pnode, d.obj, &T::CommandRemoteDelete,
      // pnode->GetConfEntry().cluster,
      // (int) pnode->GetConfEntry().pnode,
      // (int) elt);
      ret = (bool) @d,reply,<T>:CommandRemoteDelete(pnode->GetConfEntry().cluster,
						    (int) pnode->GetConfEntry().pnode,
						    (int) elt);
    }
    else {
      // ret = (bool) remoteRPC(d.pnode, d.obj, &T::CommandRemoteDelete,
      // VMDAllocator.MyConfEntry()->cluster,
      // (int) VMDAllocator.MyConfEntry()->pnode,
      // (int) elt);
      ret = (bool) @d,reply,<T>:CommandRemoteDelete(VMDAllocator.MyConfEntry()->cluster,
						    (int) VMDAllocator.MyConfEntry()->pnode,
						    (int) elt);
    }
  } while (ret == false);
}

// Delete a local or remote object.
// Declaration should be:
// bool CommandRemoteDelete(int cluster, pnode_t node, class T *elt);
template <class T>
int TDistObject<T>::CommandRemoteDelete(int cluster, int node, int elt) {
  cerr << "TDistObject<T>::CommandRemoteDelete(cluster="<<cluster<<",node="<<node<<",elt="<<(void *)elt<<")\n";

  if (exclusion_lock.Acquire() == false) return false;

  list<dist_obj<T> > llock = ActivateLockTopology();

  // Delete the object.
  // remoteRPC(VMDAllocator.FindPNode(cluster, (pnode_t) node), (T *) NULL,
  // &DistAllocator<T>::SyncDelete, (int) elt);
  class PNode *tmpnode = VMDAllocator.FindPNode(cluster, (pnode_t) node);
  @tmpnode,reply,<T>:SyncDelete((int) elt);

  // delete local reference to the object.
  InformDelete(cluster, node, elt);

  // Inform other objects to delete any reference to the one being deleted.
  dist_obj_list_m.Acquire();
  for (list<dist_obj<T> >::iterator it = dist_obj_list.begin();
       it != dist_obj_list.end();
       it++) {
    pair<PNode *, T *> p((*it).pnode, (*it).obj);
    dist_obj_list_m.Release();
    // remoteRPC(p.first, p.second, &T::InformDelete, cluster, node, elt);
    @p,reply,<T>:InformDelete(cluster, node, elt);
    dist_obj_list_m.Acquire();
  }
  dist_obj_list_m.Release();

  u_int s = llock.size();
  dist_obj<T> d;
  d.pnode = VMDAllocator.FindPNode(cluster, (pnode_t) node);
  d.obj = (class T *) elt;
  llock.remove(d);
  if (s - 1 != llock.size()) throw ExceptFatal(__FILE__, __LINE__);

  // Allow a derivated class to take an action just after the deletion.
  pair<class PNode *, class T *> p(d.pnode, d.obj);
  CommandRemoteDeleteEnd(cluster, node, p);

  ActivateUnLockTopology(llock);

  exclusion_lock.Release();

  return true;
};

// Method called at the end of the deletion, for use by derivated classes.
template <class T>
void TDistObject<T>::CommandRemoteDeleteEnd(int, pnode_t,  pair<PNode *, T *>) {};

// Declaration should be: int InformDelete(int, pnode_t, class T *);
template <class T>
int TDistObject<T>::InformDelete(int cluster, int node, int elt) {
  list<dist_obj<T> >::iterator it;

  cerr << "TDistObject<T>::InformDelete()\n";

  dist_obj_list_m.Acquire();
  for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
    if ((((*it).pnode == NULL) and
	 ((cluster == VMDAllocator.MyConfEntry()->cluster) and
	  (node == (int) VMDAllocator.MyConfEntry()->pnode)))
	or
	(((*it).pnode != NULL) and
	(((*it).pnode->GetConfEntry().cluster == cluster) and
	 ((*it).pnode->GetConfEntry().pnode == node) and
	 ((*it).obj == (class T *) elt)))) {
      dist_obj_list.erase(it);
    break;
    }
  if (it == dist_obj_list.end()) throw ExceptFatal(__FILE__, __LINE__);
  dist_obj_list_m.Release();

  return 0; // Return something to be synchronous.
};

////////////////////////////////////////////////////////////
// Migration of the exclusion owner.

// Ask the exclusion owner to migrate.
template <class T>
void TDistObject<T>::Migrate(class PNode *pnode, T *elt) {
  dist_obj<T> d;
  bool ret;

  cerr << "TDistObject<T>::Migrate()\n";

  do {
    exclusion_owner_mutex.Acquire();
    d = exclusion_owner;
    exclusion_owner_mutex.Release();

    if (pnode != NULL) {
      // ret = remoteRPC(d.pnode, d.obj, &T::CommandMigrate,
      // pnode->GetConfEntry().cluster,
      // (int) pnode->GetConfEntry().pnode,
      // (int) elt);
      ret = @d,reply,<T>:CommandMigrate(pnode->GetConfEntry().cluster,
					(int) pnode->GetConfEntry().pnode,
					(int) elt);
    }
    else {
      // ret = remoteRPC(d.pnode, d.obj, &T::CommandMigrate,
      // VMDAllocator.MyConfEntry()->cluster,
      // (int) VMDAllocator.MyConfEntry()->pnode,
      // (int) elt);
      ret = @d,reply,<T>:CommandMigrate(VMDAllocator.MyConfEntry()->cluster,
					(int) VMDAllocator.MyConfEntry()->pnode,
					(int) elt);
    }
  } while (ret == false);
};

// Migrate the exclusion owner.
// Declaration should be:
// bool CommandMigrate(int cluster, pnode_t node, class T *elt);
template <class T>
int TDistObject<T>::CommandMigrate(int cluster, int node, int elt) {
  cerr << "TDistObject<T>::CommandMigrate()\n";

  if (exclusion_lock.Acquire() == false) return false;
  // Inform objects about the new exclusion owner.
  dist_obj_list_m.Acquire();
  for (list<dist_obj<T> >::iterator it = dist_obj_list.begin();
       it != dist_obj_list.end();
       it++) {
    pair<PNode *, T *> p((*it).pnode, (*it).obj);
    dist_obj_list_m.Release();
    // remoteRPC(p.first, p.second, &T::InformExclusionOwner, cluster, node, elt);
    @p,reply,<T>:InformExclusionOwner(cluster, node, elt);
    dist_obj_list_m.Acquire();
  }
  dist_obj_list_m.Release();

  // Inform ourselves...
  InformExclusionOwner(cluster, node, elt);

  exclusion_lock.Release();

  // Make all local pending requests to retry.
  exclusion_lock.Expire();

  return true;
};

// Declaration should be:
// int InformExclusionOwner(int, pnode_t, class T *);
template <class T>
int TDistObject<T>::InformExclusionOwner(int cluster, int node, int obj) {
  cerr << "TDistObject<T>::InformExclusionOwner()\n";

  exclusion_owner_mutex.Acquire();
  exclusion_owner.pnode = VMDAllocator.FindPNode(cluster, (pnode_t) node);
  exclusion_owner.obj = (class T *) obj;
  exclusion_owner_mutex.Release();

  return 0; // Return something to be synchronous.
};

////////////////////////////////////////////////////////////

template <class T>
void TDistObject<T>::Dump(void) {
  cerr << "Dumping TTDistObject object " << this << " :\n";
  cerr << "- object_type : " << object_type << "\n"
       << "- allocator_type : " << allocator_type << "\n";

  for (list<dist_obj<T> >::iterator it = dist_obj_list.begin();
       it != dist_obj_list.end(); it++)
    if ((*it).pnode == NULL)
      cerr << "remote instance : local node\n"
	   << "                  obj = " << (void *) (*it).obj << "\n";
    else cerr << "remote instance : cluster = "
	      << (*it).pnode->GetConfEntry().cluster << "\n"
	      << "                  node = "
	      << (*it).pnode->GetConfEntry().pnode << "\n"
	      << "                  obj = " << (void *) (*it).obj << "\n";

  if (IAmOwner() == true) cerr << "exclusion owner : myself\n";
  else if (exclusion_owner.pnode != NULL)
    cerr << "exclusion owner : cluster = "
	 << exclusion_owner.pnode->GetConfEntry().cluster << "\n"
	 << "                  pnode = "
	 << exclusion_owner.pnode->GetConfEntry().pnode << "\n"
	 << "                  obj = " << (void *) exclusion_owner.obj << "\n";
  else cerr << "exclusion owner : local node\n"
	    << "                  obj = " << (void *) exclusion_owner.obj
	    << "\n";
};

////////////////////////////////////////////////////////////

class DistObject : public TDistObject<DistObject> {
  friend class DistAllocator<DistObject>;

protected:
  // Constructor and destructor are protected to assert that
  // the creation and deletion is only done by way of the allocator.

  DistObject(void);
  ~DistObject(void);

public:
};

extern DistAllocator<DistObject> DistObjectAllocator;
typedef DistAllocator<DistObject> DIST_ALLOC(DistObject);

////////////////////////////////////////////////////////////

// Distributed lock class, implementing an extension of the algorithm
// proposed by Chandy and Misra.
template <class T>
class TDistLock : public TDistObject<T> {
  enum _state { want, inside, outside };
  enum _state state;

  // Set of permissions we need to ask.
  set<dist_obj<class T>, less<dist_obj<class T> > > perm_set;
  class MutexLock perm_set_m; // Exclusion of accesses to the set.

  // Condition variable to wait for perm_set_size to become NULL.
  int perm_set_size;
  pthread_mutex_t perm_set_size_m;
  pthread_cond_t perm_set_size_c;

  // Map of permission usage (value of true means used).
  map<dist_obj<class T>, bool, less<dist_obj<class T> > > perm_map;
  class MutexLock perm_map_m; // Exclusion of accesses to the map.

  // To do list.
  list<dist_obj<class T> > to_do_list;
  class MutexLock to_do_list_m; // Exclusion of accesses to the list.

public:
  // Constructor used to instanciate DistLock.
  TDistLock(void);
  // Constructor used to instanciate derivated classes.
  TDistLock(enum type_ids obj, enum type_ids alloc);
  ~TDistLock(void);

  virtual class T *RemoteNew(class PNode *pnode) {
    return TDistObject<T>::RemoteNew(pnode);
  }

  void DistLock(void);
  void DistTryLock(void) {}
  void DistUnLock(void);

  // Declaration should be: void InformPerm(int, pnode_t, class T *);
  void InformPerm(int, int, int);

  // Declaration should be: void InformRequest(int, pnode_t, class T *);
  void InformRequest(int, int, int);

  ////////////////////////////////////////////////////////////
  // Section of methods linked with the topology.

  // Method called on an old object to inform about a new one.
  // Declaration should be: void InformNewPerm(int, pnode_t, class T *);
  int InformNewPerm(int, int, int);

  // Method called on a new object to inform about the old ones.
  int InformNewObjs(void);

  // Method called by the TDistObject class at the end of the creation.
  void CommandRemoteNewEnd(int, pnode_t, pair<PNode *, T *>);

  // Method called by the TDistObject class at the end of the deletion.
  void CommandRemoteDeleteEnd(int, pnode_t, pair<PNode *, T *>);

  ////////////////////////////////////////////////////////////

  virtual void Dump(void);
};

////////////////////////////////////////////////////////////

template <class T>
TDistLock<T>::TDistLock(void) :
  TDistObject<T>(CLASS(DistLock), DIST_ALLOC_CLASS(DistLock)), state(outside),
    perm_set_size(0) {
  int res;

  res = pthread_mutex_init(&perm_set_size_m, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_cond_init(&perm_set_size_c, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_init");
};

template <class T>
TDistLock<T>::TDistLock(enum type_ids obj, enum type_ids alloc) :
  TDistObject<T>(obj, alloc), state(outside), perm_set_size(0) {
  int res;

  res = pthread_mutex_init(&perm_set_size_m, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_cond_init(&perm_set_size_c, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_init");
};

template <class T>
TDistLock<T>::~TDistLock(void) {
  int res;

  res = pthread_mutex_destroy(&perm_set_size_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_destroy");

  res = pthread_cond_destroy(&perm_set_size_c);
  if (res != 0)
    throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_destroy");
};

template <class T>
void TDistLock<T>::DistLock(void) {
  int res;

  state = want;

  perm_set_m.Acquire();
  for (set<dist_obj<class T>, less<dist_obj<class T> > >::iterator it =
	 perm_set.begin(); it != perm_set.end(); it++) {
    pair<PNode *, T *> p((*it).pnode, (*it).obj);
    perm_set_m.Release();
    // remoteMSG(p.first, p.second, &T::InformRequest,
    // VMDAllocator.MyConfEntry()->cluster,
    // (int) VMDAllocator.MyConfEntry()->pnode, (int) this);
    @p,async,<T>:InformRequest(VMDAllocator.MyConfEntry()->cluster,
			       (int) VMDAllocator.MyConfEntry()->pnode,
			       (int) this);
    perm_set_m.Acquire();
  }
  perm_set_m.Release();

  res = pthread_mutex_lock(&perm_set_size_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

retry:
  if (perm_set_size != 0) {
    res = pthread_cond_wait(&perm_set_size_c, &perm_set_size_m);
    if ((res != 0) and (res != EINTR)) throw ExceptFatal(__FILE__, __LINE__);
    goto retry;
  }

  res = pthread_mutex_unlock(&perm_set_size_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  state = inside;

  perm_map_m.Acquire();
  for (map<dist_obj<class T>,
	 bool,
	 less<dist_obj<class T> > >::iterator it = perm_map.begin();
       it != perm_map.end();
       it++) {
    // perm_map[*it] = true;
    perm_map[(*it).first] = true;
  }
  perm_map_m.Release();
};

template <class T>
void TDistLock<T>::DistUnLock(void) {
  int res;

  state = outside;

  to_do_list_m.Acquire();
  for (list<dist_obj<class T> >::iterator it = to_do_list.begin();
       it != to_do_list.end(); it++) {
    pair<class PNode *, class T *> p((*it).pnode, (*it).obj);
    to_do_list_m.Release();
    // remoteMSG(p.first, p.second, &T::InformPerm,
    // VMDAllocator.MyConfEntry()->cluster,
    // (int) VMDAllocator.MyConfEntry()->pnode, (int) this);
    @p,async,<T>:InformPerm(VMDAllocator.MyConfEntry()->cluster,
			    (int) VMDAllocator.MyConfEntry()->pnode,
			    (int) this);
    to_do_list_m.Acquire();
  }
  to_do_list_m.Release();

  perm_set_m.Acquire();
  to_do_list_m.Acquire();

  // perm_set := to_do_list
  // perm_set.clear();
  perm_set.erase(perm_set.begin(), perm_set.end());
  
  for (list<dist_obj<class T> >::iterator it = to_do_list.begin();
       it != to_do_list.end(); it++)
    if (perm_set.insert(*it).second == false)
      throw ExceptFatal(__FILE__, __LINE__);

  res = pthread_mutex_lock(&perm_set_size_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  if ((perm_set_size = perm_set.size()) == 0) {
    res = pthread_cond_signal(&perm_set_size_c);
    if (res != 0) throw ExceptFatal(__FILE__, __LINE__);
  }

  res = pthread_mutex_unlock(&perm_set_size_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  to_do_list_m.Release();
  perm_set_m.Release();

  // to_do_list.clear();
  to_do_list.erase(to_do_list.begin(), to_do_list.end());
};

// Declaration should be: void InformPerm(int, pnode_t, class T *);
template <class T>
void TDistLock<T>::InformPerm(int cluster, int node, int obj) {
  dist_obj<T> d;
  int res;

  d.pnode = VMDAllocator.FindPNode(cluster, (pnode_t) node);
  d.obj = (class T *) obj;

  perm_set_m.Acquire();

  res = pthread_mutex_lock(&perm_set_size_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  if (perm_set.erase(d) != 1) throw ExceptFatal(__FILE__, __LINE__);

  if (--perm_set_size == 0) {
    res = pthread_cond_signal(&perm_set_size_c);
    if (res != 0) throw ExceptFatal(__FILE__, __LINE__);
  }

  res = pthread_mutex_unlock(&perm_set_size_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  perm_set_m.Release();

  perm_map_m.Acquire();
  perm_map[d] = false; // PB: verifier que ca marche !
  perm_map_m.Release();
};

// Declaration should be: void InformRequest(int, pnode_t, class T *);
template <class T>
void TDistLock<T>::InformRequest(int cluster, int node, int obj) {
  dist_obj<T> d;
  int c;
  bool priority;

  d.pnode = VMDAllocator.FindPNode(cluster, (pnode_t) node);
  d.obj = (class T *) obj;

  c = perm_set.count(d);
  if ((c > 1) or (c < 0)) throw ExceptFatal(__FILE__, __LINE__);

  if (c == 1) priority = true;
  else priority = ((state == inside) or
		   ((state == want) and (perm_map[d] == false)));

  if (priority == true) to_do_list.push_back(d);
  else {
    // remoteMSG(d.pnode, d.obj, &T::InformPerm,
    // VMDAllocator.MyConfEntry()->cluster,
    // (int) VMDAllocator.MyConfEntry()->pnode, (int) this);
    @d,async,<T>:InformPerm(VMDAllocator.MyConfEntry()->cluster,
			    (int) VMDAllocator.MyConfEntry()->pnode,
			    (int) this);

    perm_set_m.Acquire();
    if (perm_set.insert(d).second == false)
      throw ExceptFatal(__FILE__, __LINE__);
    perm_set_size++;
    perm_set_m.Release();

    if (state == want) {
      // remoteMSG(d.pnode, d.obj, &T::InformRequest,
      // VMDAllocator.MyConfEntry()->cluster,
      // (int) VMDAllocator.MyConfEntry()->pnode, (int) this);
      @d,async,<T>:InformRequest(VMDAllocator.MyConfEntry()->cluster,
				 (int) VMDAllocator.MyConfEntry()->pnode,
				 (int) this);
    }
  }
};

// Method called on an old object to inform about a new one.
// Declaration should be: void InformNewPerm(int, pnode_t, class T *);
template <class T>
int TDistLock<T>::InformNewPerm(int cluster, int node, int obj) {
  dist_obj<T> d;

  d.pnode = VMDAllocator.FindPNode(cluster, (pnode_t) node);
  d.obj = (class T *) obj;

  map<dist_obj<class T>,
    bool, less<dist_obj<class T> > >::value_type p(d, true);

  perm_map_m.Acquire();
  if (perm_map.insert(p).second == false)
    throw ExceptFatal(__FILE__, __LINE__);
  perm_map_m.Release();

  return 0;
};

// Method called on a new object to inform about the old ones.
template <class T>
int TDistLock<T>::InformNewObjs(void) {
  dist_obj_list_m.Acquire();
  for (list<dist_obj<T> >::iterator it = dist_obj_list.begin();
       it != dist_obj_list.end();
       it++) {
    dist_obj_list_m.Release();

    // Insert the permission in perm_set.
    perm_set_m.Acquire();
    if (perm_set.insert(*it).second == false)
      throw ExceptFatal(__FILE__, __LINE__);
    perm_set_size++;
    perm_set_m.Release();

    // Insert the permission in perm_map.
    map<dist_obj<class T>,
	 bool, less<dist_obj<class T> > >::value_type p(*it, true);
    perm_map_m.Acquire();
    if (perm_map.insert(p).second == false)
      throw ExceptFatal(__FILE__, __LINE__);
    perm_map_m.Release();

    dist_obj_list_m.Acquire();
  }
  dist_obj_list_m.Release();

  return 0;
};

// Method called by the TDistObject class at the end of the creation.
template <class T>
void TDistLock<T>::CommandRemoteNewEnd(int cluster, pnode_t node,
				       pair<PNode *, T *> d_obj) {
  TDistObject<T>::CommandRemoteNewEnd(cluster, node, d_obj);

  dist_obj_list_m.Acquire();
  for (list<dist_obj<T> >::iterator it = dist_obj_list.begin();
       it != dist_obj_list.end();
       it++) {
    pair<PNode *, T *> p((*it).pnode, (*it).obj);
    if (p == d_obj) continue;
    dist_obj_list_m.Release();
    // remoteRPC(p.first, p.second, &T::InformNewPerm,
    // cluster, (int) node, (int) d_obj.second);
    @p,reply,<T>:InformNewPerm(cluster, (int) node, (int) d_obj.second);
    dist_obj_list_m.Acquire();
  }
  dist_obj_list_m.Release();

  // We are not in dist_obj_list.
  InformNewPerm(cluster, (int) node, (int) d_obj.second);

  // remoteRPC(d_obj.first, d_obj.second, &T::InformNewObjs);
  @d_obj,reply,<T>:InformNewObjs();
};

// Method called by the TDistObject class at the end of the deletion.
template <class T>
void TDistLock<T>::CommandRemoteDeleteEnd(int cluster, pnode_t node,
					  pair<PNode *, T *> d_obj) {
  TDistObject<T>::CommandRemoteDeleteEnd(cluster, node, d_obj);
};

template <class T>
void TDistLock<T>::Dump(void) {
  TDistObject<T>::Dump();

  cerr << "----------\nDumping TDistLock part :\n";
  cerr << "state = ";
  switch (state) {
  case want:
    cerr << "want\n";
    break;

  case inside:
    cerr << "inside\n";
    break;

  case outside:
    cerr << "outside\n";
    break;

  default:
    throw ExceptFatal(__FILE__, __LINE__);
    /* NOTREACHED */
    break;
  }

  cerr << "perm_set_size = " << perm_set_size << "\n";

  cerr << "perm_set :\n";
  for (set<dist_obj<class T>, less<dist_obj<class T> > >::iterator it =
	 perm_set.begin(); it != perm_set.end(); it++) {
    if ((*it).pnode == NULL)
      cerr << "[local node|obj=" << (void *) (*it).obj << "]\n";
    else cerr << "[cluster="
	      << (*it).pnode->GetConfEntry().cluster
	      << "|node="
	      << (*it).pnode->GetConfEntry().pnode
	      << "|obj=" << (void *) (*it).obj << "]\n";
  }

  cerr << "perm_map :\n";
  for (map<dist_obj<class T>,
	 bool,
	 less<dist_obj<class T> > >::iterator it = perm_map.begin();
       it != perm_map.end();
       it++) {
    if ((*it).first.pnode == NULL)
      cerr << "perm_map[local node|obj=" <<
	(void *) (*it).first.obj << "]=";
    else cerr << "perm_map[cluster="
	      << (*it).first.pnode->GetConfEntry().cluster
	      << "|node="
	      << (*it).first.pnode->GetConfEntry().pnode
	      << "|obj=" << (void *) (*it).first.obj << "]=";

    switch ((*it).second) {
    case true:
      cerr << "true\n";
      break;

    case false:
      cerr << "false\n";
      break;

    default:
      throw ExceptFatal(__FILE__, __LINE__);
      /* NOTREACHED */
      break;
    }
  }

  cerr << "to_do_list :\n";
  for (list<dist_obj<class T> >::iterator it = to_do_list.begin();
       it != to_do_list.end(); it++) {
    if ((*it).pnode == NULL)
      cerr << "[local node|obj=" << (void *) (*it).obj << "]\n";
    else cerr << "[cluster="
	      << (*it).pnode->GetConfEntry().cluster
	      << "|node="
	      << (*it).pnode->GetConfEntry().pnode
	      << "|obj=" << (void *) (*it).obj << "]\n";
  }
};

////////////////////////////////////////////////////////////

class DistLock : public TDistLock<DistLock> {
  friend class DistAllocator<DistLock>;

protected:
  // Constructor and destructor are protected to assert that
  // the creation and deletion is only done by way of the allocator.

  DistLock(void);
  ~DistLock(void);

public:
};

extern DistAllocator<DistLock> DistLockAllocator;
typedef DistAllocator<DistLock> DIST_ALLOC(DistLock);

////////////////////////////////////////////////////////////

typedef struct _appclass_info {
  appclassname_t name;
  uid_t          uid;
} appclass_info_t;

#if 0
typedef int TID_t;

typedef struct _TID_info {
  TID_t             TID;
  pnode_t           pnode;
  pid_t             pid;
  appclassname_t    appclass;
  appsubclassname_t appsubclass;
} TID_info_t;  
#endif

typedef struct _shutdown1_reply {
  seq_t chan0_seqout;
  seq_t chan0_seqin;
  seq_t chan1_seqout;
  seq_t chan1_seqin;
} shutdown1_reply_t;

typedef struct _shutdown1_param {
  reply_tag<DistController> replytag;
  pnode_t node;
  channel_t chan0;
  channel_t chan1;
  int protocol;
} shutdown1_param_t;

typedef struct _shutdown2_param {
  pnode_t node;
  channel_t chan0;
  channel_t chan1;
  seq_t seq0out;
  seq_t seq0in;
  seq_t seq1out;
  seq_t seq1in;
  int protocol;
} shutdown2_param_t;

typedef struct _shutdown3_param {
  pnode_t node;
  channel_t chan0;
  channel_t chan1;
  int protocol;
} shutdown3_param_t;

template <class T>
class TDistController : public TDistLock<T> {
  map<appclassname_t, appclass_info_t, less<appclassname_t> >
    appclass_info_map;
  class MutexLock appclass_info_map_m;

#if 0
  map<TID_t, TID_info_t, less<TID_t> > TID_info_map;
  class MutexLock TID_info_map_m;
#endif

public:
  // Constructor used to instanciate DistController;
  TDistController(void);
  // Constructor used to instanciate derivated classes.
  TDistController(enum type_ids obj, enum type_ids alloc);
  ~TDistController(void) {}

  int GetAppClassInfo(appclassname_t, appclass_info_t *);

  appclassname_t CreateAppClass(uid_t);
  bool CreateAppClass(appclass_info_t);

  // Declaration should be: void InformNewAppClass(appclassname_t, uid_t)
  int InformNewAppClass(int, int);

  int GetChannel(appclassname_t, appsubclassname_t, int,  channel_t *, channel_t *);
  int CommandGetChannel(int, int, int, int);

  int CloseChannel(appclassname_t, appsubclassname_t);
  int CommandCloseChannel(int, int, int, int);

 public:
  int OpenChannel(pnode_t, pnode_t, channel_t, channel_t,
		  appclassname_t, int);
  int CommandOpenChannel(int, int, int, int);

  int ShutdownChannel_1stStep(pnode_t, pnode_t, channel_t, channel_t,
			      seq_t *, seq_t *, seq_t *, seq_t *, int);
  int CommandShutdownChannel_1stStep(long_msg_t *);

  int ShutdownChannel_2ndStep(pnode_t, pnode_t, channel_t, channel_t,
			      seq_t, seq_t, seq_t, seq_t, int);
  int CommandShutdownChannel_2ndStep(long_msg_t *);

  int ShutdownChannel_3rdStep(pnode_t, pnode_t, channel_t, channel_t, int);
  int CommandShutdownChannel_3rdStep(long_msg_t *);

  void SpawnTask(int, pnode_t, char *, appclassname_t, uid_t);
  void CommandSpawnTask(long_msg_t *);
};

template <class T>
TDistController<T>::TDistController(void) :
  TDistLock<T>(CLASS(DistController),
	       DIST_ALLOC_CLASS(DistController)) {
    VMDAllocator.controller = (class T *) this;
};

template <class T>
TDistController<T>::TDistController(enum type_ids obj, enum type_ids alloc) :
  TDistLock<T>(obj, alloc) {
    VMDAllocator.controller = (class T *) this;
};

template <class T>
int TDistController<T>::GetAppClassInfo(appclassname_t ac, appclass_info_t *ret_info) {
  appclass_info_map_m.Acquire();
  if (appclass_info_map.count(ac) == 1) {
    *ret_info = appclass_info_map[ac];
    appclass_info_map_m.Release();
    return 0;
  } else {
    appclass_info_map_m.Release();
    return -1;
  }
};

template <class T>
appclassname_t TDistController<T>::CreateAppClass(uid_t uid) {
  appclass_info_t ac_info;
  long i;

  // Mutual exclusion between threads on the local node.
  Acquire();
  // Mutual exclusion between threads on the whole system.
  DistLock();

  // Create a new class name.

  appclass_info_map_m.Acquire();

  for (i = ((uid == 0) ? MIN_SUSER_APPCLASS : MIN_USER_APPCLASS);
       i < 1 + ((uid == 0) ? MAX_SUSER_APPCLASS : MAX_USER_APPCLASS);
       i++)
    if (!appclass_info_map.count(i)) break;
  if (i == ((uid == 0) ? MAX_SUSER_APPCLASS : MAX_USER_APPCLASS)) {
    cerr << "appclass map full !\n";
    appclass_info_map_m.Release();
    Release();
    throw ExceptFileLine(__FILE__, __LINE__);
  }

  ac_info.name = i;
  ac_info.uid  = uid;

  map<appclassname_t, appclass_info_t, less<appclassname_t> >::value_type
    v(i, ac_info);

  if (appclass_info_map.insert(v).second == false) {
    appclass_info_map_m.Release();
    Release();
    throw ExceptFatal(__FILE__, __LINE__);
  }

  appclass_info_map_m.Release();

  // Inform the kernel about the new class name.
  if (VMDAllocator.device.ModuleLoaded() == true)
    VMDAllocator.device.SetAppClass(ac_info.name, ac_info.uid);

  // Inform others about this new class name.
  cerr << "inform other objects about this new class name\n";

  dist_obj_list_m.Acquire();

  for (list<dist_obj<T> >::iterator it = dist_obj_list.begin();
       it != dist_obj_list.end();
       it++) {
    pair<PNode *, T *> p((*it).pnode, (*it).obj);
    dist_obj_list_m.Release();
    @p,reply,<T>:InformNewAppClass(ac_info.name, ac_info.uid);
    dist_obj_list_m.Acquire();
  }

  dist_obj_list_m.Release();

  DistUnLock();
  Release();

  return ac_info.name;
};

template <class T>
bool TDistController<T>::CreateAppClass(appclass_info_t ac_info) {
  cerr << "Creating a new class name\n";

  if ((ac_info.uid == 0 and (ac_info.name > MAX_SUSER_APPCLASS or
			     ac_info.name < MIN_SUSER_APPCLASS)) or
      (ac_info.uid != 0 and (ac_info.name > MAX_USER_APPCLASS or
			     ac_info.name < MIN_USER_APPCLASS))) {
    cerr << "Invalid appclass name range.\n";
    return false;
  }

  appclass_info_map_m.Acquire();
  if (appclass_info_map.count(ac_info.name)) {
    cerr << "appclass already allocated\n";
    appclass_info_map_m.Release();
    return false;
  }
  appclass_info_map_m.Release();

  // Mutual exclusion between threads on the local node.
  Acquire();
  // Mutual exclusion between threads on the whole system.
  DistLock();

  appclass_info_map_m.Acquire();

  map<appclassname_t, appclass_info_t, less<appclassname_t> >::value_type
    v(ac_info.name, ac_info);

  if (appclass_info_map.insert(v).second == false) {
    appclass_info_map_m.Release();
    throw ExceptFatal(__FILE__, __LINE__);
  }

  appclass_info_map_m.Release();

  // Inform the kernel about the new class name.
  if (VMDAllocator.device.ModuleLoaded() == true)
    VMDAllocator.device.SetAppClass(ac_info.name, ac_info.uid);

  // Inform others about this new class name.
  cerr << "Inform other objects about this new class name\n";

  dist_obj_list_m.Acquire();

  for (list<dist_obj<T> >::iterator it = dist_obj_list.begin();
       it != dist_obj_list.end();
       it++) {
    pair<PNode *, T *> p((*it).pnode, (*it).obj);
    dist_obj_list_m.Release();
    @p,reply,<T>:InformNewAppClass(ac_info.name, ac_info.uid);
    dist_obj_list_m.Acquire();
  }

  dist_obj_list_m.Release();

  DistUnLock();
  Release();

  return true;
};

// Declaration should be: void InformNewAppClass(appclassname_t, uid_t)
template <class T>
int TDistController<T>::InformNewAppClass(int name, int uid) {
  appclass_info_t ac_info;

  cerr << "InformNewAppClass()\n";

  ac_info.name = (appclassname_t) name;
  ac_info.uid  = (uid_t) uid;

  // Inform the kernel about the new class name.
  if (VMDAllocator.device.ModuleLoaded() == true)
    VMDAllocator.device.SetAppClass(ac_info.name, ac_info.uid);

  appclass_info_map_m.Acquire();

  map<appclassname_t, appclass_info_t, less<appclassname_t> >::value_type
    v(ac_info.name, ac_info);
  if (appclass_info_map.insert(v).second == false) {
    appclass_info_map_m.Release();
    throw ExceptFatal(__FILE__, __LINE__);
  }

  appclass_info_map_m.Release();

  return 0;
};

template <class T>
int TDistController<T>::GetChannel(appclassname_t mainclass,
				   appsubclassname_t subclass,
				   int protocol,
				   channel_t *retchan_0, channel_t *retchan_1) {
  int ret;
  dist_obj<T> d;

  if (VMDAllocator.MyConfEntry()->cluster != subclass.controlled.prefnode_cluster)
    throw ExceptFileLine(__FILE__, __LINE__);

  if (VMDAllocator.MyConfEntry()->pnode < subclass.controlled.prefnode_pnode) {

    int par1 = mainclass;
    int par2 = ((int *) &subclass.controlled.value)[0];
    int par3 = ((int *) &subclass.controlled.value)[1];
    int par4 = (subclass.controlled.prefnode_pnode & 0xFFFF) | (protocol << 16);

    ret = CommandGetChannel(par1, par2, par3, par4);

    if (ret == ~0L) return ENOMEM;

    *retchan_0 = ret & 0xFFFF;
    *retchan_1 = ret>>16;

    return 0;

  } else {

    d.pnode = VMDAllocator.FindPNode(subclass.controlled.prefnode_cluster,
				     subclass.controlled.prefnode_pnode);
    dist_obj_list_m.Acquire();
    list<dist_obj<T> >::iterator it;
    for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
      if ((*it).pnode == d.pnode) {
	d.obj = (*it).obj;
	break;
      }
    dist_obj_list_m.Release();
    if (it == dist_obj_list.end()) throw ExceptFileLine(__FILE__, __LINE__);

    int par1 = mainclass;
    int par2 = ((int *) &subclass.controlled.value)[0];
    int par3 = ((int *) &subclass.controlled.value)[1];
    int par4 = (VMDAllocator.MyConfEntry()->pnode & 0xFFFF) | (protocol << 16);

    ret = @d,reply,<T>:CommandGetChannel(par1, par2, par3, par4);

    if (ret == ~0L) return ENOMEM;

    *retchan_0 = ret & 0xFFFF;
    *retchan_1 = ret>>16;

    return 0;

  }
};

template <class T>
int TDistController<T>::CommandGetChannel(int par1, int par2, int par3, int par4) {
  appclassname_t mainclass;
  primary_t primary;
  pnode_t pnode;
  int protocol;
  int retval;
  ChannelManager *cmp;

  mainclass = par1;
  ((int *) &primary)[0] = par2;
  ((int *) &primary)[1] = par3;
  pnode = par4 & 0xFFFF;
  protocol = par4 >> 16;

  channel_info_t ci;
  appclassdef_t  cd(mainclass, primary);

  VMDAllocator.channel_manager_map_m.Acquire();

  cmp = VMDAllocator.channel_manager_map[MAX(pnode,
					     VMDAllocator.MyConfEntry()->pnode)];
  VMDAllocator.channel_manager_map_m.Release();
  ci = cmp->GetChannel(cd, protocol);

  if (ci.classdef.first == ~0UL) retval = ~0L;
  else retval = ci.channel_pair_0 | (ci.channel_pair_1<<16);

  return retval;
};

template <class T>
int TDistController<T>::CloseChannel(appclassname_t mainclass,
				     appsubclassname_t subclass) {
  int ret;
  dist_obj<T> d;

  if (VMDAllocator.MyConfEntry()->cluster != subclass.controlled.prefnode_cluster)
    throw ExceptFileLine(__FILE__, __LINE__);

  if (VMDAllocator.MyConfEntry()->pnode < subclass.controlled.prefnode_pnode) {

    int par1 = mainclass;
    int par2 = ((int *) &subclass.controlled.value)[0];
    int par3 = ((int *) &subclass.controlled.value)[1];
    int par4 = subclass.controlled.prefnode_pnode;

    ret = CommandCloseChannel(par1, par2, par3, par4);
    return ret;

  } else {

    d.pnode = VMDAllocator.FindPNode(subclass.controlled.prefnode_cluster,
				     subclass.controlled.prefnode_pnode);
    dist_obj_list_m.Acquire();
    list<dist_obj<T> >::iterator it;
    for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
      if ((*it).pnode == d.pnode) {
	d.obj = (*it).obj;
	break;
      }
    dist_obj_list_m.Release();
    if (it == dist_obj_list.end()) throw ExceptFileLine(__FILE__, __LINE__);

    int par1 = mainclass;
    int par2 = ((int *) &subclass.controlled.value)[0];
    int par3 = ((int *) &subclass.controlled.value)[1];
    int par4 = VMDAllocator.MyConfEntry()->pnode;

    ret = @d,reply,<T>:CommandCloseChannel(par1, par2, par3, par4);
    return ret;
  }
};

template <class T>
int TDistController<T>::CommandCloseChannel(int par1, int par2, int par3, int par4) {
  appclassname_t mainclass;
  primary_t primary;
  pnode_t pnode;
  ChannelManager *cmp;

  mainclass = par1;
  ((int *) &primary)[0] = par2;
  ((int *) &primary)[1] = par3;
  pnode = par4;

  appclassdef_t cd(mainclass, primary);

  VMDAllocator.channel_manager_map_m.Acquire();
  cmp = VMDAllocator.channel_manager_map[MAX(pnode,
					     VMDAllocator.MyConfEntry()->pnode)];
  VMDAllocator.channel_manager_map_m.Release();
  return cmp->UnrefChannel(cd);
};

template <class T>
int TDistController<T>::OpenChannel(pnode_t node0, pnode_t node1,
				    channel_t chan0, channel_t chan1,
				    appclassname_t classname, int protocol) {
  int ret;
  dist_obj<T> d0, d1;
  int par0, par1, par2, par3;

  d0.pnode = VMDAllocator.FindPNode(VMDAllocator.MyConfEntry()->cluster, node0);
  d1.pnode = VMDAllocator.FindPNode(VMDAllocator.MyConfEntry()->cluster, node1);
  dist_obj_list_m.Acquire();
  list<dist_obj<T> >::iterator it;
  for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
    if ((*it).pnode == d0.pnode) {
      d0.obj = (*it).obj;
      break;
    }
  if (it == dist_obj_list.end()) d0.obj = (DistController *) this;
  for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
    if ((*it).pnode == d1.pnode) {
      d1.obj = (*it).obj;
      break;
    }
  if (it == dist_obj_list.end()) d1.obj = (DistController *) this;
  dist_obj_list_m.Release();

  par0 = (int) node1;
  par1 = (chan0 & 0xFFFFL) | ((chan1 & 0xFFFFL)<<16);
  par2 = (int) classname;
  par3 = protocol;

  ret = @d0,reply,<T>:CommandOpenChannel(par0, par1, par2, par3);
  if (ret) return ret;

  par0 = (int) node0;
  ret = @d1,reply,<T>:CommandOpenChannel(par0, par1, par2, par3);
  if (ret) return ret;

  return 0;
};

template <class T>
int TDistController<T>::CommandOpenChannel(int dest, int channels, int cn,
					   int protocol) {
  channel_t chan0, chan1;

  chan0 = channels & 0xFFFFL;
  chan1 = channels>>16;

  return VMDAllocator.device.OpenChannel((pnode_t) dest, chan0, chan1,
					 (appclassname_t) cn, protocol);
};

template <class T>
int TDistController<T>::ShutdownChannel_1stStep(pnode_t node0, pnode_t node1,
						channel_t chan0, channel_t chan1,
						seq_t *ret_seq0out, seq_t *ret_seq0in,
						seq_t *ret_seq1out, seq_t *ret_seq1in,
						int protocol) {
  dist_obj<T> d0, d1;
  shutdown1_reply_t seq_reply_0, seq_reply_1;
  shutdown1_param_t shutdown_param;
  long_msg_t lmsg;

  d0.pnode = VMDAllocator.FindPNode(VMDAllocator.MyConfEntry()->cluster, node0);
  d1.pnode = VMDAllocator.FindPNode(VMDAllocator.MyConfEntry()->cluster, node1);
  dist_obj_list_m.Acquire();
  list<dist_obj<T> >::iterator it;
  for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
    if ((*it).pnode == d0.pnode) {
      d0.obj = (*it).obj;
      break;
    }
  if (it == dist_obj_list.end()) d0.obj = (DistController *) this;
  for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
    if ((*it).pnode == d1.pnode) {
      d1.obj = (*it).obj;
      break;
    }
  if (it == dist_obj_list.end()) d1.obj = (DistController *) this;
  dist_obj_list_m.Release();

  lmsg.data = (caddr_t) &shutdown_param;
  lmsg.size = sizeof shutdown_param;

  shutdown_param.replytag = MakeReplyTag((caddr_t) &seq_reply_0, sizeof seq_reply_0);
  shutdown_param.node     = node1;
  shutdown_param.chan0    = chan0;
  shutdown_param.chan1    = chan1;
  shutdown_param.protocol = protocol;

  @d0,reply,<T>:CommandShutdownChannel_1stStep(&lmsg);

  shutdown_param.replytag = MakeReplyTag((caddr_t) &seq_reply_1, sizeof seq_reply_1);
  shutdown_param.node     = node0;

  @d1,reply,<T>:CommandShutdownChannel_1stStep(&lmsg);

  *ret_seq0out = SEQ_MAX(seq_reply_0.chan0_seqout, seq_reply_1.chan0_seqin);
  *ret_seq0in  = SEQ_MAX(seq_reply_0.chan0_seqin,  seq_reply_1.chan0_seqout);
  *ret_seq1out = SEQ_MAX(seq_reply_0.chan1_seqout, seq_reply_1.chan1_seqin);
  *ret_seq1in  = SEQ_MAX(seq_reply_0.chan1_seqin,  seq_reply_1.chan1_seqout);

  return 0;
};

template <class T>
int TDistController<T>::CommandShutdownChannel_1stStep(long_msg_t *lmsg) {
  shutdown1_param_t *shutdown_param;
  shutdown1_reply_t seq_reply;
  int res;

  shutdown_param = (shutdown1_param_t *) lmsg->data;

  res = VMDAllocator.device.ShutdownChannel_1stStep(shutdown_param->node,
						    shutdown_param->chan0,
						    shutdown_param->chan1,
						    &seq_reply.chan0_seqout,
						    &seq_reply.chan0_seqin,
						    &seq_reply.chan1_seqout,
						    &seq_reply.chan1_seqin,
						    shutdown_param->protocol);

  Reply(shutdown_param->replytag, (caddr_t) &seq_reply, sizeof seq_reply);

  return res;
};

template <class T>
int TDistController<T>::ShutdownChannel_2ndStep(pnode_t node0, pnode_t node1,
						channel_t chan0, channel_t chan1,
						seq_t seq0out, seq_t seq0in,
						seq_t seq1out, seq_t seq1in,
						int protocol) {
  dist_obj<T> d0, d1;
  shutdown2_param_t shutdown_param;
  long_msg_t lmsg;
  int res;

  d0.pnode = VMDAllocator.FindPNode(VMDAllocator.MyConfEntry()->cluster, node0);
  d1.pnode = VMDAllocator.FindPNode(VMDAllocator.MyConfEntry()->cluster, node1);
  dist_obj_list_m.Acquire();
  list<dist_obj<T> >::iterator it;
  for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
    if ((*it).pnode == d0.pnode) {
      d0.obj = (*it).obj;
      break;
    }
  if (it == dist_obj_list.end()) d0.obj = (DistController *) this;
  for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
    if ((*it).pnode == d1.pnode) {
      d1.obj = (*it).obj;
      break;
    }
  if (it == dist_obj_list.end()) d1.obj = (DistController *) this;
  dist_obj_list_m.Release();

  lmsg.data = (caddr_t) &shutdown_param;
  lmsg.size = sizeof shutdown_param;

  shutdown_param.node     = node1;
  shutdown_param.chan0    = chan0;
  shutdown_param.chan1    = chan1;
  shutdown_param.seq0out  = seq0out;
  shutdown_param.seq0in   = seq0in;
  shutdown_param.seq1out  = seq1out;
  shutdown_param.seq1in   = seq1in;
  shutdown_param.protocol = protocol;
  res = @d0,reply,<T>:CommandShutdownChannel_2ndStep(&lmsg);
  if (res) return res;

  shutdown_param.node     = node0;
  shutdown_param.seq0out  = seq0in;
  shutdown_param.seq0in   = seq0out;
  shutdown_param.seq1out  = seq1in;
  shutdown_param.seq1in   = seq1out;
  res = @d1,reply,<T>:CommandShutdownChannel_2ndStep(&lmsg);
  if (res) return res;

  return 0;
};

template <class T>
int TDistController<T>::CommandShutdownChannel_2ndStep(long_msg_t *lmsg) {
  shutdown2_param_t *shutdown_param;
  int res;

  shutdown_param = (shutdown2_param_t *) lmsg->data;

  res = VMDAllocator.device.ShutdownChannel_2ndStep(shutdown_param->node,
						    shutdown_param->chan0,
						    shutdown_param->chan1,
						    shutdown_param->seq0out,
						    shutdown_param->seq0in,
						    shutdown_param->seq1out,
						    shutdown_param->seq1in,
						    shutdown_param->protocol);
  return res;
};

template <class T>
int TDistController<T>::ShutdownChannel_3rdStep(pnode_t node0, pnode_t node1,
						channel_t chan0, channel_t chan1,
						int protocol) {
  dist_obj<T> d0, d1;
  shutdown3_param_t shutdown_param;
  long_msg_t lmsg;
  int res;

  d0.pnode = VMDAllocator.FindPNode(VMDAllocator.MyConfEntry()->cluster, node0);
  d1.pnode = VMDAllocator.FindPNode(VMDAllocator.MyConfEntry()->cluster, node1);
  dist_obj_list_m.Acquire();
  list<dist_obj<T> >::iterator it;
  for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
    if ((*it).pnode == d0.pnode) {
      d0.obj = (*it).obj;
      break;
    }
  if (it == dist_obj_list.end()) d0.obj = (DistController *) this;
  for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
    if ((*it).pnode == d1.pnode) {
      d1.obj = (*it).obj;
      break;
    }
  if (it == dist_obj_list.end()) d1.obj = (DistController *) this;
  dist_obj_list_m.Release();

  lmsg.data = (caddr_t) &shutdown_param;
  lmsg.size = sizeof shutdown_param;

  shutdown_param.node     = node1;
  shutdown_param.chan0    = chan0;
  shutdown_param.chan1    = chan1;
  shutdown_param.protocol = protocol;
  res = @d0,reply,<T>:CommandShutdownChannel_3rdStep(&lmsg);

  if (res == EAGAIN) return EAGAIN;
  if (res != ENOTCONN and res != ENOENT and res) return res;

  shutdown_param.node     = node0;
  res = @d1,reply,<T>:CommandShutdownChannel_3rdStep(&lmsg);

  if (res == EAGAIN) return EAGAIN;
  if (res != ENOTCONN and res != ENOENT and res) return res;

  return 0;
};

template <class T>
int TDistController<T>::CommandShutdownChannel_3rdStep(long_msg_t *lmsg) {
  shutdown3_param_t *shutdown_param;
  int res;

  shutdown_param = (shutdown3_param_t *) lmsg->data;

  res = VMDAllocator.device.ShutdownChannel_3rdStep(shutdown_param->node,
						    shutdown_param->chan0,
						    shutdown_param->chan1,
						    shutdown_param->protocol);
  return res;
};

typedef struct _spawn_info {
  appclassname_t mainclass;
  uid_t uid;
  char cmdline[CMDLINE_SIZE];
} spawn_info_t;

template <class T>
void TDistController<T>::SpawnTask(int cluster, pnode_t pnode,
				   char *cmdline, appclassname_t cn,
				   uid_t uid) {
  dist_obj<T> d;
  long_msg_t lmsg;
  spawn_info_t spi;

  spi.mainclass = cn;
  spi.uid = uid;
  strcpy(spi.cmdline, cmdline);

  lmsg.size = sizeof spi;
  lmsg.data = (caddr_t) &spi;

  d.pnode = VMDAllocator.FindPNode(cluster, pnode);
  dist_obj_list_m.Acquire();
  list<dist_obj<T> >::iterator it;
  for (it = dist_obj_list.begin(); it != dist_obj_list.end(); it++)
    if ((*it).pnode == d.pnode) {
      d.obj = (*it).obj;
      break;
    }
  dist_obj_list_m.Release();
  if (it == dist_obj_list.end()) throw ExceptFileLine(__FILE__, __LINE__);

  @d,async,<T>:CommandSpawnTask(&lmsg);
};

template <class T>
void TDistController<T>::CommandSpawnTask(long_msg_t *lmsg) {
#if 0
  pid_t pid;
  int oldmask;
  char envvar[64];
#endif

  fprintf(stderr, "CommandSpawnTask(task='%s')\n",
	  ((spawn_info_t *) lmsg->data)->cmdline);

  VMDAllocator.SpawnTask(((spawn_info_t *) lmsg->data)->cmdline,
			 ((spawn_info_t *) lmsg->data)->mainclass,
			 ((spawn_info_t *) lmsg->data)->uid);

#if 0
  // This code does not work because of the buggy support
  // for fork() in the FreeBSD pthreads implementation.

  // We use the obsolete sigsetmask interface instead of sigprocmask to bypass the
  // pthread stub for this syscall : we really want to mask EVERY signals.
  oldmask = sigsetmask(~0);
  if (oldmask < 0)
    throw ExceptErrno(__FILE__, __LINE__, "sigsetmask()");

  if (!(pid = fork())) {
    // Child.
    sprintf(envvar, "MAINCLASS=%#lx", ((spawn_info_t *) lmsg->data)->mainclass);
    putenv(envvar);
    system(((spawn_info_t *) lmsg->data)->cmdline);
    _exit(0);
  }
  // Parent.
  oldmask = sigsetmask(oldmask);
  if (oldmask < 0)
    throw ExceptErrno(__FILE__, __LINE__, "sigsetmask()");
#endif
};

////////////////////////////////////////////////////////////

class DistController : public TDistController<DistController> {
  friend class DistAllocator<DistController>;

protected:
  // Constructors and destructor are protected to assert that
  // the creation and deletion is only done by way of the allocator.

  // Constructor used to instanciate DistVMSpace class.
  DistController(void);
  ~DistController(void);

 public:
};

extern DistAllocator<DistController> DistControllerAllocator;
typedef DistAllocator<DistController> DIST_ALLOC(DistController);

////////////////////////////////////////////////////////////

class DistVMSpace : public TDistLock<DistVMSpace> {
  friend class DistAllocator<DistVMSpace>;

protected:
  // Constructors and destructor are protected to assert that
  // the creation and deletion is only done by way of the allocator.

  // Constructor used to instanciate DistVMSpace class.
  DistVMSpace(void);
  ~DistVMSpace(void);

public:
};

extern DistAllocator<DistVMSpace> DistVMSpaceAllocator;
typedef DistAllocator<DistVMSpace> DIST_ALLOC(DistVMSpace);


#endif

