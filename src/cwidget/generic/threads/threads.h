// threads.h                                              -*-c++-*-
//
//   Copyright (C) 2005-2009 Daniel Burrows
//
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of
//   the License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; see the file COPYING.  If not, write to
//   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//   Boston, MA 02111-1307, USA.
//
// A simple thread wrapper library.  I'm not using the existing ones
// in order to keep aptitude's dependency count low (as long as I
// don't need too much out of it, this should be fairly
// simple..right?).  The API was inspired by that of boost::threads.

#ifndef THREADS_H
#define THREADS_H

#include <errno.h>
#include <cwidget/generic/util/exception.h>

namespace cwidget
{
  /** \brief C++ wrappers for the POSIX threading primitives.
   *
   *  These provide Resource-Allocation-Is-Initialization semantics
   *  and are loosely modelled on the Boost threads library.
   */
  namespace threads
  {
    /** The base class for all thread-related exceptions. */
    class ThreadException : public util::Exception
    {
    };

    /** Thrown when thread creation fails; according to
     *  pthread_create(3), this only occurs if there aren't enough
     *  system resources to create a thread.
     */
    class ThreadCreateException : public ThreadException
    {
      int errnum;
    public:
      ThreadCreateException(int error)
	: errnum(error)
      {
      }

      int get_errnum() const { return errnum; }

      std::string errmsg() const;
    };

    /** Thrown when thread::join fails. */
    class ThreadJoinException : public ThreadException
    {
      std::string reason;

      int errnum;
    public:
      ThreadJoinException(const int error);

      int get_errnum() const { return errnum; }
      std::string errmsg() const;
    };

    /** \brief Thrown when the mutex being used to wait on a condition
     *  is not locked.
     *
     *  The mutex must be locked to preserve atomicity.
     */
    class ConditionNotLockedException : public ThreadException
    {
    public:
      std::string errmsg() const;
    };

    /** \brief Thrown when an error-checking mutex is locked twice. */
    class DoubleLockException : public ThreadException
    {
    public:
      std::string errmsg() const;
    };

    /** \brief A system thread.
     *
     *  This class represents a single thread of control.  It is
     *  conceptually based on the Boost thread class; like the Boost
     *  thread class, it is non-copyable.
     */
    class thread
    {
      pthread_t tid;
      bool joined;

      thread(const thread &other);
      thread &operator=(const thread &other);



      template<typename F>
      static void *bootstrap(void *p)
      {
	F thunk(*((F *) p));

	delete ((F *) p);

	thunk();

	return 0;
      }

    public:
      /** \brief Stores the attributes with which a thread is to be
       * created.
       *
       *  This wraps pthread_attr_t.  To use this class, create an
       *  instance and then invoke the set_* methods to set the
       *  desired attributes.  Pass the final attributes structure to
       *  thread::thread.
       */
      class attr
      {
	pthread_attr_t attrs;

	friend class thread;
      public:
	attr()
	{
	  pthread_attr_init(&attrs);
	}

	~attr()
	{
	  pthread_attr_destroy(&attrs);
	}
      };

      /** \brief Create a new thread.
       *
       *  The new thread will begin execution by calling operator() on
       *  a copy of the given function object.
       *
       *  \param thunk a function object of no parameters that will
       *  be invoked to start this thread.  Must be copyable.
       *
       *  \param a the attributes with which to create the new thread.
       */
      template<typename F>
      thread(const F &thunk, const attr &a = attr())
	:joined(false)
      {
	// Create a thunk on the heap to pass to the new thread.
	F *tmp = new F(thunk);

	if(pthread_create(&tid, &a.attrs, &thread::bootstrap<F>, tmp) != 0)
	  {
	    int errnum = errno;

	    delete tmp;

	    throw ThreadCreateException(errnum);
	  }
      }

      ~thread()
      {
	if(!joined)
	  pthread_detach(tid);
      }

      /** Wait for this thread to finish. */
      void join()
      {
	int rval = pthread_join(tid, NULL);

	if(rval != 0)
	  throw ThreadJoinException(rval);
	else
	  joined = true;
      }

      /** Cancel this thread. */
      void cancel()
      {
	pthread_cancel(tid);
      }
    };

    /** \brief Wrap noncopyable objects to bootstrap threads.
     *
     *  Stores a reference to a noncopyable nullary function object in
     *  a structure that is suitable as a bootstrap function for a
     *  thread.
     *
     *  The contained object is assumed to last for as long as the
     *  thread does, and will not be (automatically) deleted or
     *  destroyed when the thread terminates.
     *
     *  \param F   the functor type that this structure wraps.
     */
    template<typename F>
    struct noncopy_bootstrap
    {
      F &f;
    public:
      /** \brief Create a noncopyable bootstrap wrapper.
       *
       *  \param _f   the function object to wrap.
       */
      noncopy_bootstrap(F &_f)
	:f(_f)
      {
      }

