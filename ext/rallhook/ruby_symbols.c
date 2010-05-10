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

#include "ruby_symbols.h"
#include "ruby.h"

void* ruby_resolv(unsigned char* p_base, const char* symbol_name) {
	VALUE rb_mCymbol = rb_eval_string("Cymbol");
	VALUE rb_str_symbol_name = rb_str_new2(symbol_name);

	VALUE rb_offset = rb_funcall(rb_mCymbol,rb_intern("resolv"), 1, rb_str_symbol_name);

	unsigned long long offset = FIX2LONG(rb_offset);

	return (p_base + offset);
}


const char* current_libruby() {
	VALUE rb_mCymbol = rb_eval_string("Cymbol");
	VALUE rb_strSharedName = rb_funcall(rb_mCymbol,rb_intern("ruby_shared_name"), 0);

	return rb_string_ptr(rb_strSharedName);
}

