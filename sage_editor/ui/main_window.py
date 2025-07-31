"""Main window of the SAGE Studio editor."""

import customtkinter as ctk

from .scene_view import SceneView
from .object_inspector import ObjectInspector
from .blueprint_viewer import BlueprintViewer
from .flow_script_editor import FlowScriptEditor


class MainWindow(ctk.CTk):
    """Central application window."""

    def __init__(self) -> None:
        super().__init__()
        self.title("SAGE Studio")
        self.geometry("1024x768")
        self._create_widgets()

    def _create_widgets(self) -> None:
        self.scene_view = SceneView(self)
        self.scene_view.pack(side="left", fill="both", expand=True)

        right_panel = ctk.CTkFrame(self)
        right_panel.pack(side="right", fill="y")

        self.object_inspector = ObjectInspector(right_panel)
        self.object_inspector.pack(fill="y")

        bottom_panel = ctk.CTkFrame(self)
        bottom_panel.pack(side="bottom", fill="x")

        self.blueprint_viewer = BlueprintViewer(bottom_panel)
        self.blueprint_viewer.pack(side="left", fill="x", expand=True)

        self.flow_script_editor = FlowScriptEditor(bottom_panel)
        self.flow_script_editor.pack(side="right", fill="x", expand=True)
