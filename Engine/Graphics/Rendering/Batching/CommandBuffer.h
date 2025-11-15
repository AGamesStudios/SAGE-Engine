#pragma once

#include "Graphics/Core/Types/RendererTypes.h"
#include "Graphics/Core/Resources/Material.h"

#include <cstddef>
#include <string>
#include <vector>

namespace SAGE {
	class Material;
	class Texture;
	class Font;
}

namespace SAGE::Graphics::Batching {

struct QuadCommand {
	Vector2 position;
	Vector2 size;
	Vector2 uvMin;
	Vector2 uvMax;
	Color color{ 1.0f, 1.0f, 1.0f, 1.0f };
	Ref<Texture> texture;
	Ref<Material> material;
	MaterialId materialId = 0; // supplemental id for lightweight comparisons
	QuadEffect effect{};
	float layer = 0.0f;
	float rotation = 0.0f;  // Rotation in degrees
	bool screenSpace = false;
	BlendMode blendMode = BlendMode::Alpha;
	DepthSettings depthState{};
};

struct TextCommand {
	std::string text;
	Float2 position{ 0.0f, 0.0f };
	Ref<Font> font;
	float scale = 1.0f;
	Color color = Color::White();
	bool screenSpace = false;
	Ref<Material> material;
	MaterialId materialId = 0;
	QuadEffect effect{};
	float layer = 0.0f;
	BlendMode blendMode = BlendMode::Alpha;
	DepthSettings depthState{};
};

struct BatchCommand {
	enum class Type {
		Quad,
		Text
	};

	Type type = Type::Quad;
	QuadCommand quad{};
	TextCommand text{};

	static BatchCommand CreateQuad(const QuadCommand& command) {
		BatchCommand result;
		result.type = Type::Quad;
		result.quad = command;
		return result;
	}

	static BatchCommand CreateText(const TextCommand& command) {
		BatchCommand result;
		result.type = Type::Text;
		result.text = command;
		return result;
	}
};

class CommandBuffer {
public:
	CommandBuffer() = default;

	explicit CommandBuffer(std::size_t maxQuads) {
		Initialize(maxQuads);
	}

	void Initialize(std::size_t maxQuads);
	void SetMaxQuads(std::size_t maxQuads);
	void Clear();
	[[nodiscard]] bool Empty() const;
	[[nodiscard]] std::size_t Size() const;
	[[nodiscard]] std::size_t Capacity() const;

	bool PushQuad(const QuadCommand& command);
	[[nodiscard]] const std::vector<QuadCommand>& GetQuads() const { return m_QuadCommands; }
	std::vector<QuadCommand>& GetQuadsMutable() { return m_QuadCommands; }

private:
	std::size_t m_MaxQuads = 0;
	std::vector<QuadCommand> m_QuadCommands;
};

} // namespace SAGE::Graphics::Batching
