"""Canvas displaying the current scene."""

import customtkinter as ctk


class SceneView(ctk.CTkFrame):
    """Area for manipulating objects with the mouse."""

    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.canvas = ctk.CTkCanvas(self, width=600, height=400)
        self.canvas.pack(fill="both", expand=True)
