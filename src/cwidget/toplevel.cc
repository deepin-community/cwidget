// toplevel.cc
//
//  Copyright 1999-2005, 2007-2009, 2011 Daniel Burrows
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.
//
//  Implementation of the core toplevel logic for cwidget (main loop,
//  signal handling, etc).

#include "toplevel.h"

#include "curses++.h"
#include "style.h"

#include <cwidget/widgets/widget.h>

#include <cwidget/generic/util/transcode.h>

#include <cwidget/widgets/editline.h>
#include <cwidget/widgets/menu.h>
#include <cwidget/widgets/menubar.h>
#include <cwidget/widgets/pager.h>
#include <cwidget/widgets/statuschoice.h>
#include <cwidget/widgets/table.h>
#include <cwidget/widgets/text_layout.h>
#include <cwidget/widgets/tree.h>

#include <config/keybindings.h>

#include <cwidget/generic/threads/event_queue.h>
#include <cwidget/generic/threads/threads.h>

#include <cwidget/generic/util/ssprintf.h>

#include <cwidget/generic/util/i18n.h>

#include <limits.h>
#include <signal.h>
#include <unistd.h>

#include <cwidget/generic/util/eassert.h>
#include <sys/time.h>

#include <map>

#include <fcntl.h>
#include <sys/types.h>

namespace cwidget
{
  std::string version()
  {
    return PACKAGE_VERSION;
  }

  using namespace config;
  using namespace widgets;

  namespace toplevel
  {
    static threads::recursive_mutex cwidget_mutex;

    inline
    threads::mutex &get_mutex()
    {
      return cwidget_mutex;
    }

    static int suspend_count;
    static threads::mutex suspend_count_mutex;

    inline
    int get_suspend_count()
    {
      threads::mutex::lock l(suspend_count_mutex);
      return suspend_count;
    }

    inline
    void inc_suspend_count()
    {
      threads::mutex::lock l(suspend_count_mutex);
      ++suspend_count;
    }

    sigc::signal0<void> main_hook;


    static threads::event_queue<event *> eventq;

    using namespace std;

    static bool curses_avail = false;
    static bool should_exit  = false;

    static bool suspended_with_signals = false;
    static struct sigaction oldsigcont, oldsigtstp;

    // Used to queue and merge update requests
    //
    // The global event queue isn't used for this so that
    // update(); tryupdate() works as desired.  However,
    // threading magic is used to ensure that background threads can post
    // update requests.
    struct update_state
    {
      bool layout;
      bool update;
      bool cursorupdate;

      update_state()
	:layout(false), update(false), cursorupdate(false)
      {
      }
    };

    threads::recursive_mutex pending_updates_mutex;
    update_state pending_updates;


    event::~event()
    {
    }

    void slot_event::dispatch()
    {
      the_slot();
    }

    static widget_ref toplevel = NULL;
    // The widget which is displayed as the root of everything

    // Cleanly shutdown (eg, restore screen settings if possible)
    //
    // Called on SIGTERM, SIGINT, SIGSEGV, SIGABRT, and SIGQUIT
    //
    // FIXME: revert to the /previous/ handler, not just SIG_DFL?
    static void sigkilled(int sig)
    {
      endwin();

      switch(sig)
	{
	case SIGTERM:
	  fprintf(stderr, _("Ouch!  Got SIGTERM, dying..\n"));
	  break;
	case SIGSEGV:
	  fprintf(stderr, _("Ouch!  Got SIGSEGV, dying..\n"));
	  break;
	case SIGABRT:
	  fprintf(stderr, _("Ouch!  Got SIGABRT, dying..\n"));
	  break;
	case SIGQUIT:
	  fprintf(stderr, _("Ouch!  Got SIGQUIT, dying..\n"));
	  break;
	}

      signal(sig, SIG_DFL);
      raise(sig);
    }

    ///////////////////////////////////////////////////////////////////////////////
    // The following function comes from the glibc documentation.
    // Subtract the `struct timeval' values X and Y,
    // storing the result in RESULT.
    // Return 1 if the difference is negative, otherwise 0.

