local function heuristic(a, b)
	return math.sqrt((b.x - a.x)^2 + (b.y - a.y)^2)
end

local function closest_point(start, points)
  local closest_point = nil
  local closest_distance = math.huge
  for i=1,#points do
    local distance = heuristic(points[i], start)
    if distance < closest_distance then
      closest_point = i
      closest_distance = distance
    end
  end
	return closest_point
end

local function find_barrier_at(point, ignore)
	-- for _, b in ipairs(level.barriers) do
	-- 	if b ~= ignore and b:containsPoint(point.x, point.y) and not b.fixed then
	-- 		return b
	-- 	end
	-- end
end

local function mark_neighbors(index, points, ignore)
	local open_points = {}
	local closest_distance = 50
	local start = points[index]
  for i=1,#points do
		local point = points[i]
    local distance = heuristic(point, start)
		if not point.parent
			and distance < closest_distance
			and not find_barrier_at(point, ignore)
		then
			point.parent = index
			open_points[#open_points + 1] = i
		end
	end
	return open_points
end

local function fill_path(start_point, finish_point, open_list, ignore)
	local my_points = { finish_point }
	open_list[finish_point].parent = -1
	for i=1,50 do
		local new_points = {}
		for _, p1 in ipairs(my_points) do
			for _, p2 in ipairs(mark_neighbors(p1, open_list, ignore)) do
				new_points[#new_points + 1] = p2
				if p2 == start_point then
					return
				end
			end
		end
		my_points = new_points
	end
end

local function lerp2(a, b, t)
	return cg.Vector2.new(a.x, a.y) * (1 - t) + cg.Vector2.new(b.x, b.y) * t
end

local function find_path(self, target, points)
	local finish = cg.Vector2.new(target.x, target.y)
  local start = cg.Vector2.new(self.x, self.y)

	local open_list = {}

  for i=1,#points do
		open_list[i] = {
			x = points[i].x,
			y = points[i].y
		}
	end

	local finish_point = closest_point(finish, open_list)
	local start_point = closest_point(start, open_list)
	local path = {}

	fill_path(start_point, finish_point, open_list, target)

	if open_list[start_point].parent then
		local it = start_point
		while it ~= finish_point do
			path[#path + 1] = open_list[it]
			it = open_list[it].parent
		end
	end

	if #path > 0 then
		path[#path + 1] = lerp2(path[#path], finish, 0.5)
		return path
	else
		return {}
	end
end

return find_path