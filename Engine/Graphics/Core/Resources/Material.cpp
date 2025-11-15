#include "Material.h"
#include "Shader.h"
#include "Texture.h"
#include "Core/Logger.h"

#include <algorithm>
#include <memory>
#include <unordered_map>

namespace SAGE {

    namespace {
        struct MaterialLibraryState {
            std::unordered_map<std::string, Ref<Material>> materials;
            std::unordered_map<MaterialId, Ref<Material>> materialsById;
            Ref<Material> defaultMaterial;
            Ref<class Font> defaultFont;
            MaterialId nextMaterialId = 1;
            bool initialized = false;
        };

        std::unique_ptr<MaterialLibraryState> s_State;

        MaterialLibraryState& State() {
            if (!s_State) {
                s_State = std::make_unique<MaterialLibraryState>();
            }
            return *s_State;
        }

        bool HasState() {
            return static_cast<bool>(s_State);
        }

        void EnsureInitialized() {
            auto& state = State();
            if (!state.initialized) {
                state.materials.clear();
                state.materialsById.clear();
                state.defaultMaterial.reset();
                state.defaultFont.reset();
                state.nextMaterialId = 1;
                state.initialized = true;
            }
        }
    }

    Material::Material(std::string name, Ref<Shader> shader)
        : m_Name(std::move(name)), m_Shader(std::move(shader)) {
        m_Properties.tint = Color::White();
        m_Properties.pulseAmplitude = 0.0f;
        m_Properties.pulseFrequency = 0.0f;
        m_Properties.blend = BlendMode::Alpha;
    }

    Ref<Material> Material::Create(const std::string& name, const Ref<Shader>& shader) {
        EnsureInitialized();
        if (!shader || !shader->IsValid()) {
            SAGE_ERROR("Cannot create material '{0}' without a valid shader", name);
            return nullptr;
        }
        auto mat = Ref<Material>(new Material(name, shader));
        return mat;
    }

    void Material::SetShader(const Ref<Shader>& shader) {
        if (!shader || !shader->IsValid()) {
            SAGE_WARNING("Material '{0}' cannot set an invalid shader", m_Name);
            return;
        }
        m_Shader = shader;
    }

    void Material::SetPulse(float amplitude, float frequency) {
        m_Properties.pulseAmplitude = std::max(0.0f, amplitude);
        m_Properties.pulseFrequency = std::max(0.0f, frequency);
    }

    void Material::SetFloat(const std::string& name, float v) {
        m_Parameters[name] = MaterialValue{v, MaterialValueType::Float};
    }

    void Material::SetInt(const std::string& name, int v) {
        m_Parameters[name] = MaterialValue{v, MaterialValueType::Int};
    }

    void Material::SetColor(const std::string& name, const Color& c) {
        m_Parameters[name] = MaterialValue{c, MaterialValueType::Color};
    }

    void Material::SetVector3(const std::string& name, const Vector3& v) {
        m_Parameters[name] = MaterialValue{v, MaterialValueType::Vector3};
    }

    void Material::SetMatrix4(const std::string& name, const Matrix4& m) {
        m_Parameters[name] = MaterialValue{m, MaterialValueType::Matrix4};
    }

    void Material::SetTexture(const std::string& name, const Ref<Texture>& tex) {
        m_Parameters[name] = MaterialValue{tex, MaterialValueType::Texture};
    }

    bool Material::HasParameter(const std::string& name) const {
        return m_Parameters.find(name) != m_Parameters.end();
    }

    void Material::RemoveParameter(const std::string& name) {
        m_Parameters.erase(name);
    }

    void Material::ClearParameters() {
        m_Parameters.clear();
    }

    void Material::Apply() {
        if (!m_Shader || !m_Shader->IsValid()) {
            SAGE_WARNING("Material '{0}' Apply skipped: invalid shader", m_Name);
            return;
        }
        // Ensure shader is bound
        m_Shader->Bind();
        // Always push core properties if uniforms exist
        m_Shader->SetFloat4("u_Tint", m_Properties.tint);
        if (m_Properties.pulseAmplitude > 0.0f && m_Properties.pulseFrequency > 0.0f) {
            m_Shader->SetFloat("u_PulseAmplitude", m_Properties.pulseAmplitude);
            m_Shader->SetFloat("u_PulseFrequency", m_Properties.pulseFrequency);
        }

        int nextTextureSlot = 0; // simplistic allocation
        for (auto& [name, value] : m_Parameters) {
            switch (value.type) {
            case MaterialValueType::Float:
                m_Shader->SetFloat(name, std::get<float>(value.data));
                break;
            case MaterialValueType::Int:
                m_Shader->SetInt(name, std::get<int>(value.data));
                break;
            case MaterialValueType::Color: {
                const Color& c = std::get<Color>(value.data);
                m_Shader->SetFloat4(name, c);
                break; }
            case MaterialValueType::Vector3: {
                const Vector3& v = std::get<Vector3>(value.data);
                m_Shader->SetFloat3(name, v);
                break; }
            case MaterialValueType::Matrix4: {
                const Matrix4& m = std::get<Matrix4>(value.data);
                m_Shader->SetMat4(name, m);
                break; }
            case MaterialValueType::Texture: {
                Ref<Texture> tex = std::get<Ref<Texture>>(value.data);
                if (tex && tex->IsLoaded()) {
                    tex->Bind(static_cast<unsigned int>(nextTextureSlot));
                    m_Shader->SetInt(name, nextTextureSlot);
                    ++nextTextureSlot;
                }
                break; }
            }
        }
    }

    void MaterialLibrary::Init() {
        EnsureInitialized();
    }

