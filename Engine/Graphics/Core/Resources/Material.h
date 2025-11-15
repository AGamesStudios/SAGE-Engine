#pragma once

#include "Graphics/Core/Types/Color.h"
#include "Memory/Ref.h"
#include "Math/Vector3.h"
#include "Math/Matrix4.h"
#include <cstdint>
#include <string>
#include <variant>
#include <unordered_map>

namespace SAGE {

    class Shader;
    class Texture;

    using MaterialId = std::uint32_t;

    enum class BlendMode {
        Alpha = 0,
        Additive,
        Multiply
    };

    struct MaterialProperties {
        Color tint = Color::White();
        float pulseAmplitude = 0.0f;
        float pulseFrequency = 0.0f;
        BlendMode blend = BlendMode::Alpha;
    };

    enum class MaterialValueType {
        Int,
        Float,
        Color,
        Vector3,
        Matrix4,
        Texture
    };

    struct MaterialValue {
        using Variant = std::variant<int, float, Color, Vector3, Matrix4, Ref<Texture>>;
        Variant data;
        MaterialValueType type;
    };

    class Material {
    public:
        static Ref<Material> Create(const std::string& name, const Ref<Shader>& shader);

        const std::string& GetName() const { return m_Name; }
        MaterialId GetId() const { return m_Id; }

        void SetShader(const Ref<Shader>& shader);
        const Ref<Shader>& GetShader() const { return m_Shader; }

    void SetTint(const Color& tint) { m_Properties.tint = tint; SetColor("u_Tint", tint); }
    const Color& GetTint() const { return m_Properties.tint; }

        /// Sets pulse parameters; amplitude and frequency are clamped to non-negative values.
        void SetPulse(float amplitude, float frequency);
        float GetPulseAmplitude() const { return m_Properties.pulseAmplitude; }
        float GetPulseFrequency() const { return m_Properties.pulseFrequency; }

        void SetBlendMode(BlendMode mode) { m_Properties.blend = mode; }
        BlendMode GetBlendMode() const { return m_Properties.blend; }

    const MaterialProperties& GetProperties() const { return m_Properties; }

    // Parameter management
    void SetFloat(const std::string& name, float v);
    void SetInt(const std::string& name, int v);
    void SetColor(const std::string& name, const Color& c);
    void SetVector3(const std::string& name, const Vector3& v);
    void SetMatrix4(const std::string& name, const Matrix4& m);
    void SetTexture(const std::string& name, const Ref<Texture>& tex);
    bool HasParameter(const std::string& name) const;
    void RemoveParameter(const std::string& name);
    void ClearParameters();
    void Apply(); // push all parameters to shader (expects shader bound)

    private:
        Material(std::string name, Ref<Shader> shader);

        std::string m_Name;
        Ref<Shader> m_Shader;
    MaterialProperties m_Properties{};
    std::unordered_map<std::string, MaterialValue> m_Parameters; // user-defined uniforms
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

    static void ReplaceShader(const Ref<Shader>& oldShader, const Ref<Shader>& newShader);

        // Font system support
        static void SetDefaultFont(const Ref<class Font>& font);
        static Ref<class Font> GetDefaultFont();

        static bool Exists(const std::string& name);
        static void Remove(const std::string& name);
        static void Clear();
    };

} // namespace SAGE
