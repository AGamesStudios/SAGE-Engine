"""Main window of the SAGE Studio editor."""
import customtkinter as ctk

from .top_bar import TopBar
from .left_bar import LeftBar
from .world_panel import WorldPanel
from .logic_panel import LogicPanel
from .game_viewport import GameViewport
from .bottom_log import BottomLog


class MainWindow(ctk.CTk):
    """Central application window arranging all panels."""

    def __init__(self) -> None:
        super().__init__()
        self.title("SAGE Studio")
        self.geometry("1024x768")
        self._create_widgets()

    def _create_widgets(self) -> None:
        self.top_bar = TopBar(self)
        self.top_bar.pack(side="top", fill="x")

        center_frame = ctk.CTkFrame(self)
        center_frame.pack(side="top", fill="both", expand=True)

        self.left_bar = LeftBar(center_frame)
        self.left_bar.pack(side="left", fill="y")

        self.main_view = GameViewport(center_frame)
        self.main_view.pack(side="left", fill="both", expand=True)

        right_frame = ctk.CTkFrame(center_frame)
        right_frame.pack(side="left", fill="y")

        self.world_panel = WorldPanel(right_frame)
        self.world_panel.pack(fill="x")

        self.logic_panel = LogicPanel(right_frame)
        self.logic_panel.pack(fill="both", expand=True)

        self.bottom_log = BottomLog(self)
        self.bottom_log.pack(side="bottom", fill="x")
