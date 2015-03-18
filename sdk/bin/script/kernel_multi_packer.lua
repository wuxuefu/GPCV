require "struct"
require "common"

local function print_usage()
	print(string.format("Usage:\n\t%s kernel=<kernel.img> [initrd=<initrd.img>] [cmdline=<cmdline.txt>] out=<output.bin>", arg[0]))
end

local function packer_create(outname)
	-- Open output file.
	local f = io.open(outname, "wb")
	if not f then
		error(string.format("Failed to write file %s", outname))
		return nil
	end

	local packer = {
		outname = outname,
		f = f,
		sections = {},
		sectCmdline = {},
		offset = 0,
		address = {
			cmdline = 0x00200100,
			kernel  = 0x00208000,
			--kernel  = 0x00000000,
			initrd  = 0x00a00000,
		},
	}

	return packer
end

local function packer_addSection(packer, sectName, sectFile)
	local infile = io.open(sectFile, "rb")
	local data = infile:read("*a")
	local size = infile:seek("end", 0)
	infile:close()

	section = {
		name = sectName,
		data = data,
		len = size,
		address = packer.address[sectName],
	}
	packer.sections[#packer.sections + 1] = section

	if sectName == "initrd" then
		packer.sectInitrd = section
	end
	
	return section
end

local function packer_addSections(packer, sectName, sectFile)
	for arch,file in pairs(sectFile) do
		local infile = io.open(file, "rb")
		local data = infile:read("*a")
		local size = infile:seek("end", 0)
		infile:close()

		section = {
			arch = arch,
			name = sectName,
			data = data,
			len = size,
			address = packer.address[sectName],
		}
		packer.sections[#packer.sections + 1] = section
		--printf("sectName = %s, arch = %s, file=%s\n",sectName,arch,file)
		if sectName == "cmdline" then
			packer.sectCmdline[arch] = section
		end
	end
	
	return section
end

local function packer_setupAtags(packer)
	if not packer.sectCmdline and not packer.sectInitrd then
		return
	end
	for arch,section in pairs(packer.sectCmdline) do
		-- append initrd boot param
		if packer.sectInitrd then
			if not packer.sectCmdline then
				packer.sectCmdline = {
					name = "cmdline",
					data = "",
					len = 0,
					address = packer.address["cmdline"],
				}
				packer.sections[#packer.sections + 1] = packer.sectCmdline
			end
	
			local initrdParam = string.format(" initrd=0x%08x,0x%08x", packer.sectInitrd.address, packer.sectInitrd.len)
			packer.sectCmdline[arch].data = packer.sectCmdline[arch].data .. initrdParam
			packer.sectCmdline[arch].len = #packer.sectCmdline[arch].data
		end
	
		-- exclude ceva memory area	
		--packer.sectCmdline[arch].data = packer.sectCmdline[arch].data .. " memmap=0x100000$0x100000"
		--packer.sectCmdline[arch].len = #packer.sectCmdline[arch].data
	
		-- append '\0' to cmdline
		if packer.sectCmdline[arch] then
			local align4 = 4 - (packer.sectCmdline[arch].len % 4)
			packer.sectCmdline[arch].data = packer.sectCmdline[arch].data .. string.rep("\000", align4)
			packer.sectCmdline[arch].len = #packer.sectCmdline[arch].data
		end
	
		local ATAG_CORE = 0x54410001
		local ATAG_MEM = 0x54410002
		local ATAG_CMDLINE = 0x54410009
		local ATAG_NONE = 0x00000000
		
		--local LINUX_MEM_START = 0x200000
		--local LINUX_MEM_SIZE = tonumber(string.match(packer.sectCmdline[arch].data, " mem=(%d+)MB ")) * 1024 * 1024 - LINUX_MEM_START
		
		local atagCore = struct.pack("<i4i4", 2, ATAG_CORE)
		--local atagMem = struct.pack("<i4i4i4i4", 4, ATAG_MEM, LINUX_MEM_SIZE, LINUX_MEM_START)
		atagCmdlineLen = (8 + packer.sectCmdline[arch].len + 4) / 4
		local atagCmdline = struct.pack("<i4i4", atagCmdlineLen, ATAG_CMDLINE)
		local atagNone = struct.pack("<i4i4", 0, ATAG_NONE)
		--atags = atagCore .. atagMem .. atagCmdline .. packer.sectCmdline[arch].data .. atagNone
		atags = atagCore .. atagCmdline .. packer.sectCmdline[arch].data .. atagNone
		
		packer.sectCmdline[arch].data = atags
		packer.sectCmdline[arch].len = #atags	
	end
end

local function packer_generate(packer)
	local n = 0
	local f = packer.f
	local offset = #packer.sections * 36 + 12
	
	packer_setupAtags(packer)

	-- generate tags
	for i, v in ipairs(packer.sections) do
		local tag = 0x01
		v.offset = offset
		offset = offset + v.len
		local tagData = struct.pack("<c16i4i4i4", v.name .. string.rep("\000", 16), v.offset, v.len, v.address)
		-- Tag 0x10 for need to check IC version tag
		if v.name=="cmdline" or v.name=="kernel" then
			tag=0x10
			if v.arch=="GPL32900B" then
				tag = tag+1
			end
		end
		
		local tagHeader = struct.pack("<i4i4", tag, #tagData)
		f:write(tagHeader)
		f:write(tagData)
		n = n + #tagHeader + #tagData
	end

	-- jump tag
	local tag = struct.pack("<i4i4i4", 0x02, 4, packer.address["kernel"])
	f:write(tag)
	n = n + #tag

	-- write data
	for i, v in ipairs(packer.sections) do
		f:write(v.data)
		n = n + #v.data
	end
	
	-- align 4 bytes
	if (n % 4) ~= 0 then
		local len = 4 - (n % 4)
		f:write(string.rep("\000", len))
		n = n + len
	end
end

local function packer_destroy(packer)
	packer.f:close()
end

local function main()
	local sections = {}
	local kernel = {}
	local initrd
	local cmdline = {}
	local outname

	for i = 1, #arg do
		local arch, s = string.match(arg[i], "kernel_(%w+)=(.+)")
		if s and arch then
			kernel[arch] = s
		end
		s = string.match(arg[i], "initrd=(.+)")
		if s then
			initrd = s
		end
		arch, s = string.match(arg[i], "cmdline_(%w+)=(.+)")
		if s and arch then
			cmdline[arch] = s
		end
		s = string.match(arg[i], "out=(.+)")
		if s then
			outname = s
		end
	end

	if not kernel or not outname then
		print_usage()
		return
	end
	--print("kernel=", kernel)
	--print("initrd=", initrd)
	--print("cmdline=", cmdline)
	--print("out=", outname)

	local packer = packer_create(outname)
	packer_addSections(packer, "kernel", kernel)
	if initrd then
		packer_addSection(packer, "initrd", initrd)	
	end
	if cmdline then
		packer_addSections(packer, "cmdline", cmdline)
	end
	packer_generate(packer)
	packer_destroy(packer)
end

main()
