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
#ifndef __HOOK_H
#define __HOOK_H

void* put_jmp_hook(void* function_address, void* fake_function, int instructions_size);
void* put_jmp_hook_with_regs(void* function_address, void* fake_function, int instructions_size);
int get_jmp_size();

extern int get_instructions_size(void* code, int size);

#endif
