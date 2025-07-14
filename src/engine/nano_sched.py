from concurrent.futures import ThreadPoolExecutor, wait
from dataclasses import dataclass, field
from typing import Callable, Iterable, Set, List

__all__ = ["PatcherTask", "run_sched"]


@dataclass
class PatcherTask:
    func: Callable[[], None]
    read: Set[str] = field(default_factory=set)
    write: Set[str] = field(default_factory=set)


def _build_deps(tasks: List[PatcherTask]) -> List[Set[int]]:
    deps = [set() for _ in tasks]
    for i, t1 in enumerate(tasks):
        for j, t2 in enumerate(tasks):
            if i == j:
                continue
            if t1.write & (t2.read | t2.write):
                deps[j].add(i)
    return deps


def run_sched(tasks: Iterable[PatcherTask], max_workers: int = 8) -> None:
    tasks_list = list(tasks)
    deps = _build_deps(tasks_list)
    remaining = set(range(len(tasks_list)))
    results = [None] * len(tasks_list)
    with ThreadPoolExecutor(max_workers=max_workers) as pool:
        while remaining:
            ready = [i for i in remaining if not deps[i] - (set(range(len(tasks_list))) - remaining)]
            future_map = {pool.submit(tasks_list[i].func): i for i in ready}
            wait(future_map.keys())
            for fut, idx in future_map.items():
                results[idx] = fut.result()
                remaining.remove(idx)
    return results
