from sage_engine import core_boot, framesync, input, time

core_boot()
input.press_key("A")
for _ in range(3):
    framesync.regulate()
    dt = time.get_delta()
    if input.is_key_down("A"):
        print("A held", dt)
input.release_key("A")
