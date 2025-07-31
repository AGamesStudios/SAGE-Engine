"""Bottom log console panel."""
import customtkinter as ctk


class BottomLog(ctk.CTkFrame):
    def __init__(self, master: ctk.CTkBaseClass) -> None:
        super().__init__(master, height=120)
        self.pack_propagate(False)
        self.log = ctk.CTkTextbox(self)
        self.log.pack(fill="both", expand=True)
