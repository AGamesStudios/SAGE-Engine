import cmd
from pathlib import Path

from ..lua_runner import run_lua_script
from ..python_runner import run_python_script
from sage_fs import FlowRunner

class SageTerminal(cmd.Cmd):
    intro = "SAGE Terminal. Type help or ? to list commands."
    prompt = "(sage) "

    def do_run(self, arg: str) -> None:
        """run <script>: execute a FlowScript, Lua or Python file"""
        path = Path(arg)
        if not path.exists():
            print("file not found")
            return
        if path.suffix == ".lua":
            run_lua_script(str(path))
        elif path.suffix == ".py":
            run_python_script(str(path))
        else:
            FlowRunner().run_file(str(path))

    def do_exit(self, arg: str) -> bool:  # type: ignore[override]
        """Exit the terminal"""
        return True


def launch_terminal() -> None:
    """Start the interactive terminal."""
    SageTerminal().cmdloop()

__all__ = ["SageTerminal", "launch_terminal"]
