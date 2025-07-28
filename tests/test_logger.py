from sage_engine.logger import logger, DEBUG, INFO


def test_logger_levels(capsys):
    logger.set_level('INFO')
    logger.debug('hidden')
    captured = capsys.readouterr()
    assert 'hidden' not in captured.out
    logger.info('shown', tag='test')
    captured = capsys.readouterr()
    assert 'shown' in captured.out

