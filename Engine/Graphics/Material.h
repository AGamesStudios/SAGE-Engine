#pragma once

#include "../Graphics/Color.h"
#include "../Memory/Ref.h"
#include <cstdint>
#include <string>

namespace SAGE {

    class Shader;

    using MaterialId = std::uint32_t;

    enum class BlendMode {
        Alpha = 0,
        Additive
    };

    struct MaterialProperties {
        Color tint = Color::White();
        float pulseAmplitude = 0.0f;
        float pulseFrequency = 0.0f;
        BlendMode blend = BlendMode::Alpha;
    };

    class Material {
    public:
        static Ref<Material> Create(const std::string& name, const Ref<Shader>& shader);

        const std::string& GetName() const { return m_Name; }
        MaterialId GetId() const { return m_Id; }

        void SetShader(const Ref<Shader>& shader);
        const Ref<Shader>& GetShader() const { return m_Shader; }

        void SetTint(const Color& tint) { m_Properties.tint = tint; }
        const Color& GetTint() const { return m_Properties.tint; }

        void SetPulse(float amplitude, float frequency);
        float GetPulseAmplitude() const { return m_Properties.pulseAmplitude; }
        float GetPulseFrequency() const { return m_Properties.pulseFrequency; }

        void SetBlendMode(BlendMode mode) { m_Properties.blend = mode; }
        BlendMode GetBlendMode() const { return m_Properties.blend; }

        const MaterialProperties& GetProperties() const { return m_Properties; }

    private:
        Material(std::string name, Ref<Shader> shader);

        std::string m_Name;
        Ref<Shader> m_Shader;
        MaterialProperties m_Properties{};
        MaterialId m_Id = 0;

        friend class MaterialLibrary;
    };

    class MaterialLibrary {
    public:
        static void Init();
        static void Shutdown();

        static Ref<Material> CreateMaterial(const std::string& name, const Ref<Shader>& shader);
        static Ref<Material> RegisterMaterial(const Ref<Material>& material);

        static Ref<Material> Get(const std::string& name);
        static Ref<Material> Get(MaterialId id);
        static Ref<Material> GetDefault();
        static MaterialId GetDefaultId();

        static bool Exists(const std::string& name);
        static void Remove(const std::string& name);
        static void Clear();
    };

} // namespace SAGE
