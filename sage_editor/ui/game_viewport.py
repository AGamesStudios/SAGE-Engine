"""Canvas displaying the current scene."""

import customtkinter as ctk


class GameViewport(ctk.CTkFrame):
    """Area for displaying the current world and handling mouse input."""

    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.canvas = ctk.CTkCanvas(self, width=600, height=400)
        self.canvas.pack(fill="both", expand=True)

