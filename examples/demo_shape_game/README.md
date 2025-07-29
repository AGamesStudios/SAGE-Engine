# Shape Shooter Demo

This example shows a minimal game built only from vector shapes using the
`Shape` role. No sprite or resource loading is required.
The game runs even if `demo.assets` is missing. Built-in shape placeholders are used.

## Controls
- **Left/Right** – move the player
- **Space** – shoot upward

The game spawns red triangle enemies every two seconds. Shoot them with yellow
circle bullets. Collision with an enemy ends the game and the score counts how
many enemies were destroyed.
