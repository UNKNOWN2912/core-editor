#include "EditorUi.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "misc/cpp/imgui_stdlib.h"
#include <filesystem>
#include <format>

const char *const imguiColorName[] =
    {
        "ImGuiCol_Text",
        "ImGuiCol_TextDisabled",
        "ImGuiCol_WindowBg",
        "ImGuiCol_ChildBg",
        "ImGuiCol_PopupBg",
        "ImGuiCol_Border",
        "ImGuiCol_BorderShadow",
        "ImGuiCol_FrameBg",
        "ImGuiCol_FrameBgHovered",
        "ImGuiCol_FrameBgActive",
        "ImGuiCol_TitleBg",
        "ImGuiCol_TitleBgActive",
        "ImGuiCol_TitleBgCollapsed",
        "ImGuiCol_MenuBarBg",
        "ImGuiCol_ScrollbarBg",
        "ImGuiCol_ScrollbarGrab",
        "ImGuiCol_ScrollbarGrabHovered",
        "ImGuiCol_ScrollbarGrabActive",
        "ImGuiCol_CheckMark",
        "ImGuiCol_CheckboxSelectedBg",
        "ImGuiCol_SliderGrab",
        "ImGuiCol_SliderGrabActive",
        "ImGuiCol_Button",
        "ImGuiCol_ButtonHovered",
        "ImGuiCol_ButtonActive",
        "ImGuiCol_Header",
        "ImGuiCol_HeaderHovered",
        "ImGuiCol_HeaderActive",
        "ImGuiCol_Separator",
        "ImGuiCol_SeparatorHovered",
        "ImGuiCol_SeparatorActive",
        "ImGuiCol_ResizeGrip",
        "ImGuiCol_ResizeGripHovered",
        "ImGuiCol_ResizeGripActive",
        "ImGuiCol_InputTextCursor",
        "ImGuiCol_TabHovered",
        "ImGuiCol_Tab",
        "ImGuiCol_TabSelected",
        "ImGuiCol_TabSelectedOverline",
        "ImGuiCol_TabDimmed",
        "ImGuiCol_TabDimmedSelected",
        "ImGuiCol_TabDimmedSelectedOverline",
        "ImGuiCol_DockingPreview",
        "ImGuiCol_DockingEmptyBg",
        "ImGuiCol_PlotLines",
        "ImGuiCol_PlotLinesHovered",
        "ImGuiCol_PlotHistogram",
        "ImGuiCol_PlotHistogramHovered",
        "ImGuiCol_TableHeaderBg",
        "ImGuiCol_TableBorderStrong",
        "ImGuiCol_TableBorderLight",
        "ImGuiCol_TableRowBg",
        "ImGuiCol_TableRowBgAlt",
        "ImGuiCol_TextLink",
        "ImGuiCol_TextSelectedBg",
        "ImGuiCol_TreeLines",
        "ImGuiCol_DragDropTarget",
        "ImGuiCol_DragDropTargetBg",
        "ImGuiCol_UnsavedMarker",
        "ImGuiCol_NavCursor",
        "ImGuiCol_NavWindowingHighlight",
        "ImGuiCol_NavWindowingDimBg",
        "ImGuiCol_ModalWindowDimBg",
};

bool FileDialog::OpenFile(std::string_view label, std::string_view rootDirectory, std::string &file, bool *opened, const FileDialogOpenFileConfig &config)
{
    WidgetData &data = mData[label.data()];
    if (data.opened)
    {
    }
    else
    {
        if (rootDirectory == "." || rootDirectory == "" || rootDirectory == "./")
        {
            data.currentDirectory = std::filesystem::current_path();
        }
        else
        {
            data.currentDirectory = std::filesystem::absolute(rootDirectory);
        }
    }

    ImGui::Begin(label.data(), opened);
    ImGui::Text("dir: %s", data.currentDirectory.c_str());

    if (ImGui::Selectable(".."))
    {
        data.currentDirectory = data.currentDirectory.parent_path();
    }

    std::string typeString = "F: ";

    std::vector<std::filesystem::directory_entry> entries;

    for (const auto &entry : std::filesystem::directory_iterator(data.currentDirectory))
    {
        if (!entry.is_directory())
            continue;

        entries.push_back(entry);
    }
    for (const auto &entry : std::filesystem::directory_iterator(data.currentDirectory))
    {
        if (entry.is_directory())
            continue;

        entries.push_back(entry);
    }

    for (const auto &entry : entries)
    {
        typeString = "F: ";
        if (entry.is_directory())
        {
            typeString = "D: ";
        }

        std::string displayPath = typeString + std::filesystem::relative(entry.path(), data.currentDirectory).string();
        if (ImGui::Selectable(displayPath.c_str(), data.selectedPath == entry.path(), ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (entry.is_directory())
                {
                    data.currentDirectory = std::filesystem::absolute(entry.path());
                }
                else
                {
                    data.opened = false;
                    file = std::filesystem::absolute(entry.path());
                    ImGui::End();
                    return true;
                }
            }
            else
            {
                data.selectedPath = entry.path();
            }
        }
    }

    ImGui::End();

    data.opened = true;
    return false;
}

