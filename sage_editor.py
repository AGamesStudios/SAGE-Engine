import dearpygui.dearpygui as dpg


# Function to load a scene (placeholder for SAGE Engine integration)
def load_scene(sender, app_data):
    print("Loading scene...")  # Replace with SAGE Engine call if needed


# Create DearPyGUI context
dpg.create_context()

# Set up the application viewport
dpg.create_viewport(title="SAGEEditor", width=800, height=600)

# Main editor window
with dpg.window(label="Main Window", no_close=True):
    # Menu bar
    with dpg.menu_bar():
        with dpg.menu(label="File"):
            dpg.add_menu_item(label="New")
            dpg.add_menu_item(label="Open", callback=load_scene)
            dpg.add_menu_item(label="Save")
            dpg.add_menu_item(label="Exit", callback=lambda: dpg.stop_dearpygui())

    # Toolbar window (using pos instead of x_pos and y_pos)
    with dpg.window(label="Toolbar", no_title_bar=True, no_resize=True, no_move=True, menubar=False, pos=[0, 0],
                    width=800, height=30):
        dpg.add_button(label="Play")
        dpg.add_button(label="Pause")
        dpg.add_button(label="Stop")

    # Scene area window (using pos for positioning)
    with dpg.window(label="Scene", pos=[0, 30], width=800, height=570):
        dpg.add_text("Scene view area")  # Future 3D widget can go here

# Initialize and run the application
dpg.setup_dearpygui()
dpg.show_viewport()
dpg.start_dearpygui()

# Clean up resources
dpg.destroy_context()