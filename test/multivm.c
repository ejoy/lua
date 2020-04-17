#define LUA_LIB

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

struct boxvm {
	lua_State *L;
};

static int
lvm_delete(lua_State *L) {
	struct boxvm *vm = lua_touserdata(L, 1);
	if (vm->L) {
		lua_close(vm->L);
		vm->L = NULL;
	}
	return 0;
}

static int
vm_init(lua_State *L) {
	luaL_openlibs(L);
	return 0;
}

static int
vm_dostring(lua_State *L) {
	const char *code = (const char *)lua_touserdata(L, 1);
	lua_pop(L, 1);
	luaL_loadstring(L, code);
	lua_call(L, 0, LUA_MULTRET);
	return lua_gettop(L);
}

static int
lvm_dostring(lua_State *L) {
	luaL_checktype(L, 1, LUA_TUSERDATA);
	struct boxvm *vm = lua_touserdata(L, 1);
	lua_State *cL = vm->L;
	const char * code = luaL_checkstring(L, 2);
	lua_settop(cL, 0);
	lua_pushcfunction(cL, vm_dostring);
	lua_pushlightuserdata(cL, (void *)code);
	if (lua_pcall(cL, 1, LUA_MULTRET, 0) != LUA_OK) {
		const char * err = lua_tostring(cL, -1);
		lua_pushstring(L, err);
		lua_pop(cL, 1);
		return lua_error(L);
	}
	int retn = lua_gettop(cL);
	int i;
	for (i=1;i<=retn;i++) {
		switch(lua_type(cL, i)) {
		case LUA_TBOOLEAN:
			lua_pushboolean(L, lua_toboolean(cL, i));
			break;
		case LUA_TNUMBER:
			if (lua_isinteger(cL, i)) {
				lua_pushinteger(L, lua_tointeger(cL, i));
			} else {
				lua_pushnumber(L, lua_tonumber(cL, i));
			}
			break;
		case LUA_TLIGHTUSERDATA:
			lua_pushlightuserdata(L, lua_touserdata(cL, i));
			break;
		default:
			return luaL_error(L, "Unsupported return type %s", lua_typename(L, lua_type(cL, i)));
		}
	}
	return retn;
}

static int
lnew_vm(lua_State *L) {
	struct boxvm *vm = lua_newuserdata(L, sizeof(struct boxvm));
	vm->L = NULL;
	if (luaL_newmetatable(L, "MULTIVM")) {
		luaL_Reg meta[] = {
			{ "__gc", lvm_delete },
			{ "__index", NULL },
			{ "dostring", lvm_dostring },
			{ NULL, NULL },
		};
		luaL_setfuncs(L, meta, 0);
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);
	vm->L = luaL_newstate();
	lua_pushcfunction(vm->L, vm_init);
	if (lua_pcall(vm->L, 0, 0, 0) != LUA_OK) {
		lua_close(vm->L);
		vm->L = NULL;
		return luaL_error(L, "new vm failed");
	}
	return 1;
}

LUAMOD_API int
luaopen_multivm(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{ "new", lnew_vm },
		{ NULL, NULL },
	};
	luaL_newlib(L, l);
	return 1;
}
