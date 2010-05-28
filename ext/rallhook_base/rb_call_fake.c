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

#include <dlfcn.h>
#include <stdarg.h>
#include "ruby_symbols.h"
#include "rb_call_fake.h"
#include "ruby_redirect.h"

void* rb_call_copy;

extern REDIRECTHANDLER current_redirect_handler;

#ifdef __i386__

VALUE read_eax( ) {
}

VALUE read_edx( ) {
__asm__("mov %edx, %eax");
}

VALUE read_ecx( ) {
__asm__("mov %ecx, %eax");
}

int rb_call_write_eax(int value) {
	return value;
}

int is_fastcall = 1;
int is_calibrate = 0;
int vm_is_fastcall = 1;
int vm_is_calibrate = 0;
VALUE calibrate_klass;
VALUE calibrate_recv;
ID calibrate_mid;

#endif

#ifdef RUBY1_9

VMCALLMETHOD vm_call_method_copy;

typedef void rb_vm_t_;
#endif

VALUE restore_hook_status_ensure(VALUE unused) {
	enable_redirect();
	return Qnil;
}


VALUE rb_call_copy_i(
    VALUE klass, VALUE recv,
    ID    mid,
    int argc,			/* OK */
    const VALUE *argv,		/* OK */
    int scope,
    VALUE self
) {
#ifdef __i386__
	if (is_fastcall == 1) {

		int array[7] = { (int)klass, (int)recv, mid, argc, (int)argv, scope, (int)self };
		rb_call_write_eax(array);

		__asm__("push %ebp\n");	// save all registers
		__asm__("push %esi\n");
		__asm__("push %edi\n");
		__asm__("push %ebx\n");
		__asm__("push %edx\n");
		__asm__("push %ecx\n");
		__asm__("mov 0x4(%eax), %edx\n");
		__asm__("mov 0x8(%eax), %ecx\n");
		__asm__("push 0x18(%eax)\n");
		__asm__("push 0x14(%eax)\n");
		__asm__("push 0x10(%eax)\n");
		__asm__("push 0x0c(%eax)\n");
		__asm__("mov (%eax), %eax\n");
		__asm__("call *rb_call_copy\n");
		__asm__("add $0x10, %esp\n");
		__asm__("pop %ecx\n");
		__asm__("pop %edx\n");
		__asm__("pop %ebx\n");
		__asm__("pop %edi\n");
		__asm__("pop %esi\n");
		__asm__("pop %ebp\n");
		return read_eax();

	} else if ( is_fastcall == 2) {
		int array[7] = { (int)klass, (int)recv, mid, argc, (int)argv, scope, (int)self };
		rb_call_write_eax(array);

		__asm__("push %ebp\n");	// save all registers
		__asm__("push %esi\n");
		__asm__("push %edi\n");
		__asm__("push %ebx\n");
		__asm__("push %edx\n");
		__asm__("push %ecx\n");
		__asm__("mov 0x4(%eax), %edx\n");
		__asm__("push 0x18(%eax)\n");
		__asm__("push 0x14(%eax)\n");
		__asm__("push 0x10(%eax)\n");
		__asm__("push 0x0c(%eax)\n");
		__asm__("push 0x08(%eax)\n");
		__asm__("mov (%eax), %eax\n");
		__asm__("call *rb_call_copy\n");
		__asm__("add $0x14, %esp\n");
		__asm__("pop %ecx\n");
		__asm__("pop %edx\n");
		__asm__("pop %ebx\n");
		__asm__("pop %edi\n");
		__asm__("pop %esi\n");
		__asm__("pop %ebp\n");
		return read_eax();

	} else {
#endif
	return ((RBCALL)rb_call_copy)(klass,recv,mid,argc,argv,scope,self);
#ifdef __i386__
	}
#endif

}

#ifdef RUBY1_9

VALUE
vm_call_method_i(rb_thread_t_ * const th, rb_control_frame_t_ * const cfp,
	       const int num, rb_block_t_ * const blockptr, const VALUE flag,
	       const ID id, void * mn, const VALUE recv_, VALUE klass)
{

#ifdef __i386__
	if (vm_is_fastcall == 1) {

		int array[9] = { (int)th, (int)cfp, num, (int)blockptr, (int)flag, (int)id, (int)mn, (int)recv_, (int)klass };
		rb_call_write_eax(array);

		__asm__("push %ebp\n");	// save all registers
		__asm__("push %esi\n");
		__asm__("push %edi\n");
		__asm__("push %ebx\n");
		__asm__("push %edx\n");
		__asm__("push %ecx\n");
		__asm__("mov 0x4(%eax), %edx\n");
		__asm__("mov 0x8(%eax), %ecx\n");
		__asm__("push 0x20(%eax)\n");
		__asm__("push 0x1c(%eax)\n");
		__asm__("push 0x18(%eax)\n");
		__asm__("push 0x14(%eax)\n");
		__asm__("push 0x10(%eax)\n");
		__asm__("push 0x0c(%eax)\n");
		__asm__("mov (%eax), %eax\n");
		__asm__("call *vm_call_method_copy\n");
		__asm__("add $0x18, %esp\n");
		__asm__("pop %ecx\n");
		__asm__("pop %edx\n");
		__asm__("pop %ebx\n");
		__asm__("pop %edi\n");
		__asm__("pop %esi\n");
		__asm__("pop %ebp\n");
		return read_eax();

	} else if ( vm_is_fastcall == 2) {
		int array[9] = { (int)th, (int)cfp, num, (int)blockptr, (int)flag, (int)id, (int)mn, (int)recv_, (int)klass };
		rb_call_write_eax(array);

		__asm__("push %ebp\n");	// save all registers
		__asm__("push %esi\n");
		__asm__("push %edi\n");
		__asm__("push %ebx\n");
		__asm__("push %edx\n");
		__asm__("push %ecx\n");
		__asm__("mov 0x4(%eax), %edx\n");
		__asm__("push 0x20(%eax)\n");
		__asm__("push 0x1c(%eax)\n");
		__asm__("push 0x18(%eax)\n");
		__asm__("push 0x14(%eax)\n");
		__asm__("push 0x10(%eax)\n");
		__asm__("push 0x0c(%eax)\n");
		__asm__("push 0x08(%eax)\n");
		__asm__("mov (%eax), %eax\n");
		__asm__("call *vm_call_method_copy\n");
		__asm__("add $0x1c, %esp\n");
		__asm__("pop %ecx\n");
		__asm__("pop %edx\n");
		__asm__("pop %ebx\n");
		__asm__("pop %edi\n");
		__asm__("pop %esi\n");
		__asm__("pop %ebp\n");
		return read_eax();

	} else {
#endif
		return vm_call_method_copy(th,cfp,num,blockptr,flag,id,mn,recv_,klass);
#ifdef __i386__
	}
#endif


}