    static int
    timeval_subtract (timeval *result, timeval *x, timeval *y)
    {
      /* Perform the carry for the later subtraction by updating Y. */
      if (x->tv_usec < y->tv_usec)
	{
	  int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
	  y->tv_usec -= 1000000 * nsec;
	  y->tv_sec += nsec;
	}
      if (x->tv_usec - y->tv_usec > 1000000)
	{
	  int nsec = (x->tv_usec - y->tv_usec) / 1000000;
	  y->tv_usec += 1000000 * nsec;
	  y->tv_sec -= nsec;
	}

      /* Compute the time remaining to wait.
	 `tv_usec' is certainly positive. */
      result->tv_sec = x->tv_sec - y->tv_sec;
      result->tv_usec = x->tv_usec - y->tv_usec;

      /* Return 1 if result is negative. */
      return x->tv_sec < y->tv_sec;
    }
    ///////////////////////////////////////////////////////////////////////////////

    widget_ref settoplevel(const widget_ref &w)
    {
      if(toplevel.valid())
	toplevel->unfocussed();

      widget_ref oldw = toplevel;

      toplevel = w;

      if(curses_avail)
	{
	  toplevel->set_owner_window(rootwin, 0, 0, rootwin.getmaxx(), rootwin.getmaxy());
	  toplevel->show_all();
	  toplevel->focussed();
	  redraw();
	}

      return oldw;
    }

    //////////////////////////////////////////////////////////////////////
    void post_event(event *ev)
    {
      eventq.put(ev);
    }

    int write(int fd, const char *s)
    {
      return ::write(fd, s, strlen(s));
    }

    //////////////////////////////////////////////////////////////////////
    // Event management threads

    /** This thread is responsible for posting wget_wch() calls.
     *
     *  Note that the actual call to wget_wch must take place in the
     *  foreground thread, because wget_wch will invoke wrefresh().
     *  So instead of calling it in the background thread, I post
     *  input events to the foreground thread.
     *
     *  To prevent the background thread from spamming the foreground
     *  thread with events, I suspend it until the event actually
     *  triggers.
     */
    class input_thread
    {
      class fatal_input_exception : public util::Exception
      {
	int err;
      public:
	fatal_input_exception(int _err)
	  : err(_err)
	{
	}

	std::string errmsg() const
	{
	  return "Unable to read from stdin: " + util::sstrerror(err);
	}
      };

      class fatal_input_error : public event
      {
	int err;
      public:
	fatal_input_error(int _err)
	  : err(_err)
	{
	}

	void dispatch()
	{
	  throw fatal_input_exception(err);
	}
      };

      class get_input_event : public event
      {
	// A reference to the parent's condition mutex;
	// should be held while we signal the condition.
	threads::mutex &m;
	// A reference to the parent's variable indicating
	// whether the event has triggered.  Will be set
	// to "true" after we try to read all available
	// keystrokes.
	bool &b;
	// A reference to the parent's condition variable.
	threads::condition &c;

	/** \brief The suspend count when this thread was created.
	 *
	 *  If this has changed, it means that curses was shut down
	 *  and the input thread has been restarted, so we should
	 *  abort rather than doing Bad Things such as fighting with
	 *  the input thread for access to stdin.
	 */
	int my_suspend_count;

      public:
	get_input_event(threads::mutex &_m, bool &_b, threads::condition &_c)
	  : m(_m), b(_b), c(_c), my_suspend_count(get_suspend_count())
	{
	}

