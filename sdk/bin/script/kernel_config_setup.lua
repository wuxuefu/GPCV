require "lfs"

function checkKernelConfig(kernelPath, arch)
	local flag = false
	local pattern = "CONFIG_ARCH_" .. arch .. "=y"
	for line in io.lines(kernelPath .. "/.config") do
		if string.find(line, pattern) then
			flag = true
			break;
		end
	end
	
	return flag
end

function parseConfigFile(filename)
	local arch
	local configFile

	local file = io.open(filename, "r")
	if not file then
		return nil
	end
	
	for line in file:lines() do
		arch = string.match(line, "CONFIG_ARCH_([^_]*)=y")
		if arch then
			break;
		end
	end
	for line in file:lines() do
		local defArch, defFile = string.match(line, "CONFIG_DEFCONFIG_(.*)=\"(.*)\"")
		if defArch == arch then
			--print("DEFCONFIG", defFile)
			configFile = defFile
			break;
		end
	end
	file:close()
	
	return arch, configFile
end

function isDefConfigUpdated(kernelPath, configFile)
	local currAttr = lfs.attributes(kernelPath .. "/.config")
	local defConfigAttr = lfs.attributes(kernelPath .. "/arch/arm/configs/" .. configFile)
	--print(currAttr.modification, defConfigAttr.modification)
	return defConfigAttr.modification > currAttr.modification
end

function configKernel(kernelPath, arch, configFile)
	if not configFile then
		if arch == "SPMP8050" then
			configFile = "spmp8050_defconfig"
		elseif arch == "GPL32900" then
			configFile = "gpl32900_defconfig"
		elseif arch == "GPL32900B" then
			configFile = "gpl32900b_defconfig"
		elseif arch == "GPL64000" then
			configFile = "gpl64000_defconfig"
		else
			print("Unknown arch")
			return false
		end
	end
	
	os.execute(string.format("cd %s; make %s", kernelPath, configFile))
	return true
end

function main(kernelPath, arch, defconfig)
	local sysconfig = {
		arch = arch,
		defconfig = defconfig,
	}
	
	local currArch, currDefConfig = parseConfigFile(kernelPath .. "/.config")
	if currArch == nil then
		print("Kernel arch config is nil, reconfig kernel to " .. sysconfig.arch)
		configKernel(kernelPath, sysconfig.arch, sysconfig.defconfig)
		print("Reconfig kernel done.\n")
	elseif currArch ~= sysconfig.arch then
		print("Kernel arch config : " .. currArch)
		print("SYSCONFIG_ARCH : " .. sysconfig.arch)
		print("Kernel arch not match, reconfig kernel to " .. sysconfig.arch)
		configKernel(kernelPath, sysconfig.arch, sysconfig.defconfig)
		print("Reconfig kernel done.\n")
	elseif sysconfig.defconfig then
		if currDefConfig ~= sysconfig.defconfig then
			print("Kernel config file : " .. (currDefConfig or "unknown"))
			print("Wish config file : " .. sysconfig.defconfig)
			print("Kernel config file not match, reconfig kernel to " .. sysconfig.defconfig)
			configKernel(kernelPath, sysconfig.arch, sysconfig.defconfig)
			print("Reconfig kernel done.\n")
		elseif isDefConfigUpdated(kernelPath, sysconfig.defconfig) then
			print("!!!!!! Kernel config file is updated, reconfig kernel to " .. sysconfig.defconfig)
			configKernel(kernelPath, sysconfig.arch, sysconfig.defconfig)
			print("Reconfig kernel done.\n")
		end
	else
		--print("Kernel arch config : " .. currArch)
		--print("SYSCONFIG_ARCH : " .. arch)
	end
end

main(...)

