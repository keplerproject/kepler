package = "Kepler"

version = "1.1.1-1"

description = {
  summary = "The Lua Web Development Platform",
  detailed = [[
Kepler is a Lua web development platform that offers the same advantages of Lua. Kepler is simple, portable, light, embeddable and extensible.  ]],
  license = "MIT/X11",
  homepage = "http://www.keplerproject.org/kepler"
}

dependencies = { "wsapi >= 1.1", "cgilua >= 5.1.3", "orbit >= 2.0.2" }

source = {
  url = "http://luaforge.net/frs/download.php/3983/kepler-1.1.1.tar.gz"
}

build = {
   platforms = {
     unix = {
       type = "none",
       install = {
         bin = { "src/setup-kepler", "src/cgilua.cgi", "src/op.cgi" },
         lua = { kepler_init = "src/kepler_init.lua" }
       },
       copy_directories = { "web", "doc", "css", "conf" },
     },
     windows = {
       type = "make",
       build_pass = true,
       build_target = "all",
       build_variables = {
         CFLAGS = "$(CFLAGS)",
	 LUA_INCDIR = "$(LUA_INCDIR)",
	 LUA_LIB = "$(LUA_LIBDIR)\\lua5.1.lib"
       },
       install_target = "install",
       install_variables = {
         PREFIX  = "$(PREFIX)",
         LUA_DIR = "$(LUADIR)",
         BIN_DIR = "$(BINDIR)"
       },
     }
   }
}

hooks = {
  platforms = {
    unix = {
      post_install = "$(SCRIPTS_DIR)/setup-kepler --prefix=$(LUAROCKS_PREFIX)/kepler --web=$(KEPLER_WEB) --plat=unix"
    },
    windows = {
      post_install = '"$(SCRIPTS_DIR)/setup-kepler" "--prefix=$(LUAROCKS_PREFIX)/kepler" "--web=$(KEPLER_WEB)" --plat=windows'
    }
  }
}
