-------------------------------------------------------------------------------
-- Xavante configuration file.
--
-- The configuration defines the Xavante environment
-- Commented fields are optional and assume default values
--
-- Xavante defines virtualhosts for each site running with Xavante.
-- Each virtualhost can define the handlers for specific files extensions.
-- Xavante currently offers a fileHandler and a CGILuaHandler.
--
-- Xavante configuration can be redefined on the default file structure by the
-- the use of LUA_PATH, see more details in the online documentation.
--
-- Authors: Javier Guerra and Andre Carregal
-- Copyright (c) 2004-2006 Kepler Project
---
-- $Id: config.lua,v 1.1 2008/06/30 14:29:59 carregal Exp $
------------------------------------------------------------------------------
require "xavante.filehandler"
require "xavante.cgiluahandler"
require "xavante.redirecthandler"
require "orbit.ophandler"

-- Define here where Xavante HTTP documents scripts are located
local webDir = XAVANTE_WEB

local simplerules = {

    { -- URI remapping example
      match = "^/$",
      with = xavante.redirecthandler,
      params = {"index.lp"}
    }, 

    { -- cgiluahandler example
      match = {"%.lp$", "%.lp/.*$", "%.lua$", "%.lua/.*$" },
      with = xavante.cgiluahandler.makeHandler (webDir)
    },

    { -- ophandler example
      match = {"%.op$", "%.op/.*$" },
      with = orbit.ophandler.makeHandler (webDir)
    },

    { -- wsapihandler example
      match = {"%.ws$", "%.ws/" },
      with = wsapi.xavante.makeGenericHandler (webDir)
    },
    
    { -- filehandler example
      match = ".",
      with = xavante.filehandler,
      params = {baseDir = webDir}
    },
} 

-- Displays a message in the console with the used ports
xavante.start_message(function (ports)
    local date = os.date("[%Y-%m-%d %H:%M:%S]")
    print(string.format("%s Xavante started on port(s) %s",
      date, table.concat(ports, ", ")))
  end)

xavante.HTTP{
    server = {host = "*", port = 8080},
    
    defaultHost = {
    	rules = simplerules
    },
}
