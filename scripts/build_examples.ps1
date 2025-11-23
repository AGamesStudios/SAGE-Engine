# Build Examples Script
Write-Host 'Building Examples' -ForegroundColor Cyan
$examples = @('SimpleGameLoop', 'Box2DPhysicsDemo', 'ParticleSystemDemo', 'PluginDemo', 'MusicSystemExample')
$success = 0
foreach ($ex in $examples) {
    Write-Host "Building $ex..." -ForegroundColor Yellow
    cmake --build build --config Release --target $ex 2>&1 | Out-Null
    if ($LASTEXITCODE -eq 0) { $success++ }
}
Write-Host "Built $success/$($examples.Count) examples" -ForegroundColor Green