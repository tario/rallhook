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
#include "ruby_redirect.h"
#include "ruby_symbols.h"
#include "rb_call_fake.h"
#include "hook_rb_call.h"
#include "pthread.h"

int code_changed = 0;

REDIRECTHANDLER current_redirect_handler;

void init_redirect();

int put_redirect_handler_( REDIRECTHANDLER redirect_handler) {
	current_redirect_handler = redirect_handler;

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

	return 0;
}


pthread_mutex_t put_redirect_handler_mutex;

int put_redirect_handler( REDIRECTHANDLER redirect_handler) {

	pthread_mutex_lock(&put_redirect_handler_mutex);
	int ret = put_redirect_handler_(redirect_handler);
	pthread_mutex_unlock(&put_redirect_handler_mutex);
	return ret;
}


void init_redirect() {

	pthread_mutexattr_t attr;

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr,  PTHREAD_MUTEX_NORMAL);

	pthread_mutex_init( &put_redirect_handler_mutex, &attr);

	init_hook_rb_call();
	rb_call_fake_init();
}



