import unittest
import tempfile
import os
from engine.utils import load_json

class TestLoadJson(unittest.TestCase):
    def test_load_with_trailing_commas_and_comments(self):
        content = '{"a":1, // comment\n "b":2,}\n'
        with tempfile.NamedTemporaryFile('w+', delete=False) as tf:
            tf.write(content)
            path = tf.name
        try:
            data = load_json(path)
        finally:
            os.unlink(path)
        self.assertEqual(data, {"a":1, "b":2})

    def test_empty_file_returns_empty_dict(self):
        with tempfile.NamedTemporaryFile("w", delete=False) as tf:
            path = tf.name
        try:
            data = load_json(path)
        finally:
            os.unlink(path)
        self.assertEqual(data, {})

if __name__ == '__main__':
    unittest.main()
