local str = io.read()
local t

local function h(t)
	return math.min(math.max(math.abs(3 - t % 6) - 1, 0), 1)
end

local function r(x)
	if select(2, math.modf(x)) < 0.5 then
	return math.floor(x) end
	return math.ceil(x)
end

for i = 1, #str do
	t = 6 * (i - 1) / (#str - 1)
	io.write(string.format("\x1B[38;2;%i;%i;%im%s",
		r(h(t - 0) * 255),
		r(h(t - 2) * 255),
		r(h(t - 4) * 255),
		str:sub(i, i)
	))
end

io.write("\x1B[m\n")
