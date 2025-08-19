from secrets import choice
import string

DEFAULT_SETS = {
    'upper': string.ascii_uppercase,
    'lower': string.ascii_lowercase,
    'digits': string.digits,
    'symbols': '!@#$%^&*()-_=+[]{};:,.<>?/',
}

# Ambiguous characters to avoid when avoid_ambiguous=True
AMBIGUOUS = set('0O1lI|`\\\'"{}[]()<>.,;:')

def build_charset(sets=('upper','lower','digits','symbols'), avoid_ambiguous=False, custom=None):
    chars = ''
    for s in sets:
        chars += DEFAULT_SETS.get(s, '')
    if custom:
        chars += ''.join(set(custom))
    if avoid_ambiguous:
        chars = ''.join([c for c in chars if c not in AMBIGUOUS])
    # ensure unique
    chars = ''.join(sorted(set(chars)))
    if not chars:
        raise ValueError('Empty charset')
    return chars

def generate(length=16, sets=('upper','lower','digits','symbols'),
            ensure_each=True, avoid_ambiguous=True, custom=None):
    charset = build_charset(sets, avoid_ambiguous, custom)
    pwd = []
    if ensure_each:
        # гарантируем хотя бы по одному символу из каждого выбранного множества, если он есть в итоговом наборе
        for s in sets:
            base = DEFAULT_SETS.get(s, '')
            pick = next((c for c in base if c in charset), None)
            if pick:
                pwd.append(pick)
    while len(pwd) < length:
        pwd.append(choice(charset))
    from random import shuffle
    shuffle(pwd)
    return ''.join(pwd[:length])
