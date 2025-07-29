class LogHandler:
    def emit(self, record: str, level: int) -> None:
        raise NotImplementedError
