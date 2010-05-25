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

VALUE rehook(VALUE unused) {
	hook_enabled = 1;
	return Qnil;
}

extern void Init_rallhook_base() {

	const char* initcode = 	"require 'rubygems'\n"
						"require 'ruby-cymbol'\n";

	rb_eval_string(initcode);
	rb_mRallHook = rb_define_module("RallHook");
	rb_cHook = rb_define_class_under(rb_mRallHook, "Hook", rb_cObject);

	rb_define_singleton_method( rb_cHook, "rehook", rehook, 0 );

	rb_define_method(rb_cHook, "hook", hook, 1);
	rb_define_method(rb_cHook, "unhook", unhook, 0);
	rb_define_method(rb_cHook, "from", from, 1);

	rb_mMethodRedirect = rb_define_module_under(rb_mRallHook, "MethodRedirect");

	init_hook_rb_call();
	rb_call_fake_init();
	init_node();

#ifdef RUBY1_8
	init_rb_yield_fake();
#endif
	id_call_ = rb_intern("call");
/*

*/
}

