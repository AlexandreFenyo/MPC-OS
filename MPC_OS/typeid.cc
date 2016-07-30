
// $Id: typeid.cc,v 1.1.1.1 1998/10/28 21:07:31 alex Exp $

#include <stdio.h>

#include <new>
#include <cerrno>

// with egcs: comment next line
#include <g++/typeinfo>

#include <stdexcept>
#include <exception>
#include <g++/iostream.h>

void cerr_typeid(exception &exc)
{
  // with egcs: comment next line
cerr << typeid(exc).name();
}

