function run(cmd)
	if type(cmd) == 'string' then
		os.execute(cmd)
	elseif type(cmd) == 'table' then
		cmd = table.concat(cmd, ' ')
		os.execute(cmd)
	else
		error("Invalid cmd type", 2)
	end
end

