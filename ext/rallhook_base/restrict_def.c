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
#include "ruby.h"
#include "restrict_def.h"
#include "ruby_symbols.h"
#include "hook.h"

#ifndef __USE_GNU
#define __USE_GNU
#endif

#include <dlfcn.h>

ID __shadow___id;
ID __unshadow___id;
ID shadow_id;
int restrict_def = 0;

typedef void NODE_;

typedef void (*RBADDMETHOD) (
    VALUE klass,
    ID id,
    NODE_* node,
    int noex
);

RBADDMETHOD rb_add_method_copy;

void disable_overwrite() {
	restrict_def = 1;
}
void enable_overwrite() {
	restrict_def = 0;
}

VALUE shadow_or_create(VALUE klass) {
	if (FL_TEST(klass, FL_SINGLETON)) {
		// the singleton classes has no shadow
		return klass;
	}

	VALUE shadow_klass = rb_ivar_get(klass, __shadow___id);
	if ( shadow_klass == Qnil ) {
		shadow_klass = rb_obj_dup(klass);
		rb_ivar_set(klass, __shadow___id, shadow_klass );
		rb_ivar_set(shadow_klass, __unshadow___id, klass );
	}
	return shadow_klass;
}

VALUE shadow_or_original(VALUE klass) {
	if (FL_TEST(klass, FL_SINGLETON)) {
		// the singleton classes has no shadow
		return klass;
	}

	VALUE shadow_klass = rb_ivar_get(klass, __shadow___id);
	if ( shadow_klass == Qnil ) {
		return klass;
	}
	return shadow_klass;

}

VALUE unshadow(VALUE klass) {
	VALUE unshadow_klass = rb_ivar_get(klass, __unshadow___id);
	if (unshadow_klass == Qnil) {
		return klass;
	} else {
		return unshadow_klass;
	}

}

void rb_add_method_fake(
    VALUE klass,
    ID id,
    NODE_* node,
    int noex
) {
	if (restrict_def) {
		if (FL_TEST(klass, FL_SINGLETON)) {
    		// singleton method over classes are illegal
			if ( strcmp( rb_class2name(klass), "Class") == 0) {
				rb_raise(rb_eSecurityError, "Illegal singleton method %s", rb_id2name(id) );
			} else {
				rb_add_method_copy(klass,id,node,noex);
			}
		} else {
			rb_add_method_copy(shadow_or_create(klass),id,node,noex);
		}
	} else {
		rb_add_method_copy(klass,id,node,noex);
	}

}

void shadow_redirect(VALUE* klass, VALUE* recv, ID* mid) {
	// shadow redirection if restrict_def (ever)

	if (restrict_def) {
	*klass = shadow_or_original(*klass);
	}
}


void init_restrict_def() {
	void* handle = dlopen(current_libruby(),0x101);
	char* rb_funcall = (char*)dlsym(handle, "rb_funcall");
	Dl_info info;
	dladdr(rb_funcall, &info);

	unsigned char* base = (unsigned char*)info.dli_fbase;

	void* rb_add_method_original = ruby_resolv(base,"rb_add_method");

	int inst_size = get_instructions_size(rb_add_method_original, 256);
	rb_add_method_copy = put_jmp_hook(rb_add_method_original, rb_add_method_fake, inst_size);

	__shadow___id = rb_intern("__shadow__");
	__unshadow___id = rb_intern("__unshadow__");
	shadow_id = rb_intern("shadow");

	rb_define_method(rb_cClass, "shadow", shadow_or_original, 0);
	rb_define_method(rb_cClass, "create_shadow", shadow_or_create, 0);
	rb_define_method(rb_cClass, "unshadow", unshadow, 0);

}
