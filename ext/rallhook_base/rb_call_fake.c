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
#include "tag_container.h"

ID id_call;
ID id_method_wrapper;
ID id_handle_method;

ID id_return_value_var, id_klass_var, id_recv_var, id_method_var;

VALUE rb_hook_proc = Qnil;

// extern, exported variables
int hook_enabled = 0;
int hook_enable_left = 0;
void* rb_call_copy;
extern VALUE rb_mMethodRedirect;
extern VALUE rb_mMethodReturn;



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

typedef void rb_vm_t_;

typedef struct rb_thread_struct
{
    VALUE self;
    rb_vm_t_ *vm;

    /* execution information */
    VALUE *stack;		/* must free, must mark */
    unsigned long stack_size;
    rb_control_frame_t_ *cfp;
    int safe_level;
    int raised_flag;
    VALUE last_status; /* $? */

    /* passing state */
    int state;

    /* for rb_iterate */
    rb_block_t_ *passed_block;

    // ...
} rb_thread_t__;
#endif

VALUE restore_hook_status_ensure(VALUE unused) {
	hook_enabled = 1;
	return Qnil;
}

#ifdef RUBY1_9
VALUE restore_hook_status_ensure_rb_call(VALUE argument) {

	void** parameters = (void**)argument;
	void*  passed_block = parameters[0];
	rb_thread_t__* th = parameters[1];

	// restore the passed_block
	th->passed_block = passed_block;

	hook_enabled = 1;

	return Qnil;
}

#endif

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


typedef struct {
    VALUE klass; VALUE recv;
    ID    mid;
    int argc;			/* OK */
    const VALUE *argv;		/* OK */
    int scope;
    VALUE self;
} rb_call_parameters_t;

VALUE rb_call_wrapper(VALUE parameters){

	rb_call_parameters_t* params = (rb_call_parameters_t*)parameters;


	VALUE sym;

	// avoid to send symbols without name (crash the interpreter)
	if (rb_id2name(params->mid) == NULL){
		sym = Qnil;
	} else {
		sym = ID2SYM(params->mid);
	}

	VALUE args = rb_ary_new2(params->argc);
	int i;
	for (i = 0; i < params->argc; i ++) {
		rb_ary_store (args, i, params->argv[i] );
	}

	VALUE argv_[6];
	argv_[0] = params->klass;
	argv_[1] = params->recv;
	argv_[2] = sym;
	argv_[3] = args;
	argv_[4] = LONG2FIX(params->mid);

	return rb_funcall2( rb_hook_proc, id_handle_method, 5, argv_);
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

		VALUE sym;

		// avoid to send symbols without name (crash the interpreter)
		if (rb_id2name(params->id) == NULL){
			sym = Qnil;
		} else {
			sym = ID2SYM(params->id);
		}


		VALUE argv_[6];
		argv_[0] = params->klass;
		argv_[1] = params->recv;
		argv_[2] = sym;
		argv_[3] = Qnil;
		argv_[4] = LONG2FIX(params->id);

		return rb_funcall2( rb_hook_proc, id_handle_method, 5, argv_);
}

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

		return vm_call_method_i(th,cfp,num,blockptr,flag,id,mn,recv,klass);
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

		VALUE result = rb_ensure(vm_call_method_wrapper,(VALUE)&params,restore_hook_status_ensure,Qnil);

		if (rb_obj_is_kind_of(result,rb_mMethodReturn) == Qtrue ) {
			return rb_ivar_get(result, id_return_value_var );
		}
		if (rb_obj_is_kind_of(result,rb_mMethodRedirect) == Qtrue ) {

			VALUE klass_ = rb_ivar_get(result,id_klass_var );
			VALUE recv_ = rb_ivar_get(result,id_recv_var );
			ID mid_ = rb_to_id( rb_ivar_get(result,id_method_var) );

			void *mn_ = rb_method_node( klass_, mid_);

			if (mn_ == 0) {
				rb_bug("Null method node for method %s", rb_id2name(mid_) );
			}

			return vm_call_method_i(
				th,
				cfp,
				num,
				blockptr,
				flag,
				mid_,
				mn_,
				recv_,
				klass_);
		}

		return vm_call_method_i(
				th,
				cfp,
				num,
				blockptr,
				flag,
				id,
				mn,
				recv,
				klass);


	}
}


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
		return rb_call_copy_i(klass,recv,mid,argc,argv,scope,self);
	} else {

		hook_enabled = 0;

		rb_call_parameters_t params;

		params.klass = klass;
		params.recv = recv;
		params.mid = mid;
		params.argc = argc;
		params.argv = argv;
		params.scope = scope;
		params.self = self;

#ifdef RUBY1_9
		rb_thread_t__* th;
		VALUE current_thread = rb_thread_current();
		Data_Get_Struct( current_thread, rb_thread_t__, th );
		void* parameters[2] = {th->passed_block, th};

		VALUE result = rb_ensure(rb_call_wrapper,(VALUE)&params,restore_hook_status_ensure_rb_call,(VALUE)parameters);
#else
		VALUE result = rb_ensure(rb_call_wrapper,(VALUE)&params,restore_hook_status_ensure,Qnil);
#endif


		if (rb_obj_is_kind_of(result,rb_mMethodReturn) == Qtrue ) {
			return rb_ivar_get(result, id_return_value_var );
		}
		if (rb_obj_is_kind_of(result,rb_mMethodRedirect) == Qtrue ) {

			VALUE klass_ = rb_ivar_get(result,id_klass_var );
			VALUE recv_ = rb_ivar_get(result,id_recv_var );
			ID mid_ = rb_to_id( rb_ivar_get(result,id_method_var) );

			return rb_call_copy_i(klass_,recv_,mid_,argc,argv,scope,self);
		}

		return rb_call_copy_i(klass,recv,mid,argc,argv,scope,self);

	}

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

	id_call = rb_intern("call");
	id_method_wrapper = rb_intern("method_wrapper");
	id_handle_method = rb_intern("handle_method");
	id_return_value_var = rb_intern("@return_value");
	id_klass_var = rb_intern("@klass");
	id_recv_var = rb_intern("@recv");
	id_method_var = rb_intern("@method");
}
