from sage_engine import Engine, GameObject, ResourceManager

res = ResourceManager()
tex = res.load_image('examples/Resources/logo.png')
obj = GameObject(texture=tex, x=320, y=240)

Engine(scene=[obj]).run()
