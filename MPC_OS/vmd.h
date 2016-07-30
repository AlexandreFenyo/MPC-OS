
// TOUS LES ACCESS concurrents a une meme instance d un objet C++
// doivent etre proteges (cf les News) - par ex tous les access
// a un iostream

// voir pthread_exit pour trouver comment implementer pthread_cancel()

// $Id: vmd.h,v 1.1.1.1 1998/10/28 21:07:31 alex Exp $

#ifndef _VMD_H_
#define _VMD_H_

#include <g++/list>
#include <pthread.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

extern "C" {
#include "mpcshare.h"
}

class Lock;
class MutexLock;
class ConditionalLock;
class ConditionalLockExpire;

class VMD;
class EventDriver;
class EventSource;
class FDEventSource;
class SLREventSource;
class PNode;
class Thread;
class CommunicatorThread;
class ThreadInitiator;
class Server;
class EventInitiator;
class CommandLine;
class Initializer;
class Finalizer;
class ChannelManager;
class DeviceInterface;

class ExceptFileLine;
class ExceptStream;
class ExceptSpecificData;
class ExceptErrno;
class ExceptMutex;
class ExceptFileDesc;
class ExceptFatal;
class ExceptFatalMsg;

#define N_EVENT_DRIVER_THREADS 2
#define N_EVENT_INITIATOR_THREADS 2
#define BACKLOG 5
#define CONF_FILE "../config_mpc/example.cluster"
#define OPT_STRING_SIZE 160
#define MSQ_BASEKEY 100

#ifndef MSGMAX
#define MSGMAX (8 * 2048)
#endif

typedef struct _generic_msg {
  long mtype;
  char mtext[MSGMAX];
} generic_msg_t;

////////////////////////////////////////////////////////////
// Add classes that export methods over the network here.

// Define an handler for class X.
#define CLASS(X) TYPE_ ## X
// Define a class name for DistAllocator<X>.
#define DIST_ALLOC(X) template_DistAllocator_ ## X
// Define an handler for class DistAllocator<X>.
#define DIST_ALLOC_CLASS(X) TYPE_template_DistAllocator_ ## X

enum type_ids { // Handle class VMD.
		CLASS(VMD),
                // Handle class DistObject.
                CLASS(DistObject),
                // Handle class DistAllocator<DistObject>.
                DIST_ALLOC_CLASS(DistObject),
		// Handle class DistController.
		CLASS(DistController),
		// Handle class DistAllocator<DistController>.
		DIST_ALLOC_CLASS(DistController),
                // Handle class DistVMSpace.
		CLASS(DistVMSpace),
		// Handle class DistAllocator<DistVMSpace>.
		DIST_ALLOC_CLASS(DistVMSpace),
                // Handle class DistLock.
                CLASS(DistLock),
                // Handle class DistAllocator<DistLock>.
                DIST_ALLOC_CLASS(DistLock),
		// Necessary to handle the declaration of allocator_tab.

		// Insert new classes here.

                TYPE_IDS_LAST_TOKEN };

#define HANDLE_CLASS_BLOCK                  \
  HANDLE_CLASS(VMD);                        \
  HANDLE_CLASS(DistObject);                 \
  HANDLE_CLASS(DIST_ALLOC(DistObject));     \
  HANDLE_CLASS(DistController);             \
  HANDLE_CLASS(DIST_ALLOC(DistController)); \
  HANDLE_CLASS(DistVMSpace);                \
  HANDLE_CLASS(DIST_ALLOC(DistVMSpace));    \
  HANDLE_CLASS(DistLock);                   \
  /* Insert new classes here. */            \
  HANDLE_CLASS(DIST_ALLOC(DistLock));

////////////////////////////////////////////////////////////

#define SEPARATOR ,

#define REMOTE_MSG(AA, BB, CC)                             \
template <class T>                                         \
void remoteMSG(PNode *pnode, enum type_ids cl, T *p_t,     \
	       int (T::*xyz)(AA) BB) {                     \
  long long int lxyz; /* Do not ask why a long long int */ \
  memcpy(&lxyz, &xyz, sizeof lxyz);                        \
  VMDAllocator.CurrentCommunicatorThread()->               \
    RemoteCallMsg(pnode, cl, (caddr_t) p_t, lxyz CC);      \
};