      /** \brief Invoke F::operator() on the wrapped object. */
      void operator()()
      {
	f();
      }
    };

    class condition;

    // The mutex abstraction
    class mutex
    {
    public:
      class lock;
      class try_lock;

    private:
      pthread_mutex_t m;

      friend class lock;
      friend class try_lock;

      // Conditions need to look inside mutexes and locks to find the
      // real mutex object so the underlying thread library can do an
      // atomic unlock-and-wait.
      friend class condition;

      mutex(const mutex &other);
      mutex &operator=(const mutex &other);
    public:
      /** A mutex attributes object. */
      class attr
      {
	pthread_mutexattr_t attrs;

	friend class mutex;

      public:
	attr()
	{
	  pthread_mutexattr_init(&attrs);
	}

	attr(int kind)
	{
	  pthread_mutexattr_init(&attrs);
	  pthread_mutexattr_settype(&attrs, kind);
	}

	~attr()
	{
	  pthread_mutexattr_destroy(&attrs);
	}

	int settype(int kind)
	{
	  return pthread_mutexattr_settype(&attrs, kind);
	}

	int gettype()
	{
	  int rval;
	  pthread_mutexattr_gettype(&attrs, &rval);
	  return rval;
	}
      };

      /** Represents a lock on a mutex.  Can be released and re-asserted
       *  as desired; when the lock goes out of scope, it will
       *  automatically be released if necessary.
       */
      class lock
      {
	mutex &parent;

	bool locked;

	friend class condition;

	lock(const lock &other);
	lock &operator=(const lock &other);
      public:
	lock(mutex &_parent)
	  :parent(_parent), locked(false)
	{
	  acquire();
	}

	/** Lock the associated mutex. */
	void acquire()
	{
	  if(locked)
	    throw DoubleLockException();

	  pthread_mutex_lock(&parent.m);
	  locked = true;
	}

	/** Unlock the associated mutex. */
	void release()
	{
	  pthread_mutex_unlock(&parent.m);
	  locked = false;
	}

	bool get_locked() const
	{
	  return locked;
	}

	~lock()
	{
	  if(locked)
	    pthread_mutex_unlock(&parent.m);
	}
      };

      /** Represents a non-blocking lock on a mutex. */
      class try_lock
      {
	mutex &parent;

	bool locked;

	friend class condition;

	try_lock(const try_lock &other);
	try_lock &operator=(const try_lock &other);
      public:
	try_lock(mutex &_parent)
	  :parent(_parent)
	{
	  acquire();
	}

	~try_lock()
	{
	  if(locked)
	    pthread_mutex_unlock(&parent.m);
	}

	void acquire()
	{
	  if(locked)
	    throw DoubleLockException();

	  locked = pthread_mutex_trylock(&parent.m);
	}

	void release()
	{
	  pthread_mutex_unlock(&parent.m);
	  locked = false;
	}

	bool get_locked() const
	{
	  return locked;
	}
      };

      mutex()
      {
	pthread_mutex_init(&m, NULL);
      }

      mutex(const attr &a)
      {
	pthread_mutex_init(&m, &a.attrs);
      }

      ~mutex()
      {
	pthread_mutex_destroy(&m);
      }
    };

    /** A mutex that is initialized to be recursive.  Used because
     *  apparently C++ global constructors can't take arguments.
     */
    class recursive_mutex : public mutex
    {
    public:
      recursive_mutex()
	:mutex(attr(PTHREAD_MUTEX_RECURSIVE))
      {
      }
    };

    /** A abstraction over conditions.  When a condition variable is
     *  destroyed, any threads that are still blocked on it are woken
     *  up.
     */
    class condition
    {
      pthread_cond_t cond;
    public:
      condition()
      {
	pthread_cond_init(&cond, NULL);
      }

      ~condition()
      {
	// Wakey wakey
	pthread_cond_broadcast(&cond);
	pthread_cond_destroy(&cond);
      }

      void wake_one()
      {
	pthread_cond_signal(&cond);
      }

      void wake_all()
      {
	pthread_cond_broadcast(&cond);
      }

      /** Wait with the given guard (should be a lock type that is a
       *  friend of this condition object).
       *
       *  This is a cancellation point.  If the thread is cancelled
       *  while waiting on the condition, the mutex will be unlocked.
       */
      template<typename Lock>
      void wait(const Lock &l)
      {
	if(!l.get_locked())
	  throw ConditionNotLockedException();

	pthread_cleanup_push((void (*)(void*))pthread_mutex_unlock, &l.parent.m);
	pthread_cond_wait(&cond, &l.parent.m);
	pthread_cleanup_pop(0);
      }

