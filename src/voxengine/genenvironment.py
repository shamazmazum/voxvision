#!/usr/local/bin/python

def addname (name):
    print ('lua_getglobal (L, "' +name+ '");')
    print ('lua_setfield  (L, -2, "' +name+ '");')

def addnamefromtable (name):
    print ('lua_getfield (L, -2, "' +name+ '");')
    print ('lua_setfield  (L, -2, "' +name+ '");')

def addfrommodule (modname, names):
    if modname is None:
        map (addname, names)
    else:
        print 'if (luaL_loadstring (L, "return require \\"' +modname+ '\\"") || lua_pcall (L, 0, 1, 0))'
        print '    luaL_error (L, "Cannot load module: %s", "'+modname+ '");'
        print 'lua_newtable (L);'
        map (addnamefromtable, names)
        print 'lua_setfield (L, -3, "' +modname+ '");'
        print 'lua_pop (L, 1);'
    print ('')

addfrommodule (None, ["pairs", "ipairs", "print", "next", "tostring", "tonumber"])
addfrommodule ("table", ["insert", "maxn", "remove", "sort", "unpack"])
addfrommodule ("math", ["sin", "cos", "tan",
                        "asin", "acos", "atan", "atan2",
                        "abs", "exp", "floor", "ceil",
                        "huge", "log", "log10", "sqrt",
                        "max", "min", "pi", "pow",
                        "random", "randomseed"])
addfrommodule ("os", ["clock", "difftime", "time"])
addfrommodule ("string", ["format"])

def addfromcmodule (modname, names):
    # Load a module, but do not add it in environment
    print 'load_module_restricted (L, "'+modname+'");'
    # Create an empty table
    print 'lua_newtable (L);'
    map (addnamefromtable, names)
    print 'lua_setfield (L, -3, "' +modname+ '");'
    print 'lua_pop (L, 1);'
    print ''

addfromcmodule ("voxsdl", ["getKeyboardState", "scancode", "pumpEvent", "pollEvent", "waitEvent",
                           "pushEvent", "event", "key", "scancode", "getRelativeMouseState"])
