/*

This file is part of the rallhook project, http://github.com/tario/rallhook

Copyright (c) 2009-2010 Roberto Dario Seminara <robertodarioseminara@gmail.com>

rallhook is free software: you can redistribute it and/or modify
it under the terms of the gnu general public license as published by
the free software foundation, either version 3 of the license, or
(at your option) any later version.

rallhook is distributed in the hope that it will be useful,
but without any warranty; without even the implied warranty of
merchantability or fitness for a particular purpose.  see the
gnu general public license for more details.

you should have received a copy of the gnu general public license
along with rallhook.  if not, see <http://www.gnu.org/licenses/>.

*/
#ifndef __METHOD_WRAPPER_H
#define __METHOD_WRAPPER_H

#include <ruby.h>

VALUE new_method_wrapper();

VALUE method_wrapper_set_klass(VALUE methodw, VALUE klass);
VALUE method_wrapper_set_recv(VALUE methodw, VALUE recv);
void method_wrapper_set_method_id(VALUE methodw, ID method_id);

VALUE method_wrapper_get_klass(VALUE methodw);
VALUE method_wrapper_get_recv(VALUE methodw);
VALUE method_wrapper_get_method_id(VALUE methodw);

VALUE method_wrapper_set_method_id_(VALUE methodw, VALUE method_id);

void init_method_wrapper();

extern VALUE rb_cMethodWrapper;

#endif
