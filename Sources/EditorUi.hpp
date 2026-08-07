#pragma once
#include "CameraController.hpp"
#include "imgui.h"
#include <Engine.hpp>
#include <filesystem>

struct FileDialogOpenFileConfig
{
};

struct FileDialogOpenDirectoryConfig
{
};

struct WidgetData
{
    bool opened = false;
    std::filesystem::path currentDirectory;
    std::filesystem::path selectedPath;
};

class FileDialog
{
public:
    bool OpenFile(std::string_view label, std::string_view rootDirectory, std::string &file, bool *opened = nullptr, const FileDialogOpenFileConfig &config = {});
    bool OpenDirectory(std::string_view label, std::string_view rootDirectory, std::string &directory, bool *opened = nullptr, const FileDialogOpenDirectoryConfig &config = {});

private:
    std::unordered_map<std::string, WidgetData> mData;
};

class EditorUI
{
public:
    void Initialize(const Window &window, const Surface &surface);

    void SetScene(Scene &scene);
    void OnRender(Camera &camera, CameraController &controller);
    void Terminate();
    void AddImages(const Image &image);

    void Present(const Surface &surface);

private:
    void MainMenuBar();
    // Panels
    void EntityPanel();
    void PropertyPanel();
    void ValuePanel(Camera &camera);
    void GameViewPanel(Camera &camera, CameraController &controller);
    void StyleEditorPanel();

    void ImageImporterPanel();
    void ModelImporterPanel();

    // Component Editor
    void TransformController();
    void MeshRendererController();
    void LightController();
    void EntityMetadataController();
    void TextComponentController();

    void SetImageForViewer(VkImageView view);
    void SetColor(const ImGuiStyle &style);
    void ShowCursor();
    void DisableCursor();

    void Button(std::string_view label, const std::function<void()> &onClick);
    void MenuItem(std::string_view label, const std::function<void()> &callback);

    glm::vec3 DragFloat3(std::string_view name, const glm::vec3 &initialValue, float speed = 1.f);
    void DragFloat3(std::string_view name, glm::vec3 &value, float speed = 1.f);
    float DragFloat(std::string_view name, float initialValue, float speed = 1.f);
    glm::vec3 ColorEdit3(std::string_view name, const glm::vec3 &initialValue);

    void TextureSelector(std::string_view label, std::string& textureId);
    void FontSelector(std::string_view label, std::string &fontId);

private:
    ImGuiStyle mEditingStyle;
    VkDescriptorSet mGameViewTexture;
    VkDescriptorSet mImageViewTexture;
    std::string mEntityName;
    Scene *mScene;
    Entity mSelectedEntity;

    bool mDisableCursor = false;

    FileDialog mFileDialog;

    std::vector<std::pair<VkImageView, glm::uvec2>> mViewImages;
    int mImageIndex = 0;

    bool mEnableImageImporter = false;
    bool mEnableModelImporter = false;
};
