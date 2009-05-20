#!/usr/bin/lua

-- CGILua (SAPI) launcher, extracts script to launch
-- either from the command line (use #!cgilua in the script)
-- or from SCRIPT_FILENAME/PATH_TRANSLATED

pcall(require, "luarocks.require")
pcall(require, "kepler_init")

local common = require "wsapi.common"
local fastcgi = require "wsapi.fastcgi"

local reload = false -- set to true if you want to always run in a new state

local function sapi_loader(wsapi_env)
  common.normalize_paths(wsapi_env, nil, "cgilua.fcgi")
  local bootstrap = ""
  for _, global in ipairs(RINGS_CGILUA_GLOBALS) do
    bootstrap = bootstrap .. 
      "_, _G[\"" .. global .. "\"] = remotedostring(\"return _G['" ..
      global .. "']\")\n"
  end
  local app = wsapi.common.load_isolated_launcher(wsapi_env.PATH_TRANSLATED, "wsapi.sapi",
    bootstrap, reload)
  return app(wsapi_env)
end 

fastcgi.run(sapi_loader)

