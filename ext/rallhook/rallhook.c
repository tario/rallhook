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
#include "ruby_symbols.h"
#include "rb_call_fake.h"
#include "hook_rb_call.h"
#include "method_node.h"
#include "method_wrapper.h"
#include <tag_container.h>
#include "rb_yield_fake.h"

VALUE rb_cRallHook;
ID id_call_;

VALUE unhook(VALUE self) {
	hook_enabled = 0;
	return Qnil;
}

RBCALL rb_call_copy;

#ifdef RUBY1_9
VMCALLMETHOD vm_call_method_copy;
#endif


int code_changed = 0;

VALUE hook(VALUE self, VALUE hook_proc) {
	rb_hook_proc = hook_proc;
	hook_enabled = 1;

	if (!code_changed) {
		// insert inconditional jmp from rb_call to rb_call_copy
		rb_call_copy = (RBCALL)hook_rb_call(rb_call_fake);

		if (!rb_call_copy) {
			rb_raise( rb_eFatal, "libruby incompatible with rallhook");
		}

		#ifdef RUBY1_9
			vm_call_method_copy = (VMCALLMETHOD)hook_vm_call_method(vm_call_method_fake);

			if (!vm_call_method_copy) {
				rb_raise( rb_eFatal, "libruby incompatible with rallhook");
			}
		#endif

		code_changed = 1;
	}

	if (rb_block_given_p() ) {
		return rb_ensure(rb_yield, Qnil, unhook, self);
	}

	return Qnil;
}


VALUE from(VALUE self, VALUE num) {
	hook_enable_left = FIX2INT(num)+1;
	return self;
}

#ifdef RUBY1_8
VALUE get_rb_yield_0_avalue();
#endif


VALUE restore_hook_status( VALUE unused) {
	hook_enabled = 1;
	return Qnil;
}

VALUE restore_unhook_status( VALUE unused) {
	hook_enabled = 0;
	return Qnil;
}


#ifdef RUBY1_8

VALUE reunhook_reyield_ensure( VALUE arguments) {
	hook_enabled = 0;
	// only work and is needed in ruby1.8
	if (! (get_rb_yield_0_avalue()==Qtrue) ) {
		arguments = rb_ary_new3(1,arguments);
	}
	return rb_yield_splat(arguments);
}


VALUE reunhook_reyield( VALUE arguments, VALUE args) {
	return rb_ensure(reunhook_reyield_ensure, arguments, restore_hook_status, Qnil);
}
#endif

#ifdef RUBY1_9
VALUE reunhook_reyield_ensure( VALUE arguments) {
	hook_enabled = 0;
	return rb_yield_splat(arguments);
}


VALUE reunhook_reyield( VALUE arguments, VALUE args, int argc, VALUE* argv) {
	return rb_ensure(reunhook_reyield_ensure, rb_ary_new4(argc, argv), restore_hook_status, Qnil);
}
#endif

VALUE ensured_recall( VALUE arguments ) {
	VALUE* vect = (VALUE*)arguments;
	VALUE klass = vect[0];
	VALUE recv = vect[1];
	ID mid = (ID)vect[2];
	int argc = (int)vect[3];
	VALUE* argv = (VALUE*)vect[4];

	if (rb_block_given_p() ) {
		return rb_block_call(recv, mid, argc, argv, reunhook_reyield, Qnil );
	} else {
		return rb_call_fake(klass,recv,mid,argc,argv,1,Qnil);
	}


}

static VALUE
rb_f_send_copy(argc, argv, recv)
    int argc;
    VALUE *argv;
    VALUE recv;
{
    VALUE vid;
    VALUE klass;

    if (argc < 1) rb_raise(rb_eArgError, "no method name given");



    klass = *argv++; argc--;
    vid = *argv++; argc--;

	ID mid;

	if (rb_obj_is_kind_of(vid,rb_cSymbol) ) {
		mid = rb_to_id(vid);
	} else {
		mid = FIX2LONG(vid);
	}

	if (rb_block_given_p() ) {
		recv = tag_container(recv,klass);
	}

	hook_enabled = 1;
	hook_enable_left = 1;

	VALUE args[7] = {klass,recv,(VALUE)mid,(VALUE)argc,(VALUE)argv};

	return rb_ensure(ensured_recall, (VALUE)args, restore_unhook_status, Qnil);
}

