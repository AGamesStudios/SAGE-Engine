# Viewport

The viewport shows the current scene using the OpenGL renderer by default.
Use the **Renderer** menu to switch between OpenGL and SDL if both backends are available.
Drag with the left mouse button to pan the camera, use the mouse wheel to zoom
and left click to select objects under the cursor.
Hold **Ctrl** or **Shift** and click to add or remove objects from the selection.
Hold **Alt** and drag a rectangle to perform a box selection that picks every
object or vertex inside the area.
Right clicking a selected object opens a menu with **Copy**, **Paste** and
**Delete** actions. Right clicking empty space opens a menu to create a new
object. You can also create square, triangle, circle or polygon shapes from this menu.
In Model mode additional controls let you extrude selected vertices, add faces from edges and perform ``Loop Cut`` to split edges evenly. A **Fill** button toggles polygon fill and a modeling menu lists these actions. Boolean tools let you **Union**, **Difference** or **Intersect** shapes while a **Negative** flag subtracts modifiers. **Bake** converts the polygon to triangles. Re‑entering Model mode restores the polygon so you can keep editing later. Shortcuts **E**, **F** and **L** trigger them.
object at the cursor position. Selecting a camera object shows a small preview
in the bottom-right corner of the viewport. The preview frame matches the
**SAGE Ember** style and keeps the camera's aspect ratio even if letterbox bars
are required.
Toolbar toggles enable the grid, rulers, wireframe view and local coordinates.
An optional statistics panel in the bottom-left corner shows the object count and current frame rate.
