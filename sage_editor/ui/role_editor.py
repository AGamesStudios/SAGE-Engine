"""Role configuration widgets."""

import customtkinter as ctk


class RoleEditor(ctk.CTkFrame):
    """Edits the active role settings."""

    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.label = ctk.CTkLabel(self, text="Role Editor")
        self.label.pack(fill="x")

