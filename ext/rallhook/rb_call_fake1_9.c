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
#include <ruby_version.h>

#ifdef RUBY1_8
#include <node.h>
#endif

#ifdef RUBY1_9
#include <ruby/node.h>
#endif

//#include <ruby/env.h> // from ruby
#include <dlfcn.h>
#include <stdarg.h>
#include "ruby_symbols.h"
#include "rb_call_fake.h"
#include "tag_container.h"

#ifdef RUBY1_9

// same constant as eval.c
#define CSTAT_PRIV  1
#define CSTAT_PROT  2
#define CSTAT_VCALL 4
#define CSTAT_SUPER 8

// from vm_core.h

RUBY_EXTERN rb_thread_t *ruby_current_thread;
#define GET_THREAD() ruby_current_thread
#define UNLIKELY(x) (x)


// same defs as eval.c
#ifdef HAVE_STDARG_PROTOTYPES
#include <stdarg.h>
#define va_init_list(a,b) va_start(a,b)
#else
#include <varargs.h>
#define va_init_list(a,b) va_start(a)
#endif


ID missing, id_call;
VALUE rb_hook_proc;

typedef void (*STACKCHECK)(void) ;

typedef VALUE (*METHODMISSING)(VALUE obj,
    ID    id,
    int   argc,
    const VALUE *argv,
    int   call_status);

typedef VALUE (*VMCALL0) (
   	rb_thread_t * th, VALUE klass, VALUE recv,
    VALUE   id,
    ID    oid,
    int argc,			/* OK */
    VALUE *argv,		/* OK */
    NODE * volatile body,
    int nosuper);

typedef VALUE (*RBCALL0) (
    VALUE klass, VALUE recv,
    ID    id,
    ID    oid,
    int argc,			/* OK */
    VALUE *argv,		/* OK */
    int scope,
    VALUE self);


typedef NODE* (*RBGETMETHODBODY) (
    VALUE *klassp,
    ID *idp,
    int *noexp
    );



void* rb_call0_original;
METHODMISSING _method_missing;
RBCALL0 _rb_call0;
RBGETMETHODBODY _rb_get_method_body;
VMCALL0 _vm_call0;
STACKCHECK _stack_check;

// extern, exported variables
int hook_enabled = 0;
int hook_enable_left = 0;

extern VALUE rb_cRallHook;

static VALUE
rb_call0_copy(VALUE klass, VALUE recv, ID mid, int argc, const VALUE *argv,
	 int scope, VALUE self)
{
    NODE *body, *method;
    int noex;
    ID id = mid;
    struct cache_entry *ent;
    rb_thread_t *th = GET_THREAD();

    if (!klass) {
	rb_raise(rb_eNotImpError,
		 "method `%s' called on terminated object (%p)",
		 rb_id2name(mid), (void *)recv);
    }
    /* is it in the method cache? */
    /*ent = cache + EXPR1(klass, mid);

    if (ent->mid == mid && ent->klass == klass) {
	if (!ent->method)
	    return _method_missing(recv, mid, argc, argv,
				  scope == 2 ? NOEX_VCALL : 0);
	id = ent->mid0;
	noex = ent->method->nd_noex;
	klass = ent->method->nd_clss;
	body = ent->method->nd_body;
    }
    else */

    if ((method = _rb_get_method_body(klass, id, &id)) != 0) {
	noex = method->nd_noex;
	klass = method->nd_clss;
	body = method->nd_body;
    }
    else {
	if (scope == 3) {
	    return _method_missing(recv, mid, argc, argv, NOEX_SUPER);
	}
	return _method_missing(recv, mid, argc, argv,
			      scope == 2 ? NOEX_VCALL : 0);
    }


    if (mid != missing) {
	/* receiver specified form for private method */
	if (UNLIKELY(noex)) {
	    if (((noex & NOEX_MASK) & NOEX_PRIVATE) && scope == 0) {
		return _method_missing(recv, mid, argc, argv, NOEX_PRIVATE);
	    }

	    /* self must be kind of a specified form for protected method */
	    if (((noex & NOEX_MASK) & NOEX_PROTECTED) && scope == 0) {
		VALUE defined_class = klass;

		if (TYPE(defined_class) == T_ICLASS) {
		    defined_class = RBASIC(defined_class)->klass;
		}

		if (self == Qundef) {
		//    self = th->cfp->self; // FIXME: discomment
		}
		if (!rb_obj_is_kind_of(self, rb_class_real(defined_class))) {
		    return _method_missing(recv, mid, argc, argv, NOEX_PROTECTED);
		}
	    }

//	    if (NOEX_SAFE(noex) > th->safe_level) {	// FIXME: discomment
//		rb_raise(rb_eSecurityError, "calling insecure method: %s", rb_id2name(mid));
//	    }
	}
    }

    _stack_check();
    return _vm_call0(th, klass, recv, mid, id, argc, argv, body, noex & NOEX_NOSUPER);
}

VALUE restore_hook_status_ensure(VALUE ary) {
	hook_enabled = 1;
	return Qnil;
}

VALUE rb_call_wrapper(VALUE ary){
		VALUE* argv = (VALUE*)ary;
		return rb_call0_copy(CLASS_OF(rb_cRallHook), rb_cRallHook, id_call,5,argv,1,Qundef);
}

VALUE
rb_call0_fake(
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

		return rb_call0_copy(klass,recv,mid,argc,argv,scope,self);
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

//	    if (recv == Qundef) recv = (*_ruby_frame)->self;

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
rb_call_fake1_9_init() {

	missing = rb_intern("method_missing");
	const char* strlibruby = current_libruby();

	void* handle = dlopen(strlibruby,0x101);

	if (handle == NULL) {
		rb_raise(rb_eLoadError, "Cannot find %s", strlibruby);
	}

	char* rb_funcall = (char*)dlsym(handle, "rb_funcall");

	if (rb_funcall == NULL) {
		rb_raise(rb_eLoadError, "Cannot find rb_funcall in %s",  strlibruby);
	}

	Dl_info info;
	dladdr(rb_funcall, &info);

	unsigned char* base = (unsigned char*)info.dli_fbase;

	rb_call0_original = ruby_resolv(base, "rb_call0");
	_method_missing = (METHODMISSING)ruby_resolv(base, "method_missing");
	_rb_call0 = (RBCALL0)ruby_resolv(base,"rb_call0");
	_rb_get_method_body = (RBGETMETHODBODY)ruby_resolv(base,"rb_get_method_body");
	_vm_call0 = (VMCALL0)ruby_resolv(base,"vm_call0");
	_stack_check = (STACKCHECK)ruby_resolv(base,"stack_check");

	id_call = rb_intern("call");


}

#endif
