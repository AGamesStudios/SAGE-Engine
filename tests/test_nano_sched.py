from sage_engine.nano_sched import PatcherTask, run_sched


def test_run_sched_dependencies():
    order = []

    def first():
        order.append('a')

    def second():
        order.append('b')

    tasks = [
        PatcherTask(first, write={'x'}),
        PatcherTask(second, read={'x'}),
    ]
    run_sched(tasks, max_workers=2)
    assert order == ['a', 'b']
