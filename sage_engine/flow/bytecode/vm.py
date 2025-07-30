from __future__ import annotations

import ast
from typing import Any

__all__ = ["run"]


class _Interpreter(ast.NodeVisitor):
    def __init__(self, ctx: dict[str, Any]):
        self.ctx = ctx

    # expression helpers
    def visit_Name(self, node: ast.Name):
        return self.ctx.get(node.id)

    def visit_Attribute(self, node: ast.Attribute):
        obj = self.visit(node.value)
        return getattr(obj, node.attr)

    def visit_Subscript(self, node: ast.Subscript):
        val = self.visit(node.value)
        key = self.visit(node.slice)
        return val[key]

    def visit_Index(self, node: ast.Index):
        return self.visit(node.value)

    def visit_Constant(self, node: ast.Constant):
        return node.value

    def visit_BinOp(self, node: ast.BinOp):
        left = self.visit(node.left)
        right = self.visit(node.right)
        if isinstance(node.op, ast.Add):
            return left + right
        if isinstance(node.op, ast.Sub):
            return left - right
        if isinstance(node.op, ast.Mult):
            return left * right
        if isinstance(node.op, ast.Div):
            return left / right
        raise RuntimeError("Unsupported binary operator")

    def visit_Compare(self, node: ast.Compare):
        left = self.visit(node.left)
        for op, comp in zip(node.ops, node.comparators):
            right = self.visit(comp)
            if isinstance(op, ast.Gt):
                if not left > right:
                    return False
            elif isinstance(op, ast.Lt):
                if not left < right:
                    return False
            elif isinstance(op, ast.GtE):
                if not left >= right:
                    return False
            elif isinstance(op, ast.LtE):
                if not left <= right:
                    return False
            elif isinstance(op, ast.Eq):
                if not left == right:
                    return False
            elif isinstance(op, ast.NotEq):
                if not left != right:
                    return False
            left = right
        return True

    # statement handlers
    def visit_Module(self, node: ast.Module):
        for stmt in node.body:
            self.visit(stmt)

    def visit_Expr(self, node: ast.Expr):
        self.visit(node.value)

    def visit_Assign(self, node: ast.Assign):
        if len(node.targets) != 1:
            return
        target = node.targets[0]
        val = self.visit(node.value)
        if isinstance(target, ast.Name):
            self.ctx[target.id] = val
        elif isinstance(target, ast.Subscript):
            obj = self.visit(target.value)
            key = self.visit(target.slice)
            obj[key] = val

    def visit_AugAssign(self, node: ast.AugAssign):
        if isinstance(node.target, ast.Name):
            cur_ref = self.ctx
            key = node.target.id
        elif isinstance(node.target, ast.Subscript):
            cur_ref = self.visit(node.target.value)
            key = self.visit(node.target.slice)
        else:
            return
        cur = cur_ref.get(key, 0)
        val = self.visit(node.value)
        if isinstance(node.op, ast.Add):
            cur += val
        elif isinstance(node.op, ast.Sub):
            cur -= val
        elif isinstance(node.op, ast.Mult):
            cur *= val
        elif isinstance(node.op, ast.Div):
            cur /= val
        cur_ref[key] = cur

    def visit_If(self, node: ast.If):
        if self.visit(node.test):
            for s in node.body:
                self.visit(s)
        else:
            for s in node.orelse:
                self.visit(s)

    def visit_For(self, node: ast.For):
        if not isinstance(node.target, ast.Name):
            return
        iterable = self.visit(node.iter)
        for item in iterable:
            self.ctx[node.target.id] = item
            for s in node.body:
                self.visit(s)

    def visit_Call(self, node: ast.Call):
        func = self.visit(node.func)
        args = [self.visit(a) for a in node.args]
        return func(*args)

    def visit_FunctionDef(self, node: ast.FunctionDef):
        def _fn(*args):
            local = dict(self.ctx)
            for n, a in zip([a.arg for a in node.args.args], args):
                local[n] = a
            intr = _Interpreter(local)
            for s in node.body:
                intr.visit(s)
            self.ctx.update(local)
            _fn.__globals__.update(self.ctx)
        _fn.__globals__.update(self.ctx)
        self.ctx[node.name] = _fn
        
    def visit_AsyncFunctionDef(self, node: ast.AsyncFunctionDef):
        async def _fn(*args):
            local = dict(self.ctx)
            for n, a in zip([a.arg for a in node.args.args], args):
                local[n] = a
            intr = _Interpreter(local)
            for s in node.body:
                intr.visit(s)
            self.ctx.update(local)
            _fn.__globals__.update(self.ctx)
        _fn.__globals__.update(self.ctx)
        self.ctx[node.name] = _fn

    def visit_Global(self, node: ast.Global):
        # globals are in the same context; nothing to do
        pass

    def generic_visit(self, node: ast.AST):
        raise RuntimeError(f"Unsupported syntax: {type(node).__name__}")


def run(code: ast.Module, ctx: dict[str, Any]) -> None:
    """Execute FlowScript AST using a restricted interpreter."""
    _Interpreter(ctx).visit(code)
