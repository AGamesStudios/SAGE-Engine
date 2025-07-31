"""Left sidebar showing resources and blueprints."""
import customtkinter as ctk


class LeftBar(ctk.CTkFrame):
    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master, width=180)
        self.pack_propagate(False)
        ctk.CTkLabel(self, text="Resources").pack(anchor="w", padx=5, pady=5)
        ctk.CTkLabel(self, text="Blueprints").pack(anchor="w", padx=10)
        ctk.CTkLabel(self, text="Roles").pack(anchor="w", padx=10)
        ctk.CTkLabel(self, text="Assets").pack(anchor="w", padx=10)
        ctk.CTkLabel(self, text="Scripts").pack(anchor="w", padx=10)
