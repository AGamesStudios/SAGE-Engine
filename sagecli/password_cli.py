#!/usr/bin/env python3
import argparse
from plugins.passwordgen.generator import generate

def main():
    ap = argparse.ArgumentParser('sage-password')
    ap.add_argument('--length', type=int, default=16)
    ap.add_argument('--sets', type=str, default='upper,lower,digits,symbols', help='comma separated')
    # флаги-«выключатели» через dest, чтобы не путаться с дефисами
    ap.add_argument('--no-ensure-each', dest='ensure_each', action='store_false',
                    help='do not force at least one char from each selected set')
    ap.add_argument('--allow-ambiguous', dest='avoid_ambiguous', action='store_false',
                    help='allow ambiguous chars like 0/O/1/l/I')
    ap.set_defaults(ensure_each=True, avoid_ambiguous=True)
    ap.add_argument('--custom', type=str, default='')
    args = ap.parse_args()

    pwd = generate(
        length=args.length,
        sets=tuple(s for s in args.sets.split(',') if s),
        ensure_each=args.ensure_each,
        avoid_ambiguous=args.avoid_ambiguous,
        custom=args.custom or None,
    )
    print(pwd)

if __name__ == '__main__':
    main()
