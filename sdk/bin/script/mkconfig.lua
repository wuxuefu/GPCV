require "common"
lfs = require "lfs"

---------------------------------------------------------------------
-- Config utility
---------------------------------------------------------------------
function configToString(config)
	local s = StringBuffer:New()
	for k, v in pairsBySortedKey(config) do
		if k:sub(1, 1) ~= "_" then
			if type(v) == "boolean" then
				if v == true then
					s:WriteFormat("SYSCONFIG_%s = y\n", k)
				else
					s:WriteFormat("SYSCONFIG_%s = n\n", k)
				end
			else
				--s:WriteFormat("SYSCONFIG_%s = \"%s\"\n", k, v)
				s:WriteFormat("SYSCONFIG_%s = %s\n", k, v)
			end
		end
	end
	return s:ToString()
end

function configToHeaderFormat(config)
	local s = StringBuffer:New()
	s:WriteFormat("#ifndef _MKCONFIG_\n")
	s:WriteFormat("#define _MKCONFIG_\n")
	s:WriteFormat("\n")
	for k, v in pairsBySortedKey(config) do
		if k:sub(1, 1) == "_" then
			-- skip
		elseif (v == true) then
			s:WriteFormat("#define SYSCONFIG_%s\n\n", k)
		elseif (v == false) then
			s:WriteFormat("/* #undef SYSCONFIG_%s */\n\n", k)
		elseif (v == "y") then
			s:WriteFormat("#define SYSCONFIG_%s\n\n", k)
		elseif (v == "n") then
			s:WriteFormat("/* #undef SYSCONFIG_%s */\n\n", k)
		elseif (type(v) == "number") then
			s:WriteFormat("#define SYSCONFIG_%s %d\n\n", k, v)
		else
			s:WriteFormat("#define SYSCONFIG_%s \"%s\"\n\n", k, v)
		end
	end
	s:WriteFormat("\n")
	s:WriteFormat("#endif\n")
	return s:ToString()
end

function configToLuaFormat(config)
	local s = StringBuffer:New()
	s:WriteFormat("local sysconfig = {\n")
	for k, v in pairsBySortedKey(config) do
		if k:sub(1, 1) == "_" then
			-- skip
		elseif (type(v) == "boolean") then
			s:WriteFormat("\t[%q] = %s,\n", k, tostring(v))
		elseif (type(v) == "number") then
			s:WriteFormat("\t[%q] = %s,\n", k, tostring(v))
		else
			s:WriteFormat("\t[%q] = %q,\n", k, tostring(v))
		end
	end
	s:WriteFormat("}\n")
	s:WriteFormat("return sysconfig\n")
	return s:ToString()
end

function dumpConfig(config)
	printf("=== Summary ===\n")
	printf(configToString(config))
	printf("\n");
end


function saveConfig(filename, config)
	local f = assert(io.open(filename, "wb"));
	fprintf(f, "#=== Project Configuration ===\n\n")
	f:write(configToString(config))
	fprintf(f, "\n");
	f:close();
end


function saveToFile(filename, str)
	local f = assert(io.open(filename, "wb"));
	f:write(str)
	f:close();
end