#define REMOTE_RPC(AA, BB, CC)                             \
template <class T>                                         \
int remoteRPC(PNode *pnode, enum type_ids cl, T *p_t,      \
	       int (T::*xyz)(AA) BB) {                     \
  long long int lxyz;                                      \
  memcpy(&lxyz, &xyz, sizeof lxyz);                        \
  return VMDAllocator.CurrentCommunicatorThread()->        \
    RemoteCallRPC(pnode, cl, (caddr_t) p_t, lxyz CC);      \
};

#define VARARG_VIRTLEN 1024
typedef struct _long_msg {
  ssize_t size;
  caddr_t data;
} long_msg_t;

template <class T>
int remoteRPC(PNode *pnode, enum type_ids cl, T *p_t,
	      int (T::*xyz)(long_msg_t *), long_msg_t *p1) {
  long long int lxyz;
  memcpy(&lxyz, &xyz, sizeof lxyz);
  return VMDAllocator.CurrentCommunicatorThread()->
    RemoteCallRPC(pnode, cl, (caddr_t) p_t, lxyz,
		  VARARG_VIRTLEN, (int) p1);
};

template <class T>
int remoteMSG(PNode *pnode, enum type_ids cl, T *p_t,
	      int (T::*xyz)(long_msg_t *), long_msg_t *p1) {
  long long int lxyz;
  memcpy(&lxyz, &xyz, sizeof lxyz);
  VMDAllocator.CurrentCommunicatorThread()->
    RemoteCallMsg(pnode, cl, (caddr_t) p_t, lxyz,
		  VARARG_VIRTLEN, (int) p1);
};

REMOTE_MSG(void,,);
REMOTE_MSG(int, SEPARATOR int p1, SEPARATOR 1 SEPARATOR p1);
REMOTE_MSG(int SEPARATOR int, SEPARATOR int p1 SEPARATOR int p2,
	   SEPARATOR 2 SEPARATOR p1 SEPARATOR p2);
REMOTE_MSG(int SEPARATOR int SEPARATOR int,
	   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3,
	   SEPARATOR 3 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3);
REMOTE_MSG(int SEPARATOR int SEPARATOR int SEPARATOR int,
	   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3 SEPARATOR int p4,
	   SEPARATOR 4 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3 SEPARATOR p4);

REMOTE_RPC(void,,);
REMOTE_RPC(int, SEPARATOR int p1, SEPARATOR 1 SEPARATOR p1);
REMOTE_RPC(int SEPARATOR int, SEPARATOR int p1 SEPARATOR int p2,
	   SEPARATOR 2 SEPARATOR p1 SEPARATOR p2);
REMOTE_RPC(int SEPARATOR int SEPARATOR int,
	   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3,
	   SEPARATOR 3 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3);
REMOTE_RPC(int SEPARATOR int SEPARATOR int SEPARATOR int,
	   SEPARATOR int p1 SEPARATOR int p2 SEPARATOR int p3 SEPARATOR int p4,
	   SEPARATOR 4 SEPARATOR p1 SEPARATOR p2 SEPARATOR p3 SEPARATOR p4);

////////////////////////////////////////////////////////////

extern class VMD VMDAllocator;

extern void exit_vmd(const int) __attribute__ ((noreturn));

struct conf_entry {
  enum _type {node, intercluster};
  enum _type type;
  class string name;
  int cluster;
  pnode_t pnode;
  class string ip;
  enum _transport {TCP, HSL};
  union {
    enum _transport type;
    struct {
      enum _transport type;
      int port;
    } tcp;
    struct {
      enum _transport type;
      int channel;
    } hsl;
  } inside;
  union {
    enum _transport type;
    struct {
      enum _transport type;
      int port;
    } tcp;
    struct {
      enum _transport type;
      int channel;
    } hsl;
  } outside;
};

#define MAX_DATA_SIZE 2048

struct msg_hdr_1 {
  enum msg_type {
    // Messages and RPC with no argument list.
    MSG, RPC,
    // Messages and RPC with argument list.
    MSG_ARG_INT, RPC_ARG_INT,
    MSG_ARG_INTx2, RPC_ARG_INTx2,
    MSG_ARG_INTx3, RPC_ARG_INTx3,
    MSG_ARG_INTx4, RPC_ARG_INTx4,
    // Long messages.
    MSG_ARG_VARARG, RPC_ARG_VARARG
  };