	void dispatch()
	{
	  // NB: use the GLOBAL getch function to avoid weirdness
	  // referencing the state of toplevel.
	  wint_t wch = 0;
	  int status;

	  bool done = false;

	  // We need to make sure that we shut down cwidget on EOF
	  // instead of spinning forever (Debian bug #451770).  This
	  // is currently done by calling exitmain; another option to
	  // consider is throwing an exception.
	  //
	  // We claim to have reached EOF when we get ERR on the very
	  // first call to get_wch. (i.e., we don't read any actual
	  // characters) This is tracked by read_anything.  To ensure
	  // that we don't abort spuriously, it's necessary to make
	  // sure that resize keypresses (which are silently thrown
	  // away) are accounted for, and that interrupted system
	  // calls count as events.
	  bool read_anything = false;
	  // Tracks the error corresponding to the last read; used to
	  // report errors to the user if we're unable to read a
	  // keystroke.
	  int last_read_error = 0;
	  while(!done)
	    {
	      // Abort if the world went away underneath us.
	      //
	      // We need to check up here since we might have just
	      // finished invoking a key handler, which could have
	      // suspended cwidget.
	      if(get_suspend_count() != my_suspend_count)
		return;

	      // I assume here that init() set nodelay.
	      do
		{
		  errno = 0;
		  status = get_wch(&wch);
		  last_read_error = errno;

		  if(status == KEY_CODE_YES && wch == KEY_RESIZE)
		    // Don't confuse resize events with EOF.
		    read_anything = true;
		} while(status == KEY_CODE_YES && wch == KEY_RESIZE);

	      if(status == ERR) // No more to read.
		{
		  // Ignore signals caught during read() (EINTR) and
		  // beep at the user if there's a problem decoding a
		  // multibyte character.
		  if(last_read_error == EINTR)
		    read_anything = true;
		  else if(last_read_error == EILSEQ)
		    {
		      read_anything = true;
		      beep();
		    }

		  threads::mutex::lock l(m);
		  b = true;
		  c.wake_all();
		  done = true;
		}
	      else
		{
		  read_anything = true;

		  key k(wch, status == KEY_CODE_YES);

		  if(wch == KEY_MOUSE)
		    {
		      if(toplevel.valid())
			{
			  MEVENT ev;
			  getmouse(&ev);

			  toplevel->dispatch_mouse(ev.id, ev.x, ev.y, ev.z, ev.bstate);
			}
		    }
		  else
		    {
		      if(global_bindings.key_matches(k, "Refresh"))
			redraw();
		      else
			toplevel->dispatch_key(k);
		    }
		}
	    }

	  // If nothing was read (where "stuff being read" includes
	  // resize keystrokes and interrupted system calls), kill the
	  // main loop off.
	  if(!read_anything)
	    throw fatal_input_exception(last_read_error);
	}
      };

      static threads::mutex input_event_mutex;
      // Used to block this thread until an event to read input fires.

      // These variables must be static so that they survive if the
      // thread is cancelled.
      static bool input_event_fired;
      static threads::condition input_event_condition;

      static input_thread instance;

      static threads::mutex instance_mutex;
      static threads::thread *instancet;

    public:
      static void start()
      {
	threads::mutex::lock l(instance_mutex);

	if(instancet == NULL)
	  instancet = new threads::thread(threads::make_bootstrap_proxy(&instance));
      }

      static void stop()
      {
	threads::mutex::lock l(instance_mutex);

	if(instancet != NULL)
	  {
	    instancet->cancel();
	    instancet->join();
	    delete instancet;
	    instancet = NULL;
	  }
      }

      void operator()()
      {
	// Block all signals so we don't interfere with the main
	// thread's signal handling.
	sigset_t all_signals;
	sigfillset(&all_signals);
	sigprocmask(SIG_BLOCK, &all_signals, NULL);

	// Don't hold the lock for longer than we need to (we lock it
	// here to be extra-paranoid about changing the value of
	// input_event_fired).
	{
	  threads::mutex::lock l(input_event_mutex);
	  input_event_fired = false;
	}

	// Important note: this routine only blocks indefinitely in
	// pthread_cond_wait(), assuming no bugs in post_event().
	// pthread_cond_wait() is a cancellation point.  select()
	// should be but isn't, but there is a workaround below.

	while(1)
	  {
	    // Only sleep for a few seconds at a time, to be extra
	    // careful not to block forever.
	    struct timeval timeout;
	    timeout.tv_sec = 3;
	    timeout.tv_usec = 0;

	    // Select on stdin; when we see that input is available, spawn
	    // an input event.
	    fd_set selectfds;
	    FD_ZERO(&selectfds);
	    FD_SET(0, &selectfds);

	    pthread_testcancel();
	    int result = select(1, &selectfds, NULL, NULL, &timeout);
	    pthread_testcancel();	// Workaround for Linux threads suckage.
	    // See pthread_cancel(3).

	    if(result != 1)
	      {
		if(errno != EINTR && errno != 0)
		  {
		    // Probably means that there was an error reading
		    // standard input.  (could also be ENOMEM)
		    post_event(new fatal_input_error(errno));
		    break;
		  }
	      }
	    else
	      {
		// Lock the mutex and wait on the condition variable.
		// We have to be careful to release the mutex when we
		// leave this scope; otherwise we could end up being
		// canceled while we hold the mutex, which leads to
		// horrible stuff like bug #511708.
		threads::mutex::lock l(input_event_mutex);
		post_event(new get_input_event(input_event_mutex,
					       input_event_fired,
					       input_event_condition));

		while(!input_event_fired)
		  input_event_condition.wait(l);
		input_event_fired = false;
	      }
	  }
      }
    };

