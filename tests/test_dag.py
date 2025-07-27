from sage_engine import dag
import pytest


def test_dag_order():
    dag.reset()
    order = []
    dag.add_phase("a", lambda: order.append("a"))
    dag.add_phase("b", lambda: order.append("b"), deps=["a"])
    dag.run()
    assert order == ["a", "b"]


def test_dag_cycle():
    dag.reset()
    dag.add_phase("a", lambda: None, deps=["b"])
    dag.add_phase("b", lambda: None, deps=["a"])
    with pytest.raises(RuntimeError):
        dag.run()
