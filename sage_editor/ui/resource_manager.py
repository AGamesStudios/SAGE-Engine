"""Panel for browsing images, sounds and other resources."""

import customtkinter as ctk


class ResourceManager(ctk.CTkFrame):
    """Displays available assets with simple tree view."""

    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.label = ctk.CTkLabel(self, text="Resource Manager")
        self.label.pack(fill="x")

