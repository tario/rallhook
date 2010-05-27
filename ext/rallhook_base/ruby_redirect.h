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
#ifndef __RUBY_REDIRECT_H
#define __RUBY_REDIRECT_H

// Acts as facade of the division between the ruby interpreter modification

#include <ruby.h>

typedef struct _CallData {
	VALUE klass;
	VALUE recv;
	ID mid;
	VALUE args;
} CallData;

typedef void (*REDIRECTHANDLER) ( CallData* call_data );

int put_redirect_handler( REDIRECTHANDLER redirect_handler);
void disable_redirect();
void enable_redirect();
void redirect_left(int left);
REDIRECTHANDLER get_current_redirect_handler();
int redirect_enabled();
int redirect_enable_left();

#endif
