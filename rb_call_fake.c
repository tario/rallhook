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

#include "rb_call_fake.h"
#include <ruby.h>
#include <node.h>
#include <dlfcn.h>
#include <stdarg.h>

// same constant as eval.c
#define CSTAT_PRIV  1
#define CSTAT_PROT  2
#define CSTAT_VCALL 4
#define CSTAT_SUPER 8

ID missing;

typedef VALUE (*METHODMISSING)(VALUE obj,
    ID    id,
    int   argc,
    const VALUE *argv,
    int   call_status);

typedef VALUE (*RBCALL0) (
    VALUE klass, recv,
    ID    id,
    ID    oid,
    int argc,			/* OK */
    VALUE *argv,		/* OK */
    NODE * volatile body,
    int flags);


void* rb_call_original;
METHODMISSING _method_missing;
RBCALL0 _rb_call0;

static VALUE
rb_call_copy(
    VALUE klass, VALUE recv,
    ID    mid,
    int argc,			/* OK */
    const VALUE *argv,		/* OK */
    int scope,
    VALUE self
) {
    NODE  *body;		/* OK */
    int    noex;
    ID     id = mid;
    struct cache_entry *ent;

    if (!klass) {
	rb_raise(rb_eNotImpError, "method `%s' called on terminated object (0x%lx)",
		 rb_id2name(mid), recv);
    }
    /* is it in the method cache? */
/*    ent = cache + EXPR1(klass, mid);
    if (ent->mid == mid && ent->klass == klass) {
	if (!ent->method)
	    goto nomethod;
	klass = ent->origin;
	id    = ent->mid0;
	noex  = ent->noex;
	body  = ent->method;
    }
    else
    */ if ((body = rb_get_method_body(&klass, &id, &noex)) == 0) {
      nomethod:
	if (scope == 3) {
	    return _method_missing(recv, mid, argc, argv, CSTAT_SUPER);
	}
	return _method_missing(recv, mid, argc, argv, scope==2?CSTAT_VCALL:0);
    }

    if (mid != missing && scope == 0) {
	/* receiver specified form for private method */
	if (noex & NOEX_PRIVATE)
	    return _method_missing(recv, mid, argc, argv, CSTAT_PRIV);

	/* self must be kind of a specified form for protected method */
	if (noex & NOEX_PROTECTED) {
	    VALUE defined_class = klass;

//	    if (self == Qundef) self = ruby_frame->self;
	    if (TYPE(defined_class) == T_ICLASS) {
		defined_class = RBASIC(defined_class)->klass;
	    }
	    if (!rb_obj_is_kind_of(self, rb_class_real(defined_class)))
		return _method_missing(recv, mid, argc, argv, CSTAT_PROT);
	}
    }

    return _rb_call0(klass, recv, mid, id, argc, argv, body, noex);
}

static VALUE
rb_call_fake(
    VALUE klass, VALUE recv,
    ID    mid,
    int argc,			/* OK */
    const VALUE *argv,		/* OK */
    int scope,
    VALUE self
) {
	return Qnil;
}

void
rb_call_fake_init() {
	missing = rb_intern("method_missing");

	void* handle = dlopen("/usr/lib/libruby1.8.so.1.8.7",0x101);
	char* rb_funcall = (char*)dlsym(handle, "rb_funcall");
	Dl_info info;
	dladdr(rb_funcall, &info);

	unsigned char* base = (unsigned char*)info.dli_fbase;

	rb_call_original = ruby_resolv(base, "rb_call");
	_method_missing = (METHODMISSING)ruby_resolv(base, "method_missing");
	_rb_call0 = (RBCALL0)ruby_resolv(base,"rb_call0");

	printf("rb_call: %p\n", rb_call_original);
	printf("method_missing: %p\n", _method_missing);
	printf("rb_call0: %p\n", _rb_call0);


}



