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

#ifndef __RB_CALL_FAKE1_9_H
#define __RB_CALL_FAKE1_9_H

#include <ruby.h>
#include "ruby_version.h"

#ifdef RUBY1_9

void
rb_call_fake1_9_init();


VALUE
rb_call0_copy(
    VALUE klass, VALUE recv,
    ID    mid,
    int argc,			/* OK */
    const VALUE *argv,		/* OK */
    int scope,
    VALUE self
);

VALUE
rb_call0_fake(
    VALUE klass, VALUE recv,
    ID    mid,
    int argc,			/* OK */
    const VALUE *argv,		/* OK */
    int scope,
    VALUE self
);

extern void* rb_call0_original;
extern int hook_enabled;
extern int hook_enable_left;
extern VALUE rb_hook_proc;

#endif

#endif
