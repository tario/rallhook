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

#ifndef __NODE_DEFS_H
#define __NODE_DEFS_H


#ifdef RUBY1_9


enum node_type {
    NODE_METHOD,
#define NODE_METHOD      NODE_METHOD
    NODE_FBODY,
#define NODE_FBODY       NODE_FBODY
    NODE_CFUNC,
#define NODE_CFUNC       NODE_CFUNC
    NODE_SCOPE,
#define NODE_SCOPE       NODE_SCOPE
    NODE_BLOCK,
#define NODE_BLOCK       NODE_BLOCK
    NODE_IF,
#define NODE_IF          NODE_IF
    NODE_CASE,
#define NODE_CASE        NODE_CASE
    NODE_WHEN,
#define NODE_WHEN        NODE_WHEN
    NODE_OPT_N,
#define NODE_OPT_N       NODE_OPT_N
    NODE_WHILE,
#define NODE_WHILE       NODE_WHILE
    NODE_UNTIL,
#define NODE_UNTIL       NODE_UNTIL
    NODE_ITER,
#define NODE_ITER        NODE_ITER
    NODE_FOR,
#define NODE_FOR         NODE_FOR
    NODE_BREAK,
#define NODE_BREAK       NODE_BREAK
    NODE_NEXT,
#define NODE_NEXT        NODE_NEXT
    NODE_REDO,
#define NODE_REDO        NODE_REDO
    NODE_RETRY,
#define NODE_RETRY       NODE_RETRY
    NODE_BEGIN,
#define NODE_BEGIN       NODE_BEGIN
    NODE_RESCUE,
#define NODE_RESCUE      NODE_RESCUE
    NODE_RESBODY,
#define NODE_RESBODY     NODE_RESBODY
    NODE_ENSURE,
#define NODE_ENSURE      NODE_ENSURE
    NODE_AND,
#define NODE_AND         NODE_AND
    NODE_OR,
#define NODE_OR          NODE_OR
    NODE_MASGN,
#define NODE_MASGN       NODE_MASGN
    NODE_LASGN,
#define NODE_LASGN       NODE_LASGN
    NODE_DASGN,
#define NODE_DASGN       NODE_DASGN
    NODE_DASGN_CURR,
#define NODE_DASGN_CURR  NODE_DASGN_CURR
    NODE_GASGN,
#define NODE_GASGN       NODE_GASGN
    NODE_IASGN,
#define NODE_IASGN       NODE_IASGN
    NODE_IASGN2,
#define NODE_IASGN2      NODE_IASGN2
    NODE_CDECL,
#define NODE_CDECL       NODE_CDECL
    NODE_CVASGN,
#define NODE_CVASGN      NODE_CVASGN
    NODE_CVDECL,
#define NODE_CVDECL      NODE_CVDECL
    NODE_OP_ASGN1,
#define NODE_OP_ASGN1    NODE_OP_ASGN1
    NODE_OP_ASGN2,
#define NODE_OP_ASGN2    NODE_OP_ASGN2
    NODE_OP_ASGN_AND,
#define NODE_OP_ASGN_AND NODE_OP_ASGN_AND
    NODE_OP_ASGN_OR,
#define NODE_OP_ASGN_OR  NODE_OP_ASGN_OR
    NODE_CALL,
#define NODE_CALL        NODE_CALL
    NODE_FCALL,
#define NODE_FCALL       NODE_FCALL
    NODE_VCALL,
#define NODE_VCALL       NODE_VCALL
    NODE_SUPER,
#define NODE_SUPER       NODE_SUPER
    NODE_ZSUPER,
#define NODE_ZSUPER      NODE_ZSUPER
    NODE_ARRAY,
#define NODE_ARRAY       NODE_ARRAY
    NODE_ZARRAY,
#define NODE_ZARRAY      NODE_ZARRAY
    NODE_VALUES,
#define NODE_VALUES      NODE_VALUES
    NODE_HASH,
#define NODE_HASH        NODE_HASH
    NODE_RETURN,
#define NODE_RETURN      NODE_RETURN
    NODE_YIELD,
#define NODE_YIELD       NODE_YIELD
    NODE_LVAR,
#define NODE_LVAR        NODE_LVAR
    NODE_DVAR,
#define NODE_DVAR        NODE_DVAR
    NODE_GVAR,
#define NODE_GVAR        NODE_GVAR
    NODE_IVAR,
#define NODE_IVAR        NODE_IVAR
    NODE_CONST,
#define NODE_CONST       NODE_CONST
    NODE_CVAR,
#define NODE_CVAR        NODE_CVAR
    NODE_NTH_REF,
#define NODE_NTH_REF     NODE_NTH_REF
    NODE_BACK_REF,
#define NODE_BACK_REF    NODE_BACK_REF
    NODE_MATCH,
#define NODE_MATCH       NODE_MATCH
    NODE_MATCH2,
#define NODE_MATCH2      NODE_MATCH2
    NODE_MATCH3,
#define NODE_MATCH3      NODE_MATCH3
    NODE_LIT,
#define NODE_LIT         NODE_LIT
    NODE_STR,
#define NODE_STR         NODE_STR
    NODE_DSTR,
#define NODE_DSTR        NODE_DSTR
    NODE_XSTR,
#define NODE_XSTR        NODE_XSTR
    NODE_DXSTR,
#define NODE_DXSTR       NODE_DXSTR
    NODE_EVSTR,
#define NODE_EVSTR       NODE_EVSTR
    NODE_DREGX,
#define NODE_DREGX       NODE_DREGX
    NODE_DREGX_ONCE,
#define NODE_DREGX_ONCE  NODE_DREGX_ONCE
    NODE_ARGS,
#define NODE_ARGS        NODE_ARGS
    NODE_ARGS_AUX,
#define NODE_ARGS_AUX    NODE_ARGS_AUX
    NODE_OPT_ARG,
#define NODE_OPT_ARG     NODE_OPT_ARG
    NODE_POSTARG,
#define NODE_POSTARG     NODE_POSTARG
    NODE_ARGSCAT,
#define NODE_ARGSCAT     NODE_ARGSCAT
    NODE_ARGSPUSH,
#define NODE_ARGSPUSH    NODE_ARGSPUSH
    NODE_SPLAT,
#define NODE_SPLAT       NODE_SPLAT
    NODE_TO_ARY,
#define NODE_TO_ARY      NODE_TO_ARY
    NODE_BLOCK_ARG,
#define NODE_BLOCK_ARG   NODE_BLOCK_ARG
    NODE_BLOCK_PASS,
#define NODE_BLOCK_PASS  NODE_BLOCK_PASS
    NODE_DEFN,
#define NODE_DEFN        NODE_DEFN
    NODE_DEFS,
#define NODE_DEFS        NODE_DEFS
    NODE_ALIAS,
#define NODE_ALIAS       NODE_ALIAS
    NODE_VALIAS,
#define NODE_VALIAS      NODE_VALIAS
    NODE_UNDEF,
#define NODE_UNDEF       NODE_UNDEF
    NODE_CLASS,
#define NODE_CLASS       NODE_CLASS
    NODE_MODULE,
#define NODE_MODULE      NODE_MODULE
    NODE_SCLASS,
#define NODE_SCLASS      NODE_SCLASS
    NODE_COLON2,
#define NODE_COLON2      NODE_COLON2
    NODE_COLON3,
#define NODE_COLON3      NODE_COLON3
    NODE_DOT2,
#define NODE_DOT2        NODE_DOT2
    NODE_DOT3,
#define NODE_DOT3        NODE_DOT3
    NODE_FLIP2,
#define NODE_FLIP2       NODE_FLIP2
    NODE_FLIP3,
#define NODE_FLIP3       NODE_FLIP3
    NODE_ATTRSET,
#define NODE_ATTRSET     NODE_ATTRSET
    NODE_SELF,
#define NODE_SELF        NODE_SELF
    NODE_NIL,
#define NODE_NIL         NODE_NIL
    NODE_TRUE,
#define NODE_TRUE        NODE_TRUE
    NODE_FALSE,
#define NODE_FALSE       NODE_FALSE
    NODE_ERRINFO,
#define NODE_ERRINFO     NODE_ERRINFO
    NODE_DEFINED,
#define NODE_DEFINED     NODE_DEFINED
    NODE_POSTEXE,
#define NODE_POSTEXE     NODE_POSTEXE
    NODE_ALLOCA,
#define NODE_ALLOCA      NODE_ALLOCA
    NODE_BMETHOD,
#define NODE_BMETHOD     NODE_BMETHOD
    NODE_MEMO,
#define NODE_MEMO        NODE_MEMO
    NODE_IFUNC,
#define NODE_IFUNC       NODE_IFUNC
    NODE_DSYM,
#define NODE_DSYM        NODE_DSYM
    NODE_ATTRASGN,
#define NODE_ATTRASGN    NODE_ATTRASGN
    NODE_PRELUDE,
#define NODE_PRELUDE     NODE_PRELUDE
    NODE_LAMBDA,
#define NODE_LAMBDA      NODE_LAMBDA
    NODE_OPTBLOCK,
#define NODE_OPTBLOCK    NODE_OPTBLOCK
    NODE_LAST
#define NODE_LAST        NODE_LAST
};

#define RUBY_VM_METHOD_NODE NODE_METHOD

#else
#define RUBY_VM_METHOD_NODE 0
#define NODE_METHOD 0

#endif


typedef struct RNode {
    unsigned long flags;
    char *nd_file;
    union {
	struct RNode *node;
	ID id;
	VALUE value;
	VALUE (*cfunc)(ANYARGS);
	ID *tbl;
    } u1;
    union {
	struct RNode *node;
	ID id;
	long argc;
	VALUE value;
    } u2;
    union {
	struct RNode *node;
	ID id;
	long state;
	struct global_entry *entry;
	long cnt;
	VALUE value;
    } u3;
} NODE;

#define RNODE(obj)  (R_CAST(RNode)(obj))

#define NODE_TYPESHIFT 8
#define NODE_TYPEMASK  (((VALUE)0x7f)<<NODE_TYPESHIFT)

#define nd_body  u2.node
#define nd_type(n) ((int) (((RNODE(n))->flags & NODE_TYPEMASK)>>NODE_TYPESHIFT))
#define nd_clss  u1.value

#endif
