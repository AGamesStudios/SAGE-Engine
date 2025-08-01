from sage_engine.gui import widgets, drag, manager

btn1 = widgets.Button(text="Drag")
btn2 = widgets.Button(text="Drop")
manager.root.add_child(btn1)
manager.root.add_child(btn2)
controller = drag.DragController(btn1)
controller.start(0, 0)
controller.end(0, 0)
drag.drop(btn2, btn1)