  enum msg_direction {
    REQUEST,
    RESPONSE // RESPONSE is for RPC only.
  };

  int size; // Global size of the message.
  enum msg_type type;
  enum msg_direction direction;

  pthread_cond_t *wait_cond;
  pnode_t dest_node;
  int dest_cluster;
  pnode_t src_node;
  int src_cluster;
};

struct msg_hdr_2 {
  enum type_ids dest_type; // Type of object.
  caddr_t dest_object;
  long long int dest_method;
};

////////////////////////////////////////////////////////////

class Lock {
public:
  Lock(void);
  virtual ~Lock(void);
  virtual bool Acquire(void) = 0;
  virtual void Release(void) = 0;
};

class MutexLock : public virtual Lock {
  pthread_mutex_t lock_m;
#ifdef DEBUG_HSL
  int cnt;
#endif
public:
  MutexLock(void);
  MutexLock(const class MutexLock &);
  virtual ~MutexLock(void);
  bool Acquire(void);
  void Release(void);
};

class ConditionalLock : public virtual Lock {
  pthread_mutex_t lock_m;
  pthread_cond_t lock_c;
  bool lock;
public:
  ConditionalLock(void);
  ConditionalLock(const class ConditionalLock &);
  virtual ~ConditionalLock(void);
  bool Acquire(void);
  void Release(void);
};

class ConditionalLockExpire : public virtual Lock {
  pthread_mutex_t lock_m;
  pthread_cond_t lock_c;
  enum state { state_false, state_true, state_exiting } lock;
  int count;
public:
  ConditionalLockExpire(void);
  ConditionalLockExpire(const class ConditionalLockExpire &);
  virtual ~ConditionalLockExpire(void);
  bool Acquire(void);
  void Release(void);
  void Expire(void);
};

////////////////////////////////////////////////////////////

class ThreadInitiator {
  friend class Thread;
 public:
  ThreadInitiator(void);
  virtual ~ThreadInitiator(void);
  virtual void *StartThread(void *) = 0;

  class Thread *CurrentThread(void);
  class CommunicatorThread *CurrentCommunicatorThread(void);
};

class Thread : public virtual MutexLock {
  friend int main(const int, char *const *, const char **);

  void *(*start_routine)(void *);
  bool activated;
  pthread_t thread;
  string thread_name;
  class ThreadInitiator *initiator_class;
  void *parameter;
  bool should_exit;
  class CommunicatorThread *communicator;

  static void *StartThread(void *);

 public:
  Thread(void);
  Thread(const string &, void *(*)(void *), void *);
  Thread(const string &, class ThreadInitiator *, void *);
  ~Thread(void);
  Thread(const class Thread &);

  string &Name(void);
  pthread_t ThreadID(void);
  void Start(void);
  void ExitYield(void);
  bool ShouldExit(void) const;

  void SetCommunicator(class CommunicatorThread *);
};

class CommunicatorThread : public Thread {
  pthread_mutex_t rpc_m;
  pthread_cond_t rpc_c;

  void BasicRemoteCall(msg_hdr_1::msg_type, class PNode *, enum type_ids,
		       caddr_t, long long int, int = 0, int = 0, int = 0,
                       int = 0, int = 0);

public:
  CommunicatorThread(void);
  CommunicatorThread(const string &, void *(*)(void *), void *);
  CommunicatorThread(const string &, class ThreadInitiator *, void *);
  ~CommunicatorThread(void);
  CommunicatorThread(const class CommunicatorThread &);

  void RemoteCallMsg(class PNode *, enum type_ids, caddr_t, long long int,
                     int = 0, int = 0, int = 0, int = 0, int = 0);
  int RemoteCallRPC(class PNode *, enum type_ids, caddr_t, long long int,
                    int = 0, int = 0, int = 0, int = 0, int = 0);
};

////////////////////////////////////////////////////////////

class EventSource : public iostream, virtual public ConditionalLock {
public:
  EventSource(class streambuf *);
  ~EventSource(void);
  virtual bool IsSelectable(void) const = 0;
  virtual int FileDescriptor(void) const = 0;
  virtual int CanRead(void) const = 0;