      /** Wait until the given predicate returns \b true.
       *
       *  This is a cancellation point.  If the thread is cancelled
       *  while waiting on the condition, the mutex will be unlocked.
       *  This does not apply to the predicate; it is responsible for
       *  cleaning up the mutex itself if the thread is cancelled
       *  while it is running.
       */
      template<typename Lock, typename Pred>
      void wait(const Lock &l, Pred p)
      {
	if(!l.get_locked())
	  throw ConditionNotLockedException();

	while(!p())
	  wait(l);
      }

      /** Wait until either the condition is signalled or until the
       *  given time.
       *
       *  This is a cancellation point.  If the thread is cancelled
       *  while waiting on the condition, the mutex will be unlocked.
       *  This does not apply to the predicate; it is responsible for
       *  cleaning up the mutex itself if the thread is cancelled
       *  while it is running.
       *
       *  \param l the guard of the condition
       *  \param until the time at which the wait should terminate
       *
       *  \return \b true if the condition occurred or \b false if time
       *                  ran out.
       */
      template<typename Lock>
      bool timed_wait(const Lock &l, const timespec &until)
      {
	if(!l.get_locked())
	  throw ConditionNotLockedException();

	int rval;

	pthread_cleanup_push((void(*)(void *))&pthread_mutex_unlock, &l.parent.m);
	while((rval = pthread_cond_timedwait(&cond, &l.parent.m, &until)) == EINTR)
	  ;
	pthread_cleanup_pop(0);

	return rval != ETIMEDOUT;
      }

      /** Wait either until the condition is signalled while the given
       *  predicate is \b true or until the given time.
       *
       *  This is a cancellation point.  If the thread is cancelled
       *  while waiting on the condition mutex will be unlocked.  If
       *  the thread is cancelled while invoking the predicate, no
       *  guarantees are made by this routine; if the predicate
       *  invokes a cancellation point, it is responsible for pushing
       *  a cleanup handler.
       */
      template<typename Lock, typename Pred>
      bool timed_wait(const Lock &l, const timespec &until, const Pred &p)
      {
	if(!l.get_locked())
	  throw ConditionNotLockedException();

	while(!p())
	  {
	    if(!timed_wait(l, until))
	      return false;
	  }

	return true;
      }
    };

    /** A higher-level abstraction borrowed from Concurrent Haskell,
     *  which borrowed it from another language I forget.  This
     *  represents a "box" that can either hold a value or be empty.
     *  Any thread can take the current value of the box or place a new
     *  value inside it; the attempt will block until a value is
     *  available or the box is empty, respectively.  It's sort of a
     *  single-element bounded communications channel.
     *
     *  The value in the box is stored with copying semantics.  Like the
     *  other threading primitives, boxes are not copyable.
     */
    template<typename T>
    class box
    {
      T val;
      bool filled;

      condition cond;
      mutex m;

      box(const box &other);
      box &operator=(const box &other);
    public:
      /** Create an empty box. */
      box()
	:filled(false)
      {
      }

      /** Create a box containing the given value. */
      box(const T &_val)
	:val(_val), filled(true)
      {
      }

      /** Retrieve the current value of this box.  If the box is empty,
       *  block until it is full.
       */
      T take();

      /** Fill this box with a value.  If the box is full, block until
       *  it is empty.
       */
      void put(const T &t);

      /** If there is a value in the box, retrieve it immediately;
       *  otherwise do nothing.
       *
       *  \param out the location in which the value should be stored
       *  \return \b true iff a value was found in the box
       */
      bool try_take(T &out);

      /** If the box is empty, place a value in it; otherwise, do
       *  nothing.
       *
       *  \param t the value to place in the box
       *
       *  \return \b true iff the box was empty (and hence was filled
       *  with t)
       */
      bool try_put(const T &t);

      /** As try_take(), but wait for the given amount of time before
       *  giving up.
       */
      bool timed_take(T &out, const timespec &until);

      /** As try_put(), but wait for the given amount of time before
       *  giving up.
       */
      bool timed_put(const T &t, const timespec &until);

      /** Atomically modify the contents of the box; if an exception is
       *  thrown by the given function object, no action will be
       *  performed.
       */
      template<typename Mutator>
      void update(const Mutator &m);
    };

    /** A box specialized for 'void'; may make it easier to write
     *  other templated classes.  Could maybe just be a mutex, but I
     *  don't think you can quite mimic the box API that way.
     */
    template<>
    class box<void>
    {
      bool filled;
      mutex m;
      condition cond;
    public:
      box()
	:filled(false)
      {
      }

