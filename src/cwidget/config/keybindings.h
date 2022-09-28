// keybindings.h, -*-c++-*-
//
//  Copyright 1999-2001, 2003-2005, 2008 Daniel Burrows
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

#ifndef KEYBINDINGS_H
#define KEYBINDINGS_H

#include <map>
#include <string>
#include <vector>

#include <cwidget/curses++.h>

/** \file keybindings.h
 *
 *  \brief Support for defining and remapping keybindings.
 */

namespace cwidget
{
  namespace config
  {
    /** Represents a keystroke as seen by curses.  Since the function keys
     *  can overlap Unicode codepoints, we need to include a value that
     *  distinguishes them.
     */
    struct key
    {
      /** The key code. */
      wint_t ch;

      /** If \b true, this is a function key. */
      bool function_key;

      key()
	:ch((wint_t) ERR), function_key(true)
      {
      }

      key(wint_t _ch, bool _function_key)
	:ch(_ch), function_key(_function_key)
      {
      }

      /** Lexicographic ordering on keys. */
      bool operator<(const key &other) const
      {
	return ch < other.ch || (ch == other.ch &&
				 !function_key && other.function_key);
      }

      bool operator==(const key &other) const
      {
	return ch == other.ch && function_key == other.function_key;
      }
    };

    /** \brief The type used to store the keybindings of a function. */
    typedef std::vector<key> keybinding;

    /** \brief Stores the keys bound to various functions.
     *
     *  Functions are simply arbitrary strings chosen by the user of
     *  this class.  For instance, "QuitProgram" might be the function
     *  that quits the program.
     *
     *  Each keybindings object represents a scope in which bindings
     *  can be defined.  Scopes are arranged hierarchically, and the
     *  bindings defined in child scopes override the bindings defined
     *  in parent scopes.
     */
    class keybindings
    {
      std::map<std::string, keybinding> keymap;

      keybindings *parent;

      // It's way too easy to accidentally invoke the automatic copy
      // constructor instead of the real one.
      keybindings(const keybindings &_parent);
    public:
      /** \brief Create a new key-binding scope.
       *
       *  \param _parent   The parent of this scope, if any, or NULL for no parent.
       */
      keybindings(keybindings *_parent = nullptr) : parent(_parent) {}

      /** \return the first binding of the given function, in a format
       *  that can be passed to parse_key().
       *
       *  \param tag The function whose keystroke is to be returned.
       */
      std::wstring keyname(const std::string &tag) const;


      /** \return a human-readable string describing the keystroke
       *  bound to the given function.
       *
       *  \param tag The name of the function whose keystroke is to be
       *  returned.
       */
      std::wstring readable_keyname(const std::string &tag) const;

      /** \brief Retrieve the binding of the given function. */
      keybinding get(const std::string &tag) const
      {
	auto it = keymap.find(tag);

	if (it == keymap.end())
	  return keybinding();
	else
	  return it->second;
      }

      /** \brief Modify a binding in this scope.
       *
       *  \param tag      The name of the function to be bound.
       *  \param strokes  The keystrokes to bind to the function.
       *
       *  This routine throws away any previous bindings for the given
       *  function and replaces them with the bindings stored in
       *  strokes.
       */
      void set(std::string tag, keybinding strokes);

      /** \brief Modify a binding in this scope.
       *
       *  \param tag     The name of the function to be bound.
       *  \param stroke  A keystroke to bind to the function.
       *
       *  This routine throws away any previous bindings for the
       *  given function and replaces them with stroke.
       */
      void set(std::string tag, const key &stroke)
      {
	keybinding strokes;
	strokes.push_back(stroke);
	set(tag, strokes);
      }

      /** \brief Test whether a key is bound to a function.
       *
       *  \param k   The key to test.
       *  \param tag The function to test against.
       *
       *  \return \b true if k is bound to tag in this scope.
       */
      bool key_matches(const key &k, std::string tag) const;
    };

    /** \brief Parse a keystroke definition.
     *
     *  \param keystr  The definition to parse.
     *
     *  \return the corresponding key, or ERR if the parse fails.
     */
    key parse_key(const std::wstring &keystr);

    /** \brief Convert a keystroke to its string definition.
     *
     *  \param k  The key that is to be converted to a string.
     *
     *  \return a string that, when passed to parse_key(), will return
     *  #k.
     *
     *  \sa readable_keyname
     */
    std::wstring keyname(const key &k);

    /** \brief Convert a keystroke to a human-readable keyname.
     *
     *  \return a human-readable string identifying the given keystroke.
     *
     *  \sa keyname
     */
    std::wstring readable_keyname(const key &k);

    /** \brief The global keybindings object.
     *
     *  This object is the root of the keybindings hierarchy; normally
     *  all other keybindings objects should be descendents of it.
     */
    extern keybindings global_bindings;
  }
}

// Stolen from pinfo.  I don't like the looks of it, but presumably it works
// (in some circumstances).  This is a FIXME, btw :)
/* adapted from Midnight Commander */

// Having read a bit more, it appears that the control modifier
// clears bits 5 and 4.  I think KEY_ALT is utterly broken.

/** \brief Attempt to compute the control character related to a
 *  terminal key.
 *
 *  \param x The character to modify (for instance, 'A' to return
 *  'Control-A').
 */
#define KEY_CTRL(x) key(((x)&~(64|32)), false)
#define KEY_ALT(x) key((0x200 | (x)), false)


#endif