VALUE
vm_call_method_fake(rb_thread_t_ * const th, rb_control_frame_t_ * const cfp,
	       const int num, rb_block_t_ * const blockptr, const VALUE flag,
	       const ID id, void * mn, const VALUE recv, VALUE klass)
{
	CallData call_data;

	call_data.klass = klass;
	call_data.recv = recv;
	call_data.mid = id;
	call_data.args = Qnil;

	current_redirect_handler(&call_data);

	return vm_call_method_i(
			th,
			cfp,
			num,
			blockptr,
			flag,
			call_data.mid,
			mn,
			call_data.recv,
			call_data.klass);


}

#ifdef __i386__

VALUE vm_call_method_fake_regs(
	_WORD eax, _WORD edx, _WORD ecx, _WORD* esp
) {

	esp++;

	if (vm_is_calibrate) {
		if ((VALUE)esp[7] == calibrate_recv && (VALUE)esp[8] == calibrate_klass && (ID)esp[5] == calibrate_mid) {
			vm_is_fastcall = 0;
		} else if ( (VALUE)esp[5] == calibrate_recv ) {
			vm_is_fastcall = 2;
		} else {
			vm_is_fastcall = 1;
		}
		vm_is_calibrate = 0;
	}

	if (vm_is_fastcall == 0) {
		return vm_call_method_fake(
			(rb_thread_t_*)esp[0],
			(rb_control_frame_t_*)esp[1],
			(int)esp[2],
			(rb_block_t_*)esp[3],
			(VALUE)esp[3],
			(ID)esp[4],
			(void*)esp[5],
			(VALUE)esp[6],
			(VALUE)esp[7]
			);
	} else if (vm_is_fastcall == 2 ) {
		return vm_call_method_fake(
			(rb_thread_t_*)eax,
			(rb_control_frame_t_*)edx,
			(int)esp[0],
			(rb_block_t_*)esp[1],
			(VALUE)esp[2],
			(ID)esp[3],
			(void*)esp[4],
			(VALUE)esp[5],
			(VALUE)esp[6]
			);

	} else {
		return vm_call_method_fake(
			(rb_thread_t_*)eax,
			(rb_control_frame_t_*)edx,
			(int)ecx,
			(rb_block_t_*)esp[0],
			(VALUE)esp[1],
			(ID)esp[2],
			(void*)esp[3],
			(VALUE)esp[4],
			(VALUE)esp[5]
			);

	}

}

#endif

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

	VALUE args;
	if (argv == 0) {
		args = rb_ary_new2(0);
	} else {
		args = rb_ary_new4(argc, argv);
	}


	CallData call_data;

	call_data.klass = klass;
	call_data.args = args;
	call_data.recv = recv;
	call_data.mid = mid;

	current_redirect_handler(&call_data);
	return rb_call_copy_i(call_data.klass,call_data.recv,call_data.mid,argc,argv,scope,self);

}


#ifdef __i386__

VALUE
rb_call_fake_regs(
	_WORD eax, _WORD edx, _WORD ecx, _WORD* esp
) {
	VALUE klass;
	VALUE recv;
	ID    mid;
	int argc;			/* OK */
	const VALUE *argv;
	int scope;
	VALUE self;

	esp++;


	if (is_calibrate) {

		if ((VALUE)edx == calibrate_recv && (VALUE)eax == calibrate_klass && (ID)ecx == calibrate_mid) {
			is_fastcall = 1;
		} else if ( (VALUE)edx == calibrate_recv && (VALUE)eax == calibrate_klass && (ID)esp[0] == calibrate_mid ) {
			is_fastcall = 2;
		} else {
			is_fastcall = 0;
		}
		is_calibrate = 0;
		return Qnil;
	}

	if (is_fastcall == 0) {
		klass = (VALUE)esp[0];
		recv = (VALUE)esp[1];
		mid = (ID)esp[2];
		argc = (int)esp[3];
		argv = (VALUE*)esp[4];
		scope = (int)esp[5];
		self = (VALUE)esp[6];
	} else if (is_fastcall == 2) {
		klass = (VALUE)eax;
		recv = (VALUE)edx;
		mid = (VALUE)esp[0];
		argc = (int)esp[1];
		argv = (VALUE*)esp[2];
		scope = (int)esp[3];
		self = (VALUE)esp[4];
	} else {
		klass = (VALUE)eax;
		recv = (VALUE)edx;
		mid = (VALUE)ecx;
		argc = (int)esp[0];
		argv = (VALUE*)esp[1];
		scope = (int)esp[2];
		self = (VALUE)esp[3];
	}
	return rb_call_fake(klass,recv,mid,argc,argv,scope,self);
}

#endif

void
rb_call_fake_init() {

}
