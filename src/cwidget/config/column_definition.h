// column_definition.h   -*-c++-*-
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

/** \file column_definition.h
 *
 *  \brief Support for parsing configuration strings that describe how
 *  to display columnar data.
 *
 *  This routine provides a general interface for parsing
 *  configuration data about a column format and later instantiating
 *  that information.  The caller has to provide some information, in
 *  the form of tables and callbacks, that's used to do the actual
 *  formatting.
 *
 *  Column strings are printf-style; the caller determines how
 *  %-escapes are interpreted by mapping them to integer codes that
 *  identify column types (-1 is reserved for internal use).
 */

#ifndef COLUMN_DEFINITION_H
#define COLUMN_DEFINITION_H

#include <list>
#include <string>

#include <cwidget/generic/util/eassert.h>

#include <cwidget/columnify.h>

namespace cwidget
{
  namespace config
  {
    /** Defines the default settings for a particular column type. */
    struct column_type_defaults
    {
      unsigned int width;
      bool expand, shrink;
    };

    /** Defines the string arguments passed into the layout process.
     *
     *  Column layouts can contain positional arguments (for instance,
     *  to substitute the results of a search); instances of this
     *  class are used to describe the arguments to the layout
     *  routine.
     */
    class column_parameters
    {
    public:
      virtual int param_count()=0;
      virtual std::wstring get_param(int n)=0;

      virtual ~column_parameters();
    };

    /** An empty list of parameters. */
    class empty_column_parameters : public column_parameters
    {
    public:
      int param_count();
      std::wstring get_param(int n);
    };

    /** Defines how a single column is to be generated.
     *
     *  \sa column_generator
     */
    struct column_definition
    {
      /** \brief The available column types. */
      enum column_type
      {
        /** \brief A literal column.
	 *
	 *  The text of a literal column is taken from its #arg member.
	 */
        COLUMN_LITERAL,
	/** \brief A dynamically generated column.
	 *
	 *  The text and width of a generated column are computed by
	 *  calling column_generator::setup_column.
	 */
	COLUMN_GENERATED,
	/** \brief A column defined by a positional parameter.
	 *
	 *  The text of a column defined by a positional parameter is
	 *  constructed by calling column_parameters::get_param.
	 *
	 *  \sa column_parameters
	 */
	COLUMN_PARAM
      };

      /** \brief The type of this column. */
      column_type type;

      /** \brief The parameter number (for positional parameter
       * columns) or column type (for generated columns).
       *
       *  \sa COLUMN_PARAM, COLUMN_GENERATED
       */
      int ival;

      /** \brief The text of this column if it is a literal column.
       *
       *  \sa COLUMN_LITERAL
       */
      std::wstring arg;

      /** \brief The width of this column if it is generated or taken from
       *  a positional parameter.
       *
       *  \sa COLUMN_GENERATED, COLUMN_PARAM
       */
      unsigned int width;
      /** \brief If \b true, this column is allowed to expand during layout. */
      bool expand:1;
      /** \brief If \b true, this column is allowed to shrink during layout. */
      bool shrink:1;

      /** \brief Whether to redefine the column width based on the
       * actual string (for generated and parametric columns).
       *
       *  If \b true, then #width will be ignored and the true width of the
       *  actual string will be used in layout.
       */
      bool dynamic_size:1;

      /** \brief Create a literal column. */
      column_definition(const std::wstring &_arg, bool _expand, bool _shrink)
	:type(COLUMN_LITERAL), arg(_arg), expand(_expand), shrink(_shrink)
      {
      }

      /** \brief Create a generated or parametric column. */
      column_definition(column_type _type,
			int _ival, int _width, bool _expand, bool _shrink,
			bool _dynamic_size)
	:type(_type), ival(_ival), width(_width),
	 expand(_expand), shrink(_shrink), dynamic_size(_dynamic_size)
      {
	eassert(_width>=0);
      }
    };

    /** \brief The type used to store lists of column definitions. */
    typedef std::list<column_definition> column_definition_list;

    /** \brief The type of a function that parses a single-character
     *  column type flag and returns an integer identifying the column
     *  type.
     */
    typedef int (*column_parser_func)(char id);

    /** \brief The class that defines how to parse and generate columns.
     *
     *  Typically, when formatting a list of columnar data, one
     *  instance of a subclass of column_generator will be created for
     *  each row.
     */
    class column_generator
    {
      column_definition_list columns;
    public:
      /** \brief Computes the text and column offset of a column of the given type. */
      virtual column_disposition setup_column(int type)=0;

      /** \brief Create a column generator for the given list of
       *  column specifications.
       */
      column_generator(const column_definition_list &_columns)
	:columns(_columns) {}

      virtual ~column_generator();

      /** \brief Given the target width and positional parameters,
       *  construct an output string to be displayed on the terminal.
       *
       *  \param width  The width of the terminal for which the columns
       *                are being formatted.
       *  \param p      The positional parameters passed to the layout operation.
       *
       *  \return a string of width #width formatted according to this
       *  object's definition list.
       */
      std::wstring layout_columns(unsigned int width,
				  column_parameters &p);
    };

    /** \brief Parse the given string into a list of column definitions.
     *
     *  \param config    The format string describing the columns to create.
     *  \param parser    The parsing function used to parse %-escapes.
     *  \param defaults  An array of the default options for each column;
     *                   must have elements corresponding to each column id
     *                   returned by the parser.
     *
     *  \return A freshly allocated column definition list.  The caller
     *  is responsible for deleting it.
     */
    column_definition_list *parse_columns(std::wstring config,
					  column_parser_func parser,
					  column_type_defaults *defaults);
  }
}

#endif
