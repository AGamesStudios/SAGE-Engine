import os, sys
project = 'SAGE Engine'
author = 'SAGE Team'
extensions = [
    'myst_parser',
    'sphinxcontrib.mermaid',
]
myst_enable_extensions = ["colon_fence"]
source_suffix = {
    '.rst': 'restructuredtext',
    '.md': 'markdown',
}
html_theme = 'sphinx_material'
html_title = 'SAGE Engine Docs'
html_static_path = ['_static']
# Material theme options (safe defaults)
html_theme_options = {
    'color_primary': 'blue',
    'color_accent': 'light-blue',
    'nav_title': 'SAGE Engine',
    'heroes': {'index': 'Чистое ядро. Плагины решают всё.'},
}
