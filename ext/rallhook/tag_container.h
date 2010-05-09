/*

This file is part of the rallhook project, http//github.com/tario/rallhook

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
#ifndef __TAG_CONTAINTER
#define __TAG_CONTAINTER

#include <ruby.h>

VALUE tag_container(VALUE self, VALUE tag);
VALUE tag_container_get_self(VALUE tag);
VALUE tag_container_get_klass(VALUE tag);
int is_tag(VALUE tag);

void init_tag_container();

#endif