from dataclasses import dataclass

@dataclass
class Color:
    r: int
    g: int
    b: int
    a: int = 255

    def to_tuple(self) -> tuple[int, int, int, int]:
        return self.r, self.g, self.b, self.a
