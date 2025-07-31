"""Top bar with application controls."""
import customtkinter as ctk


class TopBar(ctk.CTkFrame):
    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master)
        self.title_label = ctk.CTkLabel(self, text="SAGE Studio")
        self.title_label.pack(side="left", padx=10)

        self.settings_btn = ctk.CTkButton(self, text="Settings")
        self.settings_btn.pack(side="right", padx=5)
        self.docs_btn = ctk.CTkButton(self, text="Docs")
        self.docs_btn.pack(side="right", padx=5)
        self.run_btn = ctk.CTkButton(self, text="Run")
        self.run_btn.pack(side="right", padx=5)
        self.stop_btn = ctk.CTkButton(self, text="Stop")
        self.stop_btn.pack(side="right", padx=5)
        self.new_btn = ctk.CTkButton(self, text="New Game")
        self.new_btn.pack(side="right", padx=5)