bool FileDialog::OpenDirectory(std::string_view label, std::string_view rootDirectory, std::string &directory, bool *opened, const FileDialogOpenDirectoryConfig &config)
{
    ImGui::Begin(label.data(), opened);

    ImGui::End();
}

void EditorUI::Initialize(const ImageDeprecated &sceneImage, const Window &window, const Surface &surface)
{
    mSurface = surface;
    VkDescriptorPool descriptorPool = CreateDescriptorPool({
                                                               {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE},
                                                               {VK_DESCRIPTOR_TYPE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE},
                                                           },
                                                           IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE + IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/GoogleSans-Regular.ttf", 18);

    {
        FILE *fp = fopen("style.bin", "rb");
        fread(mEditingStyle.Colors, sizeof(mEditingStyle.Colors), 1, fp);
        fclose(fp);
    }

    ImGui_ImplGlfw_InitForVulkan(window.GetNativeWindow(), true);
    ImGui_ImplVulkan_InitInfo initInfo =
        {
            .Instance = GraphicsContext::GetInstance(),
            .PhysicalDevice = GraphicsContext::GetPhysicalDevice(),
            .Device = GraphicsContext::GetDevice(),
            .QueueFamily = GraphicsContext::GetQueueIndices().graphics,
            .Queue = GraphicsContext::GetQueues().graphics,
            .DescriptorPool = descriptorPool,
            .MinImageCount = mSurface.swapchain.GetImageCount(),
            .ImageCount = mSurface.swapchain.GetImageCount(),
            .PipelineInfoMain =
                {
                    .RenderPass = Renderer::GetPresentRenderPass().GetHandle(),
                    .Subpass = 0,
                    .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
                },
        };

    ImGui_ImplVulkan_Init(&initInfo);

    ImageView view;
    view.CreateImageView(sceneImage, ViewType::TwoDimensional, ImageAspect::Color, 0, 1, 0, 1, {ComponentSwizzle::R, ComponentSwizzle::G, ComponentSwizzle::B, ComponentSwizzle::One});

    mGameViewTexture = ImGui_ImplVulkan_AddTexture(view.GetHandle(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    mImageViewTexture = ImGui_ImplVulkan_AddTexture(sceneImage.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    mViewImages.emplace_back(sceneImage.view, sceneImage.size);
}

glm::vec3 EditorUI::DragFloat3(std::string_view name, const glm::vec3 &initialValue, float speed)
{
    glm::vec3 v = initialValue;
    ImGui::DragFloat3(name.data(), &v.x, speed);
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::IsItemActive())
    {
        HideCursor();
    }
    return v;
}

void EditorUI::DragFloat3(std::string_view name, glm::vec3 &value, float speed)
{
    ImGui::DragFloat3(name.data(), &value.x, speed);
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::IsItemActive())
    {
        HideCursor();
    }
}

float EditorUI::DragFloat(std::string_view name, float initialValue, float speed)
{
    float v = initialValue;
    ImGui::DragFloat(name.data(), &v, speed);
    if (ImGui::IsItemActive())
    {
        HideCursor();
    }
    return v;
}

glm::vec3 EditorUI::ColorEdit3(std::string_view name, const glm::vec3 &initialValue)
{
    glm::vec3 v = initialValue;
    ImGui::ColorEdit3(name.data(), &v.x);
    return v;
}

void EditorUI::TextureSelector(std::string_view label, TextureID &textureId)
{
    std::string textureName = "None";
    if (TextureManager::HasTexture(textureId))
    {
        textureName = TextureManager::GetTexture(textureId)->GetName();
    }

    if (!ImGui::BeginCombo(label.data(), textureName.c_str()))
    {
        return;
    }

    for (const auto &[id, texture] : TextureManager::GetMap())
    {
        ImGui::PushID((uint64_t)id);
        if (ImGui::Selectable(texture->GetName().c_str(), id == textureId))
        {
            textureId = id;
        }
        ImGui::PopID();
    }
    ImGui::EndCombo();
}

void EditorUI::FontSelector(std::string_view label, FontID &fontId)
{
    std::string fontName = "None";
    if (FontManager::HasFont(fontId))
    {
        fontName = FontManager::GetFont(fontId).GetName();
    }

    if (!ImGui::BeginCombo(label.data(), fontName.c_str()))
    {
        return;
    }

    for (const auto &[id, font] : FontManager::GetMap())
    {
        if (ImGui::Selectable(font.GetName().c_str(), id == fontId))
        {
            fontId = id;
        }
    }

    ImGui::EndCombo();
}

void EditorUI::SetScene(Scene &scene)
{
    mScene = &scene;
}
void EditorUI::OnRender(Camera &camera, CameraController &controller)
{
    ShowCursor();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    MainMenuBar();
    StyleEditorPanel();
    ValuePanel(camera);
    GameViewPanel(camera, controller);
    PropertyPanel();
    EntityPanel();
    ModelImporterPanel();
    ImageImporterPanel();

    ImGui::Render();
}
void EditorUI::Terminate()
{
}
void EditorUI::EntityPanel()
{
    ImGui::Begin("Entities");

    if (ImGui::Button("Create entity"))
    {
        ImGui::OpenPopup("CreateEntity");
    }

    if (ImGui::BeginPopup("CreateEntity"))
    {
        ImGui::InputText("Name", &mEntityName);
        if (ImGui::Button("Create"))
        {
            Entity entity = mScene->CreateEntity(mEntityName);
            entity.AddComponent<Transform>();
            entity.GetComponent<EntityMetadata>().enableSerializing = true;
        }
        ImGui::EndPopup();
    }

    for (const auto &[entity, component] : mScene->GetEntities<EntityMetadata>())
    {
        if (!component.enableSerializing)
        {
            continue;
        }

        ImGui::PushID((int)entity.GetId());
        if (ImGui::Selectable(component.name.c_str(), mSelectedEntity.GetId() == entity.GetId()))
        {
            mSelectedEntity = entity;
        }
        ImGui::PopID();
    }

    ImGui::End();
}
void EditorUI::PropertyPanel()
{
    ImGui::Begin("Properties");

    if (mSelectedEntity.IsValid())
    {
        if (ImGui::Button("Add component"))
        {
            ImGui::OpenPopup("AddComponent");
        }

        if (ImGui::BeginPopup("AddComponent"))
        {
            if (ImGui::Button("Mesh Renderer"))
            {
                mSelectedEntity.AddComponent<MeshRendererComponent>();
            }
            if (ImGui::Button("Light"))
            {
                mSelectedEntity.AddComponent<Light>();
            }
            if (ImGui::Button("Text"))
            {
                mSelectedEntity.AddComponent<TextComponent>();
            }
            ImGui::EndPopup();
        }

        Button("Delete", [&] {
            mSelectedEntity.GetComponent<EntityMetadata>().enableSerializing = false;
        });

        TransformController();
        MeshRendererController();
        LightController();
        EntityMetadataController();
        TextComponentController();
    }

    ImGui::End();
}

FileDialog dialog;

void EditorUI::ValuePanel(Camera &camera)
{
    ImGui::Begin("Value editor");

    int in = (int)Renderer::GetInputInt();
    ImGui::SliderInt("renderer mode", &in, 0, 4);
    Renderer::SetInputInt(in);

    ImGui::Text("Fps: %d", Application::GetInstance()->GetFps());

    ImGui::SeparatorText("Camera");
    camera.SetPosition(DragFloat3("Position", camera.GetPosition(), 0.01f));
    camera.SetFront(DragFloat3("Front", camera.GetFront(), 0.01f));

    ImGui::SeparatorText("Text Renderer");

    ImGui::SliderInt("Text Mode", &TextRenderer::GetPushConstant().mode, 0, 10);

    ImGui::End();
}

void EditorUI::ModelImporterPanel()
{
    if (!mEnableModelImporter)
        return;

    std::string filename;
    if (mFileDialog.OpenFile("Import Model", "", filename, &mEnableModelImporter))
    {
        ModelImporter importer;
        importer.Import(filename.c_str(), *mScene);
    }
}

ImVec2 glmVec2toImVec2(const glm::vec2 &value)
{
    return {value.x, value.y};
}

int count = 0;

glm::vec2 QuadraticBezierCurve(const glm::vec2 &p0, const glm::vec2 &p1, const glm::vec2 &p2, float t)
{
    glm::vec2 c = glm::mix(p0, p1, t);
    glm::vec2 d = glm::mix(p1, p2, t);

    glm::vec2 result = glm::mix(c, d, t);
    return result;
}

void EditorUI::GameViewPanel(Camera &camera, CameraController &controller)
{
    ImGui::Begin("Game view");
    ImVec2 size = ImGui::GetContentRegionAvail();
    float aspectRatio = size.x / size.y;
    ImGui::Image((ImTextureID)mGameViewTexture, size);

    camera.SetAspectRatio(aspectRatio);

    controller.EnableMouseControl(false);
    controller.EnableKeyboardControl(false);
    if (ImGui::IsItemHovered())
    {
        controller.EnableKeyboardControl(true);
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) || ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            HideCursor();
            controller.EnableMouseControl(true);
        }
    }
    ImGui::End();
}

