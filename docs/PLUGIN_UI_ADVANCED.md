
# Advanced Plugin UI

- **SSE realtime progress**: backend exposes `/events`, frontend subscribes via `EventSource` for live updates.
- **Update all**: `/api/update_all` installs or updates all plugins found in the registry.
- **Signature verification**: if `docs/keys/public_ed25519.pub` present and a registry entry has `"sig"` (base64), installer verifies ed25519 signature using PyNaCl.
- **Venv isolation**: pip installs use a local `.sage_env` virtual environment (no global Python pollution).
