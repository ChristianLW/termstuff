local function tput(arg)
	local p = io.popen("tput "..arg)
	local r = p:read("n")
	p:close()
	return r
end

local ESC <const> = "\x1B["
local BLOCK <const> = "\u{2588}"
local TOP <const> = "\u{2580}"
local BOTTOM <const> = "\u{2584}"

local TRAIL_DECAY <const> = 1.0

io.write(ESC, "?1049h", ESC, "?25l")

local w <const> = tput("cols")
local h <const> = tput("lines") * 2

local x, y = w/2, h/2
local vx, vy

do
	local theta = math.random() * 2.0*math.pi
	vx, vy = math.cos(theta), math.sin(theta)
end

local function round(x)
	if select(2, math.modf(x)) < 0.5 then
		return math.floor(x)
	end
	return math.ceil(x)
end

local buffer = {}

for f = 1, 200 do
	-- Render
	for n, p in pairs(buffer) do
		local written = false
		local other = nil
		for i = 1, n - 1 do if buffer[i].x == p.x and buffer[i].y == p.y then written = true break end end
		if not written then
			io.write(ESC, string.format("%i;%iH", p.y//2, p.x))
			io.write(ESC, string.format("38;2;%i;%i;%i;48;2;%i;%i;%im",
				round(vb * 255), round(vb * 255), round(vb * 255),
				round(vt * 255), round(vt * 255), round(vt * 255)
			), BOTTOM)
		end
		if p.v <= 0.0 then
			table.remove(buffer, n)
		end
		p.v = p.v - TRAIL_DECAY
	end
	-- Postrender
	io.write(ESC, string.format("1;%iH", w-3), ESC, "30;47m", string.format("%04i", f), ESC, "m")
	io.flush()
	-- Update
	x, y = x + vx, y + vy
	if x < 1 or x > w then vx = -vx end
	if y < 1 or y > h then vy = -vy end
	-- Draw onto buffer
	for _, p in pairs(buffer) do if buffer[i].x == x and buffer[i].y == y then table.remove(buffer, i) end end
	table.insert(buffer, {x = math.tointeger(round(x)), y = math.tointeger(round(y)), v = 1.0})
	-- Time
	os.execute("sleep 0.05s")
end

io.write(ESC, "?25h", ESC, "?1049l")
