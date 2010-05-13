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
typedef void rb_block_t;
typedef void rb_iseq_t;
// from vm_core.h
typedef struct {
    VALUE *pc;			/* cfp[0] */
    VALUE *sp;			/* cfp[1] */
    VALUE *bp;			/* cfp[2] */
    rb_iseq_t *iseq;		/* cfp[3] */
    VALUE flag;			/* cfp[4] */
    VALUE self;			/* cfp[5] / block[0] */
    VALUE *lfp;			/* cfp[6] / block[1] */
    VALUE *dfp;			/* cfp[7] / block[2] */
    rb_iseq_t *block_iseq;	/* cfp[8] / block[3] */
    VALUE proc;			/* cfp[9] / block[4] */
    ID method_id;               /* cfp[10] saved in special case */
    VALUE method_class;         /* cfp[11] saved in special case */
} rb_control_frame_t;

typedef VALUE (*VMCALLMETHOD) (
	rb_thread_t * const th, rb_control_frame_t * const cfp,
    const int num, rb_block_t * const blockptr, const VALUE flag,
    const ID id, const NODE * mn, const VALUE recv, VALUE klass
) ;
#endif

extern RBCALL rb_call_copy;

#ifdef RUBY1_9
extern VMCALLMETHOD vm_call_method_copy;
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

VALUE
vm_call_method_fake(rb_thread_t * const th, rb_control_frame_t * const cfp,
	       const int num, rb_block_t * const blockptr, const VALUE flag,
	       const ID id, const NODE * mn, const VALUE recv, VALUE klass);

extern void* rb_call_original;
extern int hook_enabled;
extern int hook_enable_left;
extern VALUE rb_hook_proc;

#endif
