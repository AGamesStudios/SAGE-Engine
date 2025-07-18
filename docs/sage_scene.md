# SAGE Scene

The Scene system manages `SAGEObject` instances and their parent/child
relationships. Each object can reference a `parent_id`. When added to the
scene, caches are updated so lookups by role or layer are fast.

Use `add_object(obj)` to register objects and `remove_object(id)` to delete an
object along with all of its children. Objects marked via `mark_for_removal()`
are purged on `cleanup()`.
