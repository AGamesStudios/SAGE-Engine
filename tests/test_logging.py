import unittest
import logging
from io import StringIO

from engine.log import logger
from engine.core.scene import Scene, Camera
from engine.logic.base import EventSystem, Event

class TestDebugLogging(unittest.TestCase):
    def test_event_logging(self):
        stream = StringIO()
        handler = logging.StreamHandler(stream)
        logger.addHandler(handler)
        old_level = logger.level
        logger.setLevel(logging.DEBUG)
        try:
            scene = Scene()
            cam = Camera(0, 0, 640, 480)
            scene.add_object(cam)
            scene.remove_object(cam)

            es = EventSystem()
            es.add_event(Event([], [], name='e1'))
            es.enable_event('e1')
            es.disable_event('e1')
            es.reset_event('e1')
            es.remove_event('e1')
        finally:
            logger.removeHandler(handler)
            logger.setLevel(old_level)
        log_output = stream.getvalue()
        self.assertIn('Added object', log_output)
        self.assertIn('Removed object', log_output)
        self.assertIn('Added event', log_output)
        self.assertIn('Enabled event', log_output)
        self.assertIn('Disabled event', log_output)
        self.assertIn('Reset event', log_output)
        self.assertIn('Removed event', log_output)

if __name__ == '__main__':
    unittest.main()
