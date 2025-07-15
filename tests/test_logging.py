import unittest
import logging
from io import StringIO

from sage_engine.utils.log import logger, init_logger
from sage_engine.core.scenes.scene import Scene
from sage_engine.core.camera import Camera
from sage_engine.logic.base import EventSystem, Event

class TestDebugLogging(unittest.TestCase):
    def test_event_logging(self):
        init_logger()
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

    def test_set_stream_console_only(self):
        from sage_engine.utils.log import set_stream
        original_files = [h.baseFilename for h in logger.handlers if isinstance(h, logging.FileHandler)]
        stream = StringIO()
        set_stream(stream)
        logger.info('redirect check')
        self.assertIn('redirect check', stream.getvalue())
        new_files = [h.baseFilename for h in logger.handlers if isinstance(h, logging.FileHandler)]
        self.assertEqual(original_files, new_files)

    def test_set_level_helper(self):
        from sage_engine.utils.log import set_level
        old = logger.level
        set_level('DEBUG')
        try:
            self.assertEqual(logger.level, logging.DEBUG)
        finally:
            set_level(old)

    def test_init_logger_idempotent(self):
        logger.handlers.clear()
        init_logger()
        count = len(logger.handlers)
        init_logger()
        self.assertEqual(count, len(logger.handlers))

    def test_faulthandler_enabled(self):
        import faulthandler
        logger.handlers.clear()
        if faulthandler.is_enabled():
            faulthandler.disable()
        init_logger()
        self.assertTrue(faulthandler.is_enabled())

if __name__ == '__main__':
    unittest.main()
