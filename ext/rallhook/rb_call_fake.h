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

#ifndef __RB_CALL_FAKE_H
#define __RB_CALL_FAKE_H

#include <ruby.h>
#include "ruby_version.h"

void
rb_call_fake_init();

typedef VALUE (*RBCALL) (
    VALUE klass, VALUE recv,
    ID    mid,
    int argc,			/* OK */
    const VALUE *argv,		/* OK */
    int scope,
    VALUE self
) ;

//typedef void rb_control_frame_t;


#ifdef RUBY1_9

typedef void rb_block_t_;
typedef void rb_iseq_t_;
typedef void rb_thread_t_;
// from vm_core.h
typedef struct {
    VALUE *pc;
    VALUE *sp;
    VALUE *bp;
    rb_iseq_t_ *iseq;
    VALUE flag;
    VALUE self;
    VALUE *lfp;
    VALUE *dfp;
    rb_iseq_t_ *block_iseq;
    VALUE proc;
    ID method_id;
    VALUE method_class;
} rb_control_frame_t_;

typedef VALUE (*VMCALLMETHOD) (
	rb_thread_t_ * th, rb_control_frame_t_ *cfp,
    const int num, rb_block_t_ * blockptr, const VALUE flag,
    const ID id, void * mn, const VALUE recv, VALUE klass
) ;
#endif

extern RBCALL rb_call_copy;

#ifdef RUBY1_9
extern VMCALLMETHOD vm_call_method_copy;
VALUE
vm_call_method_fake(rb_thread_t_ * const th, rb_control_frame_t_ * const cfp,
	       const int num, rb_block_t_ * const blockptr, const VALUE flag,
	       const ID id, void * mn, const VALUE recv_, VALUE klass);
#endif

VALUE
rb_call_fake(
    VALUE klass, VALUE recv,
    ID    mid,
    int argc,			/* OK */
    const VALUE *argv,		/* OK */
    int scope,
    VALUE self
);


extern void* rb_call_original;
extern int hook_enabled;
extern int hook_enable_left;
extern VALUE rb_hook_proc;

#endif
