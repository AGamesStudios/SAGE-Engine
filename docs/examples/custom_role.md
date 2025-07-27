# 📘 Пример: своя роль

```python
from sage_engine.roles import Category, Col, RoleSchema, register_role

MY_SCHEMA = RoleSchema(
    name="dummy",
    categories=[Category("dummy", [Col("value", "i32", 0)])]
)

register_role(MY_SCHEMA)
```
