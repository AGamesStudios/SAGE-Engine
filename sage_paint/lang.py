LANGUAGES = {
    'English': {},
    'Русский': {
        'File': 'Файл',
        'New': 'Новый',
        'Open...': 'Открыть...',
        'Save': 'Сохранить',
        'Export PNG...': 'Экспорт PNG...',
        'Settings': 'Настройки',
        'Set Window Size': 'Размер окна',
        'Width': 'Ширина',
        'Window width': 'Ширина окна',
        'Height': 'Высота',
        'Window height': 'Высота окна',
        'Unsaved Changes': 'Несохраненные изменения',
        'Save changes before closing?': 'Сохранить изменения перед закрытием?',
        'White Background': 'Белый фон',
        'Dark Background': 'Темный фон',
        'Transparent': 'Прозрачный',
        'New Project': 'Новый проект',
        'Template': 'Шаблон',
        'Brush': 'Кисть',
        'Eraser': 'Ластик',
        'Fill': 'Заливка',
        'Select': 'Выделение',
        'Line': 'Линия',
        'Rect': 'Прямоугольник',
        'Color': 'Цвет',
        'Smooth': 'Сглаживание',
        'Circle': 'Круг',
        'Square': 'Квадрат',
        'Undo': 'Отменить',
        'Redo': 'Повторить',
        'Zoom +': 'Увеличить',
        'Zoom -': 'Уменьшить',
        'Experimental': 'Экспериментально',
        'EXPERIMENTAL': 'ЭКСПЕРИМЕНТАЛЬНО',
        'Language': 'Язык',
        'SAGE Paint is experimental. Features may change and stability is not guaranteed.':
            'SAGE Paint является экспериментальным. Функции могут измениться, стабильность не гарантируется.'
    }
}

import locale  # noqa: E402

def get_default_language() -> str:
    lang, _ = locale.getdefaultlocale()
    if lang and lang.lower().startswith('ru'):
        return 'Русский'
    return 'English'

DEFAULT_LANGUAGE = get_default_language()
