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
#include <tag_container.h>
#include "rb_yield_fake.h"

VALUE rb_cHook;
VALUE rb_mRallHook;
VALUE rb_mMethodRedirect;
VALUE rb_mMethodReturn;
ID id_call_;

VALUE unhook(VALUE self) {
	hook_enabled = 0;
	return Qnil;
}

#ifdef RUBY1_9
VMCALLMETHOD vm_call_method_copy;
#endif


int code_changed = 0;

VALUE hook(VALUE self, VALUE hook_proc) {
	rb_hook_proc = hook_proc;
	hook_enabled = 1;

	if (!code_changed) {
		// insert inconditional jmp from rb_call to rb_call_copy

		#ifdef __i386__
		rb_call_copy = (RBCALL)hook_rb_call(rb_call_fake_regs);
		#endif

		#ifdef __x86_64__
		rb_call_copy = (RBCALL)hook_rb_call(rb_call_fake);
		#endif
		// calibrate protocol of rb_call
		#ifdef __i386__
		is_calibrate = 1;

		VALUE test_value = LONG2FIX(0);

		calibrate_recv = test_value;
		calibrate_klass = CLASS_OF(test_value);
		calibrate_mid = rb_intern("to_s");

		rb_funcall(test_value, calibrate_mid, 0);
		

		#endif

		if (!rb_call_copy) {
			rb_raise( rb_eFatal, "libruby incompatible with rallhook");
		}

		#ifdef RUBY1_9
			#ifdef __i386__

			rb_eval_string("def Object.___calibrate_x(x); end");

			calibrate_recv = rb_cObject;
			calibrate_klass = CLASS_OF(rb_cObject);
			calibrate_mid = rb_intern("___calibrate_x");

			vm_call_method_copy = (VMCALLMETHOD)hook_vm_call_method(vm_call_method_fake_regs);
			vm_is_calibrate = 1;

			char code[256];
			snprintf(code, sizeof(code), "Object.___calibrate_x(%i)", test_value);
			rb_eval_string(code);

			#endif
			#ifdef __x86_64__
			vm_call_method_copy = (VMCALLMETHOD)hook_vm_call_method(vm_call_method_fake);
			#endif

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

		VALUE other_arguments;
		other_arguments = rb_ary_new3(1,arguments);
/*
		VALUE argumentsinspect = rb_inspect(other_arguments);
		VALUE arguments_inspect = rb_inspect(arguments);

		printf("_ changed %s by %s\n", rb_string_value_ptr( &arguments_inspect ), rb_string_value_ptr( &argumentsinspect ) );
*/
		return rb_yield_splat(rb_ary_new3(1,other_arguments));
	} else {
		return rb_yield_splat(arguments);

	}
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

VALUE rb_reunhook_proc_call( VALUE arguments ) {
	VALUE* vect = (VALUE*)arguments;
	VALUE recv = vect[1];
	ID mid = (ID)vect[2];
	int argc = (int)vect[3];
	VALUE* argv = (VALUE*)vect[4];

	return rb_funcall2(recv, mid, argc, argv );
}


VALUE ensured_recall( VALUE arguments ) {
	VALUE* vect = (VALUE*)arguments;
	VALUE recv = vect[1];
	ID mid = (ID)vect[2];
	int argc = (int)vect[3];
	VALUE* argv = (VALUE*)vect[4];

	if (rb_block_given_p() ) {
		return rb_iterate(rb_reunhook_proc_call, (VALUE)vect, reunhook_reyield, Qnil );
	} else {
		return rb_funcall2(recv,mid,argc,argv);
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

	if (klass != CLASS_OF(recv)) {
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
		VALUE other_arguments;
		other_arguments = rb_ary_new3(1,arguments);
/*
		VALUE argumentsinspect = rb_inspect(other_arguments);
		VALUE arguments_inspect = rb_inspect(arguments);

		printf("_ changed %s by %s\n", rb_string_value_ptr( &arguments_inspect ), rb_string_value_ptr( &argumentsinspect ) );
*/
		hook_enabled = 1;
		return rb_yield_splat(rb_ary_new3(1,other_arguments));
	} else {

		hook_enabled = 1;
		return rb_yield_splat(arguments);
	}

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

VALUE rb_rehook_proc_call( VALUE arg ) {
	VALUE* argv = (VALUE*)arg;
	return rb_funcall2(rb_hook_proc, id_call_, 5, argv );
}

VALUE rallhook_call(VALUE self, VALUE klass, VALUE recv, VALUE sym, VALUE args, VALUE mid){
	if (rb_block_given_p() ) {
		VALUE argv[6] = {klass, recv, sym, args, mid};
		return rb_iterate(rb_rehook_proc_call, (VALUE)argv, rehook_reyield, Qnil );
	} else {
		return rb_funcall(rb_hook_proc, id_call_, 5, klass, recv, sym, args, mid );
	}
}

extern void Init_rallhook_base() {

	const char* initcode = 	"require 'rubygems'\n"
						"require 'ruby-cymbol'\n";

	rb_eval_string(initcode);
	rb_mRallHook = rb_define_module("RallHook");
	rb_cHook = rb_define_class_under(rb_mRallHook, "Hook", rb_cObject);
	rb_define_method(rb_cHook, "hook", hook, 1);
	rb_define_method(rb_cHook, "unhook", unhook, 0);
	rb_define_method(rb_cHook, "from", from, 1);

	rb_define_singleton_method(rb_cHook, "call", rallhook_call, 5);

	rb_mMethodRedirect = rb_define_module_under(rb_mRallHook, "MethodRedirect");
	rb_mMethodReturn = rb_define_module_under(rb_mRallHook, "MethodReturn");


	rb_define_method(rb_cObject, "hooked_send", rb_f_send_copy, -1);

	init_hook_rb_call();
	rb_call_fake_init();
	init_node();
	init_tag_container();

#ifdef RUBY1_8
	init_rb_yield_fake();
#endif
	id_call_ = rb_intern("call");
/*

*/
}