  // Because of an unknown bug, we can t override write() when using filebuf.
  // (this assumption must be verified)
#ifndef WITH_FILEBUF
  class ostream &write(const void *, streamsize);
#endif

#if ! defined WITH_STDIOBUF && ! defined WITH_FILEBUF
  class istream &read(void *, streamsize); // No buffering.
#endif
};

class FDEventSource : public EventSource {
public:
  FDEventSource(class streambuf *);
  ~FDEventSource(void);
  virtual bool IsSelectable(void) const;
  virtual int FileDescriptor(void) const;
  virtual int CanRead(void) const;
};

class SLREventSource : public EventSource {
public:
  SLREventSource(class streambuf *);
  ~SLREventSource(void);
  virtual bool IsSelectable(void) const;
  virtual int FileDescriptor(void) const;
  virtual int CanRead(void) const;
};

////////////////////////////////////////////////////////////
// Structures for exception handling

// Since "g++ -frtti" doesn t work well, we are obliged to do silly things
// (without -frtti, the expression "catch (class X &)" doesn t handle
// derivated classes from X)...

#define CATCH(X) catch (X &exc) CATCH_BLOCK
#define CATCH_ExceptFileDesc CATCH(ExceptFileDesc)
#define CATCH_ExceptStream   CATCH(ExceptStream)
#define CATCH_ExceptSpecificData CATCH(ExceptSpecificData)
#define CATCH_ExceptStream   CATCH(ExceptStream)
#define CATCH_ExceptMutex    CATCH(ExceptMutex)
#define CATCH_ExceptFatalMsg CATCH(ExceptFatalMsg)
#define CATCH_ExceptErrno    CATCH(ExceptErrno) CATCH_ExceptFileDesc
#define CATCH_ExceptFatal    CATCH(ExceptFatal) CATCH_ExceptFatalMsg
#define CATCH_ExceptFileLine CATCH(ExceptFileLine) CATCH_ExceptStream   \
                             CATCH_ExceptSpecificData CATCH_ExceptErrno \
                             CATCH_ExceptStream CATCH_ExceptMutex       \
                             CATCH_ExceptFatal
#define CATCH_exception      CATCH(exception) CATCH_ExceptFileLine

struct ExceptFileLine : public exception {
  const string file;
  const int line;
  ExceptFileLine(const string &, const int);
  virtual void Perror(void);
};

struct ExceptErrno : public ExceptFileLine {
  const string str;
  ExceptErrno(const string &, const int, const string &);
  virtual void Perror(void);
};

struct ExceptFileDesc : public ExceptErrno {
  const int fdesc;
  const bool should_close;
  ExceptFileDesc(const string &, const int, const int, const string &, bool);
  virtual void Perror(void);
};

struct ExceptFILE : public ExceptErrno {
  FILE *fdesc;
  const bool should_close;
  ExceptFILE(const string &, const int, FILE *, const string &,
		 bool);
  virtual void Perror(void);
};

struct ExceptMutex : public ExceptFileLine {
  const string str;
  ExceptMutex(const string &, int, const string &);
  virtual void Perror(void);
};

struct ExceptSpecificData : public ExceptFileLine {
  const string str;
  ExceptSpecificData(const string &, int, const string &);
  virtual void Perror(void);
};

struct ExceptStream : public ExceptFileLine {
  const string str;
  const class iostream *stream;
  const bool should_delete;
  ExceptStream(const string &, int, class iostream *, const string &, bool);
  virtual void Perror(void);
};

struct ExceptFatal : public ExceptFileLine {
  ExceptFatal(const string &, int);
};

struct ExceptFatalMsg : public ExceptFatal {
  const string msg;
  ExceptFatalMsg(const string &, int, const string &);
  virtual void Perror(void);
};

////////////////////////////////////////////////////////////

class PNode : virtual public MutexLock {
  friend int main(int, char **, char **);

  bool connected;
  class EventSource *event_source;
  struct conf_entry conf;

public:
  PNode(void);
  PNode(const struct conf_entry entry);
  PNode(const struct conf_entry entry, class EventSource *);
  ~PNode(void);
  class EventSource *GetEventSource(void) const;
  const struct conf_entry GetConfEntry(void) const;
//  PNode(const PNode &);