#ifdef RUBY1_8

VALUE get_rb_yield_0_avalue() {
	return last_avalue;
}

#endif

#ifdef RUBY1_8

VALUE rehook_reyield_ensure( VALUE arguments) {

	// only work and is needed in ruby1.8
	if (! (get_rb_yield_0_avalue()==Qtrue) ) {
		arguments = rb_ary_new3(1,arguments);
	}

	hook_enabled = 1;
	return rb_yield_splat(arguments);
}

VALUE rehook_reyield( VALUE arguments, VALUE args) {
	return rb_ensure(rehook_reyield_ensure, arguments, restore_unhook_status, Qnil);
}

#endif

#ifdef RUBY1_9

VALUE rehook_reyield_ensure( VALUE arguments) {
	hook_enabled = 1;
	return rb_yield_splat(arguments);
}

VALUE rehook_reyield( VALUE arguments, VALUE args, int argc, VALUE* argv) {
	return rb_ensure(rehook_reyield_ensure, rb_ary_new4(argc,argv), restore_unhook_status, Qnil);
}

#endif

VALUE rallhook_call(VALUE self, VALUE klass, VALUE recv, VALUE sym, VALUE args, VALUE mid){
	if (rb_block_given_p() ) {
		VALUE argv[6] = {klass, recv, sym, args, mid};
		return rb_block_call(rb_hook_proc, id_call_, 5, argv, rehook_reyield, Qnil );
	} else {
		return rb_funcall(rb_hook_proc, id_call_, 5, klass, recv, sym, args, mid );
	}
}

VALUE rallhook_new_method_wrapper(VALUE self, VALUE recv, VALUE klass, VALUE method_id) {
	VALUE mw = new_method_wrapper();

	method_wrapper_set_klass(mw, klass);
	method_wrapper_set_recv(mw, recv);
	method_wrapper_set_method_id_(mw, method_id);

	return mw;
}


VALUE method_wrapper_call(VALUE self, VALUE args) {

	VALUE klass = method_wrapper_get_klass(self);
	VALUE recv = method_wrapper_get_recv(self);
	VALUE sym;
	VALUE method_id = method_wrapper_get_method_id(self);
	ID mid = FIX2LONG( method_id );

	if (rb_id2name(mid) == NULL){
		sym = Qnil;
	} else {
		sym = ID2SYM(mid);
	}

	if (rb_block_given_p()) {
		VALUE argv[6] = {klass, recv, sym, args, method_id};
		return rb_block_call(rb_hook_proc, id_call_, 5, argv, rehook_reyield, Qnil );
	} else {

		return rb_funcall(rb_hook_proc, id_call_ ,5, klass, recv, sym, args, method_id);
	}
}

void Init_rallhook() {

	const char* initcode = 	"require 'rubygems'\n"
						"require 'ruby-cymbol'\n";

	rb_eval_string(initcode);

	rb_cRallHook = rb_define_class("RallHook", rb_cObject);
	rb_define_method(rb_cRallHook, "hook", hook, 1);
	rb_define_method(rb_cRallHook, "unhook", unhook, 0);
	rb_define_method(rb_cRallHook, "from", from, 1);

	rb_define_singleton_method(rb_cRallHook, "call", rallhook_call, 5);
	rb_define_singleton_method(rb_cRallHook, "method_wrapper", rallhook_new_method_wrapper, 3);

	rb_define_method(rb_cObject, "hooked_send", rb_f_send_copy, -1);

	init_hook_rb_call();
	rb_call_fake_init();
	init_node();
	init_tag_container();
	init_method_wrapper();

#ifdef RUBY1_8
	init_rb_yield_fake();
#endif

	rb_define_method(rb_cMethodWrapper, "call", method_wrapper_call, -2);

	id_call_ = rb_intern("call");
/*

*/
}

