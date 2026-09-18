-- vim:set sw=4 ts=4:
--
-- Copyright (C) 2009  Daniel Aquino
--
-- Redistribution and use in source and binary forms, with or without
-- modification, are permitted provided that the following conditions
-- are met:
-- 
-- 1. Redistributions of source code must retain the above copyright
--    notice, this list of conditions and the following disclaimer.
-- 2. Redistributions in binary form must reproduce the above copyright
--    notice, this list of conditions and the following disclaimer in the
--    documentation and/or other materials provided with the distribution.
-- 3. The name of the author may not be used to endorse or promote
--    products derived from this software without specific prior written
--    permission.
-- 
-- THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
-- IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
-- WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
-- ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
-- INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
-- (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
-- SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
-- HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
-- STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
-- IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
-- POSSIBILITY OF SUCH DAMAGE.

local base 	= _G
local table 	= require("table")
-- socket.http is the only thing in this module that needs luasocket, and it is
-- one GET against the master server's game list. A build without the Lua socket
-- module - Android, and any Makefile build with LUASOCKET=0, since no
-- distribution ships luasocket for Lua 5.1 any more - must still be able to
-- load this file: init() requires it before anything else, so a hard require
-- here fails the whole Lua startup and the engine never reaches its menu.
local have_http, http = pcall(require, "socket.http")
if not have_http then http = nil end
local assert	= assert
local pcall	= pcall
local loadstring	= loadstring

module("games")

local url = "http://fly.thruhere.net/status/games.json"

function get( url )
	local list = {}
	-- No luasocket in this build: the internet game list is simply empty, and
	-- LAN play over ENet is unaffected.
	if not http then return list end
	local body, code, headers = http.request(url)
	if code ~= 200 then return list end
	body = "return " .. body
	local rv,f = pcall(loadstring, body)
	if not (rv and f) then return list end
	list = f()
	return list
end

-----------------------------------------------------------------------------
-- C Helpers
----------------------------------------------------------------------------

local list = {}
local last_time = 0
local refresh_time = 1 -- seconds

function update()
	list = get( url )
end

function length()
	return table.maxn(list)
end

function name_at( index )
	return list[index].nick
end

function ip_at( index )
	return list[index].ip
end

function port_at( index )
	return list[index].port
end


