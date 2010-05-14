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

#include <ruby.h>

#ifdef RUBY1_8
#include <node.h>
#include <env.h> // from ruby
#endif

#ifdef RUBY1_9
#include <ruby/node.h>
#endif

#include <dlfcn.h>
#include <stdarg.h>
#include "ruby_symbols.h"
#include "rb_call_fake.h"
#include "tag_container.h"

ID id_call;
ID id_method_wrapper;
VALUE rb_hook_proc = Qnil;

// extern, exported variables
int hook_enabled = 0;
int hook_enable_left = 0;

extern VALUE rb_cRallHook;

VALUE restore_hook_status_ensure(VALUE ary) {
	hook_enabled = 1;
}

VALUE rb_call_wrapper(VALUE ary){
		VALUE* argv = (VALUE*)ary;
		return rb_call_copy(CLASS_OF(rb_cRallHook), rb_cRallHook, id_call,5,argv,1,Qundef);
}

#ifdef RUBY1_9

typedef struct {
	rb_thread_t_* th;
	rb_control_frame_t_* cfp;
	int num;
	rb_block_t_* blockptr;
	VALUE flag;
	ID id;
	void *mn;
	VALUE recv;
	VALUE klass;
} vm_call_method_parameters_t;

VALUE vm_call_method_wrapper(VALUE ary ) {

		vm_call_method_parameters_t* params = (vm_call_method_parameters_t*)ary;

		// redirigir la llamada del metodo a otro objeto
		VALUE obj = rb_funcall(
			rb_cRallHook,
			id_method_wrapper,
			3,
			params->recv,
			params->klass,
			LONG2FIX(params->id)
			);

		params->recv = obj;
		params->klass = CLASS_OF(obj);
		params->mn = rb_method_node( params->klass, id_call);
		if (params->mn == 0) rb_bug("Null method node for method %s", rb_id2name(id_call) );

		params->id = id_call;


		return vm_call_method_copy(
				params->th,
				params->cfp,
				params->num,
				params->blockptr,
				params->flag,
				params->id,
				params->mn,
				params->recv,
				params->klass);
}

VALUE
vm_call_method_fake(rb_thread_t_ * const th, rb_control_frame_t_ * const cfp,
	       const int num, rb_block_t_ * const blockptr, const VALUE flag,
	       const ID id, void * mn, const VALUE recv_, VALUE klass)
{
	int must_hook = hook_enabled;
	volatile VALUE recv = recv_;

	if (is_tag(recv) ) {
		volatile VALUE orig_recv = recv;
		volatile VALUE klass_;
		recv = tag_container_get_self(orig_recv);
		klass_ = tag_container_get_tag(orig_recv);

		if (klass_ != Qnil ) {
			klass = klass_;
		}
	}

	if (must_hook == 0 || hook_enable_left > 0 ) {
		if (hook_enable_left > 0) hook_enable_left--;

		return vm_call_method_copy(th,cfp,num,blockptr,flag,id,mn,recv,klass);
	} else {
		hook_enabled = 0;

		vm_call_method_parameters_t params;

		params.th = th;
		params.cfp = cfp;
		params.num = num;
		params.blockptr = blockptr;
		params.flag = flag;
		params.id = id;
		params.mn = mn;
		params.recv = recv_;
		params.klass = klass;


//vm_call_method_fake(rb_thread_t_ * const th, rb_control_frame_t_ * const cfp,
	//       const int num, rb_block_t_ * const blockptr, const VALUE flag,
	  //     const ID id, void * mn, const VALUE recv_, VALUE klass)

		return rb_ensure(vm_call_method_wrapper,(VALUE)&params,restore_hook_status_ensure,Qnil);

	}
}

#endif


VALUE
rb_call_fake(
    VALUE klass, VALUE recv,
    ID    mid,
    int argc,			/* OK */
    const VALUE *argv,		/* OK */
    int scope,
    VALUE self
) {

	int must_hook = hook_enabled;

	if (is_tag(recv) ) {
		volatile VALUE orig_recv = recv;
		volatile VALUE klass_;
		recv = tag_container_get_self(orig_recv);
		klass_ = tag_container_get_tag(orig_recv);

		if (klass_ != Qnil ) {
			klass = klass_;
		}
	}

	if (must_hook == 0 || hook_enable_left > 0 ) {
		if (hook_enable_left > 0) hook_enable_left--;

		return rb_call_copy(klass,recv,mid,argc,argv,scope,self);
	} else {

//	    printf("called %s for %d, scope: %i klass: %i\n", rb_id2name(mid), recv, scope, klass);

		hook_enabled = 0;

		VALUE sym;

		// avoid to send symbols without name (crash the interpreter)
		if (rb_id2name(mid) == NULL){
			sym = Qnil;
		} else {
			sym = ID2SYM(mid);
		}

		VALUE args = rb_ary_new2(argc);
		int i;
		for (i = 0; i < argc; i ++) {
			rb_ary_store (args, i, argv[i] );
		}

		VALUE argv_[6];
		argv_[0] = klass;
		argv_[1] = recv;
		argv_[2] = sym;
		argv_[3] = args;
		argv_[4] = LONG2FIX(mid);

		return rb_ensure(rb_call_wrapper,(VALUE)argv_,restore_hook_status_ensure,Qnil);

	}
}

void
rb_call_fake_init() {

	id_call = rb_intern("call");
	id_method_wrapper = rb_intern("method_wrapper");
}