    void MaterialLibrary::Shutdown() {
        if (!HasState()) {
            return;
        }

        s_State->materials.clear();
        s_State->materialsById.clear();
        s_State->defaultMaterial.reset();
        s_State->defaultFont.reset();
        s_State->nextMaterialId = 1;
        s_State->initialized = false;
        s_State.reset();
    }

    Ref<Material> MaterialLibrary::CreateMaterial(const std::string& name, const Ref<Shader>& shader) {
        EnsureInitialized();
        auto material = Material::Create(name, shader);
        return RegisterMaterial(material);
    }

    Ref<Material> MaterialLibrary::RegisterMaterial(const Ref<Material>& material) {
        EnsureInitialized();
        if (!material) {
            return nullptr;
        }

        auto& state = State();

        if (auto existing = state.materials.find(material->GetName()); existing != state.materials.end()) {
            if (existing->second != material) {
                SAGE_WARNING("Material '{0}' already registered; keeping existing instance", material->GetName());
            }
            return existing->second;
        }

        if (!material->GetShader() || !material->GetShader()->IsValid()) {
            SAGE_WARNING("Material '{0}' has no valid shader and will not be registered", material->GetName());
            return nullptr;
        }

        if (!state.defaultMaterial) {
            material->m_Id = 0;
        } else if (material->m_Id == 0) {
            material->m_Id = state.nextMaterialId++;
        }

        if (material->m_Id != 0) {
            auto existingById = state.materialsById.find(material->m_Id);
            if (existingById != state.materialsById.end() && existingById->second != material) {
                SAGE_WARNING("Material id {0} already registered to '{1}', reassigning id for '{2}'", material->m_Id, existingById->second ? existingById->second->GetName() : "<null>", material->GetName());
                material->m_Id = state.nextMaterialId++;
                while (material->m_Id != 0 && state.materialsById.find(material->m_Id) != state.materialsById.end()) {
                    material->m_Id = state.nextMaterialId++;
                }
            }
        }

        state.materials[material->GetName()] = material;
        if (material->GetId() != 0) {
            state.materialsById[material->GetId()] = material;
        }
        if (!state.defaultMaterial) {
            state.defaultMaterial = material;
        }

        SAGE_INFO("Material '{0}' registered (id={1})", material->GetName(), material->GetId());
        return material;
    }

    Ref<Material> MaterialLibrary::Get(const std::string& name) {
        EnsureInitialized();
        auto& state = State();
        auto it = state.materials.find(name);
        if (it == state.materials.end()) {
            SAGE_WARNING("Material '{0}' not found", name);
            return nullptr;
        }
        return it->second;
    }

    Ref<Material> MaterialLibrary::Get(MaterialId id) {
        EnsureInitialized();
        auto& state = State();
        if (id == 0) {
            return state.defaultMaterial;
        }

        auto it = state.materialsById.find(id);
        if (it == state.materialsById.end()) {
            SAGE_WARNING("Material with id {0} not found", id);
            return state.defaultMaterial;
        }
        return it->second;
    }

    Ref<Material> MaterialLibrary::GetDefault() {
        EnsureInitialized();
        return State().defaultMaterial;
    }

    MaterialId MaterialLibrary::GetDefaultId() {
        EnsureInitialized();
        auto& state = State();
        return state.defaultMaterial ? state.defaultMaterial->GetId() : 0;
    }

    void MaterialLibrary::ReplaceShader(const Ref<Shader>& oldShader, const Ref<Shader>& newShader) {
        EnsureInitialized();
        if (!oldShader) {
            return;
        }

        auto& state = State();
        for (auto& [name, material] : state.materials) {
            if (!material) {
                continue;
            }

            if (material->GetShader() == oldShader) {
                if (newShader && newShader->IsValid()) {
                    material->m_Shader = newShader;
                } else {
                    SAGE_WARNING("Material '{0}' retaining old shader; replacement is invalid", name);
                }
            }
        }
    }

    bool MaterialLibrary::Exists(const std::string& name) {
        EnsureInitialized();
        auto& state = State();
        return state.materials.find(name) != state.materials.end();
    }

    void MaterialLibrary::Remove(const std::string& name) {
        EnsureInitialized();
        auto& state = State();
        auto it = state.materials.find(name);
        if (it == state.materials.end()) {
            return;
        }
        bool removingDefault = (state.defaultMaterial && state.defaultMaterial->GetName() == name);
        MaterialId id = it->second ? it->second->GetId() : 0;
        state.materials.erase(it);
        if (id != 0) {
            state.materialsById.erase(id);
        }
        if (removingDefault) {
            if (!state.materials.empty()) {
                state.defaultMaterial = state.materials.begin()->second;
                if (state.defaultMaterial) {
                    if (state.defaultMaterial->m_Id != 0) {
                        state.materialsById.erase(state.defaultMaterial->m_Id);
                        state.defaultMaterial->m_Id = 0;
                    }
                }
            }
            else {
                state.defaultMaterial.reset();
            }
        }
    }

    void MaterialLibrary::Clear() {
        EnsureInitialized();
        auto& state = State();
        state.materials.clear();
        state.materialsById.clear();
        state.defaultMaterial.reset();
        state.defaultFont.reset();
        state.nextMaterialId = 1;
    }

    void MaterialLibrary::SetDefaultFont(const Ref<class Font>& font) {
        EnsureInitialized();
        State().defaultFont = font;
        if (font) {
            SAGE_INFO("Default font set");
        }
    }

    Ref<class Font> MaterialLibrary::GetDefaultFont() {
        EnsureInitialized();
        return State().defaultFont;
    }

} // namespace SAGE
