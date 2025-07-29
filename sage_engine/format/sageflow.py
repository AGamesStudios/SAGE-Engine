from . import SAGECompiler, SAGEDecompiler


def compile_yaml(src, dst):
    SAGECompiler().compile(src, dst)


def load(path):
    return SAGEDecompiler().decompile(path)
