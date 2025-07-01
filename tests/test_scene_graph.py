import unittest
from engine.core.scene_graph import SceneGraph


class TestSceneGraph(unittest.TestCase):
    def test_add_and_connect(self):
        graph = SceneGraph()
        graph.add_scene('A', 'a.sage')
        graph.add_scene('B', 'b.sage', screenshot='shot.png')
        graph.connect('A', 'B')
        self.assertEqual(graph.start, 'A')
        self.assertEqual(graph.nodes['A'].next_nodes, ['B'])

    def test_serialise_roundtrip(self):
        graph = SceneGraph()
        graph.add_scene('A', 'a.sage')
        graph.add_scene('B', 'b.sage', screenshot='shot.png')
        graph.connect('A', 'B')
        data = graph.to_dict()
        new_graph = SceneGraph.from_dict(data)
        self.assertEqual(new_graph.nodes['A'].next_nodes, ['B'])
        self.assertEqual(new_graph.start, 'A')
        self.assertEqual(new_graph.nodes['B'].screenshot, 'shot.png')


if __name__ == '__main__':
    unittest.main()