    threads::condition input_thread::input_event_condition;
    threads::mutex input_thread::input_event_mutex;
    bool input_thread::input_event_fired = false;
    threads::mutex input_thread::instance_mutex;
    threads::thread *input_thread::instancet = NULL;
    input_thread input_thread::instance;

    class signal_thread
    {
      class signal_event : public event
      {
	int signal;
      public:
	signal_event(int _signal)
	  :signal(_signal)
	{
	}

	void dispatch()
	{
	  switch(signal)
	    {
	    case SIGWINCH:
	      handleresize();
	      break;
	    default:
	      exitmain();
	      break;
	    }
	}
      };

      static signal_thread instance;
      static threads::thread *t;
    public:
      static void start()
      {
	if(t == NULL)
	  t = new threads::thread(instance);
      }

      static void stop()
      {
	if(t != NULL)
	  {
	    t->cancel();
	    t->join();
	    delete t;
	    t = NULL;
	  }
      }

      void operator()() const
      {
	// Block all signals so we don't interfere with the main
	// thread's signal handling.
	sigset_t all_signals;
	sigfillset(&all_signals);
	sigprocmask(SIG_BLOCK, &all_signals, NULL);


	sigset_t s;

	sigemptyset(&s);
	sigaddset(&s, SIGWINCH);

	while(1)
	  {
	    int signum;

	    int result = sigwait(&s, &signum);

	    if(result == 0)
	      post_event(new signal_event(signum));
	  }
      }
    };

    signal_thread signal_thread::instance;
    threads::thread *signal_thread::t = NULL;

    class timeout_thread
    {
      class SingletonViolationException
      {
      public:
	string errmsg() const
	{
	  return "Attempt to run a singleton thread twice!";
	}
      };


      /** Information about a single time-out. */
      struct timeout_info
      {
	event *ev;
	timeval activate_time; // tells when this timeout should be triggered
	timeout_info(event *_ev,
		     const timeval &_activate_time)
	  :ev(_ev), activate_time(_activate_time)
	{
	}

	timeout_info()
	{
	  activate_time.tv_sec = 0;
	  activate_time.tv_usec = 0;
	}
      };


      // The set of active timeouts.
      map<int, timeout_info> timeouts;

      /** If \b true, the thread should stop. */
      bool cancelled;

      // A lock for the set of timeouts and the cancelled flag.
      threads::mutex timeouts_mutex;

      // A condition to be broadcast when the set of timeouts is expanded
      // by add_timeout.
      threads::condition timeout_added;

      /** The thread that is currently executing in this object. */
      threads::box<threads::thread *> running_thread;



      /** Post messages about any timeouts that occurred. Should be called
       *  with timeouts_mutex locked.
       */
      void check_timeouts()
      {
	map<int, timeout_info>::iterator i,j;
	for(i = timeouts.begin(); i != timeouts.end(); i = j)
	  {
	    j = i;
	    j++;
	    timeval result,curtime;
	    gettimeofday(&curtime, 0);

	    if(timeval_subtract(&result, &i->second.activate_time, &curtime) == 1 ||
	       (result.tv_sec == 0 && result.tv_usec <= 10))
	      {
		post_event(i->second.ev);
		timeouts.erase(i);
	      }
	  }
      }

      /** timeouts_mutex should be locked when this is called.
       *
       *  \param tv_out the time at which the first active timeout should trigger.
       *  \return \b true if a timeout was found.
       */
      bool first_timeout(timeval &tv_out)
      {
	bool found_one = false;
	timeval mintime;
	mintime.tv_sec = INT_MAX/1000;
	mintime.tv_usec = (INT_MAX % 1000) * 1000;

	timeval curtime;
	gettimeofday(&curtime, 0);

	map<int, timeout_info>::iterator i,j;
	for(i = timeouts.begin(); i != timeouts.end(); i = j)
	  {
	    j = i;
	    j++;
	    timeval diff;
	    if(timeval_subtract(&diff, &i->second.activate_time, &curtime) == 1 ||
	       (diff.tv_sec == 0 && diff.tv_usec <= 10))
	      {
		tv_out = curtime;
		return true;
	      }
	    else
	      {
		if(diff.tv_sec < mintime.tv_sec ||
		   (diff.tv_sec == mintime.tv_sec && diff.tv_usec < mintime.tv_usec))
		  {
		    found_one = true;
		    mintime = i->second.activate_time;
		  }
	      }
	  }
	if(found_one)
	  {
	    tv_out = mintime;
	    return true;
	  }
	else
	  return false;
      }


