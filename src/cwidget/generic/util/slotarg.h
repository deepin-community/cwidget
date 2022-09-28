// slotarg.h			-*-c++-*-
//
//  Copyright 2000 Daniel Burrows
//
//  Provides a mechanism for nicely passing in optional slots to a function.
// (you can pass either a reference to one or a pointer (which can be NULL))
//
//  Eg: some_slot_function(slotarg, slotarg, slotarg) can be called as:
// some_slot_function(arg(slota), NULL, arg(slotb)) to omit the second slot.

/** \file slotarg.h
 *
 *  \brief Provides a simple mechanism for passing in optional slots
 *  to a function.
 */

#ifndef SLOTARG_H
#define SLOTARG_H

#include <sigc++/functors/slot.h>

namespace cwidget
{
  /** \brief Miscellaneous utility functions that are not directly
   *  related to the core functionality of cwidget.
   */
  namespace util
  {
    /** \brief Wraps a slot that may not be present.
     *
     *  \tparam T The slot type that is wrapped by this argument.
     *
     *  See also cwidget::util::arg, cwidget::util::slot0arg.
     */
    template<typename T>
    class slotarg
    {
      bool hasslot;
      T theslot;
    public:
      /** \brief Create a slotarg from an optional slot.
       *
       *  \param slot  The slot to store, or NULL to store no slot.
       */
      slotarg(const T *slot)
      {
	if(slot)
	  {
	    theslot=*slot;
	    hasslot=true;
	  }
	else
	  hasslot=false;
      }

      /** \brief Create a slotarg from an existing slot. */
      slotarg(const T &slot)
	:hasslot(true), theslot(slot)
      {
      }

      /** \brief Convert between compatible slotarg types. */
      template <typename S>
      operator slotarg<S>() const
      {
	if(hasslot)
	  return slotarg<S>(theslot);
	else
	  return slotarg<S>(NULL);
      }

      /** \brief Return \b true if this argument stores a slot. */
      operator bool() const {return hasslot;}
      /** \brief Return the encapsulated slot, if any. */
      const T & operator*() const {return theslot;}
      /** \brief Return the encapsulated slot, if any. */
      T & operator*() {return theslot;}
    };

    /** \brief Convenience typedefs for slot arguments that take no
     *  parameters and return nothing.
     */
    typedef slotarg<sigc::slot0<void> > slot0arg;

    /** \brief Convenience routine to construct a slotarg.
     *
     *  The purpose of this routine is to allow C++'s local type
     *  inference to determine the template parameters to slotarg.
     *  It is the recommended way to construct slotarg objects when
     *  passing them as parameters to a function.
     *
     *  \tparam T The slot type that is wrapped by the returned
     *  slotarg object.
     *
     *  \param slot The slot that is to be wrapped.
     */
    template<typename T>
    slotarg<T> arg(const T &slot)
    {
      return slotarg<T>(slot);
    }
  }
}

#endif