  bool Connected(void) const;
  void Connect(void);
};

////////////////////////////////////////////////////////////

class EventDriver : public ThreadInitiator {
  class EventSource *WaitForEvent(void);
  void ManageEvent(class EventSource *);

public:
  EventDriver();
  ~EventDriver();

  void *StartThread(void *);

  static int CallMethod(enum type_ids, caddr_t, long long int,
			int = 0, int = 0, int = 0, int = 0, int = 0);
};

class EventInitiator : public ThreadInitiator {
  int   sock;
  uid_t uid;
  pid_t pid;
  char progname[32];

  int Read(char *, int);
  int Write(char *, int);


public:
  EventInitiator();
  ~EventInitiator();

  void *StartThread(void *);
};

class Server : public ThreadInitiator {
  const bool inside;
public:
  Server(bool);
  ~Server(void);

  void *StartThread(void *);
  void AcceptConnections(void);
};

class CommandLine : public ThreadInitiator {
public:
  CommandLine(void);
  ~CommandLine(void);

  void *StartThread(void *);
};

class Initializer : public ThreadInitiator {
public:
  Initializer(void);
  ~Initializer(void);

  void *StartThread(void *);
};

class Finalizer : public ThreadInitiator {
public:
  Finalizer(void);
  ~Finalizer(void);

  void *StartThread(void *);
};

////////////////////////////////////////////////////////////

typedef pair<appclassname_t, primary_t> appclassdef_t;

typedef struct _channel_info {
  appclassdef_t classdef;
  channel_t     channel_pair_0;
  channel_t     channel_pair_1; // channel_pair_1 == channel_pair_1 + 1
  int           protocol;
} channel_info_t;

// ChannelManager is derivated from MutexLock to assume atomic operations.
class ChannelManager : public virtual MutexLock {
  typedef map<appclassdef_t, channel_info_t, less<appclassdef_t> >
    request_map_t;
  typedef map<channel_t, channel_info_t, less<channel_t> >
    channel_map_t;
  typedef map<channel_t, int, less<channel_t> >
    chanref_map_t;

  request_map_t   request_map;
  class MutexLock request_map_m;

  channel_map_t   channel_map;
  class MutexLock channel_map_m;

  chanref_map_t   chanref_map;
  class MutexLock chanref_map_m;

  pnode_t dist_node;

  void _RefChannel(appclassdef_t);
  int _UnrefChannel(appclassdef_t);

  int OpenChannel(channel_t, channel_t, appclassname_t, int);
  int ShutdownChannel(channel_t, channel_t, int);

 public:
  ChannelManager(void) {
    cerr << "WARNING: USE OF ChannelManager(void) IS PROHIBITED.\n";
  }
  ChannelManager(pnode_t);
  ~ChannelManager(void);

  /* This X(const X &) constructor needed due to an internal bug of gcc
     (gcc breaks if we don't define this constructor). */
  ChannelManager(const ChannelManager &cm) :
    request_map(cm.request_map), request_map_m(cm.request_map_m),
    channel_map(cm.channel_map), channel_map_m(cm.channel_map_m),
    chanref_map(cm.chanref_map), chanref_map_m(cm.chanref_map_m),
    dist_node(cm.dist_node) {
    cerr << "WARNING: USE OF ChannelManager(ChannelManager &) IS PROHIBITED.\n";
    cerr << "dist_node = " << dist_node << "\n";
  }

  channel_info_t GetChannel(appclassdef_t, int);
  void RefChannel(appclassdef_t);
  int UnrefChannel(appclassdef_t);

  void Dump(void);

#if 0
  void AcceptChannel(appclassname_t, appsubclassname_t, uid_t,
		     uid_t *, pnode_t *, channel_t *, bool);
#endif
};

////////////////////////////////////////////////////////////

class DeviceInterface : public virtual MutexLock {
  int dev_hsl_fd;
 public:
  DeviceInterface(void);
  ~DeviceInterface(void);

  void OpenDevice(void);
  void CloseDevice(void);
  bool ModuleLoaded(void);

