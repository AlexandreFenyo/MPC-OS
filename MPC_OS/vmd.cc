
// $Id: vmd.cc,v 1.2 1999/01/09 18:58:36 alex Exp $

// man funopen pour creer un FILE * dont on maitrise les fonctions de base
// pbs avec const : solution == mutable

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

#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include "mpcshare.h"

#include "vmd.h"
#include "distobj.h"

#include "typeid.h"

/* To instanciate pair templates with appsubclassname_t, we need a comparator. */
bool operator < (const appsubclassname_t &x, const appsubclassname_t &y) {
  u_long xval, yval;

  xval = (x.raw.is_raw << 16) | x.raw.secondary;
  yval = (y.raw.is_raw << 16) | y.raw.secondary;

  return (x.raw.primary == y.raw.primary) ? (x.raw.primary < y.raw.primary) :
    (xval < yval);
}

class VMD VMDAllocator;

////////////////////////////////////////////////////////////

// Linking with libc_r and C++ class librairies causes the global
// destructors to be called twice at the end. Bug fixed by
// calling here the global destructors and performing the _exit
// libc_r call instead of the exit one.

#ifndef _SMP_SUPPORT
extern "C" {
 void __do_global_dtors(void);
}
#endif

void exit_vmd(const int status)
{
  cerr << "vmd terminating\n";
#ifndef _SMP_SUPPORT
  // A.OUT FORMAT

  __do_global_dtors();
#else
  // ELF FORMAT

  // Here we should call every global destructors, but we can't call
  // __do_global_dtors because it is not available with the ELF binary
  // format (_SMP_SUPPORT => ELF).
#endif
  _exit(status);
}

////////////////////////////////////////////////////////////

void perror_cerr(const string &file, const int line, const string &str)
{
  cerr << "(" << file << "+" << line << "): ";
  cerr << str << "\n";
}

void perror_cerr_errno(const string &file, const int line, const string &str)
{
  cerr << "(" << file << "+" << line << "): ";
  if (errno)
    cerr << str << ": " << (char *) strerror(errno) << "\n";
  else
    cerr << str << "\n";
}

static void terminate_on_exception(void) __attribute__ ((noreturn));

static void terminate_on_exception(void)
{
  cerr << "exception not catched\n";
  exit_vmd(1);
}

static void unexpected_exception(void) __attribute__ ((noreturn));

static void unexpected_exception(void)
{
  cerr << "unexpected exception\n";
  terminate_on_exception();
}

////////////////////////////////////////////////////////////

Lock::Lock() {}

Lock::~Lock() {}

////////////////////////////////////////////////////////////