      box(bool _filled)
	:filled(_filled)
      {
      }

      void take();

      void put();

      bool try_take();
      bool try_put();

      bool timed_take(const timespec &until);
      bool timed_put(const timespec &until);

      template<typename Mutator>
      void update(const Mutator &m)
      {
	take();
	try
	  {
	    m();
	  }
	catch(...)
	  {
	    put();
	    throw;
	  }

	put();
      }
    };

    /** Internal helper struct. */
    struct bool_ref_pred
    {
      const bool &b;
    public:
      bool_ref_pred(const bool &_b)
	:b(_b)
      {
      }

      bool operator()() const
      {
	return b;
      }
    };

    /** Internal helper struct. */
    struct not_bool_ref_pred
    {
      const bool &b;
    public:
      not_bool_ref_pred(const bool &_b)
	:b(_b)
      {
      }

      bool operator()() const
      {
	return !b;
      }
    };

    template<typename T>
    inline
    T box<T>::take()
    {
      mutex::lock l(m);

      cond.wait(l, bool_ref_pred(filled));

      filled = false;

      // Interesting question: does l get released before or after the
      // copy?  To be safe, I explicitly copy before I return.
      T rval = val;
      return rval;
    }

    inline
    void box<void>::take()
    {
      mutex::lock l(m);
      cond.wait(l, bool_ref_pred(filled));
      filled = false;
    }

    template<typename T>
    inline
    bool box<T>::try_take(T &out)
    {
      mutex::lock l(m);

      if(filled)
	{
	  filled = false;
	  out = val;
	  return true;
	}
      else
	return false;
    }

    inline
    bool box<void>::try_take()
    {
      mutex::lock l(m);

      if(filled)
	{
	  filled = false;
	  return true;
	}
      else
	return false;
    }

    template<typename T>
    inline
    bool box<T>::timed_take(T &out, const timespec &until)
    {
      mutex::lock l(m);

      if(cond.timed_wait(l, until, bool_ref_pred(filled)))
	{
	  filled = false;
	  out = val;
	  return true;
	}
      else
	return false;
    }

    inline
    bool box<void>::timed_take(const timespec &until)
    {
      mutex::lock l(m);

      if(cond.timed_wait(l, until, bool_ref_pred(filled)))
	{
	  filled = false;
	  return true;
	}
      else
	return false;
    }

    template<typename T>
    inline
    void box<T>::put(const T &new_val)
    {
      mutex::lock l(m);

      cond.wait(l, not_bool_ref_pred(filled));

      filled = true;
      val = new_val;
      cond.wake_one();
    }

    inline
    void box<void>::put()
    {
      mutex::lock l(m);

      cond.wait(l, not_bool_ref_pred(filled));

      filled = true;
      cond.wake_one();
    }

    template<typename T>
    inline
    bool box<T>::try_put(const T &new_val)
    {
      mutex::lock l(m);

      if(!filled)
	{
	  filled = true;
	  val = new_val;
	  cond.wake_one();
	  return true;
	}
      else
	return false;
    }

    inline
    bool box<void>::try_put()
    {
      mutex::lock l(m);

      if(!filled)
	{
	  filled = true;
	  cond.wake_one();
	  return true;
	}
      else
	return false;
    }

    template<typename T>
    inline
    bool box<T>::timed_put(const T &new_val, const timespec &until)
    {
      mutex::lock l(m);

      if(cond.timed_wait(l, until, not_bool_ref_pred(filled)))
	{
	  filled = true;
	  val = new_val;
	  cond.wake_one();
	  return true;
	}
      else
	return false;
    }

    inline
    bool box<void>::timed_put(const timespec &until)
    {
      mutex::lock l(m);

      if(cond.timed_wait(l, until, not_bool_ref_pred(filled)))
	{
	  filled = true;
	  cond.wake_one();
	  return true;
	}
      else
	return false;
    }

    template<typename T>
    template<typename Mutator>
    inline
    void box<T>::update(const Mutator &m)
    {
      mutex::lock l(m);

      cond.wait(l, bool_ref_pred(filled));

      T new_val = m(val);

      val = new_val;
      cond.wake_one();
    }

    // A utility that proxies for noncopyable thread bootstrap
    // objects.  The only requirement is that the pointer passed
    // to the constructor must not be destroyed until the thread
    // completes.
    template<typename F>
    class bootstrap_proxy
    {
      F *f;
    public:
      bootstrap_proxy(F *_f)
	: f(_f)
      {
      }

      void operator()() const
      {
	(*f)();
      }
    };

    template<typename F>
    bootstrap_proxy<F> make_bootstrap_proxy(F *f)
    {
      return bootstrap_proxy<F>(f);
    }
  }
}

#endif // THREADS_H