  void SetAppClass(appclassname_t, uid_t);
  // Not yet implemented :
  // void UnsetAppClass(appclassname_t, uid_t);

  int OpenChannel(pnode_t, channel_t, channel_t, appclassname_t, int);
  int ShutdownChannel_1stStep(pnode_t, channel_t, channel_t,
			      seq_t *, seq_t *, seq_t *, seq_t *, int);
  int ShutdownChannel_2ndStep(pnode_t, channel_t, channel_t,
			      seq_t, seq_t, seq_t, seq_t, int);
  int ShutdownChannel_3rdStep(pnode_t, channel_t, channel_t, int);
};

////////////////////////////////////////////////////////////

// SHOULD BE class VMD : public VIRTUAL MutexLock

class VMD : public MutexLock {
  friend void *Thread::StartThread(void *);
  friend void treat_sigint(int);

  pthread_key_t current_thread_key;
  pthread_key_t current_communicator_thread_key;

#define CONF_TAB_LEN 50
  // conf_tab is protected by the global lock for VMD.
  struct conf_entry conf_tab[CONF_TAB_LEN];
  int conf_tab_len;
  struct conf_entry *my_conf_entry;

  const class PNode &FindInterCluster(int);

  pid_t loader_pid;
  int loader_pipe;
  class MutexLock loader_pipe_m;

public:
  u_int ref_cnt;

  const struct conf_entry *MyConfEntry(void) const;
  struct conf_entry FindConfEntry(int, pnode_t);
  class PNode *FindPNode(int, pnode_t);
  int NClusters(void);
  int NNodes(int);

  class Thread *CurrentThread(void);
  class CommunicatorThread *CurrentCommunicatorThread(void);

  list<class PNode> pnode_list;
  pthread_mutex_t pnode_list_mutex;

  list<class Thread> thread_list;
  pthread_mutex_t thread_list_mutex;

  list<class CommunicatorThread> communicator_thread_list;
  pthread_mutex_t communicator_thread_list_mutex;

  list<class EventSource *> event_source_list;
  pthread_mutex_t event_source_list_mutex;

  list<class EventDriver *> event_driver_list;
  pthread_mutex_t event_driver_list_mutex;

  list<class EventInitiator *> event_initiator_list;
  pthread_mutex_t event_initiator_list_mutex;

  class Server internal_server, external_server;

  class CommandLine command_line;
  class Initializer initializer;
  class Finalizer finalizer;

  bool main_thread_interrupted;

  int all_connected;
  pthread_mutex_t all_connected_cmutex;
  pthread_cond_t all_connected_cond;

  class Thread *scanning_thread;
  pthread_mutex_t scanning_thread_cmutex;
  pthread_cond_t scanning_thread_cond;

  int exit_pipe;
  class EventSource *exit_pipe_event_source;

  // Set to true when the destruction process is entered.
  bool finishing;

  // Synchronisation for garbage collection.
  pthread_mutex_t garbage_coll_cmutex;
  pthread_cond_t garbage_coll_cond;

  // Base key.
  int msq_basekey;
  // Incoming message queue.
  int msq_incoming;

  // Socket for connections with local tasks.
  int unixdomain_sock;

  class DistController *controller;

  // We use a pointer to ChannelManager instead of a ChannelManager itself
  // because the access to a map with operator [] calls X() and X(X&).
  // Since ChannelManager contains locks, we can_t invoke them after start-up :
  // There is no easy method to export the lock state in such a constructor.
  map<pnode_t, ChannelManager *, less<pnode_t> > channel_manager_map;
  class MutexLock channel_manager_map_m;

  class DeviceInterface device;

  VMD();
  ~VMD();
  class PNode &AddNode(const struct conf_entry entry);
  class PNode &AddNode(const struct conf_entry entry, class EventSource *);
  void ReadConf(const char *, const char *);
  void ConnectToNodes(void);
  void InitChannelManagerMap(void);
  void DestroyChannelManagerMap(void);

  void MainThreadYield(void);
  void MainThreadYield(pthread_mutex_t *);

  void TestServiceMsg(int = 0, int = 0, int = 0);
  int TestServiceRPC(int = 0, int = 0, int = 0);

  void StartLoader(void);
  void SpawnTask(const char *, appclassname_t, uid_t);
};

#endif