MutexLock::MutexLock(void) {
  int res;

#ifdef DEBUG_HSL
  cnt = 0;
#endif

  res = pthread_mutex_init(&lock_m, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Lock()");
}

MutexLock::~MutexLock(void) {
  int res;

  res = pthread_mutex_destroy(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "~Lock()");
}

// This constructor is called when a lock object is inserted in a list.
// It doesn't preserve the lock on the new object if it was set on
// the original one.
MutexLock::MutexLock(const class MutexLock &) {
  int res;

#ifdef DEBUG_HSL
  cnt = 0;
#endif

  res = pthread_mutex_init(&lock_m, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Lock(Lock &)");
}

bool MutexLock::Acquire(void) {
  int res;

  res = pthread_mutex_lock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Acquire()");

#ifdef DEBUG_HSL
  cnt++;
  if (cnt > 1) throw ExceptMutex(__FILE__, __LINE__,
				 "Try to acquire the same mutex twice.");
#endif

  return true;
}

void MutexLock::Release(void) {
  int res;

#ifdef DEBUG_HSL
  cnt--;
  if (cnt < 0) throw ExceptMutex(__FILE__, __LINE__,
				 "Try to release the same mutex twice.");
#endif

  res = pthread_mutex_unlock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Release()");
}

////////////////////////////////////////////////////////////

ConditionalLock::ConditionalLock(void) : lock(false) {
  int res;

  res = pthread_mutex_init(&lock_m, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Lock()");

  res = pthread_cond_init(&lock_c, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "Lock()");
}

ConditionalLock::~ConditionalLock(void) {
  int res;

  res = pthread_mutex_destroy(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "~Lock()");

  res = pthread_cond_destroy(&lock_c);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "~Lock()");
}

// This constructor is called when a lock object is inserted in a list.
// It doesn't preserve the lock on the new object if it was set on
// the original one.
ConditionalLock::ConditionalLock(const class ConditionalLock &) {
  int res;

  res = pthread_mutex_init(&lock_m, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Lock(Lock &)");

  res = pthread_cond_init(&lock_c, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "Lock(Lock &)");
}

bool ConditionalLock::Acquire(void) {
  int res;

  res = pthread_mutex_lock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Acquire()");

retry:
  if (lock == true) {
    res = pthread_cond_wait(&lock_c, &lock_m);
    if ((res != 0) and (res != EINTR)) throw ExceptFatal(__FILE__, __LINE__);
    goto retry;
  }

  lock = true;

  res = pthread_mutex_unlock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Acquire()");

  return true;
}

void ConditionalLock::Release(void) {
  int res;

  res = pthread_mutex_lock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Release()");

  lock = false;
  res = pthread_cond_signal(&lock_c);
  if (res != 0) throw ExceptFatal(__FILE__, __LINE__);

  res = pthread_mutex_unlock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Release()");
}

////////////////////////////////////////////////////////////

ConditionalLockExpire::ConditionalLockExpire(void) : lock(state_false),
  count(0) {
  int res;

  res = pthread_mutex_init(&lock_m, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Lock()");

  res = pthread_cond_init(&lock_c, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "Lock()");
}

ConditionalLockExpire::~ConditionalLockExpire(void) {
  int res;

  res = pthread_mutex_destroy(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "~Lock()");

  res = pthread_cond_destroy(&lock_c);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "~Lock()");
}

// This constructor is called when a lock object is inserted in a list.
// It doesn't preserve the lock on the new object if it was set on
// the original one.
ConditionalLockExpire::ConditionalLockExpire(const class
					     ConditionalLockExpire &) {
  int res;

  res = pthread_mutex_init(&lock_m, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Lock(Lock &)");

  res = pthread_cond_init(&lock_c, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "Lock(Lock &)");
}

bool ConditionalLockExpire::Acquire(void) {
  int res;

  res = pthread_mutex_lock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Acquire()");

  count++;

retry:
  if (lock == state_exiting) {
    if (--count == 0) {
      pthread_cond_signal(&lock_c);
      if (res != 0)
	throw ExceptErrno(__FILE__, __LINE__, "pthread_cond_broadcast");
    }

    res = pthread_mutex_unlock(&lock_m);
    if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Acquire()");

    return false;
  }

  if (lock == state_true) {
    res = pthread_cond_wait(&lock_c, &lock_m);
    if ((res != 0) and (res != EINTR)) throw ExceptFatal(__FILE__, __LINE__);
    goto retry;
  }

  lock = state_true;

  res = pthread_mutex_unlock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Acquire()");

  return true;
}

void ConditionalLockExpire::Release(void) {
  int res;

  res = pthread_mutex_lock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Release()");

  count--;
  if (lock == state_true) lock = state_false;

  res = pthread_cond_signal(&lock_c);
  if (res != 0) throw ExceptFatal(__FILE__, __LINE__);

  res = pthread_mutex_unlock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Release()");
}

// Must be called by a thread that HAS NOT TAKEN the lock.
void ConditionalLockExpire::Expire(void) {
  int res;

  res = pthread_mutex_lock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Expire()");

  if (count > 0) {
    lock = state_exiting;

    res = pthread_cond_broadcast(&lock_c);
    if (res != 0) throw ExceptFatal(__FILE__, __LINE__);

  retry:
    res = pthread_cond_wait(&lock_c, &lock_m);

    if ((res != 0) and (res != EINTR)) throw ExceptFatal(__FILE__, __LINE__);
    if (count > 0) goto retry;
  }

  res = pthread_mutex_unlock(&lock_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "Expire()");
}

////////////////////////////////////////////////////////////
// Structures for exception handling

ExceptFileLine::ExceptFileLine(const string &f, const int l) :
  exception(/*sans EGCS "ExceptFileLine"*/), file(f), line(l) {
    cerr << "(" << f << "+" << l << "): exception thrown\n";
}

void ExceptFileLine::Perror(void) {
  perror_cerr(file, line, "exception ExceptFileLine");
}

ExceptErrno::ExceptErrno(const string &file, const int line, const string &s) :
  ExceptFileLine(file, line), str(s) {}

void ExceptErrno::Perror(void) {
  perror_cerr_errno(file, line, "exception ExceptErrno: " + str);
}

ExceptFileDesc::ExceptFileDesc(const string &file, const int line,
			       const int fd, const string &s,
			       bool close) :
  ExceptErrno(file, line, s), fdesc(fd), should_close(close) {
    cerr << "exception ExceptFileDesc in file : " << file << "\n";
}

void ExceptFileDesc::Perror(void) {
  struct stat sb;
  int res;

  cerr << "[fd=" << fdesc << "] ";
  perror_cerr_errno(file, line,
		    "exception ExceptFileDesc: " + str);

  if (should_close == true) {
    cerr << "[fd=" << fdesc << "] closing fd : ";
    res = fstat(fdesc, &sb);
    close(fdesc);
    if (res < 0) cerr << "can't stat fd\n";
    else switch (S_IFMT & sb.st_mode) {
    case S_IFIFO:
      cerr << "named pipe (fifo)\n";
      break;

    case S_IFCHR:
      cerr << "character special file\n";
      break;

    case S_IFDIR:
      cerr << "directory\n";
      break;

    case S_IFBLK:
      cerr << "block special file\n";
      break;

    case S_IFREG:
      cerr << "regular file\n";
      break;

    case S_IFLNK:
      cerr << "symbolic link\n";
      break;

    case S_IFSOCK:
      cerr << "socket\n";
      break;

    default:
      cerr << "file descriptor type unknown\n";
      break;
    }
  }
}

ExceptFILE::ExceptFILE(const string &file, const int line,
			       FILE *fd, const string &s,
			       bool close) :
  ExceptErrno(file, line, s), fdesc(fd), should_close(close) {
    cerr << "exception ExceptFILE in file : " << file << "\n";
}

void ExceptFILE::Perror(void) {
  perror_cerr_errno(file, line,
		    "exception ExceptFILE: " + str);
  if (should_close) fclose(fdesc);
}

ExceptMutex::ExceptMutex(const string &file, int line, const string &s) :
    ExceptFileLine(file, line), str(s) {}

void ExceptMutex::Perror(void) {
  perror_cerr_errno(file, line, "exception ExceptMutex: " + str);
}

ExceptSpecificData::ExceptSpecificData(const string &file, int line,
				       const string &s) :
  ExceptFileLine(file, line), str(s) {}

void ExceptSpecificData::Perror(void) {
  perror_cerr_errno(file, line, "exception ExceptSpecificData: " + str);
}

ExceptStream::ExceptStream(const string &file, int line,
			   class iostream *strm, const string &s, bool del) :
  ExceptFileLine(file, line), str(s), stream(strm), should_delete(del) {}

void ExceptStream::Perror(void) {
  cerr << "stream: state:";
  if ((stream->rdstate() & ios::goodbit) != 0) cerr << " GOOD (goodbit)";
  if ((stream->rdstate() & ios::eofbit) != 0) cerr << " EOF (eofbit)";
  if ((stream->rdstate() & ios::failbit) != 0) cerr << " FAIL (failbit)";
  if ((stream->rdstate() & ios::badbit) != 0) cerr << " BAD (badbit)";
  cerr << "\n";
  perror_cerr_errno(file, line, "exception ExceptStream: " + str);
    if (should_delete == true) {
      // Deleting an fstream closes the underlying file descriptor if there
      // is one (in case of TCP/IP for instance).
      // Deleting an stdiostream doesn't close the underlying FILE.
      // If typeid worked correctly, we should examine the type of
      // stream we deal with, and we would then sweep the memory
      // correctly.
      // It would start by evaluating such an expression :
      // typeid(*stream).before(typeid(class stdiobuf))
      // But before() isn't implemented in libg++ and typeid(class stdiobuf)
      // doesn't work (possibly a bug in g++).

      class streambuf *strbuf = stream->rdbuf();

      // When linking with -frtti, this delete causes a crash !!!
      delete stream;

      // Deleting the underlying streambuf should NOT be done in case we'd
      // implement the iostream as an fstream, because the underlying
      // streambuf would have not been specified when calling the constructor.
      // If dynamic type identification worked correctly, we would do a better
      // implementation...
      // We can here delete strbuf because in every case we create
      // an iostream, we explicitly affect it a streambuf.

      if (strbuf != NULL) delete strbuf;
    }
}

ExceptFatal::ExceptFatal(const string &file, int line) :
  ExceptFileLine(file, line) {}

ExceptFatalMsg::ExceptFatalMsg(const string &file, int line,
			       const string &message) :
  ExceptFatal(file, line), msg(message) {}

void ExceptFatalMsg::Perror(void) {
  perror_cerr(file, line, "exception ExceptFatalMsg (" + msg + ")");
}

////////////////////////////////////////////////////////////

ThreadInitiator::ThreadInitiator(void) {}

ThreadInitiator::~ThreadInitiator(void) {}

class Thread *ThreadInitiator::CurrentThread(void) {
  return VMDAllocator.CurrentThread();
}

class CommunicatorThread *ThreadInitiator::CurrentCommunicatorThread(void) {
  return VMDAllocator.CurrentCommunicatorThread();
}

////////////////////////////////////////////////////////////

void *Thread::StartThread(void *arg)
{
  class Thread *thr = (class Thread *) arg;
  void *res;
  int ret;

  try {
    cerr << "thread \"" << thr->thread_name << "\" starting\n";

    sigset_t sigs;

    sigemptyset(&sigs);
    sigaddset(&sigs, SIGABRT);
    ret = pthread_sigmask(SIG_UNBLOCK, &sigs, NULL);

    // The threads must be all started at the beginning (i.e. before
    // signals may be posted) because when the thread is started,
    // if a signal is sent before we block it, this can call a handler
    // that should not be...
    // A better way to do this would be to set the signal mask
    // in Thread::Start before we start the thread; a complete example
    // is given in the manual page of pthread_create() of Solaris 2.5.
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    ret = pthread_sigmask(SIG_BLOCK, &sigs, NULL);

    if (ret != 0) throw ExceptErrno(__FILE__, __LINE__, "pthread_sigmask");

    if (thr->activated == true)
      throw ExceptFatalMsg(__FILE__, __LINE__, "thread already activated");
    thr->activated = true;

    if (thr->start_routine != NULL) {
      res = thr->start_routine(thr->parameter);
    } else {
      if (thr->initiator_class != NULL) {
	pthread_setspecific(VMDAllocator.current_thread_key, thr);
	pthread_setspecific(VMDAllocator.current_communicator_thread_key,
			    thr->communicator);
	res = thr->initiator_class->StartThread(thr->parameter);
      } else
	throw ExceptFatalMsg(__FILE__, __LINE__, "no mean to start thread");
    }
  }

  // It's very important to call Perror because some exceptions
  // make a special cleaning action during Perror (for instance,
  // file descriptors may be closed during Perror).
#ifdef CATCH_BLOCK
#undef CATCH_BLOCK
#endif
#define CATCH_BLOCK {                                               \
    cerr << "exception in thread \"" << thr->thread_name << "\"\n"; \
    exc.Perror();                                                   \
    res = NULL; }

  CATCH_ExceptFileLine

  catch (exception &exc) {
    cerr << "exception in thread \"" << thr->thread_name << "\"\n";
    cerr << "exception type        = ";
        cerr_typeid(exc);
    cerr << "\n";
    cerr << "exception description = " << exc.what() << "\n";
    res = NULL;
  }

  thr->activated = false;

  cerr << "thread \"" << thr->thread_name << "\" exiting\n";
  return res;
}

Thread::Thread(void) : start_routine(NULL), activated(false), thread_name(""),
  initiator_class(NULL), parameter(NULL), should_exit(false),
  communicator(NULL) {}

Thread::Thread(const string &name, void *(*start)(void *), void *param) :
  start_routine(start), activated(false), thread_name(name),
  initiator_class(NULL), parameter(param), should_exit(false),
  communicator(NULL) {}

Thread::Thread(const string &name, class ThreadInitiator *thr_i, void *param) :
  start_routine(NULL), activated(false), thread_name(name),
  initiator_class(thr_i), parameter(param), should_exit(false),
  communicator(NULL) {}

Thread::~Thread(void) {
  if (activated == true)
    cerr << "thread running but class Thread object \"" <<
      thread_name << "\" deleted\n";
}

// handling insertion of a thread in VMD::thread_list
Thread::Thread(const class Thread &orig) {
  start_routine   = orig.start_routine;
  activated       = orig.activated;
  thread          = orig.thread;
  thread_name     = orig.thread_name;
  parameter       = orig.parameter;
  initiator_class = orig.initiator_class;
  should_exit     = orig.should_exit;
  communicator    = orig.communicator;
}

void Thread::Start(void) {
  pthread_create(&thread, NULL, StartThread, this);
}

string &Thread::Name(void) {
  return thread_name;
}

pthread_t Thread::ThreadID(void) {
  return thread;
}

void Thread::ExitYield(void) {
  if (should_exit == true) {
    activated = false;
    cerr << "Thread::ExitYield: thread \"" << thread_name << "\" exiting\n";
    throw ExceptFatalMsg(__FILE__, __LINE__, "ExitYield exception");
  }
}

bool Thread::ShouldExit(void) const {
  return should_exit;
}

void Thread::SetCommunicator(class CommunicatorThread *ct) {
  communicator = ct;
}

////////////////////////////////////////////////////////////

CommunicatorThread::CommunicatorThread(void) : Thread() {
  int res;

  res = pthread_mutex_init(&rpc_m, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_cond_init(&rpc_c, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_init");

  SetCommunicator(this);
}

CommunicatorThread::CommunicatorThread(const string &name,
				       void *(*start)(void *), void *param) :
  Thread(name, start, param) {
  int res;

  res = pthread_mutex_init(&rpc_m, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_cond_init(&rpc_c, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_init");

  SetCommunicator(this);
}

CommunicatorThread::CommunicatorThread(const string &name,
				       class ThreadInitiator *thr_i,
				       void *param) :
  Thread(name, thr_i, param) {
  int res;

  res = pthread_mutex_init(&rpc_m, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_cond_init(&rpc_c, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_init");

  SetCommunicator(this);
}

CommunicatorThread::~CommunicatorThread() {
  int res;

  res = pthread_mutex_destroy(&rpc_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_destroy");

  res = pthread_cond_destroy(&rpc_c);
  if (res != 0)
    throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_destroy");
}

CommunicatorThread::CommunicatorThread(const class CommunicatorThread &orig) :
  Thread(orig) {
    int res;

    res = pthread_mutex_init(&rpc_m, NULL);
    if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

    res = pthread_cond_init(&rpc_c, NULL);
    if (res != 0)
      throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_init");
}

void CommunicatorThread::BasicRemoteCall(msg_hdr_1::msg_type msg_type,
					 class PNode *node,
					 enum type_ids type, caddr_t object,
					 long long int method, int nargs = 0,
					 int par1 = 0, int par2 = 0,
					 int par3 = 0, int par4 = 0) {
  struct msg_hdr_1 hdr1;
  struct msg_hdr_2 hdr2;
  int data[nargs];
  class EventSource *es;
  struct conf_entry conf;

  es = node->GetEventSource();
  conf = node->GetConfEntry();

  if (nargs != VARARG_VIRTLEN)
    hdr1.size = sizeof hdr1 + sizeof hdr2 + nargs * sizeof(int);
  else
    hdr1.size = sizeof hdr1 + sizeof hdr2 + ((long_msg_t *) par1)->size;

  hdr1.type = msg_type;

  switch (nargs) {
  case 0:
    break;

  case 1:
    data[0] = par1;
    break;

  case 2:
    data[0] = par1;
    data[1] = par2;
    break;

  case 3:
    data[0] = par1;
    data[1] = par2;
    data[2] = par3;
    break;

  case 4:
    data[0] = par1;
    data[1] = par2;
    data[2] = par3;
    data[3] = par4;
    break;

  case VARARG_VIRTLEN:
    break;

  default:
    throw ExceptFatal(__FILE__, __LINE__);
  }

  hdr1.direction = msg_hdr_1::REQUEST;
  hdr1.wait_cond = &rpc_c;
  hdr1.dest_node = conf.pnode;
  hdr1.dest_cluster = conf.cluster;
  hdr1.src_node = VMDAllocator.MyConfEntry()->pnode;
  hdr1.src_cluster = VMDAllocator.MyConfEntry()->cluster;

  cerr << "BasicRemoteCall : WE SEND A MESSAGE TO cluster " << hdr1.dest_cluster
       << " node " << hdr1.dest_node << " from cluster " << hdr1.src_cluster
       << " node " << hdr1.src_node << "\n";


  hdr2.dest_type = type;
  hdr2.dest_object = object;
  hdr2.dest_method = method;

  es->Acquire();

  es->write(&hdr1, sizeof hdr1);
  es->write(&hdr2, sizeof hdr2);
  if (nargs > 0 and nargs != VARARG_VIRTLEN) es->write(data, nargs * sizeof(int));
  if (nargs == VARARG_VIRTLEN) es->write(((long_msg_t *) par1)->data,
					 ((long_msg_t *) par1)->size);
  es->flush();

  es->Release();

  if (!*es) throw ExceptFatal(__FILE__, __LINE__);
}

void CommunicatorThread::RemoteCallMsg(class PNode *node,
				       enum type_ids type, caddr_t object,
				       long long int method, int nargs = 0,
				       int par1 = 0, int par2 = 0,
				       int par3 = 0, int par4 = 0) {
  msg_hdr_1::msg_type msg_type;

  switch (nargs) {
  case 0:
    msg_type = msg_hdr_1::MSG;
    break;

  case 1:
    msg_type = msg_hdr_1::MSG_ARG_INT;
    break;

  case 2:
    msg_type = msg_hdr_1::MSG_ARG_INTx2;
    break;

  case 3:
    msg_type = msg_hdr_1::MSG_ARG_INTx3;
    break;

  case 4:
    msg_type = msg_hdr_1::MSG_ARG_INTx4;
    break;

  case VARARG_VIRTLEN:
    msg_type = msg_hdr_1::MSG_ARG_VARARG;
    break;

  default:
    throw ExceptFatal(__FILE__, __LINE__);
  }

  BasicRemoteCall(msg_type, node, type, object, method, nargs,
		  par1, par2, par3, par4);
}

int CommunicatorThread::RemoteCallRPC(class PNode *node,
				      enum type_ids type, caddr_t object,
				      long long int method, int nargs = 0,
				      int par1 = 0, int par2 = 0,
				      int par3 = 0, int par4 = 0) {
  cerr << "RemoteCallRPC()\n";

  int res, ret_val;
  msg_hdr_1::msg_type msg_type;
  class EventSource *es;

  switch (nargs) {
  case 0:
    msg_type = msg_hdr_1::RPC;
    break;

  case 1:
    msg_type = msg_hdr_1::RPC_ARG_INT;
    break;

  case 2:
    msg_type = msg_hdr_1::RPC_ARG_INTx2;
    break;

  case 3:
    msg_type = msg_hdr_1::RPC_ARG_INTx3;
    break;

  case 4:
    msg_type = msg_hdr_1::RPC_ARG_INTx4;
    break;

  case VARARG_VIRTLEN:
    msg_type = msg_hdr_1::RPC_ARG_VARARG;
    break;

  default:
    throw ExceptFatal(__FILE__, __LINE__);
  }

  res = pthread_mutex_lock(&rpc_m);

  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  BasicRemoteCall(msg_type, node, type, object, method, nargs,
		  par1, par2, par3, par4);

  // Wait for the answer.
retry:
  res = pthread_cond_wait(&rpc_c, &rpc_m);

  if ((res != 0) and (res != EINTR)) throw ExceptFatal(__FILE__, __LINE__);
  if (res == EINTR) goto retry;

  res = pthread_mutex_unlock(&rpc_m);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  // The event source has been locked by the event driver thread that
  // waked us up.

  es = node->GetEventSource();

  es->read(&ret_val, sizeof ret_val);
  es->Release();

  if (!*es) throw ExceptFatal(__FILE__, __LINE__);

  return ret_val;
}

////////////////////////////////////////////////////////////

EventSource::EventSource(class streambuf *stb) : iostream(stb) {}

EventSource::~EventSource(void) {}

#ifdef WITH_STDIOBUF

class ostream &EventSource::write(const void *s, streamsize n) {
  // We write on the exit pipe because of a race condition of the
  // implementation of libc_r : if a thread blocks on a select()
  // for the file descriptor fd, other threads will block when
  // writing on fd, until the select returns. To avoid this,
  // we write on the exit_pipe.
  int res = ::write(VMDAllocator.exit_pipe, "W", 1);
  if (res < 0) throw ExceptFatal(__FILE__, __LINE__);
  return iostream::write(s, n);
}

#elif defined WITH_FILEBUF

// Don't implement write().

#else // No buffering.

class ostream &EventSource::write(const void *s, streamsize n) {
  int res = ::write(VMDAllocator.exit_pipe, "W", 1);
  if (res < 0) throw ExceptFatal(__FILE__, __LINE__);

  res = ::write(FileDescriptor(), s, n);
  // We should better retry if not enough data have been send...
  if (res != n) throw ExceptFatal(__FILE__, __LINE__);
  // We should change the stream state...
  if (res < 0) throw ExceptFatal(__FILE__, __LINE__);
  return *this;
}

#endif

#if ! defined WITH_STDIOBUF && ! defined WITH_FILEBUF
class istream &EventSource::read(void *s, streamsize n) {
  int res = ::read(FileDescriptor(), s, n);
  // We should better retry if not enough data have been send...
  if (res != n) throw ExceptFatal(__FILE__, __LINE__);
  // We should change the stream state...
  if (res < 0) throw ExceptFatal(__FILE__, __LINE__);
  return *this;
}
#endif

FDEventSource::FDEventSource(class streambuf *stb) : EventSource(stb) {}

FDEventSource::~FDEventSource(void) {}

bool FDEventSource::IsSelectable(void) const {
  return true;
}

int FDEventSource::FileDescriptor(void) const {
#if defined WITH_STDIOBUF || ! defined WITH_FILEBUF
  return ((class stdiobuf *) rdbuf())->stdiofile()->_file;
#elif defined WITH_FILEBUF
  return ((class filebuf *) rdbuf())->fd();
#endif
}

int FDEventSource::CanRead(void) const {
  return -1;
}

SLREventSource::SLREventSource(class streambuf *stb) : EventSource(stb) {}

SLREventSource::~SLREventSource(void) {}

bool SLREventSource::IsSelectable(void) const {
  return false;
}

int SLREventSource::FileDescriptor(void) const {
  return -1;
}

int SLREventSource::CanRead(void) const {
  // Not implemented yet...
  return false;
}

////////////////////////////////////////////////////////////

PNode::PNode(void) : connected(false), event_source(NULL) {
  throw ExceptFatal(__FILE__, __LINE__);
}

PNode::PNode(const struct conf_entry entry) : connected(false),
  event_source(NULL), conf(entry) {}

PNode::PNode(const struct conf_entry entry, class EventSource *es) :
  connected(true), event_source(es), conf(entry) {}

PNode::~PNode(void) {}

bool PNode::Connected(void) const { return connected; }

void PNode::Connect(void) {
  int res;
  struct protoent *pent;
  struct hostent *hent;
  struct sockaddr_in saddr;

#define INSIDE (VMDAllocator.MyConfEntry()->cluster == conf.cluster)

  if (INSIDE and (conf.inside.type == conf_entry::HSL))
    throw ExceptFatalMsg(__FILE__, __LINE__,
			 "intracluster transport by HSL not yet implemented");

  if (connected == true)
    throw ExceptFatalMsg(__FILE__, __LINE__, "already connected");

  pent = getprotobyname("tcp");
  if (pent == NULL) throw ExceptErrno(__FILE__, __LINE__, "getprotobyname");

  int fd = socket(PF_INET, SOCK_STREAM, pent->p_proto);
  if (fd < 0) throw ExceptFileDesc(__FILE__, __LINE__, fd, "socket", true);

  int flags = fcntl(fd, F_GETFL, NULL);
  res = fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
  if (res != 0) throw ExceptFileDesc(__FILE__, __LINE__, fd, "fcntl", true);

  hent = gethostbyname((const char *) conf.ip.data());
  if (hent == NULL)
    throw ExceptFileDesc(__FILE__, __LINE__, fd, "gethostbyname", true);

  bzero(&saddr, sizeof saddr);
  saddr.sin_family = AF_INET;
  saddr.sin_port =
    htons(INSIDE ? conf.inside.tcp.port : conf.outside.tcp.port);

  saddr.sin_len = sizeof saddr;
  memcpy(&saddr.sin_addr, hent->h_addr_list[0],
 	 min((int) sizeof saddr.sin_addr, (int) hent->h_length));

  res = connect(fd, (sockaddr *) &saddr, sizeof saddr);
  if (res < 0) throw ExceptFileDesc(__FILE__, __LINE__, fd, "connect", true);

#if defined WITH_STDIOBUF || ! defined WITH_FILEBUF
  FILE *file_sock = fdopen(fd, "r+");
  if (file_sock == NULL)
    throw ExceptFileDesc(__FILE__, __LINE__, fd, "fdopen", true);

  // Should be replaced by class streambuf *...
  class stdiobuf *stdbuffer = new stdiobuf(file_sock);
#else // WITH_FILEBUF
  class streambuf *stdbuffer = new filebuf(fd);
#endif

  event_source = new FDEventSource(stdbuffer);

  res = pthread_mutex_lock(&VMDAllocator.event_source_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "event_source_list");

  VMDAllocator.event_source_list.push_back(event_source);

  res = pthread_mutex_unlock(&VMDAllocator.event_source_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "event_source_list");

  // Identify ourself.

  event_source->write(&VMDAllocator.MyConfEntry()->cluster,
		      sizeof VMDAllocator.MyConfEntry()->cluster);
  event_source->write(&VMDAllocator.MyConfEntry()->pnode,
		      sizeof VMDAllocator.MyConfEntry()->pnode);
  event_source->flush();

  if (!*event_source)
    throw
      ExceptStream(__FILE__, __LINE__, event_source, "event_source", true);

  connected = true;

  // Handle all_connected condition variable.

  res = pthread_mutex_lock(&VMDAllocator.all_connected_cmutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  VMDAllocator.all_connected++;

  res = pthread_cond_signal(&VMDAllocator.all_connected_cond);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_cond_signal");

  res = pthread_mutex_unlock(&VMDAllocator.all_connected_cmutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

#undef INSIDE
}

class EventSource *PNode::GetEventSource(void) const {
  return event_source;
}

const struct conf_entry PNode::GetConfEntry(void) const {
  return conf;
}

////////////////////////////////////////////////////////////

CommandLine::CommandLine(void) {}

CommandLine::~CommandLine(void) {}

void *CommandLine::StartThread(void *) {
  string cmd;
  static class DistVMSpace *dl;

  int flags = fcntl(0, F_GETFL, NULL);
  int res = fcntl(0, F_SETFL, flags & ~O_NONBLOCK);
  if (res != 0)
    throw ExceptFileDesc(__FILE__, __LINE__, 0, "fcntl", true);

  for (;;) {
    struct timeval timeout;
    fd_set fdset;

    // When using a filebuf, the delay must be low because a bug
    // disallows us to override write(), to write on the exit_pipe.
    // (should be verified...)
#ifdef WITH_FILEBUF
    timeout.tv_sec = 1;
#else
    timeout.tv_sec = 5;
#endif
    timeout.tv_usec = 0;
    FD_ZERO(&fdset);
    FD_SET(0, &fdset);

    res = select(1, &fdset, NULL, NULL, &timeout);
    if (res < 0) throw ExceptErrno(__FILE__, __LINE__, "select");

    CurrentThread()->ExitYield();
    cin >> cmd;

    if (cmd == "rpc") {
      for (list<class PNode>::iterator it = VMDAllocator.pnode_list.begin();
	   it != VMDAllocator.pnode_list.end();
	   it++) {
	res = remoteRPC(&*it, CLASS(VMD), (VMD *) NULL, &VMD::TestServiceRPC, 5, 6, 7);
	cerr << "****************************************\n";
	cerr << "RPC RESULT : " << res << "\n";
      }
    }

    if (cmd == "new") {

cerr << "AVANT DistVMSpaceAllocator\n";
    dl = (class DistVMSpace *) DistVMSpaceAllocator.New();
cerr << "APRES DistVMSpaceAllocator\n";

      for (list<class PNode>::iterator it = VMDAllocator.pnode_list.begin();
	   it != VMDAllocator.pnode_list.end();
	   it++) {
	class DistVMSpace *res;

cerr << "AVANT RemoteNew\n";
	res = dl->RemoteNew(NULL);
cerr << "APRES RemoteNew\n";
	res = dl->RemoteNew(&*it);
	res = dl->RemoteNew(&*it);

	cerr << "****************************************\n";
	cerr << "NEW RESULT : " << res << "\n";
      }
    }

    if (cmd == "delete") {
      if (dl != NULL) DistVMSpaceAllocator.AsyncDelete((int) dl);
    }

    if (cmd == "dump") {
      DistVMSpaceAllocator.Dump();
    }

    if (cmd == "dumpobj") {
      char str[256];
      class TDistObject<DistVMSpace> *obj;

      cin >> str;
      sscanf(str, "%p", &obj);
      obj->Dump();

      cerr << "Dumping done.\n";
    }

    if (cmd == "deleteobj") {
      char str[256];
      class TDistObject<DistVMSpace> *obj1;
      class DistVMSpace *obj2;

      cin >> str;
      sscanf(str, "%p", &obj1);

      cin >> str;
      sscanf(str, "%p", &obj2);

      obj1->RemoteDelete((class PNode *) NULL, obj2);

      cerr << "Delete done.\n";
    }

    if (cmd == "migrate") {
      char str[256];
      class TDistObject<DistVMSpace> *obj1;
      class DistVMSpace *obj2;

      cin >> str;
      sscanf(str, "%p", &obj1);

      cin >> str;
      sscanf(str, "%p", &obj2);

      obj1->Migrate((class PNode *) NULL, obj2);

      cerr << "Migration done.\n";
    }

    if (cmd == "dumpchan") {
      VMDAllocator.channel_manager_map_m.Acquire();
      for (map<pnode_t, ChannelManager *, less<pnode_t> >::iterator
	     it = VMDAllocator.channel_manager_map.begin();
	   it != VMDAllocator.channel_manager_map.end(); it++) {
	cerr << "---- Dumping ChannelManager for pnode " << (*it).first << "\n";
	(*it).second->Dump();
      }
      VMDAllocator.channel_manager_map_m.Release();

      cerr << "Dumping done.\n";
    }

    if (cmd == "fulltest") {
      for (int cpt = 0; cpt < 100; cpt++) {
	for (list<class PNode>::iterator it = VMDAllocator.pnode_list.begin();
	     it != VMDAllocator.pnode_list.end();
	     it++) {

	  res = remoteRPC(&*it, CLASS(VMD), (VMD *) NULL, &VMD::TestServiceRPC, cpt, cpt, cpt);
	  cerr << "****************************************\n";
	  cerr << "RPC RESULT : " << res << "\n";
	}
      }
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////

Initializer::Initializer(void) {}

Initializer::~Initializer(void) {}

void *Initializer::StartThread(void *) {
  appclass_info_t ac_info;
  bool ret;

  // Create the controller instances.

  if (VMDAllocator.MyConfEntry()->type == conf_entry::intercluster and
      VMDAllocator.MyConfEntry()->cluster == 0) {
    // Allocate the object throw the allocator interface, not with
    // the primitive `new'. This is to have an automatic garbage
    // collection on exit : no need to delete the object.
    cerr << "Creating the controller instances\n";

    // NOTE THAT we dont need to register the controller instances
    // in the VMDAllocator.controller variable, since its constructor
    // do it by itself. This was the only method to have the
    // variable VMDAllocator.controller updated on each node,
    // not only on the node that initiate the creation.
    DistControllerAllocator.New();

    int res = pthread_mutex_lock(&VMDAllocator.pnode_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

    for (list<class PNode>::iterator it = VMDAllocator.pnode_list.begin();
	 it != VMDAllocator.pnode_list.end();
	 it++) {
      res = pthread_mutex_unlock(&VMDAllocator.pnode_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

      VMDAllocator.controller->RemoteNew(&*it);

      res = pthread_mutex_lock(&VMDAllocator.pnode_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");
    }

    res = pthread_mutex_unlock(&VMDAllocator.pnode_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

    cerr << "Controller instances created\n";

    // Create the predefined super-user application classes.
    ac_info.uid = 0;
    ac_info.name = APPCLASS_INTERNET;
    ret = VMDAllocator.controller->CreateAppClass(ac_info);
    if (ret == false) throw ExceptFileLine(__FILE__, __LINE__);
  } else VMDAllocator.controller = NULL;

  cerr << "Thread initializer terminating\n";
  return NULL;
}

////////////////////////////////////////////////////////////

Finalizer::Finalizer(void) {}

Finalizer::~Finalizer(void) {}

void *Finalizer::StartThread(void *) {
  // Destroy the controller instances.

  if (VMDAllocator.MyConfEntry()->type == conf_entry::intercluster and
      VMDAllocator.MyConfEntry()->cluster == 0) {
    cerr << "Destroying the controller instances\n";

    VMDAllocator.controller->dist_obj_list_m.Acquire();
    for (list<dist_obj<DistController> >::iterator
	   it = VMDAllocator.controller->dist_obj_list.begin();
	 it != VMDAllocator.controller->dist_obj_list.end();
	 it++) {
      pair<PNode *, DistController *> p((*it).pnode, (*it).obj);
      VMDAllocator.controller->dist_obj_list_m.Release();
      VMDAllocator.controller->RemoteDelete(p.first, p.second);
      VMDAllocator.controller->dist_obj_list_m.Acquire();
    }
    VMDAllocator.controller->dist_obj_list_m.Release();

    cerr << "Controller instances destroyed\n";
  }

  cerr << "Thread finalizer terminating\n";
  return NULL;
}

////////////////////////////////////////////////////////////

VMD::VMD() : conf_tab_len(0), my_conf_entry(NULL), ref_cnt(0),
  internal_server(true), external_server(false),
  main_thread_interrupted(false), all_connected(0), scanning_thread(NULL),
  exit_pipe(-1), exit_pipe_event_source(NULL), finishing(false),
  controller(NULL) {
    cerr << "VMD::VMD\n";

  int res;

  cerr << "VMDAllocator: being constructed\n";

  res = pthread_mutex_init(&pnode_list_mutex, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_mutex_init(&thread_list_mutex, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_mutex_init(&communicator_thread_list_mutex, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_mutex_init(&event_source_list_mutex, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_mutex_init(&event_driver_list_mutex, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_mutex_init(&event_initiator_list_mutex, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_key_create(&current_thread_key, NULL);
  if (res != 0)
    throw ExceptSpecificData(__FILE__, __LINE__, "pthread_key_create");

  res = pthread_key_create(&current_communicator_thread_key, NULL);
  if (res != 0)
    throw ExceptSpecificData(__FILE__, __LINE__, "pthread_key_create");

  res = pthread_setspecific(current_thread_key, NULL);
  if (res != 0)
    throw ExceptSpecificData(__FILE__, __LINE__, "pthread_set_specific");

  res = pthread_setspecific(current_communicator_thread_key, NULL);
  if (res != 0)
    throw ExceptSpecificData(__FILE__, __LINE__, "pthread_set_specific");

  res = pthread_mutex_init(&all_connected_cmutex, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_cond_init(&all_connected_cond, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_init");

  res = pthread_mutex_init(&scanning_thread_cmutex, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_cond_init(&scanning_thread_cond, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_init");

  // Create the event source needed to handle cancellation of threads.
  // This event source is required because a signal sent to the
  // thread that called select() isn't sufficient to wake it up :
  // if the signal arrive just before the select is performed,
  // a race condition occurs. With this special event source,
  // the race condition is avoided.
  int ptab[2];

  res = pipe(ptab);
  if (res < 0) throw ExceptFatal(__FILE__, __LINE__);

  exit_pipe = ptab[1];

#if defined WITH_STDIOBUF || ! defined WITH_FILEBUF
  FILE *filestr = fdopen(ptab[0], "r+");
  if (filestr == NULL)
    throw ExceptFileDesc(__FILE__, __LINE__, ptab[0], "fdopen", true);

  class streambuf *stdbuffer = new stdiobuf(filestr);
#else // WITH_FILEBUF
  class streambuf *stdbuffer = new filebuf(ptab[0]);
#endif

  exit_pipe_event_source = new FDEventSource(stdbuffer);

  event_source_list.push_back(exit_pipe_event_source);

  res = pthread_mutex_init(&garbage_coll_cmutex, NULL);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_init");

  res = pthread_cond_init(&garbage_coll_cond, NULL);
  if (res != 0) throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_init");
}

VMD::~VMD() {
  cerr << "VMD::~VMD\n";

  int res;

  cerr << "VMDAllocator: being destructed\n";

  if (exit_pipe >= 0) close(exit_pipe);

  for (list<class EventSource *>::iterator it =
	 VMDAllocator.event_source_list.begin();
       it != VMDAllocator.event_source_list.end();
       it++) delete *it;

  for (list<class EventDriver *>::iterator it =
	 VMDAllocator.event_driver_list.begin();
       it != VMDAllocator.event_driver_list.end();
       it++) delete *it;

  for (list<class EventInitiator *>::iterator it =
	 VMDAllocator.event_initiator_list.begin();
       it != VMDAllocator.event_initiator_list.end();
       it++) delete *it;

  res = pthread_mutex_destroy(&pnode_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_destroy");

  res = pthread_mutex_destroy(&thread_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_destroy");

  res = pthread_mutex_destroy(&communicator_thread_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_destroy");

  res = pthread_mutex_destroy(&event_source_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_destroy");

  res = pthread_mutex_destroy(&event_driver_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_destroy");

  res = pthread_mutex_destroy(&event_initiator_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_destroy");

  res = pthread_key_delete(current_thread_key);
  if (res != 0)
    throw ExceptSpecificData(__FILE__, __LINE__, "pthread_key_delete");

  res = pthread_key_delete(current_communicator_thread_key);
  if (res != 0)
    throw ExceptSpecificData(__FILE__, __LINE__, "pthread_key_delete");

  res = pthread_mutex_destroy(&all_connected_cmutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_destroy");

  res = pthread_cond_destroy(&all_connected_cond);
  if (res != 0)
    throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_destroy");

  res = pthread_mutex_destroy(&scanning_thread_cmutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_destroy");

  res = pthread_cond_destroy(&scanning_thread_cond);
  if (res != 0)
    throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_destroy");

  res = pthread_mutex_destroy(&garbage_coll_cmutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_destroy");

  res = pthread_cond_destroy(&garbage_coll_cond);
  if (res != 0)
    throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_destroy");
}

const class PNode &VMD::FindInterCluster(int cluster) {
  int res = pthread_mutex_lock(&pnode_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  for (list<class PNode>::iterator it = pnode_list.begin();
       it != pnode_list.end();
       it++) {
    struct conf_entry conf = (*it).GetConfEntry();
    if ((conf.cluster == cluster) and
	(conf.type == conf_entry::intercluster)) {
      res = pthread_mutex_unlock(&pnode_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");
      return *it;
    }
  }

  res = pthread_mutex_unlock(&pnode_list_mutex);
  if (res != 0)
    throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");
  throw ExceptFatal(__FILE__, __LINE__);
}

class PNode *VMD::FindPNode(int cluster, pnode_t node) {
  if ((cluster == my_conf_entry->cluster) and (node == my_conf_entry->pnode))
    return NULL;

  int res = pthread_mutex_lock(&pnode_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  for (list<class PNode>::iterator it = pnode_list.begin();
       it != pnode_list.end();
       it++) {
    struct conf_entry conf = (*it).GetConfEntry();
    if ((conf.cluster == cluster) and (conf.pnode == node)) {
      res = pthread_mutex_unlock(&pnode_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");
      return &*it;
    }
  }

  res = pthread_mutex_unlock(&pnode_list_mutex);
  if (res != 0)
    throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");
  throw ExceptFatal(__FILE__, __LINE__);
}

int VMD::NClusters(void) {
  int nclusters = 0;

  int res = pthread_mutex_lock(&pnode_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  for (list<class PNode>::iterator it = pnode_list.begin();
       it != pnode_list.end();
       it++) {
    struct conf_entry conf = (*it).GetConfEntry();
    nclusters = MAX(nclusters, conf.cluster);
  }

  res = pthread_mutex_unlock(&pnode_list_mutex);
  if (res != 0)
    throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  return nclusters;
}

int VMD::NNodes(int cluster) {
  int nnodes = -1;

  int res = pthread_mutex_lock(&pnode_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  for (list<class PNode>::iterator it = pnode_list.begin();
       it != pnode_list.end();
       it++)
    if ((*it).GetConfEntry().cluster == cluster)
      nnodes = MAX(nnodes, (*it).GetConfEntry().pnode);

  res = pthread_mutex_unlock(&pnode_list_mutex);
  if (res != 0)
    throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  return nnodes + 1;
}

struct conf_entry VMD::FindConfEntry(int cluster, pnode_t pnode) {
  Acquire();

  for (int i = 0; i < conf_tab_len; i++)
    if ((conf_tab[i].cluster == cluster) and (conf_tab[i].pnode == pnode)) {
      struct conf_entry conf = conf_tab[i];
      Release();
      return conf;
    }

  Release();
  throw ExceptFatal(__FILE__, __LINE__);
}

const struct conf_entry *VMD::MyConfEntry(void) const {
  return my_conf_entry;
}

class Thread *VMD::CurrentThread(void) {
  return (class Thread *) pthread_getspecific(current_thread_key);
}

class CommunicatorThread *VMD::CurrentCommunicatorThread(void) {
  class CommunicatorThread *ret;

  ret = (class CommunicatorThread *)
    pthread_getspecific(current_communicator_thread_key);
  if (ret == NULL) throw ExceptFatal(__FILE__, __LINE__);

  return ret;
}

class PNode &VMD::AddNode(const struct conf_entry entry) {
  int res;

  class PNode n(entry);

  res = pthread_mutex_lock(&pnode_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  pnode_list.push_back(n);
  class PNode &pres = pnode_list.back();

  res = pthread_mutex_unlock(&pnode_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  return pres;
}

class PNode &VMD::AddNode(const struct conf_entry entry,
			  class EventSource *es) {
  int res;

  class PNode n(entry, es);

  res = pthread_mutex_lock(&pnode_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  pnode_list.push_back(n);
  class PNode &pres = pnode_list.back();

  res = pthread_mutex_unlock(&pnode_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  // Handle all_connected condition variable.

  res = pthread_mutex_lock(&all_connected_cmutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  all_connected++;

  res = pthread_cond_signal(&all_connected_cond);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_cond_signal");

  res = pthread_mutex_unlock(&all_connected_cmutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  return pres;
}

void VMD::ReadConf(const char *file, const char *my_name) {
  // We use a FILE instead of an ifstream, because istream::getline seems
  // to be corrupted... (segmentation during the first call to getline)
  FILE *f;
  int i;
  char line[256];
#define TOK_SIZE 16
  char *tok[TOK_SIZE];
  char *current;

  cerr << "parsing configuration file: " << file << "\n";

  f = fopen(file, "r");
  if (f == NULL) throw ExceptErrno(__FILE__, __LINE__, "fopen");
  do {
    fgets(line, sizeof line, f);
    if (ferror(f))
      throw ExceptFILE(__FILE__, __LINE__, f, "fgets", true);

    if (feof(f)) continue;

    tok[0] = current = line;

    for (i = 0; (current != NULL) and (i < TOK_SIZE);
	 i += (((current - tok[i]) != 1) ? 1 : 0))
      tok[i] = strsep(&current, "\t /");

    if (i < 7) continue;
    if (tok[0][0] == '#') continue;

    if (!strcasecmp(tok[0], "intercluster"))
      conf_tab[conf_tab_len].type = conf_entry::intercluster;
    else if (!strcasecmp(tok[0], "node"))
      conf_tab[conf_tab_len].type = conf_entry::node;
    else continue;

    conf_tab[conf_tab_len].name = tok[1];

    if (!strcasecmp(tok[1], my_name)) my_conf_entry = conf_tab + conf_tab_len;

    if (!strcasecmp(tok[2], "primary"))
      conf_tab[conf_tab_len].cluster = 0;
    else conf_tab[conf_tab_len].cluster = atoi(tok[2]);

    conf_tab[conf_tab_len].pnode = atoi(tok[3]);

    conf_tab[conf_tab_len].ip = tok[4];

    if (!strcasecmp(tok[5], "TCP")) {
      conf_tab[conf_tab_len].inside.type = conf_entry::TCP;
      conf_tab[conf_tab_len].inside.tcp.port = atoi(tok[6]);
    }
    else if (!strcasecmp(tok[5], "HSL")) {
      conf_tab[conf_tab_len].inside.type = conf_entry::HSL;
      conf_tab[conf_tab_len].inside.hsl.channel = atoi(tok[6]);
    }

    if (i == 9) {
      if (!strcasecmp(tok[7], "TCP")) {
	conf_tab[conf_tab_len].outside.type = conf_entry::TCP;
	conf_tab[conf_tab_len].outside.tcp.port = atoi(tok[8]);
      }
      else if (!strcasecmp(tok[7], "HSL")) {
	conf_tab[conf_tab_len].outside.type = conf_entry::HSL;
	conf_tab[conf_tab_len].outside.hsl.channel = atoi(tok[8]);
      }
    }

    cerr << "adding entry for host " << conf_tab[conf_tab_len].name << "\n";

    conf_tab_len++;
  } while ((conf_tab_len < CONF_TAB_LEN) and !feof(f));
  fclose(f);

  if (my_conf_entry == NULL)
    throw ExceptFatalMsg(__FILE__, __LINE__,
			 "can't find myself in the configuration file !");
}

void VMD::ConnectToNodes(void) {
  // Because we only connect to nodes that are after us in the
  // configuration file, we don't need to protect the creation
  // of each object : no collision (A wants to connect to B and
  // B wants to connect to A at the same time) can occur.

  // nconnections is the expected length of event_source_list.
  int res;
  int nconnections = -1; // start at -1 not to count ourself.
  bool retry;

  for (int index = 0; index < conf_tab_len; index++) {
    // If we're not an intercluster node,
    // don't try to connect to a node outside our cluster.
    if ((my_conf_entry->type == conf_entry::node) and
	(conf_tab[index].cluster != my_conf_entry->cluster)) continue;

    // If we're an intercluster node, don't try to connect
    // to external nodes that are not intercluster nodes.
    if ((my_conf_entry->type == conf_entry::intercluster) and
	(conf_tab[index].cluster != my_conf_entry->cluster) and
	(conf_tab[index].type == conf_entry::node)) continue;

    // This node may contact us or we may contact it.
    nconnections++;

    // Don't try to connect to a node before us.
    if (conf_tab + index <= my_conf_entry) continue;

    class PNode &node = AddNode(conf_tab[index]);
    cerr << "trying to connect to " << conf_tab[index].name << "\n";

    do {
      MainThreadYield();
      retry = false;

      try {
	node.Connect();
      }
#ifdef CATCH_BLOCK
#undef CATCH_BLOCK
#endif
#define CATCH_BLOCK { \
      exc.Perror();   \
      sleep(2);       \
      retry = true;   \
    }

      CATCH_ExceptFileDesc

      }
    while (retry == true);
  }

  // Wait for every class EventSource instances to be created and usabled
  // (usabled means affected to a PNode).

  cerr << "waiting for every event sources to be created\n";

  res = pthread_mutex_lock(&all_connected_cmutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

retry:
  MainThreadYield(&all_connected_cmutex);

  if (all_connected != nconnections) {
    res = pthread_cond_wait(&all_connected_cond, &all_connected_cmutex);
    if ((res != 0) and (res != EINTR))
      throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_wait");

    goto retry;
  }

  res = pthread_mutex_unlock(&all_connected_cmutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  // Create the class PNode instances for internal nodes of other
  // clusters.

  cerr << "creating the possibly remaining PNodes\n";

  // No need to protect conf_tab because beginning at this point,
  // this table stay unmodified.
  switch (my_conf_entry->type) {
  case conf_entry::node:
    for (int index = 0; index < conf_tab_len; index++) {
      if (conf_tab[index].cluster == my_conf_entry->cluster) continue;

      cerr << "adding node " << conf_tab[index].name << "\n";
      AddNode(conf_tab[index],
	      FindInterCluster(my_conf_entry->cluster).GetEventSource());
    }
    break;

  case conf_entry::intercluster:
    for (int index = 0; index < conf_tab_len; index++) {
      if (conf_tab[index].cluster == my_conf_entry->cluster) continue;
      if (conf_tab[index].type == conf_entry::intercluster) continue;

      cerr << "adding node " << conf_tab[index].name << "\n";
      AddNode(conf_tab[index],
	      FindInterCluster(conf_tab[index].cluster).GetEventSource());
    }
    break;
  }
}

void VMD::InitChannelManagerMap(void) {
  channel_manager_map_m.Acquire();
  
  for (int index = 0; index < conf_tab_len; index++) {
    ChannelManager *cm = new ChannelManager((pnode_t) index);

    if (conf_tab[index].cluster != my_conf_entry->cluster) continue;
    if (conf_tab[index].pnode <= my_conf_entry->pnode) continue;

    cerr << "Inserting a ChannelManager in the channel-managers map\n";
    map<pnode_t, ChannelManager *, less<pnode_t> >::value_type
      v(conf_tab[index].pnode, cm);
    if (channel_manager_map.insert(v).second == false) {
      channel_manager_map_m.Release();
      throw ExceptFatal(__FILE__, __LINE__);
    }
  }

  channel_manager_map_m.Release();
}

void VMD::DestroyChannelManagerMap(void) {
  channel_manager_map_m.Acquire();
  for (map<pnode_t, ChannelManager *, less<pnode_t> >::iterator
	 it = channel_manager_map.begin();
       it != channel_manager_map.end();
       it++) delete (*it).second;
  channel_manager_map_m.Release();
}

void VMD::MainThreadYield(void) {
  if (main_thread_interrupted == true)
    throw ExceptFatalMsg(__FILE__, __LINE__,
			 "main thread interrupted by signal");
}

void VMD::MainThreadYield(pthread_mutex_t *m) {
  if (main_thread_interrupted == true) {
    int res = pthread_mutex_unlock(m);
    if (res != 0) cerr << "MainThreadYield : mutex cannot be unlocked\n";

    throw ExceptFatalMsg(__FILE__, __LINE__,
			 "main thread interrupted by signal");
  }
}

void VMD::TestServiceMsg(int par1 = 0, int par2 = 0, int par3 = 0) {
  cerr << "****************************************\n";
  cerr << "TestServiceMsg called : (" << par1 << ", " << par2 << ", " <<
    par3 << ")\n";
}

int VMD::TestServiceRPC(int par1 = 0, int par2 = 0, int par3 = 0) {
  cerr << "****************************************\n";
  cerr << "TestServiceMsg called : (" << par1 << ", " << par2 << ", " <<
    par3 << ") - ";

  return par1;
}

void VMD::StartLoader(void) {
  int p[2];
  int ret;

  ret = pipe(p);
  if (ret < 0) {
    perror("pipe");
    exit(1);
  }

  if (!(loader_pid = fork())) {
    // Child process running here.
    close(0);
    dup(p[0]);
    close(p[0]);
    close(p[1]);
    execlp("loader.sh", "loader.sh", NULL);
    exit(1);
  }

  close(p[0]);
  loader_pipe = p[1];
}

void VMD::SpawnTask(const char *task, appclassname_t cn, uid_t uid) {
  int ret;
  char str[16];

  loader_pipe_m.Acquire();

  // Write uid.
  sprintf(str, "%d", uid);
  ret = write(loader_pipe, str, strlen(str));
  if (ret != (int) strlen(str)) throw ExceptFileLine(__FILE__, __LINE__);
  ret = write(loader_pipe, "\n", 1);
  if (ret != 1) throw ExceptFileLine(__FILE__, __LINE__);

  // Write mainclass.
  sprintf(str, "%#lx", cn);
  ret = write(loader_pipe, str, strlen(str));
  if (ret != (int) strlen(str)) throw ExceptFileLine(__FILE__, __LINE__);
  ret = write(loader_pipe, "\n", 1);
  if (ret != 1) throw ExceptFileLine(__FILE__, __LINE__);

  // Write task.
  ret = write(loader_pipe, task, strlen(task));
  if (ret != (int) strlen(task)) throw ExceptFileLine(__FILE__, __LINE__);
  ret = write(loader_pipe, "\n", 1);
  if (ret != 1) throw ExceptFileLine(__FILE__, __LINE__);

  loader_pipe_m.Release();
}

////////////////////////////////////////////////////////////

DeviceInterface::DeviceInterface() : dev_hsl_fd(-1) {}

DeviceInterface::~DeviceInterface() {}

void DeviceInterface::OpenDevice(void) {
  int res;

  res = open(HSL_DEVICE, O_RDWR);
  if (res < 0) {
    dev_hsl_fd = -1;
    perror("open /dev/hsl");
    cerr << "Warning: FastHSL driver could not be opened\n";
  } else {
    dev_hsl_fd = res;
    cerr << "FastHSL driver opened\n";
  }
}

void DeviceInterface::CloseDevice(void) {
  if (dev_hsl_fd != -1) close(dev_hsl_fd);
  dev_hsl_fd = -1;
}

bool DeviceInterface::ModuleLoaded(void) {
  return (dev_hsl_fd == -1) ? false : true;
}

void DeviceInterface::SetAppClass(appclassname_t name, uid_t uid) {
  int res;
  manager_set_appclassname_t msa;

  if (dev_hsl_fd == -1) throw ExceptFileLine(__FILE__, __LINE__);

  msa.cn  = name;
  msa.uid = uid;
  res = ioctl(dev_hsl_fd, MANAGERSETAPPCLASSNAME, &msa);
  if (res < 0) throw ExceptErrno(__FILE__, __LINE__, "ioctl");
}

int DeviceInterface::OpenChannel(pnode_t dest,
				 channel_t chan0, channel_t chan1,
				 appclassname_t cn, int protocol) {
  int res;
  manager_open_channel_t moc;

  if (dev_hsl_fd == -1) return ENODEV;

  cerr << "DeviceInterface::OpenChannel(dest=" << dest <<
    ", chan0=" << chan0 << ", chan1=" << chan1 << ", proto=" <<
    protocol << ")\n";

  moc.node      = dest;
  moc.chan0     = chan0;
  moc.chan1     = chan1;
  moc.classname = cn;
  moc.protocol  = protocol;
  res = ioctl(dev_hsl_fd, MANAGEROPENCHANNEL, &moc);
  if (res < 0) return errno;

  return 0;
}

int DeviceInterface::ShutdownChannel_1stStep(pnode_t dest,
					     channel_t chan0, channel_t chan1,
					     seq_t *ret_chan0_seqout,
					     seq_t *ret_chan0_seqin,
					     seq_t *ret_chan1_seqout,
					     seq_t *ret_chan1_seqin,
					     int protocol) {
  int res;
  manager_shutdown_1stStep_t ms1s;

  if (dev_hsl_fd == -1) return ENODEV;

  cerr << "DeviceInterface::ShutdownChannel_1stStep(dest=" << dest <<
    ", chan0=" << chan0 << ", chan1=" << chan1 << ", proto=" <<
    protocol << ")\n";

  ms1s.node     = dest;
  ms1s.chan0    = chan0;
  ms1s.chan1    = chan1;
  ms1s.protocol = protocol;

  res = ioctl(dev_hsl_fd, MANAGERSHUTDOWN1stSTEP, &ms1s);
  if (res < 0) return errno;

  *ret_chan0_seqout = ms1s.ret_seq_send_0;
  *ret_chan0_seqin  = ms1s.ret_seq_recv_0;
  *ret_chan1_seqout = ms1s.ret_seq_send_1;
  *ret_chan1_seqin  = ms1s.ret_seq_recv_1;

  return 0;
}


int DeviceInterface::ShutdownChannel_2ndStep(pnode_t dest,
					     channel_t chan0, channel_t chan1,
					     seq_t chan0_seqout,
					     seq_t chan0_seqin,
					     seq_t chan1_seqout,
					     seq_t chan1_seqin,
					     int protocol) {
  int res;
  manager_shutdown_2ndStep_t ms2s;

  cerr << "DeviceInterface::ShutdownChannel_2ndtStep(dest=" << dest <<
    ", chan0=" << chan0 << ", chan1=" << chan1 << ", proto=" <<
    protocol << ")\n";

  ms2s.node       = dest;
  ms2s.chan0      = chan0;
  ms2s.chan1      = chan1;
  ms2s.seq_send_0 = chan0_seqout;
  ms2s.seq_recv_0 = chan0_seqin;
  ms2s.seq_send_1 = chan1_seqout;
  ms2s.seq_recv_1 = chan1_seqin;
  ms2s.protocol   = protocol;

  res = ioctl(dev_hsl_fd, MANAGERSHUTDOWN2ndSTEP, &ms2s);
  if (res < 0) return errno;

  return 0;
}

int DeviceInterface::ShutdownChannel_3rdStep(pnode_t dest,
					     channel_t chan0, channel_t chan1,
					     int protocol) {
  int res;
  manager_shutdown_3rdStep_t ms3s;

  cerr << "DeviceInterface::ShutdownChannel_3rdStep(dest=" << dest <<
    ", chan0=" << chan0 << ", chan1=" << chan1 << ", proto=" <<
    protocol << ")\n";

  ms3s.node       = dest;
  ms3s.chan0      = chan0;
  ms3s.chan1      = chan1;
  ms3s.protocol   = protocol;

  res = ioctl(dev_hsl_fd, MANAGERSHUTDOWN3rdSTEP, &ms3s);
  if (res < 0) return errno;

  return 0;
}


////////////////////////////////////////////////////////////

EventDriver::EventDriver() {}

EventDriver::~EventDriver() {}

void *EventDriver::StartThread(void *) {
  int res;

  CurrentThread()->ExitYield();

  while (true) {
    res = pthread_mutex_lock(&VMDAllocator.scanning_thread_cmutex);
    if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  retry:
    if (CurrentThread()->ShouldExit() == true) {
      res = pthread_mutex_unlock(&VMDAllocator.scanning_thread_cmutex);
      if (res != 0) cerr << "pthread_mutex_unlock error\n";
      CurrentThread()->ExitYield();
    }

    if (VMDAllocator.scanning_thread != NULL) {
      res = pthread_cond_wait(&VMDAllocator.scanning_thread_cond,
			      &VMDAllocator.scanning_thread_cmutex);

      if ((res != 0) and (res != EINTR))
	throw ExceptFatalMsg(__FILE__, __LINE__, "pthread_cond_wait");

      goto retry;
    }

    VMDAllocator.scanning_thread = VMDAllocator.CurrentThread();

    res = pthread_mutex_unlock(&VMDAllocator.scanning_thread_cmutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

    class EventSource *es = WaitForEvent();

    // The network appears FIFO, even in case of hops, because ONLY
    // ONE THREAD runs in the sequence {WaitForEvent(), ManageEvent()}
    // until the ManageEvent() follows up the message to the next event
    // source in the path.

    // ManageEvent() may activate another scanning thread.
    if (es != NULL) ManageEvent(es);

    res = pthread_mutex_lock(&VMDAllocator.scanning_thread_cmutex);
    if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

    if (VMDAllocator.scanning_thread == VMDAllocator.CurrentThread()) {
      // ManageEvent() did not activated another scanning thread.

      VMDAllocator.scanning_thread = NULL;
      res = pthread_cond_signal(&VMDAllocator.scanning_thread_cond);
      if (res != 0) {
	res = pthread_mutex_unlock(&VMDAllocator.scanning_thread_cmutex);
	if (res != 0) cerr << "pthread_mutex_unlock error\n";
	throw ExceptFatal(__FILE__, __LINE__);
      }
    }

    res = pthread_mutex_unlock(&VMDAllocator.scanning_thread_cmutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  }
  return NULL;
}

// The class EventSource instance returned is locked.
// It MUST be unlocked very soon, otherwise it can prevent other
// threads to wait for events.
class EventSource *EventDriver::WaitForEvent(void) {
  class EventSource *es = NULL;
  fd_set fdset;
  int nfds;

//  VMDAllocator.CurrentThread()->Acquire();
//   cerr << "thread \"" << VMDAllocator.CurrentThread()->Name() <<
//     "\" waiting for events\n";
//  VMDAllocator.CurrentThread()->Release();

  int res = pthread_mutex_lock(&VMDAllocator.event_source_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "event_source_list");

  FD_ZERO(&fdset);
  nfds = 0;

  bool finish = false;
  for (list<class EventSource *>::iterator it =
	 VMDAllocator.event_source_list.begin();
       (it != VMDAllocator.event_source_list.end()) and (finish == false);
       it++) {
    es = *it;

    // If the event source is not in a good state, forget it.
    if (!*es) {
      es = NULL;
      continue;
    }

    try {
      es->Acquire();

      if (es->rdbuf()->_IO_read_end - es->rdbuf()->_IO_read_ptr > 0)
	finish = true;

      if (finish == false) {
	if (es->IsSelectable() == true) {
	  nfds = max(es->FileDescriptor(), nfds);
	  FD_SET(es->FileDescriptor(), &fdset);
	}

	es->Release();
      }
    }

#ifdef CATCH_BLOCK
#undef CATCH_BLOCK
#endif
#define CATCH_BLOCK {                                                        \
      es->Release();                                                         \
      int res = pthread_mutex_unlock(&VMDAllocator.event_source_list_mutex); \
      if (res != 0) cerr << "can't unlock mutex\n";                          \
      throw exc;                                                             \
    }

    CATCH_exception

    if (finish == false) es = NULL;
  }

  // Note that if es has been found, it is locked here.

  res = pthread_mutex_unlock(&VMDAllocator.event_source_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "event_source_list");

  // Did we found an EventSource instance that contains data ?
  if (es != NULL) return es;

  // We haven't found an EventSource...

  // We must wait an event if nfds is null, because some event sources
  // may not be selectable.
  // Only one thread can select() at a time, because of scanning_thread_cond.

  struct timeval timeout;
#ifdef WITH_FILEBUF
  timeout.tv_sec = 1;
#else
  timeout.tv_sec = 5;
#endif
  timeout.tv_usec = 0;

  res = select(nfds + 1, &fdset, NULL, NULL, &timeout);
  if (res < 0) throw ExceptErrno(__FILE__, __LINE__, "select");

  CurrentThread()->ExitYield();

  if (res == 0) FD_ZERO(&fdset);

  // Search for an EventSource instance that can be read.

  res = pthread_mutex_lock(&VMDAllocator.event_source_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "event_source_list");

  for (list<class EventSource *>::iterator it =
	 VMDAllocator.event_source_list.begin();
       it != VMDAllocator.event_source_list.end();
       it++) {
    es = *it;

    try {
      es->Acquire();

      if (es->IsSelectable() == true) {
	if (FD_ISSET(es->FileDescriptor(), &fdset)) break;
      } else {
	res = es->CanRead();
	if (res == -1) throw ExceptFatal(__FILE__, __LINE__);
	if (res == 1) break;
      }

      es->Release();
    }

#ifdef CATCH_BLOCK
#undef CATCH_BLOCK
#endif
#define CATCH_BLOCK {                                                        \
      es->Release();                                                         \
      int res = pthread_mutex_unlock(&VMDAllocator.event_source_list_mutex); \
      if (res != 0) cerr << "can't unlock mutex\n";                          \
      throw exc;                                                             \
    }

    CATCH_exception

    es = NULL;
  }

  // Note that if es has been found, it is locked here.

  res = pthread_mutex_unlock(&VMDAllocator.event_source_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "event_source_list");

  return es;
}

void EventDriver::ManageEvent(class EventSource *es) {
  int res, data_size, ret_val;
  struct msg_hdr_1 hdr1, resp_hdr1;
  struct msg_hdr_2 hdr2;
  class EventSource *dest_es = NULL;
  static char data[MAX_DATA_SIZE];

  if ((es != NULL) and (es == VMDAllocator.exit_pipe_event_source)) {
    char c;

    es->read(&c, sizeof c);
    es->Release();
    return;
  }

  // Read header 1.
  // Don't need to lock the event source : it's already done.

  es->read(&hdr1, sizeof hdr1);
  if (!*es) {
    es->Release();
    throw ExceptFatal(__FILE__, __LINE__);
  }

  if ((data_size = hdr1.size - sizeof hdr1 - sizeof hdr2) > MAX_DATA_SIZE) {
    es->Release();
    throw ExceptFatal(__FILE__, __LINE__);
  }

  // Examine header 1.

  // Do we need to route this message ?
  if ((VMDAllocator.MyConfEntry()->cluster != hdr1.dest_cluster) or
      (VMDAllocator.MyConfEntry()->pnode != hdr1.dest_node)) {

    // Read header 2 and data.
    es->read(&hdr2, sizeof hdr2);
    if (data_size > 0) es->read(data, data_size);

    // The whole packet is read, we can release the event source.
    es->Release();
    if (!*es) throw ExceptFatal(__FILE__, __LINE__);

    res = pthread_mutex_lock(&VMDAllocator.pnode_list_mutex);
    if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

    // Find next hop. // REMPLACER PAR FindPNode()
    for (list<class PNode>::iterator it = VMDAllocator.pnode_list.begin();
	 it != VMDAllocator.pnode_list.end();
	 it++) {
      struct conf_entry conf = (*it).GetConfEntry();
      if ((conf.cluster == hdr1.dest_cluster) and
	  (conf.pnode == hdr1.dest_node)) {
	dest_es = (*it).GetEventSource();
	break;
      }

      dest_es = NULL;
    }

    res = pthread_mutex_unlock(&VMDAllocator.pnode_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

    if (dest_es == NULL) throw ExceptFatal(__FILE__, __LINE__);

    // Forward message.
    dest_es->Acquire();

    dest_es->write(&hdr1, sizeof hdr1);
    dest_es->write(&hdr2, sizeof hdr2);
    if (data_size > 0) dest_es->write(data, data_size);
    dest_es->flush();

    dest_es->Release();

    if (!*dest_es) throw ExceptFatal(__FILE__, __LINE__);

  } else { // The message is for us.

    // Read header 2 and data.
    es->read(&hdr2, sizeof hdr2);

    if (hdr1.direction == msg_hdr_1::REQUEST) {
      // This is a REQUEST message.

      // Read data.
      if (data_size > 0) es->read(data, data_size);

      // The whole packet is read, we can release the event source.
      es->Release();
      if (!*es) throw ExceptFatal(__FILE__, __LINE__);

      // Since the whole packet is read, we MUST activate another
      // scanning thread, because we may soon wait for a while...
      res = pthread_mutex_lock(&VMDAllocator.scanning_thread_cmutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

      VMDAllocator.scanning_thread = NULL;
      res = pthread_cond_signal(&VMDAllocator.scanning_thread_cond);
      if (res != 0) {
	res = pthread_mutex_unlock(&VMDAllocator.scanning_thread_cmutex);
	if (res != 0) cerr << "pthread_mutex_unlock error\n";
	throw ExceptFatal(__FILE__, __LINE__);
      }

      res = pthread_mutex_unlock(&VMDAllocator.scanning_thread_cmutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");


      // Call the destination method.
      switch (hdr1.type) {
      case msg_hdr_1::MSG:
      case msg_hdr_1::RPC:
	ret_val = CallMethod(hdr2.dest_type, hdr2.dest_object,
			     hdr2.dest_method);
	break;

      case msg_hdr_1::MSG_ARG_INT:
      case msg_hdr_1::RPC_ARG_INT:
	if (data_size != sizeof(int)) throw ExceptFatal(__FILE__, __LINE__);
	ret_val = CallMethod(hdr2.dest_type, hdr2.dest_object,
			     hdr2.dest_method, 1, ((int *) data)[0]);
	break;

      case msg_hdr_1::MSG_ARG_INTx2:
      case msg_hdr_1::RPC_ARG_INTx2:
	if (data_size != 2*sizeof(int)) throw ExceptFatal(__FILE__, __LINE__);
	ret_val = CallMethod(hdr2.dest_type, hdr2.dest_object,
			     hdr2.dest_method, 2,
			     ((int *) data)[0], ((int *) data)[1]);
	break;

      case msg_hdr_1::MSG_ARG_INTx3:
      case msg_hdr_1::RPC_ARG_INTx3:
	if (data_size != 3*sizeof(int)) throw ExceptFatal(__FILE__, __LINE__);
	ret_val = CallMethod(hdr2.dest_type, hdr2.dest_object,
			     hdr2.dest_method, 3,
			     ((int *) data)[0], ((int *) data)[1],
			     ((int *) data)[2]);
	break;

      case msg_hdr_1::MSG_ARG_INTx4:
      case msg_hdr_1::RPC_ARG_INTx4:
	if (data_size != 4*sizeof(int)) throw ExceptFatal(__FILE__, __LINE__);
	ret_val = CallMethod(hdr2.dest_type, hdr2.dest_object,
			     hdr2.dest_method, 4,
			     ((int *) data)[0], ((int *) data)[1],
			     ((int *) data)[2], ((int *) data)[3]);
	break;

      case msg_hdr_1::MSG_ARG_VARARG:
      case msg_hdr_1::RPC_ARG_VARARG:
	long_msg_t lngmsg;
	lngmsg.size = data_size;
	lngmsg.data = new char [data_size];
	bcopy(data, lngmsg.data, data_size);
	ret_val = CallMethod(hdr2.dest_type, hdr2.dest_object, hdr2.dest_method,
			     VARARG_VIRTLEN, (int) &lngmsg);
	delete lngmsg.data;
	break;

      default:
	throw ExceptFatal(__FILE__, __LINE__);
      }

      // Do we need to send a RESPONSE ?
      if ((hdr1.type == msg_hdr_1::RPC) or
	  (hdr1.type == msg_hdr_1::RPC_ARG_INT) or
	  (hdr1.type == msg_hdr_1::RPC_ARG_INTx2) or
	  (hdr1.type == msg_hdr_1::RPC_ARG_INTx3) or
	  (hdr1.type == msg_hdr_1::RPC_ARG_INTx4) or
	  (hdr1.type == msg_hdr_1::RPC_ARG_VARARG)) {
	// Forward the response.

	cerr << "ManageEvent : WE FORWARD A RESPONSE to cluster " <<
	  hdr1.src_cluster << " node " << hdr1.src_node << "\n";

	// The routing algorithm makes symetric routes : route from A to B
	// takes the reverse path of route from B to A.
	// Thus, the response must be forwarded to the event source
	// that sent us the request.

	resp_hdr1.size = sizeof hdr1 + sizeof hdr2 + sizeof ret_val;
	resp_hdr1.type = hdr1.type;
	resp_hdr1.direction = msg_hdr_1::RESPONSE;
	resp_hdr1.wait_cond = hdr1.wait_cond;
	resp_hdr1.dest_node = hdr1.src_node;
	resp_hdr1.dest_cluster = hdr1.src_cluster;
	resp_hdr1.src_node = hdr1.dest_node;
	resp_hdr1.src_cluster = hdr1.dest_cluster;

	es->Acquire();

	es->write(&resp_hdr1, sizeof resp_hdr1);
	es->write(&hdr2, sizeof hdr2);
	es->write(&ret_val, sizeof ret_val);
	es->flush();

	es->Release();

	if (!*es)
	  throw ExceptStream(__FILE__, __LINE__, es, "event source", false);
      }

    } else { // This is a RESPONSE message.

      // The data area will be read by the destination thread.  We
      // mustn't release the event source : this will be done by the
      // destination thread after having read the response.

      if (hdr1.wait_cond != NULL) {
	res = pthread_cond_signal(hdr1.wait_cond);
	if (res != 0) {
	  es->Release();
	  throw ExceptFatal(__FILE__, __LINE__);
	}
      } else {
	es->Release();
	throw ExceptFatal(__FILE__, __LINE__);
      }
    }
  }
}

int EventDriver::CallMethod(enum type_ids type, caddr_t obj,
			    long long int method, int nparam = 0,
			    int par1 = 0, int par2 = 0,
			    int par3 = 0, int par4 = 0) {
  int ret;

  if (obj == NULL) {
    cerr << "Calling a method in constructor class " << type << "\n";

    switch (type) {
    case CLASS(VMD):
      obj = (caddr_t) &VMDAllocator;
      break;

    case DIST_ALLOC_CLASS(DistObject):
      obj = (caddr_t) &DistObjectAllocator;
      break;

    case DIST_ALLOC_CLASS(DistVMSpace):
      obj = (caddr_t) &DistVMSpaceAllocator;
      break;

    case DIST_ALLOC_CLASS(DistLock):
      obj = (caddr_t) &DistLockAllocator;
      break;

    case DIST_ALLOC_CLASS(DistController):
      obj = (caddr_t) &DistControllerAllocator;
      break;

      // Insert new classes here.

    default:
      cerr << "Unknown object type : " << type << "\n";
      throw ExceptFatal(__FILE__, __LINE__);
      /* NOTREACHED */
      break;
    }
  } else cerr << "Calling a method in object class " << type << "\n";

#define HANDLE_CLASS(X)                                     \
  case CLASS(X): {                                          \
    class X *call_obj;                                      \
    int (X::*call_method)(...);                             \
    *((caddr_t *) &call_obj) = obj;                         \
    *((long long int *) &call_method) = method;             \
    int res = pthread_mutex_lock(&VMDAllocator.garbage_coll_cmutex); \
    if (res != 0)                                                    \
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");   \
    call_obj->ref_cnt++;                                             \
    res = pthread_mutex_unlock(&VMDAllocator.garbage_coll_cmutex);   \
    if (res != 0)                                                    \
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock"); \
    switch (nparam) {                                       \
    case 0:                                                 \
      ret = (call_obj->*call_method)();                     \
      break;                                                \
    case 1:                                                 \
      ret = (call_obj->*call_method)(par1);                 \
      break;                                                \
    case 2:                                                 \
      ret = (call_obj->*call_method)(par1, par2);           \
      break;                                                \
    case 3:                                                 \
      ret = (call_obj->*call_method)(par1, par2, par3);     \
      break;                                                \
    case 4:                                                 \
      ret = (call_obj->*call_method)(par1, par2, par3, par4); \
      break;                                                \
    case VARARG_VIRTLEN:                                    \
      ret = (call_obj->*call_method)((long_msg_t *) par1);  \
     break;                                                 \
    default:                                                \
      throw ExceptFatal(__FILE__, __LINE__);                \
    }                                                       \
    res = pthread_mutex_lock(&VMDAllocator.garbage_coll_cmutex);     \
    if (res != 0)                                                    \
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");   \
    call_obj->ref_cnt--;                                             \
    res = pthread_mutex_unlock(&VMDAllocator.garbage_coll_cmutex);   \
    if (res != 0)                                                    \
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock"); \
    }                                                       \
    break;

  switch (type) {
    HANDLE_CLASS_BLOCK;

  case TYPE_IDS_LAST_TOKEN:
    throw ExceptFileLine(__FILE__, __LINE__);
    /* NOTREACHED */
    break;
  }

  return ret;
}

////////////////////////////////////////////////////////////

EventInitiator::EventInitiator() : sock(-1), uid(65535), pid(-1) {
  progname[0] = 0;
}

EventInitiator::~EventInitiator() {}

int EventInitiator::Read(char *buf, int size)
{
  char *p;
  int count;
  int res;

  p = buf;
  count = 0;

  while (count != size) {
    res = read(sock, p, size);

    if (res < 0 and res != EINTR)
      throw ExceptFileDesc(__FILE__, __LINE__, sock, "read", false);

    if (res < 0 and res == EINTR) {
      cerr << "read: EINTR\n";
      continue;
    }

    if (res == 0)
      throw ExceptFileDesc(__FILE__, __LINE__, sock, "read->EOF", false);

    /* Here, res > 0 */
    count += res;
    p     += res;
  }

  return size;
}

int EventInitiator::Write(char *buf, int size)
{
  char *p;
  int count;
  int res;

  p = buf;
  count = 0;

  while (count != size) {
    res = write(sock, p, size);

    if (res < 0 and res != EINTR)
      throw ExceptFileDesc(__FILE__, __LINE__, sock, "write", false);

    if (res < 0 and res == EINTR) {
      cerr << "write: EINTR\n";
      continue;
    }

    if (res == 0)
      throw ExceptFileDesc(__FILE__, __LINE__, sock, "write->0", false);

    /* Here, res > 0 */
    count += res;
    p     += res;
  }

  return size;
}

void *EventInitiator::StartThread(void *) {
  int serv_sock;
  pid_t this_pid;
  int res;
  char name[] = "/tmp/mpcauth.XXXX";
  struct stat name_info;

  for (;;) {
    CurrentThread()->ExitYield();

    cerr << "thread \"" << CurrentThread()->Name()
	 << "\" accepting local connection\n";
    serv_sock = accept(VMDAllocator.unixdomain_sock, NULL, NULL);
    if (serv_sock < 0)
      throw ExceptFileDesc(__FILE__, __LINE__, VMDAllocator.unixdomain_sock,
			   "accept", true);

    sock = -1;
    uid  = 65535;
    pid  = -1;
    progname[0] = 0;

    int flags = fcntl(serv_sock, F_GETFL, NULL);
    res = fcntl(serv_sock, F_SETFL, flags & ~O_NONBLOCK);
    if (res != 0) throw ExceptFileDesc(__FILE__, __LINE__, serv_sock,
				       "fcntl", true);

    if (mktemp(name) == NULL) throw ExceptFileLine(__FILE__, __LINE__);

    res = write(serv_sock, name, strlen(name));
    if (res < 0)
      throw ExceptFileDesc(__FILE__, __LINE__, serv_sock, "write", true);
    if (res != (int) strlen(name)) throw ExceptFileLine(__FILE__, __LINE__);

    res = read(serv_sock, &this_pid, sizeof this_pid);
    if (res < 0)
      throw ExceptFileDesc(__FILE__, __LINE__, serv_sock, "read", true);
    if (res != sizeof this_pid) throw ExceptFileLine(__FILE__, __LINE__);

    res = lstat(name, &name_info);
    if (res < 0) {
      perror("lstat");
      close(serv_sock);
      cerr << "Identification failed";
      continue;
    }

    res = unlink(name);
    if (res < 0) throw ExceptErrno(__FILE__, __LINE__, "unlink");

    cerr << "Connection from UID:" << name_info.st_uid <<
      " PID:" << this_pid << "\n";

    sock = serv_sock;
    uid  = name_info.st_uid;
    pid  = this_pid;
    strncpy(progname, "not registered", sizeof progname);
    progname[sizeof(progname) - 1] = '\0';
    
    try {
      while (true) {
	unsigned long command;
	char cmdbuf[1024];

	Read((char *) &command, sizeof command);

	if (IOCPARM_LEN(command) > sizeof cmdbuf)
	  throw ExceptFileDesc(__FILE__, __LINE__, sock,
			       "command size too big", false);

	if (command & IOC_IN) Read(cmdbuf, IOCPARM_LEN(command));

	cerr << "thread \"" << CurrentThread()->Name()
	     << "\" received command " << command << " from program ["
	     << progname << "]\n";

	switch (command) {
	case MPCMSGMYNAME:
	  cerr << "action required: MPCMSGMYNAME\n";
	  strncpy(progname, cmdbuf, sizeof progname);
	  progname[sizeof(progname) - 1] = '\0';
	  break;

	case MPCMSGMAKEAPPCLASS:
	  cerr << "action required: MPCMSGMAKEAPPCLASS\n";
	  *(appclassname_t *) cmdbuf =
	    VMDAllocator.controller->CreateAppClass(uid);
	  break;

	case MPCMSGGETCHANNEL:
	  cerr << "action required: MPCMSGGETCHANNEL\n";

	  appclass_info_t ac_info;

	  if ((((mpcmsg_get_channel_t *) cmdbuf)->mainclass < MIN_SUSER_APPCLASS or
	       ((mpcmsg_get_channel_t *) cmdbuf)->mainclass > MAX_SUSER_APPCLASS) and
	      uid != 0) {
	    res = VMDAllocator.controller->GetAppClassInfo(((mpcmsg_get_channel_t *) cmdbuf)->mainclass,
							   &ac_info);
	    if (res) {
	      ((mpcmsg_get_channel_t *) cmdbuf)->status = ENOENT;
	      break;
	    }

	    if (ac_info.uid != uid) {
	      ((mpcmsg_get_channel_t *) cmdbuf)->status = EPERM;
	      break;
	    }
	  }

	  if (VMDAllocator.MyConfEntry()->cluster !=
	      ((mpcmsg_get_channel_t *) cmdbuf)->subclass.controlled.prefnode_cluster) {
	    ((mpcmsg_get_channel_t *) cmdbuf)->status = EHOSTUNREACH;
	    break;
	  }

	  if (((mpcmsg_get_channel_t *) cmdbuf)->protocol != HSL_PROTO_SLRP_P and
	      ((mpcmsg_get_channel_t *) cmdbuf)->protocol != HSL_PROTO_SLRP_V and
	      ((mpcmsg_get_channel_t *) cmdbuf)->protocol != HSL_PROTO_SCP_P and
	      ((mpcmsg_get_channel_t *) cmdbuf)->protocol != HSL_PROTO_SCP_V and
	      ((mpcmsg_get_channel_t *) cmdbuf)->protocol != HSL_PROTO_MDCP) {
	    ((mpcmsg_get_channel_t *) cmdbuf)->status = EPROTONOSUPPORT;
	    break;
	  }

	  res = VMDAllocator.controller->GetChannel(((mpcmsg_get_channel_t *) cmdbuf)->mainclass,
						    ((mpcmsg_get_channel_t *) cmdbuf)->subclass,
						    ((mpcmsg_get_channel_t *) cmdbuf)->protocol,
						    &((mpcmsg_get_channel_t *) cmdbuf)->channel_pair_0,
						    &((mpcmsg_get_channel_t *) cmdbuf)->channel_pair_1);
	  ((mpcmsg_get_channel_t *) cmdbuf)->status = res;
	  break;

	case MPCMSGCLOSECHANNEL:
	  cerr << "action required: MPCMSGCLOSECHANNEL\n";

	  if ((((mpcmsg_close_channel_t *) cmdbuf)->mainclass < MIN_SUSER_APPCLASS or
	       ((mpcmsg_close_channel_t *) cmdbuf)->mainclass > MAX_SUSER_APPCLASS) and
	      uid != 0) {
	    res = VMDAllocator.controller->GetAppClassInfo(((mpcmsg_close_channel_t *) cmdbuf)->mainclass,
							   &ac_info);
	    if (res) {
	      ((mpcmsg_close_channel_t *) cmdbuf)->status = ENOENT;
	      break;
	    }

	    if (ac_info.uid != uid) {
	      ((mpcmsg_close_channel_t *) cmdbuf)->status = EPERM;
	      break;
	    }
	  }

	  if (VMDAllocator.MyConfEntry()->cluster !=
	      ((mpcmsg_close_channel_t *) cmdbuf)->subclass.controlled.prefnode_cluster) {
	    ((mpcmsg_close_channel_t *) cmdbuf)->status = EHOSTUNREACH;
	    break;
	  }

	  res = VMDAllocator.controller->CloseChannel(((mpcmsg_close_channel_t *) cmdbuf)->mainclass,
						      ((mpcmsg_close_channel_t *) cmdbuf)->subclass);
	  ((mpcmsg_close_channel_t *) cmdbuf)->status = res;
	  break;

	  case MPCMSGGETLOCALINFOS:
	    cerr << "action required: MPCMSGGETLOCALINFOS\n";

	  ((mpcmsg_get_local_infos_t *) cmdbuf)->cluster =
	    VMDAllocator.MyConfEntry()->cluster;

	  ((mpcmsg_get_local_infos_t *) cmdbuf)->pnode =
	    VMDAllocator.MyConfEntry()->pnode;

	  ((mpcmsg_get_local_infos_t *) cmdbuf)->nclusters =
	    VMDAllocator.NClusters();
	  break;

	case MPCMSGGETNODECOUNT:
	  cerr << "action required: MPCMSGGETNODECOUNT\n";

	  ((mpcmsg_get_node_count_t *) cmdbuf)->node_count =
	    VMDAllocator.NNodes(((mpcmsg_get_node_count_t *) cmdbuf)->cluster);
	  break;

	case MPCMSGSPAWNTASK:
	  cerr << "action required: MPCMSGSPAWNTASK\n";

	  if (uid != 0) {
	    res = VMDAllocator.controller->GetAppClassInfo(((mpcmsg_spawn_task_t *) cmdbuf)->mainclass,
							   &ac_info);
	    if (res) {
	      ((mpcmsg_spawn_task_t *) cmdbuf)->status = ENOENT;
	      break;
	    }

	    if (ac_info.uid != uid) {
	      ((mpcmsg_spawn_task_t *) cmdbuf)->status = EPERM;
	      break;
	    }
	  }

	  ((mpcmsg_spawn_task_t *) cmdbuf)->cmdline[CMDLINE_SIZE - 1] = '\0';
	  VMDAllocator.controller->SpawnTask(((mpcmsg_spawn_task_t *) cmdbuf)->cluster,
					     ((mpcmsg_spawn_task_t *) cmdbuf)->pnode,
					     ((mpcmsg_spawn_task_t *) cmdbuf)->cmdline,
					     ((mpcmsg_spawn_task_t *) cmdbuf)->mainclass,
					     uid);

	  ((mpcmsg_spawn_task_t *) cmdbuf)->status = 0;
	  break;

#if 0
	case MPCMSGACCEPTB:
	  cerr << "action required: MPCMSGACCEPTB\n";

	  ((mpcmsg_accept_t *) cmdbuf)->status =
	    VMDAllocator.channel_manager.
	    AcceptChannel(((mpcmsg_accept_t *) cmdbuf)->mainclass,
			  ((mpcmsg_accept_t *) cmdbuf)->subclass,
			  uid,
			  &((mpcmsg_accept_t *) cmdbuf)->uid,
			  &((mpcmsg_accept_t *) cmdbuf)->node,
			  &((mpcmsg_accept_t *) cmdbuf)->channel,
			  FALSE);
	  break;

	case MPCMSGACCEPTNB:
	  cerr << "action required: MPCMSGACCEPTNB\n";

	  break;
#endif

	default:
	  cerr << "unknown command\n";
	  break;
	}

	if (command & IOC_OUT) Write(cmdbuf, IOCPARM_LEN(command));
      }
    }

  // It's very important to call Perror because some exceptions
  // make a special cleaning action during Perror (for instance,
  // file descriptors may be closed during Perror).
#ifdef CATCH_BLOCK
#undef CATCH_BLOCK
#endif
#define CATCH_BLOCK {                                               \
	  exc.Perror();                                             \
    }

    CATCH_ExceptFileDesc

    cerr << "Closing connection from UID:" << name_info.st_uid <<
      " PID:" << this_pid << " progname:[" << progname << "]\n";

    shutdown(serv_sock, 2);
    close(serv_sock);
  }

  return NULL;
}

////////////////////////////////////////////////////////////

ChannelManager::ChannelManager(pnode_t n) : dist_node(n) {}

ChannelManager::~ChannelManager(void) {}

channel_info_t ChannelManager::GetChannel(appclassdef_t cd, int protocol) {
  int ret;
  channel_info_t ci;

  fprintf(stderr, "ChannelManager::GetChannel(appclassdef={%#x,%#x/%#x})\n",
	  (int) cd.first,
	  (int) (cd.second & 0xFFFFFFFF), (int) (cd.second>>32));

  // We want atomic operations : creation of the channel and reference of it
  // must be done atomically.
  Acquire();

  request_map_m.Acquire();

  if (request_map.count(cd) == 1) {
    // There is an already allocated channel pair for this appclassdef.

    ci = request_map[cd];
    request_map_m.Release();

    // Is this the same protocol ?
    if (ci.protocol != protocol) {
      ci.classdef.first = ~0UL;
      Release();
      return ci;
    }

    _RefChannel(cd);
    Release();
    return ci;

  } else {
    // We need to allocate a pair of channels for this appclassdef.
    channel_t chan;

    channel_map_m.Acquire();

    // Find a free pair of channels.
    for (chan = MIN_ALLOC_CHANNEL; chan < MAX_ALLOC_CHANNEL; chan++)
      if (channel_map.count(chan) == 0) break;
    if (chan >= MAX_ALLOC_CHANNEL - 1) {
      // No free channel available.
      ci.classdef.first = ~0UL;
      channel_map_m.Release();
      request_map_m.Release();
      Release();
      return ci;
    }
    if (channel_map.count(chan + 1) != 0) throw ExceptFatal(__FILE__, __LINE__);

    // Create a channel_info.
    ci.classdef = cd;
    ci.channel_pair_0 = chan;
    ci.channel_pair_1 = chan + 1;
    ci.protocol = protocol;

    // Open the channels.
    if (VMDAllocator.device.ModuleLoaded() == true) {
      ret = OpenChannel(ci.channel_pair_0, ci.channel_pair_1,
			ci.classdef.first, ci.protocol);
      if (ret) {
	ci.classdef.first = ~0UL;
	channel_map_m.Release();
	request_map_m.Release();
	Release();
	return ci;
      }
    }

    // Insert the channel_info in the maps.
    channel_map[chan]     = ci;
    channel_map[chan + 1] = ci;
    request_map[cd]       = ci;

    channel_map_m.Release();
    request_map_m.Release();

    _RefChannel(cd);
    Release();
    return ci;
  }
}

void ChannelManager::_RefChannel(appclassdef_t cd) {
  channel_info_t ci;

  request_map_m.Acquire();
  if (request_map.count(cd) == 0) throw ExceptFatal(__FILE__, __LINE__);
  ci = request_map[cd];
  request_map_m.Release();

  chanref_map_m.Acquire();

  if (chanref_map.count(ci.channel_pair_0) == 0)
    chanref_map[ci.channel_pair_0] = 1;
  else chanref_map[ci.channel_pair_0]++;

  if (chanref_map.count(ci.channel_pair_1) == 0)
    chanref_map[ci.channel_pair_1] = 1;
  else chanref_map[ci.channel_pair_1]++;

  chanref_map_m.Release();
}

int ChannelManager::_UnrefChannel(appclassdef_t cd) {
  channel_info_t ci;
  int ret;

  request_map_m.Acquire();
  if (request_map.count(cd) == 0) {
    cerr << "_UnrefChannel: unexistant channel\n";
    request_map_m.Release();
    return ENOENT;
  }

  ret = 0;

  ci = request_map[cd];
  request_map_m.Release();

  chanref_map_m.Acquire();
  if (chanref_map.count(ci.channel_pair_0) == 0)
    throw ExceptFatal(__FILE__, __LINE__);
  if (chanref_map.count(ci.channel_pair_1) == 0)
    throw ExceptFatal(__FILE__, __LINE__);

  chanref_map[ci.channel_pair_0]--;
  chanref_map[ci.channel_pair_1]--;

  if (chanref_map[ci.channel_pair_0] == 0) {
    if (chanref_map[ci.channel_pair_1] != 0)
      throw ExceptFatal(__FILE__, __LINE__);

    if (VMDAllocator.device.ModuleLoaded() == true)
      ret = ShutdownChannel(ci.channel_pair_0, ci.channel_pair_1,
			    ci.protocol);

    chanref_map.erase(ci.channel_pair_0);
    chanref_map.erase(ci.channel_pair_1);
    chanref_map_m.Release();

    request_map_m.Acquire();
    request_map.erase(cd);
    request_map_m.Release();

    channel_map_m.Acquire();
    channel_map.erase(ci.channel_pair_0);
    channel_map.erase(ci.channel_pair_1);
    channel_map_m.Release();
  } else chanref_map_m.Release();

  return ret;
}

int ChannelManager::OpenChannel(channel_t chan0, channel_t chan1,
				appclassname_t classname, int protocol) {
  int ret;
  pnode_t node0, node1;

  node0 = VMDAllocator.MyConfEntry()->pnode;
  node1 = dist_node;

  cerr << "ChannelManager::OpenChannel(chan0=" << chan0 <<
    ", chan1=" << chan1 << ", node0=" << node0 << ", node1=" << node1 << ")\n";

  ret = VMDAllocator.controller->OpenChannel(node0, node1, chan0, chan1,
					     classname, protocol);
  if (ret) {
    errno = ret;
    perror_cerr_errno(__FILE__, __LINE__, "OpenChannel");
    return ret;
  }

  return 0;
}

int ChannelManager::ShutdownChannel(channel_t chan0, channel_t chan1,
				    int protocol) {
  int ret;
  pnode_t node0, node1;
  seq_t seq0in, seq0out, seq1in, seq1out;

  node0 = VMDAllocator.MyConfEntry()->pnode;
  node1 = dist_node;

  cerr << "ChannelManager::ShutdownChannel(chan0=" << chan0 <<
    ", chan1=" << chan1 << ", node0=" << node0 << ", node1=" << node1 << ")\n";

  // 1st step.
  ret = VMDAllocator.controller->ShutdownChannel_1stStep(node0, node1,
							 chan0, chan1,
							 &seq0out, &seq0in,
							 &seq1out, &seq1in,
							 protocol);
  if (ret) {
    errno = ret;
    perror_cerr_errno(__FILE__, __LINE__, "ShutdownChannel_1stStep");
    return ret;
  }
  cerr << "ShutdownChannel : 1st step done [seq0out=" << seq0out <<
    ", seq0in=" << seq0in << ", seq1out=" << seq1out << ", seq1in=" <<
    seq1in << "]\n";

  // 2nd step.
  ret = VMDAllocator.controller->ShutdownChannel_2ndStep(node0, node1,
							 chan0, chan1,
							 seq0out, seq0in,
							 seq1out, seq1in,
							 protocol);
  if (ret) {
    errno = ret;
    perror_cerr_errno(__FILE__, __LINE__, "ShutdownChannel_2ndStep");
    return ret;
  }
  cerr << "ShutdownChannel : 2nd step done\n";

  // loop on the 3rd step.
  do {
    ret = VMDAllocator.controller->ShutdownChannel_3rdStep(node0, node1,
							   chan0, chan1,
							   protocol);

    if (ret == EAGAIN) {
      cerr << "Shutdown not completed -> waiting 2 seconds\n";
      sleep(2);
    }
  } while (ret == EAGAIN);

  if (ret) {
    errno = ret;
    perror_cerr_errno(__FILE__, __LINE__, "ShutdownChannel_3rdStep");
    return ret;
  }

  cerr << "ShutdownChannel : 3rd step done\n" <<
    "ShutdownChannel : shutdown completed\n";

  return 0;
}

void ChannelManager::RefChannel(appclassdef_t cd) {
  Acquire();
 _RefChannel(cd);
  Release();
}

int ChannelManager::UnrefChannel(appclassdef_t cd) {
  int ret;

  Acquire();
  ret = _UnrefChannel(cd);
  Release();

  return ret;
}

void ChannelManager::Dump(void) {
  cerr << "Dumping channel map:\n";
  channel_map_m.Acquire();
  for (channel_map_t::iterator it = channel_map.begin();
       it != channel_map.end();
       it++) {
    cerr << "channel_map[chan=" << (*it).first << "] = " <<
      (*it).second.channel_pair_0 << "\n";
  }
  channel_map_m.Release();

  cerr << "Dumping chanref map:\n";
  chanref_map_m.Acquire();
  for (chanref_map_t::iterator it = chanref_map.begin();
       it != chanref_map.end();
       it++) {
    cerr << "chanref_map[chan=" << (*it).first << "] = " <<
      (*it).second << "\n";
  }
  chanref_map_m.Release();
}

#if 0
void AcceptChannel(appclassname_t mainclass, appsubclassname_t subclass,
		   uid_t local_uid, uid_t *ret_uid, pnode_t *ret_node,
		   channel_t *ret_channel, bool non_blocking) {
  int res;
  request_t req, matching_req;
  appclassdef_t cldef;
  channel_t channel;
  pthread_mutex_t request_cmutex;
  pthread_cond_t request_cond;

  cldef.mainclass = mainclass;
  cldef.subclass  = subclass;

  request_lists_m.Acquire();

  // If the request is not already present, insert it in the list.
  res = accept_request_list.count(cldef);
  if (!res) {
    req.classdef        = cldef;
    req.uid             = local_uid;
    req.request_cmutex  = NULL;
    req.request_cond    = NULL;
    req.answer_ret      = FALSE;
    accept_request_list[cldef] = req;
  } else req = accept_request_list[cl];

  // Do we have a response to our request ?
  if (req.answer_ret == TRUE) {
    // Yes, we got a response.

    // Write the results.
    *ret_uid     = req.remote_uid_ret;
    *ret_node    = req.remote_node_ret;
    *ret_channel = req.channel_ret;

    // Exit.
    request_lists_m.Release();
    return;
  }

  // We don't have any response.

  // Check if a matching entry is in the other list.
  res = connect_request_list.count(cldef);
  if (res) {
    // Yes, we found a matching entry.

    matching_req = connect_request_list[cldef];

    // Allocate a channel.
    channel = GetChannel(req.remote_node_ret);

    // Fill in the matching entry.
    matching_req.remote_uid_ret  = local_uid;
    matching_req.remote_node_ret = VMDAllocator.MyConfEntry()->pnode;
    matching_req.channel_ret     = channel;
    matching_req.answer_ret      = TRUE;

    connect_request_list[cldef] = matching_req;

    // Does this entry belong to a waiting thread ?
    if (matching_req.request_cmutex != NULL) {
      // Yes, wake-up the waiting thread.
      pthread_mutex_lock(matching_req.request_cmutex);
      pthread_cond_signal(matching_req.request_cond);
      pthread_mutex_unlock(matching_req.request_cmutex);
    }

    // Write results.
    *ret_uid     = req.remote_uid_ret;
    *ret_node    = req.remote_node_ret;
    *ret_channel = channel;

    // Exit.
    request_lists_m.Release();
    return;
  }

  // We didn't find any matching entry.

  // Do we want to wait for a response ?
  if (non_blocking == FALSE) {
    // Yes, we want to wait.

    res = pthread_mutex_init(&request_cmutex, NULL);
    if (res != 0) throw ExceptFatal(__FILE__, __LINE__);

    res = pthread_cond_init(&request_cond, NULL);
    if (res != 0) throw ExceptFatal(__FILE__, __LINE__);

    req.request_cmutex = &request_cmutex;
    req.request_cond   = &request_cond;

    accept_request_list[cldef] = req;

    res = pthread_mutex_lock(&request_cmutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

    request_lists_m.Release();

    res = pthread_cond_wait(&request_cond, &request_cmutex);
    if ((res != 0) and (res != EINTR)) throw ExceptFatal(__FILE__, __LINE__);

    // We got a response !

    // Read the response.
    req = accept_request_list[cl];

    // Write results.
    *ret_uid     = req.remote_uid_ret;
    *ret_node    = req.remote_node_ret;
    *ret_channel = req.channel_ret;

    res = pthread_mutex_unlock(&request_cmutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

    // Exit.
    request_lists_m.Release();
    return;
  }

  // We don't want to wait.

  // Exit.
  request_lists_m.Release();
  return;
}
#endif

////////////////////////////////////////////////////////////

Server::Server(const bool i) : inside(i) {}

Server::~Server() {}

void *Server::StartThread(void *) {
  AcceptConnections();
  return NULL;
}

void Server::AcceptConnections(void) {
  int cluster;
  pnode_t pnode;
  int res, s, flag, serv_sock, remote_len, port;
  struct protoent *pent;
  struct sockaddr_in saddr, remote_addr;

  if (inside == true) {
    if (VMDAllocator.MyConfEntry()->inside.type == conf_entry::HSL)
      throw ExceptFatalMsg(__FILE__, __LINE__,
			   "HSL transport not yet implemented");
    port = VMDAllocator.MyConfEntry()->inside.tcp.port;
  } else {
    if (VMDAllocator.MyConfEntry()->outside.type == conf_entry::HSL)
      throw ExceptFatalMsg(__FILE__, __LINE__,
			   "HSL transport not yet implemented");
    port = VMDAllocator.MyConfEntry()->outside.tcp.port;
  }

  pent = getprotobyname("tcp");
  if (pent == NULL) throw ExceptErrno(__FILE__, __LINE__, "getprotobyname");

  s = socket(PF_INET, SOCK_STREAM, pent->p_proto);
  if (s < 0) throw ExceptErrno(__FILE__, __LINE__, "socket");

  flag = 1;
  res = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof flag);
  if (res < 0)
    throw ExceptFileDesc(__FILE__, __LINE__, s, "setsockopt", true);

  bzero(&saddr, sizeof saddr);
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_port = htons(port);
  saddr.sin_len = sizeof saddr;

  res = bind(s, (struct sockaddr *) &saddr, sizeof saddr);
  if (res < 0) throw ExceptFileDesc(__FILE__, __LINE__, s, "bind", true);

  res = listen(s, BACKLOG);
  if (res < 0) throw ExceptFileDesc(__FILE__, __LINE__, s, "listen", true);

  for (;;) {
    VMDAllocator.CurrentThread()->ExitYield();

    remote_len = sizeof remote_addr;
    serv_sock = accept(s, (sockaddr *) &remote_addr, &remote_len);
    if (serv_sock < 0)
      throw ExceptFileDesc(__FILE__, __LINE__, s, "accept", true);

    cerr << "remote connection established\n";

    int flags = fcntl(serv_sock, F_GETFL, NULL);
    res = fcntl(serv_sock, F_SETFL, flags & ~O_NONBLOCK);
    if (res != 0) throw ExceptFileDesc(__FILE__, __LINE__, serv_sock,
				       "fcntl", true);

    // We want an iostream managing serv_sock. We should use an fstream
    // like that : class fstream *stream = new fstream(serv_sock);
    // but a bug in libc_r or g++ forces us to use a stdiobuf
    // (a core dump occurs while in stream->getline() if using an fstream).
    // Extra overhead is performed by an iostream, comparing to an fstream
    // (according to info libg++).

#if defined WITH_STDIOBUF || ! defined WITH_FILEBUF
    FILE *file_sock = fdopen(serv_sock, "r+");
    if (file_sock == NULL)
      throw ExceptFileDesc(__FILE__, __LINE__, s, "fdopen", true);

  // Should be replaced by class streambuf *...
    class stdiobuf *stdbuffer = new stdiobuf(file_sock);
#else // WITH_FILEBUF
    class streambuf *stdbuffer = new filebuf(serv_sock);
#endif

    class EventSource *event_source = new FDEventSource(stdbuffer);

    // Don't need to protect this access, because no other
    // thread knows about this iostream.
    event_source->read(&cluster, sizeof cluster);
    event_source->read(&pnode, sizeof pnode);

    if (!*event_source)
      throw
	ExceptStream(__FILE__, __LINE__, event_source, "event_source", true);

    VMDAllocator.event_source_list.push_back(event_source);
    VMDAllocator.AddNode(VMDAllocator.FindConfEntry(cluster, pnode),
			 event_source);
  }
}

////////////////////////////////////////////////////////////

void treat_sigint(int)
{
  cerr << "signal SIGINT catched\n";
  if (pthread_getspecific(VMDAllocator.current_thread_key) == NULL)
    VMDAllocator.main_thread_interrupted = true;
}

void treat_sigabrt(int)
{
  cerr << "signal SIGABRT catched\n";
  // This would be a not so bad way to terminate the thread by calling
  // pthread_exit() but it cannot be done from within a signal handler
  // (with libc_r at least). Throwing an exception too.
  // Returning from this signal, the current system call (if there is
  // one) should be terminated and EINTR should be returned. This
  // should terminate the thread by throwing an exception.
  // Another concurrent method to kill the thread is to set
  // Thread::should_exit to true and send a signal. The thread is
  // responsible to scan this variable "frequently" by calling
  // Thread::ExitYield() (the signal is needed to terminate the
  // current blocking system call).  For this method to run correctly,
  // two blocking system calls must never be used without ExitYield()
  // between them.
}

#define NEW_HANDLER_MSG "memory exhausted in \"new\".\n"

void treat_new(void)
{
  // Don't use fprintf because it may call an allocation subroutine.
  write(2, NEW_HANDLER_MSG, sizeof NEW_HANDLER_MSG);

  // Don't call exit_vmd() because it will call destructors that
  // may call an allocation subroutine. Don't delete threads
  // for the same reason. Just exit right now.
  // Instead of exiting, we could throw a memory exception...
  _exit(64);
}

void usage(char *name)
{
  printf("usage: %s [-c conffile] [-n hostname] [-k basekey]\n", name);
  exit(1);
}

int main(const int argc, char *const *argv, const char ** /* env */)
{
  char evtdrvstr[] = "event driver thread XXXXXXXXXX";
  char evtinitstr[] = "event initiator thread XXXXXXXXXX";
  class EventDriver *event_driver;
  class EventInitiator *event_initiator;
  char opt_conf_file[OPT_STRING_SIZE] = CONF_FILE;
  char opt_my_name[OPT_STRING_SIZE];
  int opt_basekey = MSQ_BASEKEY;
  struct sockaddr saddr;
  int res, ret;
  char ch;
  class CommunicatorThread *comm_thr_p;

  // Start the loader (spawn daemon), before activating any other thread.
  VMDAllocator.StartLoader();

  // Protect us from lack of memory.
  set_new_handler(treat_new);

  // Register signal handlers.
  signal(SIGINT,  treat_sigint);
  signal(SIGABRT, treat_sigabrt);

  cerr << "vmd starting in mode [";
#ifdef WITH_STDIOBUF
  cerr << "buffering with stdiobuf]\n";
#elif defined WITH_FILEBUF
  cerr << "buffering with filebuf]\n";
#else
  cerr << "direct access (no buffering)]\n";
#endif

  set_terminate(terminate_on_exception);
  set_unexpected(unexpected_exception);

  try {
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    res = pthread_sigmask(SIG_UNBLOCK, &sigs, NULL);
    if (res != 0) throw ExceptErrno(__FILE__, __LINE__, "pthread_sigmask");

    // Examine the arguments.
    ret = gethostname(opt_my_name, sizeof(opt_my_name) - 1);
    if (ret < 0) throw ExceptErrno(__FILE__, __LINE__, "gethostname");
    opt_my_name[sizeof(opt_my_name) - 1] = '\0';

    while ((ch = getopt(argc, argv, "c:n:k:")) != EOF)
      switch (ch) {
      case 'c':
	strncpy(opt_conf_file, optarg, sizeof(opt_conf_file) - 1);
	opt_conf_file[sizeof(opt_conf_file) - 1] = '\0';
	break;

      case 'n':
	strncpy(opt_my_name, optarg, sizeof(opt_my_name) - 1);
	opt_my_name[sizeof(opt_my_name) - 1] = '\0';
	break;

      case 'k':
	opt_basekey = atoi(optarg);
	break;

      case '?':
      default:
	usage(argv[0]);
      };

    VMDAllocator.msq_basekey = opt_basekey;

    VMDAllocator.ReadConf(opt_conf_file, opt_my_name);

    VMDAllocator.device.OpenDevice();

#ifdef WITH_COMMAND_LINE_THREAD
    // Start the command line thread.
    class CommunicatorThread thr_command("command line thread",
					 &VMDAllocator.command_line, NULL);

    res = pthread_mutex_lock(&VMDAllocator.communicator_thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

    VMDAllocator.communicator_thread_list.push_back(thr_command);
    comm_thr_p = &VMDAllocator.communicator_thread_list.back();

    res = pthread_mutex_unlock(&VMDAllocator.communicator_thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

    res = pthread_mutex_lock(&VMDAllocator.thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

    VMDAllocator.thread_list.push_back(thr_command);

    VMDAllocator.thread_list.back().
      SetCommunicator(comm_thr_p);
    VMDAllocator.thread_list.back().Start();

    res = pthread_mutex_unlock(&VMDAllocator.thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");
#endif

    // Start the internal server thread.
    class Thread thr_int_server("internal server thread",
				&VMDAllocator.internal_server, NULL);

    res = pthread_mutex_lock(&VMDAllocator.thread_list_mutex);
    if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

    VMDAllocator.thread_list.push_back(thr_int_server);
    VMDAllocator.thread_list.back().Start();

    res = pthread_mutex_unlock(&VMDAllocator.thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

    // Start the external server thread if we act as an intercluster node.
    if (VMDAllocator.MyConfEntry()->type == conf_entry::intercluster) {
      class Thread thr_ext_server("external server thread",
				  &VMDAllocator.external_server, NULL);

      res = pthread_mutex_lock(&VMDAllocator.thread_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

      VMDAllocator.thread_list.push_back(thr_ext_server);
      VMDAllocator.thread_list.back().Start();

      res = pthread_mutex_unlock(&VMDAllocator.thread_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");
    }

    // Connect to the distant nodes and create PNode instances.
    cerr << "Connect to distant nodes\n";
    VMDAllocator.ConnectToNodes();

    // Initialize the channel_manager_map.
    cerr << "Initialize the channel_manager_map\n";
    VMDAllocator.InitChannelManagerMap();

    // Start the event driver threads.
    cerr << "starting " << N_EVENT_DRIVER_THREADS << " event driver threads\n";

    for (int i = 0; i < N_EVENT_DRIVER_THREADS; i++) {
      event_driver = new EventDriver;
      sprintf(evtdrvstr, "event driver thread %d", i);
      class CommunicatorThread thread(evtdrvstr, event_driver, NULL);

      res = pthread_mutex_lock(&VMDAllocator.event_driver_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

      VMDAllocator.event_driver_list.push_back(event_driver);

      res = pthread_mutex_unlock(&VMDAllocator.event_driver_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

      res = pthread_mutex_lock(&VMDAllocator.communicator_thread_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

      VMDAllocator.communicator_thread_list.push_back(thread);
      comm_thr_p = &VMDAllocator.communicator_thread_list.back();

      res = pthread_mutex_unlock(&VMDAllocator.communicator_thread_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

      res = pthread_mutex_lock(&VMDAllocator.thread_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

      VMDAllocator.thread_list.push_back(thread);

      cerr << "starting event driver thread number " << i << "\n";

      VMDAllocator.thread_list.back().
	SetCommunicator(comm_thr_p);
      VMDAllocator.thread_list.back().Start();

      res = pthread_mutex_unlock(&VMDAllocator.thread_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");
    }

    // Initialize the incoming message queue.
    VMDAllocator.msq_incoming = msgget(VMDAllocator.msq_basekey, IPC_CREAT);
    if (VMDAllocator.msq_incoming < 0)
      throw ExceptErrno(__FILE__, __LINE__, "msgget");
    res = msgctl(VMDAllocator.msq_incoming, IPC_RMID, NULL);
    if (res) throw ExceptErrno(__FILE__, __LINE__, "msgctl");
    VMDAllocator.msq_incoming = msgget(VMDAllocator.msq_basekey,
				       IPC_CREAT | IPC_EXCL | 0622);
    if (VMDAllocator.msq_incoming < 0)
      throw ExceptErrno(__FILE__, __LINE__, "msgget");

    // Initialize the unix-domain listening socket.
    VMDAllocator.unixdomain_sock = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (VMDAllocator.unixdomain_sock < 0)
      throw ExceptErrno(__FILE__, __LINE__, "socket");

    bzero(&saddr, sizeof saddr);
    saddr.sa_family = AF_UNIX;
    saddr.sa_len = sizeof saddr;
    sprintf(saddr.sa_data, "/tmp/mpc-%d", VMDAllocator.msq_basekey);

    res = unlink(saddr.sa_data);
    if (res < 0 and errno != ENOENT)
      throw ExceptErrno(__FILE__, __LINE__, "unlink");

    res = bind(VMDAllocator.unixdomain_sock, &saddr, sizeof saddr);
    if (res < 0) throw ExceptFileDesc(__FILE__, __LINE__,
				      VMDAllocator.unixdomain_sock, "bind", true);

    res = listen(VMDAllocator.unixdomain_sock, BACKLOG);
    if (res < 0) throw ExceptFileDesc(__FILE__, __LINE__,
				      VMDAllocator.unixdomain_sock, "listen", true);

    res = chmod(saddr.sa_data, 0777);
    if (res < 0) throw ExceptErrno(__FILE__, __LINE__, "chmod");

    // Start an initializer thread to instanciate the DistController
    // distributed object everywhere, and do the last things before the
    // main job...
    // We can't do the initialization from the main thread, because
    // it is not a communicator thread, then no remote invocation
    // is possible.

    class CommunicatorThread thr_initializer("initializer thread",
					     &VMDAllocator.initializer,
					     NULL);

    res = pthread_mutex_lock(&VMDAllocator.communicator_thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

    VMDAllocator.communicator_thread_list.push_back(thr_initializer);
    comm_thr_p = &VMDAllocator.communicator_thread_list.back();

    res = pthread_mutex_unlock(&VMDAllocator.communicator_thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

    res = pthread_mutex_lock(&VMDAllocator.thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

    VMDAllocator.thread_list.push_back(thr_initializer);

    VMDAllocator.thread_list.back().
      SetCommunicator(comm_thr_p);
    VMDAllocator.thread_list.back().Start();

    res = pthread_mutex_unlock(&VMDAllocator.thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

    // Start the event initiator threads.
    cerr << "starting " << N_EVENT_INITIATOR_THREADS <<
      " event initiator threads\n";

    for (int i = 0; i < N_EVENT_INITIATOR_THREADS; i++) {
      event_initiator = new EventInitiator;
      sprintf(evtinitstr, "event initiator thread %d", i);
      class CommunicatorThread thread(evtinitstr, event_initiator, NULL);

      res = pthread_mutex_lock(&VMDAllocator.event_initiator_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

      VMDAllocator.event_initiator_list.push_back(event_initiator);

      res = pthread_mutex_unlock(&VMDAllocator.event_initiator_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

      res = pthread_mutex_lock(&VMDAllocator.communicator_thread_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

      VMDAllocator.communicator_thread_list.push_back(thread);
      comm_thr_p = &VMDAllocator.communicator_thread_list.back();

      res = pthread_mutex_unlock(&VMDAllocator.communicator_thread_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

      res = pthread_mutex_lock(&VMDAllocator.thread_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

      VMDAllocator.thread_list.push_back(thread);

      cerr << "starting event initiator thread number " << i << "\n";

      VMDAllocator.thread_list.back().
	SetCommunicator(comm_thr_p);
      VMDAllocator.thread_list.back().Start();

      res = pthread_mutex_unlock(&VMDAllocator.thread_list_mutex);
      if (res != 0)
	throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");
    }

    // We wait for a signal (SIGINT), to enter the termination phase.
    pause();

    // Start an finalizer thread to destroy the DistController
    // distributed object everywhere, and do the last things before the
    // final shutdown...
    // We can't destroy the DistController from the main thread, because
    // it is not a communicator thread, then no remote invocation
    // is possible.

    cerr << "starting finalizer thread\n";

    class CommunicatorThread thr_finalizer("finalizer thread",
					   &VMDAllocator.finalizer,
					   NULL);

    res = pthread_mutex_lock(&VMDAllocator.communicator_thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

    VMDAllocator.communicator_thread_list.push_back(thr_finalizer);
    comm_thr_p = &VMDAllocator.communicator_thread_list.back();

    res = pthread_mutex_unlock(&VMDAllocator.communicator_thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

    res = pthread_mutex_lock(&VMDAllocator.thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

    VMDAllocator.thread_list.push_back(thr_finalizer);

    VMDAllocator.thread_list.back().
      SetCommunicator(comm_thr_p);
    VMDAllocator.thread_list.back().Start();

    res = pthread_mutex_unlock(&VMDAllocator.thread_list_mutex);
    if (res != 0)
      throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

    // We wait for the end of the finalizer thread.
    {
      sleep(1); // Wait for the thread to start -- should not be needed...

      void *ret_value;
      cerr << "trying to join thread \"finalizer thread\"\n";
      res = pthread_join(thr_finalizer.ThreadID(), &ret_value);
      if (res != 0) cerr << "pthread_join: error " << errno << "\n";
      cerr << "join successful\n";
    }
  }

  // We use a ref to exc to handle classes whose ExceptFileLine is
  // a base class, and we want Perror defined in the original class
  // to be called, not Perror in the base class (for the same aim,
  // Perror is a virtual function). For example, we handle ExceptFileDesc
  // exceptions here too.

  // It's very important to call Perror because some exceptions
  // make a special cleaning action during Perror (for instance,
  // file descriptors may be closed during Perror).
#ifdef CATCH_BLOCK
#undef CATCH_BLOCK
#endif
#define CATCH_BLOCK {                                               \
    exc.Perror();                                                   \
  }

  CATCH_ExceptFileLine

  catch (exception &exc) {
    cerr << "exception type        = ";
    cerr_typeid(exc);
    cerr << "\n";
    cerr << "exception description = " << exc.what() << "\n";
  }

  // We start here the destruction process.
  VMDAllocator.finishing = true;

  // We ignore SIGINT because pthread_join can be interrupted
  // by signals and if so, we could start exiting before all
  // threads having exited (that usually leads to a segmentation fault).
  signal(SIGINT, SIG_IGN);

  cerr << "*** KILLING ALL THREADS - FIRST STAGE ***\n";
  cerr << "-> change status of threads to \"should exit\"\n";

  // After locking the thread list, no thread can be created (because
  // it must be inserted in the list before starting).
  // Note that we make use of a signal to ask the thread to exit. It
  // would be a much better way to call pthread_cancel() but it
  // doesn't seem to be provided by libc_r...
  res = pthread_mutex_lock(&VMDAllocator.thread_list_mutex);
  if (res != 0) cerr << "pthread_mutex_lock: error";

  for (list<class Thread>::iterator it = VMDAllocator.thread_list.begin();
       it != VMDAllocator.thread_list.end();
       it++) {
    // We can't lock the Thread because we could be blocked,
    // but we should lcok because the access to Name should be
    // protected (access to a ref to a string must be protected
    // because of the reference count...) - we could some day
    // implement a lock for the thread name itself...
    cerr << "trying to cancel thread \"" << (*it).Name() << "\"\n";

    // Ask the thread to commit suicide in case of the signal wouldn't
    // be sufficient...
    (*it).should_exit = true;

    // This usleed needed due to a probably race condition in libc_r.
    usleep(1000);
  }

  cerr << "*** KILLING ALL THREADS - SECOND STAGE ***\n";
  cerr << "-> wake-up sleeping threads waiting for the pipe event source\n";
  res = write(VMDAllocator.exit_pipe, "X", 1);
  if (res < 0) cerr << "write error\n";

  cerr << "*** KILLING ALL THREADS - THIRD STAGE ***\n";
  cerr << "-> wake-up sleeping threads waiting for an event source\n";

  res = pthread_mutex_lock(&VMDAllocator.scanning_thread_cmutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_lock");

  res = pthread_cond_broadcast(&VMDAllocator.scanning_thread_cond);
  if (res != 0)
    throw ExceptMutex(__FILE__, __LINE__, "pthread_cond_broadcast");

  res = pthread_mutex_unlock(&VMDAllocator.scanning_thread_cmutex);
  if (res != 0) cerr << "pthread_mutex_unlock error\n";

  cerr << "*** KILLING ALL THREADS - FOURTH STAGE ***\n";
  cerr << "-> killing and joining threads\n";

  for (list<class Thread>::iterator it = VMDAllocator.thread_list.begin();
       it != VMDAllocator.thread_list.end();
       it++) {
    void *ret_value;
    pthread_t thr = (*it).ThreadID();

    // An error can occur if the thread has just been canceled for
    // any reason, but it's not really an error.

    cerr << "trying to kill thread \"" << (*it).Name() << "\"\n";

    res = pthread_kill(thr, SIGABRT);
    if (res != 0)
      cerr << "pthread_kill() cannot be done (thread has probably "
	   << "already exited or been canceled)\n";

    // This usleed needed due to a probably race condition in libc_r.
    usleep(1000);

    cerr << "trying to join thread \"" << (*it).Name() << "\"\n";
    res = pthread_join(thr, &ret_value);
    if (res != 0) cerr << "pthread_join: error\n";

    // This usleed needed due to a probably race condition in libc_r.
        usleep(1000);
  }    

  res = pthread_mutex_unlock(&VMDAllocator.thread_list_mutex);
  if (res != 0) throw ExceptMutex(__FILE__, __LINE__, "pthread_mutex_unlock");

  // Destroy the channel_manager_map.
  cerr << "Destroy the channel_manager_map\n";
  VMDAllocator.DestroyChannelManagerMap();

  VMDAllocator.device.CloseDevice();

  exit_vmd(0);
}