      timeout_thread(const timeout_thread &other);
      timeout_thread &operator=(const timeout_thread &other);

      timeout_thread()
	: cancelled(false), running_thread(NULL)
      {
      }

      // The global instance; this is a Singleton.
      static timeout_thread instance;

    public:
      static timeout_thread &get_instance()
      {
	return instance;
      }

      static void start()
      {
	timeout_thread &instance = get_instance();

	threads::thread *running = instance.running_thread.take();
	if(running != NULL)
	  {
	    instance.running_thread.put(running);
	    throw SingletonViolationException();
	  }

	threads::thread *t = new threads::thread(threads::make_bootstrap_proxy(&instance));
	instance.running_thread.put(t);
      }

      static void stop()
      {
	timeout_thread &instance = get_instance();

	threads::thread *running = instance.running_thread.take();

	if(running == NULL)
	  {
	    instance.running_thread.put(NULL);
	    return;
	  }

	threads::mutex::lock l(instance.timeouts_mutex);

	instance.cancelled = true;
	instance.timeout_added.wake_all();

	l.release();

	running->join();

	instance.running_thread.put(NULL);
      }

      void operator()()
      {
	// Block all signals so we don't interfere with the main
	// thread's signal handling.
	sigset_t all_signals;
	sigfillset(&all_signals);
	sigprocmask(SIG_BLOCK, &all_signals, NULL);



	threads::mutex::lock l(timeouts_mutex);

	while(!cancelled)
	  {
	    timeval next_timeout;

	    if(first_timeout(next_timeout))
	      {
		timespec until;
		until.tv_sec = next_timeout.tv_sec;
		until.tv_nsec = next_timeout.tv_usec * 1000;

		timeout_added.timed_wait(l, until);

		// Probably don't need to do this, but it won't hurt.
		check_timeouts();
	      }
	    else
	      timeout_added.wait(l);
	  }
      }

      /** Add a timeout to the set of active timeouts.
       *
       *  \param slot a callback to activate when the timeout triggers
       *  \param msecs the number of milliseconds in which to activate the
       *               timeout.
       *  \return the ID number of the new timeout; can be used later to
       *          remove the timeout before it triggers.
       */
      int add_timeout(event *ev, int msecs)
      {
	threads::mutex::lock l(timeouts_mutex);

	timeval activate_time;
	gettimeofday(&activate_time, 0);
	activate_time.tv_sec  += msecs/1000;
	activate_time.tv_usec += (msecs%1000)*1000;
	while(activate_time.tv_usec > 1000 * 1000)
	  // Should only run through once
	  {
	    activate_time.tv_sec++;
	    activate_time.tv_usec -= 1000 * 1000;
	  }

	// Since the timeouts are sorted by ID, the back element is the
	// maximum ID in use.
	int rval;

	if(timeouts.empty())
	  rval = 0;
	else
	  rval = timeouts.rbegin()->first + 1;

	timeouts[rval] = timeout_info(ev, activate_time);

	timeout_added.wake_all();

	return rval;
      }

      void del_timeout(int id)
      {
	threads::mutex::lock l(timeouts_mutex);

	timeouts.erase(id);
      }
    };

    timeout_thread timeout_thread::instance;

