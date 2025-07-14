# Toolbar

The toolbar sits vertically in the top-left corner and starts with **Edit** and **Model** buttons. Edit mode exposes move, rotate, scale and rect tools while Model mode shows vertex handles for editing meshes and a second bar where you can pick **Verts**, **Edges** or **Faces**. Drag points, edges or the whole face according to the current mode. Selecting two neighbouring vertices automatically picks the edge between them. Press **E** to extrude vertices, **F** to create a face from the active edge and **L** to perform ``Loop Cut``. A **Fill** button toggles polygon fill and a context menu lists these tools. Boolean tools let you **Union**, **Difference** or **Intersect** two shapes while a **Negative** flag subtracts modifiers during unions. Use **Bake** to triangulate the current polygon. Entering Model mode again restores the polygon so baked meshes remain editable. A **Local** toggle aligns gizmos to the object's rotation. The move gizmo includes a small square between the arrows that lets you drag objects along both axes at once.
Right-click the viewport to create sprites, empties, cameras or simple shapes including polygons.
The *Screenshot* button opens a dialog for saving the current viewport as ``PNG``.

Below the menu bar a compact toolbar provides quick toggle buttons for the grid,
rulers, cursor coordinates, a statistics panel, wireframe view and local mode.
Toolbars are fixed in place without visible drag handles.
