# SAGE Scene

The Scene system manages `SAGEObject` instances and their parent/child
relationships. Each object can reference a `parent_id`. When added to the
scene, caches are updated so lookups by role or layer are fast.

Use `add_object(obj)` to register objects and `remove_object(id)` to delete an
object along with all of its children. Objects marked via `mark_for_removal()`
are purged on `cleanup()`.

Objects carry a `layer` field which controls draw order. Query functions like
`get_children()` and `get_parent()` let you traverse the hierarchy. For a full
depth-first traversal use `iter_dag()` which yields objects in parent → child
order starting from root objects.

Scenes can be saved to a `.sage_scene` JSON file via `save_scene(path, objs)`
and loaded back with `load_scene(path)`. This allows hot-reloading scenes by
replacing the file on disk and calling `load_scene` again.
