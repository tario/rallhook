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
			}
		}

		void* data;
		if (st_lookup(RCLASS_M_TBL(klass), id, &data)) {
			// overwrite of method, access denied...
			rb_raise(rb_eSecurityError, "Illegal overwrite of method %s", rb_id2name(id) );
		}
	}

	rb_add_method_copy(klass,id,node,noex);
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

}
