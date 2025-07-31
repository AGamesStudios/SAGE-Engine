"""Main window of the SAGE Studio editor."""

import customtkinter as ctk

from .toolbar import Toolbar
from .world_manager import WorldManager
from .game_viewport import GameViewport
from .role_editor import RoleEditor
from .blueprint_designer import BlueprintDesigner
from .flow_script_editor import FlowScriptEditor
from .resource_manager import ResourceManager
from .object_view import ObjectView


class MainWindow(ctk.CTk):
    """Central application window arranging all panels."""

    def __init__(self) -> None:
        super().__init__()
        self.title("SAGE Studio")
        self.geometry("1024x768")
        self._create_widgets()

    def _create_widgets(self) -> None:
        self.toolbar = Toolbar(self)
        self.toolbar.pack(side="top", fill="x")

        top_frame = ctk.CTkFrame(self)
        top_frame.pack(side="top", fill="both", expand=True)

        self.world_manager = WorldManager(top_frame)
        self.world_manager.pack(side="left", fill="y")

        self.game_viewport = GameViewport(top_frame)
        self.game_viewport.pack(side="left", fill="both", expand=True)

        right_frame = ctk.CTkFrame(top_frame)
        right_frame.pack(side="left", fill="y")

        self.object_view = ObjectView(right_frame)
        self.object_view.pack(fill="x")

        self.role_editor = RoleEditor(right_frame)
        self.role_editor.pack(fill="both", expand=True)

        mid_frame = ctk.CTkFrame(self)
        mid_frame.pack(side="top", fill="x")

        self.blueprint_designer = BlueprintDesigner(mid_frame)
        self.blueprint_designer.pack(side="left", fill="both", expand=True)

        self.flow_script_editor = FlowScriptEditor(mid_frame)
        self.flow_script_editor.pack(side="left", fill="both", expand=True)

        self.resource_manager = ResourceManager(self)
        self.resource_manager.pack(side="bottom", fill="x")

