local function move(id, dx, dy)
  local x = get_param(id, "x") or 0
  local y = get_param(id, "y") or 0
  set_param(id, "x", x + dx)
  set_param(id, "y", y + dy)
end

on_update(function(dt)
  local dx, dy = 0, 0
  if input.is_key_down("LEFT") then dx = dx - 100 end
  if input.is_key_down("RIGHT") then dx = dx + 100 end
  if input.is_key_down("UP") then dy = dy - 100 end
  if input.is_key_down("DOWN") then dy = dy + 100 end
  if dx ~= 0 or dy ~= 0 then
    move("Player", dx * dt, dy * dt)
  end
end)