void EditorUI::StyleEditorPanel()
{
    ImGui::Begin("Style Editor");
    if (ImGui::Button("Save"))
    {
        FILE *fp = fopen("style.bin", "wb");
        fwrite(mEditingStyle.Colors, sizeof(mEditingStyle.Colors), 1, fp);
        fclose(fp);
    }
    ImGui::SameLine();
    if (ImGui::Button("Load"))
    {
        FILE *fp = fopen("style.bin", "rb");
        fread(mEditingStyle.Colors, sizeof(mEditingStyle.Colors), 1, fp);
        fclose(fp);
    }

    for (int i = 0; i < ImGuiCol_COUNT; i++)
    {
        ImGui::ColorEdit4(imguiColorName[i], &mEditingStyle.Colors[(ImGuiCol_)i].x);
    }

    SetColor(mEditingStyle);

    ImGui::End();
}

void EditorUI::ImageImporterPanel()
{
    if (!mEnableImageImporter)
        return;

    std::string filename;
    if (mFileDialog.OpenFile("Import Image", "", filename, &mEnableImageImporter))
    {
        TextureManager::LoadTexture(filename);
    }
}

void EditorUI::TransformController()
{
    if (!mSelectedEntity.HasComponent<Transform>())
    {
        return;
    }

    ImGui::PushID("Transform");
    ImGui::SeparatorText("Transform");
    Transform &component = mSelectedEntity.GetComponent<Transform>();
    DragFloat3("position", component.position, 0.01f);
    DragFloat3("rotation", component.rotation, 0.01f);
    DragFloat3("scale", component.scale, 0.01f);
    ImGui::PopID();
}