---------------------------------------------------------------------
-- Menu utility
---------------------------------------------------------------------
function selectList(title, list, valueList)
	local n = 0
	local valid = true
	valueList = valueList or list

	repeat
		valid = true
		if #list == 1 then
			return valueList[1], 1
		end
		printf("<< %s >>\n", title)
		for i = 1, #list do
			if #list >= 10 then
				printf("%2d. %s\n", i, list[i])
			else
				printf("%d. %s\n", i, list[i])
			end
		end
		printf("Select [1 .. %d] :", #list)

		local line = io.read("*line")
		--printf("line = << %s >>\n", line)
		n = tonumber(line) or 0
		if (n == 0) or n > #list then
			printf("\n");
			printf("Error : invalid value!\n\n");
			valid = false
		end
	until (valid == true)
	printf("\n")

	return valueList[n], n
end

function selectBool(title, message)
	local n = 0
	local valid = true
	valueList = {y = 1, n = 2}

	repeat
		valid = true
		printf("<< %s >>\n", title)
		printf("%s [y/n] :", message)

		local line = io.read("*line")
		n = valueList[line] or 0
		if (n == 0) then
			printf("\n");
			printf("Error : invalid value!\n\n");
			valid = false
		end
	until (valid == true)
	printf("\n")

	return (n == 1)
end

---------------------------------------------------------------------
-- Import configs
---------------------------------------------------------------------
config = {}

function IncludeConfig(filename)
	assert(loadfile(filename))()
end

function findFiles(path, pattern, depth, list)
	list = list or {}
	for filename in lfs.dir(path) do
		if filename ~= "." and filename ~= ".." then
			local pathname = path .. "/" .. filename
			local attr = lfs.attributes(pathname)

			if attr and attr.mode == "file" then
				local capture = string.match(filename, pattern)
				if capture then
					--print("pathname:", pathname)
					list[pathname] = capture
				end		
			elseif attr and attr.mode == "directory" then
				-- recursive find
				if depth ~= 0 then
					findFiles(pathname, pattern, depth - 1, list)
				end
			end
		end
	end
	return list
end

function loadConfig(pathname)
	local t = {}
	local basepath, subpath, basename = string.match(pathname, "^([^/]*)/(.*)/([^/]*)$")
	local variant = string.match(basename, "^(.*)%.cfg$")
	--print(basepath, subpath, basename)

	t.cfgpath = pathname
	t.path = basepath .. "/" .. subpath
	t.group = string.match(subpath, "(.*)/.*$")
	t.name = string.gsub(subpath, "%/", ".")
	t.filename = basename
	t.variant = variant
	--print(t.cfgpath, t.path, t.name, t.filename)
	t.meta = assert(loadfile(t.path .. "/" .. t.variant .. ".meta")())
	
	return t
end

function findConfigs(path)
	local list = findFiles(path, "^(.*)%.cfg$", 4)
	local t = {}
	for k, v in pairsBySortedKey(list) do
		t[#t+1] = loadConfig(k)
	end
	return t
end

local function getTableContent(list, pattern)
	local path = pattern
	local pos = list

	while (type(pos) == "table") do
		local index, indices = string.match(path, "([^.]*)[.](.*)$")
		--print("index: " .. (index or "nil") .. ", indices: " .. (indices or "nil") .. ", path: " .. (path or "nil"))

		local entry = index or path
		if type(tonumber(entry)) == "number" then entry = tonumber(entry) end
		--print("enter: " .. entry)
		pos = pos[entry] or nil

		if indices then path = indices end
	end

	--print("return: " .. (pos or "nil"))
	return pos
end

local function getContentMaxLength(list, pattern)
	local len = 0;

	for i, v in ipairs(list) do
		-- TODO: support '*' character
		local value = getTableContent(v, pattern)
		if value then
			len = math.max(len, string.len(value))
		end
	end
	return len
end

local function pad(s, width, padder)
	padder = string.rep(padder or " ", width)
	if width < 0 then return strsub(padder .. s, width) end
	return string.sub(s .. padder, 1, width)
end

function selectConfig(list, title)
	local prompts = {}
	local sortedList = {}
	local width = getContentMaxLength(list, "meta.prompt")

	for i, v in ipairs(list) do
		prompts[#prompts + 1] = "[ " .. pad(v.meta.prompt, width, " ") .. " ] - " .. v.group
		sortedList[#sortedList + 1] = v
	end
	local object = selectList(title, prompts, sortedList)
	return object
end

---------------------------------------------------------------------
-- Main
---------------------------------------------------------------------
function main(forceProject)

	local promptList, confList, confSelected

	printf("\n")
	printf("==============================================\n")
	printf("== Configuration Setup\n")
	printf("==============================================\n")

	config.HOST = "linux-x86"
	
	-------------------------------------
	-- Project
	-------------------------------------
	local projects = findConfigs("project")
	local project = selectConfig(projects, "Select Project")
	assert(project ~= nil)
	config._project = project
	config.PROJECT = project.name
	config.PROJECT_DIR = project.path
	IncludeConfig(project.cfgpath)

	-------------------------------------
	-- Platform
	-------------------------------------
	local platforms = findConfigs("platform")
	local platform = selectConfig(platforms, "Select Platform")
	assert(platform ~= nil)
	config._platform = platform
	config.PLATFORM = platform.name
	config.PLATFORM_DIR = platform.path
	IncludeConfig(platform.cfgpath)

	if config.TARGET == nil then
		config.TARGET = "linux-arm"
	end
	if config.SIMULATOR == nil then
		config.SIMULATOR = false
	end

	-------------------------------------
	-- Product
	-------------------------------------
	local product = {
		name = project.name .. "__" .. platform.name,
		project = project,
		platform = platform
	}
	product.path = "out/" .. product.name
	config.PRODUCT = product.name
	config._product = product.name

	-------------------------------------
	-- Collect configs
	-------------------------------------
	IncludeConfig("sdk/mkconfig")
	IncludeConfig("project/mkconfig")
	IncludeConfig("platform/mkconfig")

	-------------------------------------
	-- Create config files.
	-------------------------------------
	dumpConfig(config)

	saveToFile("product_config.mak", string.format("PRODUCT := %s\n", product.name))

	lfs.mkdir("out")
	lfs.mkdir(product.path)
	lfs.mkdir(product.path .. "/config")
	saveConfig(product.path .. "/config/sysconfig.mak", config)
	printf("Save configuration to %s\n", product.path .. "/config/sysconfig.mak")
	saveToFile(product.path .. "/config/sysconfig.h", configToHeaderFormat(config))
	saveToFile(product.path .. "/config/sysconfig.lua", configToLuaFormat(config))

	saveToFile(product.path .. "/Makefile", string.format(
[[### DO NOT EDIT THIS FILE ###
TOPDIR := ../../
PRODUCT := %s
include $(TOPDIR)sdk/build/core/product.mak
### DO NOT EDIT THIS FILE ###
]], product.name))

end

---------------------------------------------------------------------
-- Ready, Set, Go!
---------------------------------------------------------------------
io.output():setvbuf("no")
main()