    void init()
    {
      threads::mutex::lock l(get_mutex());

      bindtextdomain(CWIDGET_DOMAIN, LOCALEDIR);

      keybinding upkey, downkey, leftkey, rightkey, quitkey, homekey, endkey;
      keybinding historynextkey, historyprevkey;
      keybinding delfkey, delbkey, ppagekey, npagekey;
      keybinding undokey, helpkey, researchkey;
      keybinding menutogglekey, cancelkey;

      upkey.push_back(key(KEY_UP, true));
      upkey.push_back(key(L'k', false));
      downkey.push_back(key(KEY_DOWN, true));
      downkey.push_back(key(L'j', false));
      leftkey.push_back(key(KEY_LEFT, true));
      leftkey.push_back(key(L'h', false));
      rightkey.push_back(key(KEY_RIGHT, true));
      rightkey.push_back(key(L'l', false));
      quitkey.push_back(key(L'q', false));

      historyprevkey.push_back(key(KEY_UP, true));
      historyprevkey.push_back(KEY_CTRL(L'p'));
      historynextkey.push_back(key(KEY_DOWN, true));
      historynextkey.push_back(KEY_CTRL(L'n'));

      homekey.push_back(key(KEY_HOME, true));
      homekey.push_back(KEY_CTRL(L'a'));
      endkey.push_back(key(KEY_END, true));
      endkey.push_back(KEY_CTRL(L'e'));

      delfkey.push_back(key(KEY_DC, true));
      delfkey.push_back(KEY_CTRL(L'd'));

      delbkey.push_back(key(KEY_BACKSPACE, true));
      delbkey.push_back(KEY_CTRL(L'h'));

      ppagekey.push_back(key(KEY_PPAGE, true));
      ppagekey.push_back(KEY_CTRL(L'b'));

      npagekey.push_back(key(KEY_NPAGE, true));
      npagekey.push_back(KEY_CTRL(L'f'));

      undokey.push_back(KEY_CTRL(L'u'));
      undokey.push_back(KEY_CTRL(L'_'));

      helpkey.push_back(key(L'?', false));
      helpkey.push_back(KEY_CTRL(L'h'));
      helpkey.push_back(key(KEY_F(1), true));

      menutogglekey.push_back(KEY_CTRL(L't'));
      menutogglekey.push_back(key(KEY_F(10), true));
      menutogglekey.push_back(KEY_CTRL(L' '));

      cancelkey.push_back(KEY_CTRL(L'g'));
      cancelkey.push_back(key(L'\e', true));
      cancelkey.push_back(KEY_CTRL(L'['));

      researchkey.push_back(key(L'n', false));

      init_curses();

      mousemask(ALL_MOUSE_EVENTS, NULL);

      curses_avail=true;
      cbreak();
      rootwin.nodelay(true);
      rootwin.keypad(true);

      global_bindings.set("Quit", quitkey);
      global_bindings.set("Cycle", key(L'\t', false));
      global_bindings.set("Refresh", KEY_CTRL(L'l'));

      global_bindings.set("Up", upkey);
      global_bindings.set("Down", downkey);
      global_bindings.set("LevelDown", key(L'J', false));
      global_bindings.set("LevelUp", key(L'K', false));
      global_bindings.set("Left", leftkey);
      global_bindings.set("Right", rightkey);
      global_bindings.set("HistoryNext", historynextkey);
      global_bindings.set("HistoryPrev", historyprevkey);
      global_bindings.set("Parent", key(L'^', false));
      global_bindings.set("PrevPage", ppagekey);
      global_bindings.set("NextPage", npagekey);
      global_bindings.set("Begin", homekey);
      global_bindings.set("End", endkey);
      global_bindings.set("Search", key(L'/', false));
      global_bindings.set("SearchBack", key(L'\\', false));
      global_bindings.set("ReSearch", researchkey);
      global_bindings.set("RepeatSearchBack", key(L'N', false));
      global_bindings.set("DelBack", delbkey);
      global_bindings.set("DelForward", delfkey);

      global_bindings.set("DelEOL", KEY_CTRL(L'k'));
      global_bindings.set("DelBOL", KEY_CTRL(L'u'));

      global_bindings.set("Confirm", key(KEY_ENTER, true));
      global_bindings.set("Cancel", cancelkey);
      global_bindings.set("Undo", undokey);
      global_bindings.set("Help", helpkey);
      global_bindings.set("ToggleMenuActive", menutogglekey);
      global_bindings.set("PushButton", key(L' ', false));
      global_bindings.set("Yes", key(util::transcode(_("yes_key"))[0], false));
      global_bindings.set("No", key(util::transcode(_("no_key"))[0], false));

      global_bindings.set("ToggleExpanded", key(KEY_ENTER, true));
      global_bindings.set("ExpandAll", key(L'[', false));
      global_bindings.set("CollapseAll", key(L']', false));
      global_bindings.set("SelectParent", key(L'^', false));

      editline::init_bindings();
      menu::init_bindings();
      menubar::init_bindings();
      pager::init_bindings();
      statuschoice::init_bindings();
      table::init_bindings();
      text_layout::init_bindings();
      tree::init_bindings();

      set_style("Error",
		style_fg(COLOR_WHITE)+style_bg(COLOR_RED)+style_attrs_on(A_BOLD));

      // The 'base' style for the display.
      set_style("Default",
		style_fg(COLOR_WHITE)+style_bg(COLOR_BLACK));

      set_style("Header",
		style_fg(COLOR_WHITE)+style_bg(COLOR_BLUE)+style_attrs_on(A_BOLD));
      set_style("Status",
		style_fg(COLOR_WHITE)+style_bg(COLOR_BLUE)+style_attrs_on(A_BOLD));

      set_style("MenuEntry", style_fg(COLOR_WHITE)+style_bg(COLOR_BLUE));
      set_style("MenuBorder", style_fg(COLOR_WHITE)+style_bg(COLOR_BLUE)+style_attrs_on(A_BOLD));
      set_style("HighlightedMenuEntry", style_bg(COLOR_BLUE)+style_fg(COLOR_WHITE)+style_attrs_on(A_BOLD|A_REVERSE));
      set_style("DisabledMenuEntry",
		style_fg(COLOR_BLACK)+style_bg(COLOR_BLUE)+style_attrs_on(A_DIM));
      set_style("MenuBar", style_fg(COLOR_WHITE)+style_bg(COLOR_BLUE)+style_attrs_on(A_BOLD));
      set_style("HighlightedMenuBar", style_fg(COLOR_BLUE)+style_bg(COLOR_WHITE));

      set_style("MultiplexTab", style_fg(COLOR_WHITE)+style_bg(COLOR_BLUE));
      set_style("MultiplexTabHighlighted", style_fg(COLOR_BLUE)+style_bg(COLOR_WHITE));

      // Edit lines will *always* appear white-on-black.
      set_style("EditLine", style_fg(COLOR_WHITE)+style_bg(COLOR_BLACK)+style_attrs_off(A_REVERSE));

      set_style("TreeBackground", style());

      if(toplevel.valid())
	settoplevel(toplevel);


      install_sighandlers();


      // Block WINCH so the signal_thread can pick it up.
      sigset_t signals;
      sigemptyset(&signals);
      sigaddset(&signals, SIGWINCH);
      pthread_sigmask(SIG_BLOCK, &signals, NULL);



      input_thread::start();
      signal_thread::start();
      timeout_thread::start();
    }

