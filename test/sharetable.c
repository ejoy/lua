#define LUA_LIB

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "lgc.h"

#ifdef makeshared

static void
mark_shared(lua_State *L) {
	if (lua_type(L, -1) != LUA_TTABLE) {
		luaL_error(L, "Not a table, it's a %s.", lua_typename(L, lua_type(L, -1)));
	}
	Table * t = (Table *)lua_topointer(L, -1);
	if (isshared(t))
		return;
	makeshared(t);
	luaL_checkstack(L, 4, NULL);
	if (lua_getmetatable(L, -1)) {
		luaL_error(L, "Can't share metatable");
	}
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		int i;
		for (i=0;i<2;i++) {
			int idx = -i-1;
			int t = lua_type(L, idx);
			switch (t) {
			case LUA_TTABLE:
				mark_shared(L);
				break;
			case LUA_TNUMBER:
			case LUA_TBOOLEAN:
			case LUA_TLIGHTUSERDATA:
				break;
			case LUA_TFUNCTION:
				if (lua_getupvalue(L, idx, 1) != NULL) {
					luaL_error(L, "Invalid function with upvalue");
				} else if (!lua_iscfunction(L, idx)) {
					LClosure *f = (LClosure *)lua_topointer(L, idx);
					makeshared(f);
				}
				break;
			case LUA_TSTRING:
				lua_sharestring(L, idx);
				break;
			default:
				luaL_error(L, "Invalid type [%s]", lua_typename(L, t));
				break;
			}
		}
		lua_pop(L, 1);
	}
}

static int
lis_sharedtable(lua_State* L) {
	int b = 0;
	if(lua_type(L, 1) == LUA_TTABLE) {
		Table * t = (Table *)lua_topointer(L, 1);
		b = isshared(t);
	}
	lua_pushboolean(L, b);
	return 1;
}

static int
lmark_shared(lua_State *L) {
	// turn off gc , because marking shared will prevent gc mark.
	lua_gc(L, LUA_GCSTOP, 0);
	mark_shared(L);
	Table * t = (Table *)lua_topointer(L, -1);
	lua_pushlightuserdata(L, t);
	return 1;
}

static int
lclone_table(lua_State *L) {
	lua_clonetable(L, lua_touserdata(L, 1));

	return 1;
}

LUAMOD_API int
luaopen_sharetable(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "clone", lclone_table },
		{ "mark_shared", lmark_shared },
		{ "is_sharedtable", lis_sharedtable },
		{ NULL, NULL },
	};
	luaL_newlib(L, l);
	return 1;
}

#else

LUAMOD_API int
luaopen_sharetable(lua_State *L) {
	return luaL_error(L, "Not supported");
}

#endif