void EditorUI::EntityMetadataController()
{
    if (!mSelectedEntity.HasComponent<EntityMetadata>())
    {
        return;
    }

    ImGui::SeparatorText("Entity metadata");
    ImGui::PushID("Entity metadata");

    EntityMetadata &component = mSelectedEntity.GetComponent<EntityMetadata>();
    ImGui::InputText("Name", &component.name);

    ImGui::PopID();
}

void EditorUI::TextComponentController()
{
    if (!mSelectedEntity.HasComponent<TextComponent>())
    {
        return;
    }

    ImGui::SeparatorText("Text");
    ImGui::PushID("Text");

    TextComponent &component = mSelectedEntity.GetComponent<TextComponent>();
    ImGui::InputText("Text", &component.text);
    component.spacing = DragFloat("Spacing", component.spacing, 0.01f);
    FontSelector("Font", component.fontId);
    ImGui::ColorEdit4("Forground", &component.forgroundColor.x);
    ImGui::ColorEdit4("Background", &component.backgroundColor.x);

    ImGui::PopID();
}

void EditorUI::MeshRendererController()
{
    if (!mSelectedEntity.HasComponent<MeshRendererComponent>())
    {
        return;
    }

    ImGui::PushID("MeshRendererComponent");
    MeshRendererComponent &component = mSelectedEntity.GetComponent<MeshRendererComponent>();
    ImGui::SeparatorText("Mesh Renderer");

    std::shared_ptr<Mesh> cMesh = MeshManager::GetMesh(component.mesh);
    Material cMaterial;
    if (MaterialManager::HasMaterial(component.material))
    {
        cMaterial = MaterialManager::GetMaterial(component.material);
    }

    if (ImGui::BeginCombo("Material", std::format("{}", MaterialManager::HasMaterial(component.material) ? cMaterial.name : "None").c_str()))
    {
        for (const auto &[id, material] : MaterialManager::GetMap())
        {
            ImGui::PushID((int)id);

            if (ImGui::Selectable(material.name.c_str(), id == component.material))
            {
                component.material = id;
            }

            ImGui::PopID();
        }

        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Mesh", std::format("{}", (cMesh != nullptr) ? cMesh->GetName() : "None").c_str()))
    {
        for (const auto &[id, mesh] : MeshManager::GetMap())
        {
            ImGui::PushID((int)id);

            if (ImGui::Selectable(mesh->GetName().c_str(), id == component.mesh))
            {
                component.mesh = id;
            }

            ImGui::PopID();
        }

        ImGui::EndCombo();
    }

    if (MaterialManager::HasMaterial(component.material))
    {
        Material &cMaterial = MaterialManager::GetMaterial(component.material);

        ImGui::SeparatorText("Material");

        ImGui::InputText("Name", &cMaterial.name);

        ImGui::SliderFloat("Roughness", &cMaterial.roughnessFactor, 0.1f, 1.f);
        ImGui::SliderFloat("Metallic", &cMaterial.metallicFactor, 0.01f, 1.f);
        ImGui::ColorEdit4("Color", &cMaterial.colorFactor.x);

        TextureSelector("Albedo Texture", cMaterial.albedoTexture);
        TextureSelector("Roughness Texture", cMaterial.roughnessTexture);
        TextureSelector("Metallic Texture", cMaterial.metallicTexture);

        const char *cullModeString[] =
            {
                "None",
                "Front",
                "Back",
            };

        if (ImGui::BeginCombo("Cull mode", cullModeString[(uint64_t)cMaterial.cullMode]))
        {
            for (int i = 0; i < 3; i++)
            {
                if (ImGui::Selectable(cullModeString[i], (uint64_t)cMaterial.cullMode == i))
                {
                    cMaterial.cullMode = (CullMode)i;
                }
            }
            ImGui::EndCombo();
        }
    }

    ImGui::PopID();
}
void EditorUI::LightController()
{
    if (!mSelectedEntity.HasComponent<Light>())
    {
        return;
    }

    ImGui::PushID("Light");
    ImGui::SeparatorText("Light");
    Light &light = mSelectedEntity.GetComponent<Light>();

    const char *lightTypeStrings[] =
        {
            "Directional",
            "Point",
            "Spot",
        };

    light.SetColor(ColorEdit3("color", light.GetColor()));
    light.SetIntensity(DragFloat("intensity", light.GetIntensity(), 0.1f));

    if (ImGui::BeginCombo("type", lightTypeStrings[(uint32_t)light.GetType()]))
    {
        for (int i = 0; i < 3; i++)
        {
            if (ImGui::Selectable(lightTypeStrings[i], (i == (uint32_t)light.GetType())))
            {
                light.SetType((LightType)i);
            }
        }
        ImGui::EndCombo();
    }
    ImGui::PopID();
}

void EditorUI::AddImages(const Image &image)
{
    mViewImages.emplace_back(image.GetImageView().GetHandle(), image.GetSize());
}

void EditorUI::MainMenuBar()
{
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit"))
    {
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View"))
    {
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Assets"))
    {
        MenuItem("Import Model", [&]() { mEnableModelImporter = true; });
        MenuItem("Import Image", [&]() { mEnableImageImporter = true; });
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void EditorUI::SetImageForViewer(VkImageView view)
{
    VkDescriptorImageInfo imageInfo = {};
    imageInfo.imageView = view;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write =
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = mImageViewTexture,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo = &imageInfo,
        };
    vkUpdateDescriptorSets(GraphicsContext::GetDevice(), 1, &write, 0, nullptr);
}

void EditorUI::SetColor(const ImGuiStyle &style)
{
    ImGui::StyleColorsDark();
    ImGuiStyle &actual = ImGui::GetStyle();
    actual = style;

    for (int i = 0; i < ImGuiCol_COUNT; i++)
    {
        ImVec4 &col = actual.Colors[i];

        col.x = (col.x <= 0.04045f) ? (col.x / 12.92f) : powf((col.x + 0.055f) / 1.055f, 2.4f);
        col.y = (col.y <= 0.04045f) ? (col.y / 12.92f) : powf((col.y + 0.055f) / 1.055f, 2.4f);
        col.z = (col.z <= 0.04045f) ? (col.z / 12.92f) : powf((col.z + 0.055f) / 1.055f, 2.4f);
    }
}

void EditorUI::ShowCursor()
{
    Application::GetInstance()->ShowCursor();
}
void EditorUI::HideCursor()
{
    Application::GetInstance()->HideCursor();
}

void EditorUI::Button(std::string_view label, const std::function<void()> &onClick)
{
    if (ImGui::Button(label.data(), ImVec2(0, 0)))
    {
        onClick();
    }
}

void EditorUI::MenuItem(std::string_view label, const std::function<void()> &callback)
{
    if (ImGui::MenuItem(label.data()))
    {
        callback();
    }
}
