#include "Material.h"
#include "Shader.h"
#include "../Core/Logger.h"

#include <algorithm>
#include <unordered_map>

namespace SAGE {

    namespace {
        std::unordered_map<std::string, Ref<Material>> s_Materials;
        std::unordered_map<MaterialId, Ref<Material>> s_MaterialsById;
        Ref<Material> s_DefaultMaterial;
        MaterialId s_NextMaterialId = 1;
        bool s_Initialized = false;

        void EnsureInitialized() {
            if (!s_Initialized) {
                s_Materials.clear();
                s_MaterialsById.clear();
                s_DefaultMaterial.reset();
                s_NextMaterialId = 1;
                s_Initialized = true;
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
        if (!shader) {
            SAGE_ERROR("Cannot create material '{0}' without shader", name);
            return nullptr;
        }
        auto mat = Ref<Material>(new Material(name, shader));
        return mat;
    }

    void Material::SetShader(const Ref<Shader>& shader) {
        if (!shader) {
            SAGE_WARNING("Material '{0}' cannot set null shader", m_Name);
            return;
        }
        m_Shader = shader;
    }

    void Material::SetPulse(float amplitude, float frequency) {
        m_Properties.pulseAmplitude = std::max(0.0f, amplitude);
        m_Properties.pulseFrequency = std::max(0.0f, frequency);
    }

    void MaterialLibrary::Init() {
        EnsureInitialized();
    }

    void MaterialLibrary::Shutdown() {
        s_Materials.clear();
        s_MaterialsById.clear();
        s_DefaultMaterial.reset();
        s_NextMaterialId = 1;
        s_Initialized = false;
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

        if (material->m_Id == 0) {
            material->m_Id = s_NextMaterialId++;
        }

        s_Materials[material->GetName()] = material;
        s_MaterialsById[material->GetId()] = material;
        if (!s_DefaultMaterial) {
            s_DefaultMaterial = material;
        }

        SAGE_INFO("Material '{0}' registered (id={1})", material->GetName(), material->GetId());
        return material;
    }

    Ref<Material> MaterialLibrary::Get(const std::string& name) {
        EnsureInitialized();
        auto it = s_Materials.find(name);
        if (it == s_Materials.end()) {
            SAGE_WARNING("Material '{0}' not found", name);
            return nullptr;
        }
        return it->second;
    }

    Ref<Material> MaterialLibrary::Get(MaterialId id) {
        EnsureInitialized();
        if (id == 0) {
            return s_DefaultMaterial;
        }

        auto it = s_MaterialsById.find(id);
        if (it == s_MaterialsById.end()) {
            SAGE_WARNING("Material with id {0} not found", id);
            return s_DefaultMaterial;
        }
        return it->second;
    }

    Ref<Material> MaterialLibrary::GetDefault() {
        EnsureInitialized();
        return s_DefaultMaterial;
    }

    MaterialId MaterialLibrary::GetDefaultId() {
        EnsureInitialized();
        return s_DefaultMaterial ? s_DefaultMaterial->GetId() : 0;
    }

    bool MaterialLibrary::Exists(const std::string& name) {
        EnsureInitialized();
        return s_Materials.find(name) != s_Materials.end();
    }

    void MaterialLibrary::Remove(const std::string& name) {
        EnsureInitialized();
        auto it = s_Materials.find(name);
        if (it == s_Materials.end()) {
            return;
        }
        bool removingDefault = (s_DefaultMaterial && s_DefaultMaterial->GetName() == name);
        MaterialId id = it->second ? it->second->GetId() : 0;
        s_Materials.erase(it);
        if (id != 0) {
            s_MaterialsById.erase(id);
        }
        if (removingDefault) {
            if (!s_Materials.empty()) {
                s_DefaultMaterial = s_Materials.begin()->second;
            }
            else {
                s_DefaultMaterial.reset();
            }
        }
    }

    void MaterialLibrary::Clear() {
        EnsureInitialized();
        s_Materials.clear();
        s_MaterialsById.clear();
        s_DefaultMaterial.reset();
        s_NextMaterialId = 1;
    }

} // namespace SAGE