    void install_sighandlers()
    {
      signal(SIGTERM, sigkilled);
      signal(SIGINT, sigkilled);
      signal(SIGSEGV, sigkilled);
      signal(SIGQUIT, sigkilled);
      signal(SIGABRT, sigkilled);
    }

    void handleresize()
    {
      threads::mutex::lock l(get_mutex());

      toplevel->set_owner_window(NULL, 0, 0, 0, 0);
      resize();
      toplevel->set_owner_window(rootwin, 0, 0, rootwin.getmaxx(), rootwin.getmaxy());
      redraw();
    }

    class try_update_event : public event
    {
    public:
      void dispatch()
      {
	tryupdate();
      }
    };

    void updatecursor()
    {
      threads::mutex::lock l(pending_updates_mutex);

      pending_updates.cursorupdate=true;
    }

    void updatecursornow()
    {
      threads::mutex::lock l(get_mutex());

      if(toplevel->get_cursorvisible())
	{
	  point p=toplevel->get_cursorloc();
	  toplevel->win.leaveok(false);
	  toplevel->win.move(p.y, p.x);
	  toplevel->win.noutrefresh();
	}
      else
	toplevel->win.leaveok(true);
    }

    void update()
    {
      threads::mutex::lock l(pending_updates_mutex);

      pending_updates.update=true;
      pending_updates.cursorupdate=true;

      post_event(new try_update_event);
    }

    void updatenow()
    {
      threads::mutex::lock l(get_mutex());

      if(toplevel.valid())
	{
	  toplevel->display(get_style("Default"));
	  toplevel->sync();
	}
    }

    void queuelayout()
    {
      threads::mutex::lock l(pending_updates_mutex);

      pending_updates.layout = true;
      pending_updates.update = true;
      pending_updates.cursorupdate = true;

      post_event(new try_update_event);
    }

