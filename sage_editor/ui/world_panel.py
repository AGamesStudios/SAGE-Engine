"""Panel with world and object settings."""
import customtkinter as ctk


class WorldPanel(ctk.CTkFrame):
    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        ctk.CTkLabel(self, text="World Settings").pack(anchor="w", padx=5, pady=5)
