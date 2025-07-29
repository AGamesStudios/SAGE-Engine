"""Simple registry for updating multiple AnimationPlayers."""

from __future__ import annotations

from typing import List

from .player import AnimationPlayer

_players: List[AnimationPlayer] = []


def register(player: AnimationPlayer) -> None:
    _players.append(player)


def unregister(player: AnimationPlayer) -> None:
    if player in _players:
        _players.remove(player)


def update(dt: int) -> None:
    for p in list(_players):
        p.update(dt)
