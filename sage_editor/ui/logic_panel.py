"""Panel for FlowScript logic."""
import customtkinter as ctk


class LogicPanel(ctk.CTkFrame):
    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        ctk.CTkLabel(self, text="Logic Flow").pack(anchor="w", padx=5, pady=5)
