// Tests for generic/util/ssprintf.
//
//   Copyright (C) 2007-2008 Daniel Burrows
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

#include <cppunit/extensions/HelperMacros.h>

#include <cppunit/TestAssert.h>

#include <cwidget/generic/util/ssprintf.h>

// For displaying assertion failures.
#include <cwidget/generic/util/transcode.h>

#include <errno.h>
#include <string.h>

using cwidget::util::ssprintf;
using cwidget::util::swsprintf;

CPPUNIT_NS_BEGIN

template <>
struct assertion_traits<std::wstring>
{
  static bool equal(const std::wstring &x, const std::wstring &y)
  {
    return x == y;
  }

  static std::string toString(const std::wstring &x)
  {
    return cwidget::util::transcode(x);
  }
};

CPPUNIT_NS_END

class SSPrintfTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SSPrintfTest);

  CPPUNIT_TEST(test_sstrerror);
  CPPUNIT_TEST(test_ssprintf);
  CPPUNIT_TEST(test_swsprintf);

  CPPUNIT_TEST_SUITE_END();
private:
  void do_test_sstrerror(int errnum)
  {
    std::string expected = strerror(errnum);
    CPPUNIT_ASSERT_EQUAL(expected, cwidget::util::sstrerror(errnum));
  }

  // This test was written because the original sstrerror was buggy
  // (returned garbage) on some platforms.  To verify that it works we
  // check that it returns the same thing as strerror for a few error
  // values.
  void test_sstrerror()
  {
    do_test_sstrerror(0);
    do_test_sstrerror(EINVAL);
    do_test_sstrerror(EINTR);
    do_test_sstrerror(EBADF);
  }

  void test_ssprintf()
  {
    // Test that inserting very long strings via ssprintf actually works.
    std::string horriblelongthing = "abcdefghijklmnopqrstuvwxyz";
    while(horriblelongthing.size() < 4096)
      horriblelongthing += horriblelongthing;

    CPPUNIT_ASSERT_EQUAL(horriblelongthing + " 20", ssprintf("%s %d", horriblelongthing.c_str(), 20));

    // Test that we can generate empty strings.
    CPPUNIT_ASSERT_EQUAL(std::string(), ssprintf("%s", ""));
  }

  void test_swsprintf()
  {
    // Test that inserting very long strings via ssprintf actually works.
    std::wstring horriblelongthing = L"abcdefghijklmnopqrstuvwxyz";
    while(horriblelongthing.size() < 4096)
      horriblelongthing += horriblelongthing;

    CPPUNIT_ASSERT_EQUAL(horriblelongthing + L" 20", swsprintf(L"%ls %d", horriblelongthing.c_str(), 20));

    // Test that we can generate empty strings.
    CPPUNIT_ASSERT_EQUAL(std::wstring(), swsprintf(L"%s", ""));
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SSPrintfTest);
