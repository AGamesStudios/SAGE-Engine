import asyncio
from sage_engine import core, world, render, events, scheduler, tasks, audio
from sage_engine.flow.python import run as run_flow

# register boot/update phases
core.register('boot', scheduler.time.boot)
core.register('boot', scheduler.timers.boot)
core.register('boot', events.boot)
core.register('boot', world.boot)
core.register('boot', tasks.boot)

core.register('update', scheduler.time.update)
core.register('update', scheduler.timers.update)
core.register('update', events.update)
core.register('update', tasks.update)
core.register('update', world.update)

core.register('draw', lambda: render.prepare(world.scene))
core.register('draw', render.sort)
core.register('flush', render.flush)


def main():
    core.core_boot({})
    edit = world.scene.begin_edit()
    edit.create(role='sprite', name='hero', tex_id=1, x=0, y=0)
    world.scene.apply(edit)
    audio.audio.play('start.wav')

    def on_click():
        audio.audio.play('click.wav')

    events.dispatcher.on(1, on_click)
    scheduler.timers.manager.set(0.0, lambda: events.dispatcher.emit(1))
    asyncio.run(run_flow("ctx['done'] = True", {'ctx': {}}))
    for _ in range(3):
        core.core_tick()
    core.core_shutdown()


if __name__ == '__main__':
    main()
