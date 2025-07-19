local function move(id, dx, dy)
  local x = get_param(id, "x") or 0
  local y = get_param(id, "y") or 0
  set_param(id, "x", x + dx)
  set_param(id, "y", y + dy)
  log(string.format("[logic] player moved to (%.2f, %.2f)", x + dx, y + dy))
end

on_update(function(dt)
  local dx, dy = 0, 0
  if input.is_key_down("W") then dy = dy - 100 end
  if input.is_key_down("S") then dy = dy + 100 end
  if input.is_key_down("A") then dx = dx - 100 end
  if input.is_key_down("D") then dx = dx + 100 end
  if dx ~= 0 or dy ~= 0 then
    move("player", dx * dt, dy * dt)
  end
end)
