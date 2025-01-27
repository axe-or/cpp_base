-------- User configuration ----------------------------------------------------
function Config()
	local RELEASE_BUILD = true

	local CFLAGS  = {'-std=c++20', '-I.', '-fPIC', '-Wall', '-Wextra', '-Wno-unused-label'}
	local LDFLAGS = {}

	if RELEASE_BUILD then
		Array.append(CFLAGS, '-Os')
	else
		Array.append(CFLAGS, '-O0', '-g')
	end

	return {
		common = {
			compiler = 'clang++',
			linker   = 'clang++',
			archiver = 'ar',
			cflags   = CFLAGS,
			ldflags  = LDFLAGS,
		},

		libraries = {
			['base.a'] = {
				sources  = {
					'src/arena.cpp',
					'src/assert.cpp',
					'src/heap_allocator.cpp',
					'src/memory.cpp',
					'src/strings.cpp',
					'src/utf8.cpp',
				},
				extra = {'src/base.hpp'},
			},
		},

		executables = {
			['demo'] = {
				sources  = {'demo.cpp'},
				binaries = {'base.a'},
				extra    = {'src/base.hpp', 'tests.cpp'},
			},
		},

	}
end

--------- Generation -----------------------------------------------------------
function Generate()
	Build = Config()
	Ninja:variables(Build.common)

	Ninja:rule('compile', '$compiler $cflags -c $in -o $out')
	Ninja:rule('link', '$linker -o $out $in $ldflags')
	Ninja:rule('archive', '$archiver rcs $out $in')
	Ninja:write('\n')

	for target, info in pairs(Build.executables) do
		local objects = {}

		for _, source in ipairs(info.sources) do
			objects[#objects+1] = source .. '.o'
			Ninja:build('compile', source, objects[#objects], info.extra, info.variables)
		end
		Array.append(objects, table.unpack(info.binaries)) -- bs related to some linkers working backwards
		Ninja:build('link', objects, target, Array.concat(info.binaries, info.extra), info.variables)
	end

	for target, info in pairs(Build.libraries) do
		local objects = {}
		for _, source in ipairs(info.sources) do
			objects[#objects+1] = source .. '.o'
			Ninja:build('compile', source, objects[#objects], info.extra, info.variables)
		end
		Ninja:build('archive', objects, target, Array.concat(info.binaries, info.extra), info.variables)
	end
end

--------- Array ----------------------------------------------------------------
Array = {}

function Array.concat(a, b)
	local res = Array:new()
	if a then
		for _, v in ipairs(a) do
			res[#res+1] = v
		end
	end
	if b then
		for _, v in ipairs(b) do
			res[#res+1] = v
		end
	end
	return res
end

function Array.map(arr, fn)
	local res = Array:new()
	for i, v in ipairs(arr) do
		res[i] = fn(v)
	end
	return res
end

function Array.rep(arr, count)
	local res = Array:new()
	for _ = 1, count, 1 do
		for _, v in ipairs(arr) do
			res[#res+1] = v
		end
	end
	return res
end

function Array:new(obj)
	obj = obj or {}
	assert(type(obj) == 'table', 'Initializer object must be a table')

	setmetatable(obj, self)
	self.__index = self

	return obj
end

function Array.append(arr, ...)
	local args = table.pack(...)
	for _, v in ipairs(args) do
		arr[#arr+1] = v
	end
end

function Array.pop(arr)
	local v = arr[#arr]
	arr[#arr] = nil
	return v
end

function Array.insert(arr, idx, v)
	table.insert(arr, idx, v)
end

function Array:remove(arr, idx)
	table.remove(arr, idx)
end

function Array.join(arr, sep)
	return table.concat(arr, sep)
end

Array.__add = Array.concat

local function lines(s)
	return table.concat(s, '\n') .. '\n'
end

local function spaces(s)
	if not s then return ' ' end
	return table.concat(s, ' ')
end

--------- Ninja ----------------------------------------------------------------
Ninja = {}

function Ninja:init(path)
	self.file = io.open(path, 'wb')
end

function Ninja:done()
	self.file:close()
	self.file = nil
end

function Ninja:write(s)
	assert(self.file, 'Ninja output not open, refusing to write')
	self.file:write(s)
end

function Ninja:rule(ruleName, command)
	local ruleDef = lines {
		'rule %s',
		'  command = %s'
	}:format(ruleName, command)

	self:write(ruleDef)
end

function Ninja:build(rule, input, output, extra, shadows)
	local srcLines = {('build %s: %s %s'):format(output, rule, type(input) == 'table' and spaces(input) or input)}
	if extra then
		srcLines[1] = srcLines[1] .. ' | ' .. spaces(extra)
	end
	if shadows then
		for k, v in pairs(shadows) do
			srcLines[#srcLines+1] = ('  %s = %s'):format(k, type(v) == 'table' and spaces(v) or tostring(v))
		end
	end

	self:write(lines(srcLines))
end

function Ninja:variables(tbl, indent)
	for key, val in pairs(tbl) do
		local line = ('%s = %s\n'):format(key, type(val) == 'table' and spaces(val) or val)
		if indent then
			self:write(('  '):rep(indent))
		end
		self:write(line)
	end
	self:write('\n')
end

-------- Main ------------------------------------------------------------------
local outfile = 'build.ninja'
Ninja:init(outfile)
Generate()
Ninja:done()
print('Updated ' .. outfile)
