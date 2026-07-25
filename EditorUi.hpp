#pragma once
#include "Assets/FontManager.hpp"
#include "CameraController.hpp"
#include "EntityComponentSystem/EntityComponentSystem.hpp"
#include "Renderer/Renderer.hpp"
#include "imgui.h"

class EditorUI
{
public:
    void Initialize(const ImageDeprecated &sceneImage, const Window &window, const Surface &surface);

    void SetScene(Scene &scene);
    void OnRender(Camera &camera, CameraController &controller);
    void Terminate();
    void AddImages(const Image &image);

    void BezierView();

private:
    void MainMenuBar();
    // Panels
    void EntityPanel();
    void PropertyPanel();
    void ValuePanel(Camera &camera);
    void GameView(Camera &camera, CameraController &controller);
    void StyleEditor();

    void ImageImporter();

    // Component Editor
    void TransformController();
    void MeshRendererController();
    void LightController();
    void EntityMetadataController();
    void TextComponentController();

    void SetImageForViewer(VkImageView view);
    void SetColor(const ImGuiStyle &style);
    void ShowCursor();
    void HideCursor();

    void Button(std::string_view label, const std::function<void()> &onClick, const std::function<void()> &onDoubleClick = []() {});
    void MenuItem(std::string_view label, const std::function<void()> &callback);

    glm::vec3 DragFloat3(std::string_view name, const glm::vec3 &initialValue, float speed = 1.f);
    void DragFloat3(std::string_view name, glm::vec3 &value, float speed = 1.f);
    float DragFloat(std::string_view name, float initialValue, float speed = 1.f);
    glm::vec3 ColorEdit3(std::string_view name, const glm::vec3 &initialValue);

    void TextureSelector(std::string_view label, TextureID &textureId);
    void FontSelector(std::string_view label, FontID &fontId);

private:
    ImGuiStyle mEditingStyle;
    Surface mSurface;
    VkDescriptorSet mGameViewTexture;
    VkDescriptorSet mImageViewTexture;
    std::string mEntityName;
    Scene *mScene;
    Entity mSelectedEntity;

    std::vector<std::pair<VkImageView, glm::uvec2>> mViewImages;
    int mImageIndex = 0;

    bool mEnableImageImporter = false;
};