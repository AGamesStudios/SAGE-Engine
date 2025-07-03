from ..base import Condition, register_condition

@register_condition('InputAxis', [('axis_id', 'value'), ('threshold', 'value'), ('comparison', 'value')])
class InputAxis(Condition):
    """Check an analog input axis against a threshold."""

    def __init__(self, axis_id: int = 0, threshold: float = 0.5, comparison: str = '>='):
        self.axis_id = axis_id
        self.threshold = threshold
        self.comparison = comparison

    def check(self, engine, scene, dt):
        val = engine.input.get_axis_value(self.axis_id)
        if val is None:
            raise ValueError(f"Axis {self.axis_id} not supported")
        if self.comparison == '>':
            return val > self.threshold
        if self.comparison == '>=':
            return val >= self.threshold
        if self.comparison == '<':
            return val < self.threshold
        if self.comparison == '<=':
            return val <= self.threshold
        return val == self.threshold
