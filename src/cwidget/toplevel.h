// toplevel.h                          -*-c++-*-
//
//  Copyright 2000, 2005, 2007-2008 Daniel Burrows
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

/** \file toplevel.h
 *
 *  \brief Routines to manage the global cwidget state.
 *
 *  This file contains routines that manage the global state of the
 *  cwidget library: initializing and shutting it down, controlling
 *  its main loop, changing the top-level widget, etc.
 */

#ifndef TOPLEVEL_H
#define TOPLEVEL_H

#include <sigc++/signal.h>

#include <cwidget/generic/util/ref_ptr.h>

/** \brief The namespace containing everything defined by cwidget. */
namespace cwidget
{
  /** \brief Return the version number of the library. */
  std::string version();

  namespace threads
  {
    class mutex;
  }

  namespace widgets
  {
    class widget;
  }

  /** \brief The namespace containing functions to manage the global
   *  state of cwidget.
   *
   *  Routines in this namespace handle initializing and shutting down
   *  cwidget, running its main loop, setting the top-level widget,
   *  and other functions related to the top-level control structures
   *  of the library.
   */
  namespace toplevel
  {
    /** An event in the global event queue.  Events are dispatched
     *  serially by the main loop.
     */
    class event
    {
    public:
      virtual void dispatch() = 0;
      virtual ~event();
    };

    /** An event based on sigc++ slots.  This is important because it can
     *  avoid some timing issues that occur when an object is destroyed
     *  before its callback triggers.  However, you cannot construct one
     *  of these threadsafely unless you are EXTREMELY careful, because it
     *  involves a slot copy; as with other sigc++ stuff, I recommend that
     *  you only create these in the 'main thread'.  (of course, passing
     *  around pointers to the resulting object is just fine, as long as
     *  the final invocation is also in the main thread)
     */
    class slot_event : public event
    {
      sigc::slot0<void> the_slot;
    public:
      slot_event(const sigc::slot0<void> &_the_slot)
	: the_slot(_the_slot)
      {
      }

      void dispatch();
    };

    /** \brief Initializes curses and the global state of the cwidget
     *  library.
     */
    void init();

    /** \brief Installs signal handlers to cleanly shut down cwidget.
     *
     *  This is always invoked by cwidget::toplevel::init().  However,
     *  you might want to invoke it manually if you have removed the
     *  cwidget signal handlers (for instance, in order to invoke an
     *  external program).
     *
     *  Installs signal handlers for TERM, INT, QUIT, SEGV, and ABRT
     *  which restore the terminal and exit the program.
     */
    void install_sighandlers();

    /** Sets the top-level widget to the new value, returning the old
     *  top-level widget.  If the top-level widget is to be destroyed,
     *  IT IS THE CALLER'S RESPONSIBILITY TO CALL destroy() BEFORE
     *  DISCARDING THE REFERENCE!
     */
    util::ref_ptr<widgets::widget> settoplevel(const util::ref_ptr<widgets::widget> &widget);

    /** Posts a request to recalculate every widget's layout and update
     *  the screen.  May be called from any thread.
     */
    void queuelayout();

    /** \brief Immediately recalculates the layout of all widgets. */
    void layoutnow();

    //   Main loop handlers:

    /** \brief Start the main event loop.
     *
     *  This routine repeatedly removes events from the global queue
     *  and invokes event::dispatch() on them.  It terminates when
     *  exitmain() is invoked or when an exception is thrown.  In
     *  particular, callers should be prepared to catch
     *  cwidget::util::Exception instances:
     *
     *     catch(cwidget::util::Exception &e)
     *     {
     *     }
     *
     *  If an exception is thrown, the caller is responsible for
     *  invoking cwidget::toplevel::shutdown() to restore the terminal
     *  state.  A simple way of testing that your code catches
     *  exceptions correctly is to redirect stdin from /dev/null; this
     *  will throw an exception stating that the program cannot read
     *  from stdin.
     */
    void mainloop();

    /** Post the given event to the main event queue.  When the event
     *  comes off the queue, its dispatch method will be invoked and it
     *  will immediately be destroyed.
     *
     *  This method is thread-safe and is the main mechanism by which
     *  other threads should communicate with the main thread.
     */
    void post_event(event *ev);

    /** Dispatch any events in the event queue.  This is deprecated in
     *  favor of the more reliable approach of using threads and
     *  post_event.
     *
     *  \return \b true if pending input was found.
     */
    bool poll();

    /** Exits the main loop.
     */
    void exitmain();

    /** Redraws the screen completely from scratch
     */
    void redraw();

    /** Posts a request to redraw the screen; may be called from any thread. */
    void update();

    /** Executes any pending draws or redraws. */
    void tryupdate();

    /** Posts a request to update the cursor location; may be called from
     *  any thread.
     */
    void updatecursor();

    /** Hides all widgets and suspends Curses operation until
     *  resume() is called.  In addition, sets SIGCONT and SIGTSTP
     *  to SIG_DFL (appropriate if you'll be running subprocesses);
     *  resume() will restore these handlers.
     */
    void suspend();

    /** Hides all widgets and suspends Curses operation until
     *  resume() is called, but does NOT reset the SIGCONT and
     *  SIGTSTP handlers.
     */
    void suspend_without_signals();

    /** Does the same thing as suspend, except that it also
     *  destroys the top-level widget.  Call this when the program is
     *  exiting.
     */
    void shutdown();

    /** Returns to Curses mode after a suspend*, restoring any
     *  signal handlers that were modified by the suspend routine.
     */
    void resume();

    /** Invoke the given event in at least msecs from the current
     *  time.
     *
     *  \return a numerical identifier of the new event; you can use this
     *          number to delete it.
     */
    int addtimeout(event *ev, int msecs);

    /** Delete the event with the given identifier. */
    void deltimeout(int id);

    void handleresize();
    // Does anything needed to handle a window resize event.
    // FIXME: I --think-- that this is now redundant

    //  Return a mutex that is held whenever the main loop is processing
    //  events or drawing the screen.  The mutex can be held by other
    //  threads that want to do their own drawing or processing, although
    //  this is highly discouraged (use the event posting system instead
    //  to run code in the main loop).
    threads::mutex &get_mutex();

    /** \return the number of times that toplevel::suspend has been
     *  invoked since toplevel::init was invoked.
     *
     *  This can be used to detect when a suspend has occurred between
     *  when an event was posted and when it fired.  For instance, the
     *  input thread generates events that actually read from stdin,
     *  but if one of them fires after a suspend, the thread will get
     *  confused about how many events are trying to read and end up
     *  reading too many times and blowing up.
     */
    int get_suspend_count();

    extern sigc::signal0<void> main_hook;
    // Called right after we finish handling input in the mainloop.  Can
    // be used (eg) to insert extra actions to be performed after all
    // user-input (aptitude uses this to check for apt errors and pop up a
    // message about them)
  }
}

#endif
