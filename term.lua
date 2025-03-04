local ESC = "\x1B["
local HALF = "\u{2584}"

local mode = (...)
local modes = {"rainbow", "grey", "grid", "noise", "colournoise", "mandelbrot", "circle"}

local minx, maxx = math.huge, -math.huge
local miny, maxy = math.huge, -math.huge

if not mode then
	io.write(
		"You have to specify an extra argument to tell the program what to draw.\n",
		"The mode can be one of the following: "..table.concat(modes, ", ").."\n"
	)
	return
elseif (function() for k, v in pairs(modes) do if mode == v then return false end end return true end)() then
	io.write(
		"'"..mode.."' is not a valid mode.\n",
		"The mode can be one of the following: "..table.concat(modes, ", ").."\n"
	)
	return
end

local function col(x, y, w, h)
	if mode == "rainbow" then
		local alpha = (x/w + y/h) * 3.0
		return
		math.min(math.max(math.abs(3.0 - (alpha - 0.0) % 6.0) - 1.0, 0.0), 1.0),
		math.min(math.max(math.abs(3.0 - (alpha - 2.0) % 6.0) - 1.0, 0.0), 1.0),
		math.min(math.max(math.abs(3.0 - (alpha - 4.0) % 6.0) - 1.0, 0.0), 1.0)
	elseif mode == "grey" then
		local value = (x/w + y/h) * 0.5
		return value, value, value
	elseif mode == "grid" then
		local value = (x + y) % 2 == 0 and 1.0 or 0.0
		return value, value, value
	elseif mode == "noise" then
		local value = math.random()
		return value, value, value
	elseif mode == "colournoise" then
		local hue = math.random() * 6.0
		return
		math.min(math.max(math.abs(3.0 - (hue - 0.0) % 6.0) - 1.0, 0.0), 1.0),
		math.min(math.max(math.abs(3.0 - (hue - 2.0) % 6.0) - 1.0, 0.0), 1.0),
		math.min(math.max(math.abs(3.0 - (hue - 4.0) % 6.0) - 1.0, 0.0), 1.0)
	elseif mode == "mandelbrot" then
		local u, v = x/w, y/h
		local r = (w + 1) / (h + 1)
		local cx = 2.5 * (u - 0.5)
		local cy = 2.5 * (v - 0.5)
		if r > 0 then cx = cx * r else cy = cy / r end
		local zx, zy = cx, cy
		for i = 0, 15 do
			zx, zy = zx*zx - zy*zy + cx, 2.0*zx*zy + cy
			if zx*zx + zy*zy > 4 then return 0.0, 0.0, i/15 end
		end
		return 0.0, 0.0, 0.0
	elseif mode == "circle" then
		local u, v = x/w, y/h
		local r = (w + 1) / (h + 1)
		local cx = 2.5 * u - 1.25
		local cy = 2.5 * v - 1.25
		if r > 0 then cx = cx * r else cy = cy / r end
		if cx*cx + cy*cy <= 1.0 then
			if cx < minx then minx = cx end if cx > maxx then maxx = cx end
			if cy < miny then miny = cy end if cy > maxy then maxy = cy end
		return 1.0, 1.0, 1.0 else return 0.0, 0.0, 0.0 end
	end
end

local function rgb(x, y, w, h)
	local t = {0, 0, 0, 0, 0, 0}
	t[1], t[2], t[3] = col(x, 2*y, w, 2*h)
	t[4], t[5], t[6] = col(x, 2*y+1, w, 2*h)
	for k, v in pairs(t) do
		t[k] = math.floor(v * 255 + 0.5)
	end
	return table.unpack(t)
end

local function stty(data)
	if not data then
		local p = io.popen("stty -g")
		local d = p:read()
		p:close()
		return d
	else
		os.execute("stty "..data)
	end
end

local function tput(arg)
	local p = io.popen("tput "..arg)
	local r = p:read("n")
	p:close()
	return r
end

local ttydata = stty()

io.write(ESC, "?1049h", ESC, "?25l")

stty("-icanon -echo")

local w = tput("cols")
local h = tput("lines")

-- Bottom row is causing issues, check the 2*h and such

for y = 1, h do
	for x = 1, w do
		io.write(ESC, string.format("48;2;%i;%i;%i;38;2;%i;%i;%im", rgb(x - 1, y - 1, w - 1, h - 1)), HALF)
	end
	if y < h then io.write("\n") end
end

io.write(ESC, "m", ESC, "H")
io.read(1)

stty(ttydata)

io.write(ESC, "?25h", ESC, "?1049l")
io.flush()
print(tostring(minx).." - "..tostring(maxx).."\n"..tostring(miny).." - "..tostring(maxy))
