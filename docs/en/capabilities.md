# Capability Flags

Capabilities describe optional engine features such as advanced rendering effects. The file `caps.toml` lists the known flags:

```
[features]
volumetric-fx = 0
skeletal-mesh = 1
rollback-netcode = 2
```

Scenes may specify required capabilities under `metadata.caps`. During loading the engine checks these flags and issues a warning if the runtime does not support them. ChunkΔ headers also encode capability bits which are validated when decoded.