    void layoutnow()
    {
      threads::mutex::lock l(get_mutex());

      toplevel->do_layout();
    }

    void tryupdate()
    {
      threads::mutex::lock l(get_mutex());

      threads::mutex::lock l2(pending_updates_mutex);

      update_state needs = pending_updates;

      if(needs.layout)
	layoutnow();

      if(needs.update)
	updatenow();

      if(needs.update || needs.cursorupdate)
	updatecursornow();

      doupdate();

      // \todo This appears to just paper over sloppiness -- screen update
      // routines shouldn't be queuing more updates!
      pending_updates = update_state();
    }

    bool poll()
    {
      threads::mutex::lock l(get_mutex());

      bool rval=false;

      event *ev = NULL;

      while(eventq.try_get(ev))
	{
	  rval = true;
	  ev->dispatch();
	  delete ev;
	}

      main_hook();

      return rval;
    }

    void mainloop()
    {
      static int main_level=0;

      main_level++;

      threads::mutex::lock l(get_mutex());

      while(!should_exit && toplevel.valid())
	{
	  l.release();

	  event *ev = eventq.get();

	  l.acquire();

	  ev->dispatch();
	  delete ev;

	  while(eventq.try_get(ev))
	    {
	      ev->dispatch();
	      delete ev;
	    }

	  main_hook();
	}

      should_exit=false;

      main_level--;
    }

    void exitmain()
    {
      should_exit=1;
    }

    void suspend_without_signals()
    {
      threads::mutex::lock l(get_mutex());

      input_thread::stop();
      signal_thread::stop();
      timeout_thread::stop();

      inc_suspend_count();

      if(toplevel.valid())
	toplevel->set_owner_window(NULL, 0, 0, 0, 0);

      rootwin.bkgdset(' ');
      rootwin.clear();
      rootwin.refresh();
      endwin();
      curses_avail=false;
    }

    void suspend()
    {
      threads::mutex::lock l(get_mutex());

      suspended_with_signals = true;

      struct sigaction act;

      memset(&act,0,sizeof(act));
      act.sa_handler = SIG_IGN;
      sigemptyset(&act.sa_mask);

      sigaction(SIGCONT, &act, &oldsigcont);
      sigaction(SIGTSTP, &act, &oldsigtstp);

      suspend_without_signals();
    }

    void shutdown()
    {
      threads::mutex::lock l(get_mutex());

      if(toplevel.valid())
	toplevel->destroy();
      toplevel = NULL;

      suspend();

      // Discard all remaining events.
      event *ev = NULL;
      while(eventq.try_get(ev))
	delete ev;
    }

    void resume()
    {
      threads::mutex::lock l(get_mutex());

      if(suspended_with_signals)
	{
	  sigaction(SIGCONT, &oldsigcont, NULL);
	  sigaction(SIGTSTP, &oldsigtstp, NULL);
	  suspended_with_signals = false;
	}

      curses_avail=true;
      if(toplevel.valid())
	{
	  toplevel->set_owner_window(rootwin, 0, 0, rootwin.getmaxx(), rootwin.getmaxy());
	  toplevel->display(get_style("Default"));
	  toplevel->sync();
	  doupdate();
	}
      else
	refresh();

      input_thread::start();
      signal_thread::start();
      timeout_thread::start();
    }

    void redraw()
    {
      threads::mutex::lock l(get_mutex());

      threads::mutex::lock l2(pending_updates_mutex);

      if(toplevel.valid())
	{
	  toplevel->focussed();
	  toplevel->get_win().touch();
	  toplevel->get_win().clearok(true);
	  toplevel->do_layout();
	  toplevel->display(get_style("Default"));
	  updatecursornow();
	  toplevel->sync();
	  doupdate();
	  toplevel->get_win().clearok(false);
	}

      // For reasons that aren't entirely clear, running a tryupdate()
      // right after a resize causes stuff to break -- but since resizing
      // triggers a redraw, the tryupdate() shouldn't be necessary anyway.
      // Suppress any pending updates after redrawing:
      pending_updates = update_state();
    }

    int addtimeout(event *ev, int msecs)
    {
      if(msecs < 0)
	return -1;

      return timeout_thread::get_instance().add_timeout(ev, msecs);
    }

    void deltimeout(int num)
    {
      timeout_thread::get_instance().del_timeout(num);
    }
  }
}
