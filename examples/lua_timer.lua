time.boot()
time.mark()

on_update(function()
  if time.get_time() > 0.1 then
    log("tick")
    time.mark()
  end
end)
