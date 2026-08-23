#include "EditorUi.hpp"
#include "EntityComponentSystem/Component.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "misc/cpp/imgui_stdlib.h"
#include <cctype>
#include <filesystem>
#include <format>

const char *const imguiColorName[] =
    {
        "Text",
        "TextDisabled",
        "Window Background",
        "Child Background",
        "Popup Background",
        "Border",
        "BorderShadow",
        "Frame Background ",
        "Frame Background Hovered",
        "Frame Background Active",
        "Title Background ",
        "Title Background Active",
        "Title Background Collapsed",
        "MenuBar Background ",
        "Scrollbar Background ",
        "Scrollbar Grab ",
        "Scrollbar Grab Hovered",
        "Scrollbar Grab Active",
        "CheckMark",
        "CheckboxSelected Background ",
        "Slider Grab ",
        "Slider Grab Active",
        "Button",
        "Button Hovered",
        "Button Active",
        "Header",
        "Header Hovered",
        "Header Active",
        "Separator",
        "Separator Hovered",
        "Separator Active",
        "Resize Grip",
        "Resize Grip Hovered",
        "Resize Grip Active",
        "Input Text Cursor",
        "Tab Hovered",
        "Tab",
        "Tab Selected",
        "Tab Selected Overline",
        "Tab Dimmed",
        "Tab Dimmed Selected",
        "Tab Dimmed Selected Overline",
        "Docking Preview",
        "Docking Empty Background ",
        "Plot Lines",
        "Plot Lines Hovered",
        "Plot Histogram",
        "Plot Histogram Hovered",
        "Table Header Background ",
        "Table BorderStrong",
        "Table BorderLight",
        "Table Row Background ",
        "Table Row Background Alt",
        "Text Link",
        "Text Selected Background ",
        "Tree Lines",
        "Drag Drop Target",
        "Drag Drop Target Background ",
        "Unsaved Marker",
        "Nav Cursor",
        "Nav Windowing Highlight",
        "Nav Windowing Dim Background ",
        "Modal Window Dim Background ",
};

std::string ToLower(const std::string &str)
{
    std::string result;
    result.reserve(str.size());

    for (char ch : str)
    {
        result += std::tolower(ch);
    }

    return result;
}

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

    static std::string search;

    ImGui::Begin(label.data(), opened);
    ImGui::InputText("Search", &search);
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
        bool f = true;
        std::string pathString = entry.path().generic_string();
        std::string relativePathString = (std::filesystem::relative(entry.path(), data.currentDirectory).generic_string());
        std::string searchS = (search);

        for (int i = 0; i < searchS.size() && i < relativePathString.size(); i++)
        {
            if (std::tolower(relativePathString[i]) != std::tolower(searchS[i]))
            {
                f = false;
            }
        }

        if (f == false)
            continue;

        if (relativePathString[0] == '.' && !config.showHiddenFiles)
        {
            continue;
        }

        typeString = "F: ";
        if (entry.is_directory())
        {
            typeString = "D: ";
        }

        std::string displayPath = typeString + relativePathString;

        if (entry.is_directory())
        {
            displayPath += '/';
        }
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
                    file = std::filesystem::absolute(entry.path()).generic_string();
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

    return false;
}

void EditorUI::Initialize(const Window &window, const Surface &surface)
{
    ImageDeprecated sceneImage = Renderer::mSceneResolveAttachment;
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
            .Instance = GraphicsContext::GetCurrentContext().GetInstance(),
            .PhysicalDevice = GraphicsContext::GetCurrentContext().GetPhysicalDevice(),
            .Device = GraphicsContext::GetCurrentContext().GetDevice(),
            .QueueFamily = GraphicsContext::GetCurrentContext().GetQueueIndices().graphics,
            .Queue = GraphicsContext::GetCurrentContext().GetQueues().graphics,
            .DescriptorPool = descriptorPool,
            .MinImageCount = surface.swapchain.GetImageCount(),
            .ImageCount = surface.swapchain.GetImageCount(),
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
        DisableCursor();
    }
    return v;
}

void EditorUI::DragFloat3(std::string_view name, glm::vec3 &value, float speed)
{
    ImGui::DragFloat3(name.data(), &value.x, speed);
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && ImGui::IsItemActive())
    {
        DisableCursor();
    }
}

float EditorUI::DragFloat(std::string_view name, float initialValue, float speed)
{
    float v = initialValue;
    ImGui::DragFloat(name.data(), &v, speed);
    if (ImGui::IsItemActive())
    {
        DisableCursor();
    }
    return v;
}

glm::vec3 EditorUI::ColorEdit3(std::string_view name, const glm::vec3 &initialValue)
{
    glm::vec3 v = initialValue;
    ImGui::ColorEdit3(name.data(), &v.x);
    return v;
}

void EditorUI::TextureSelector(std::string_view label, std::string &textureId)
{
    ImGui::PushID(label.data());
    std::string textureName = "";
    if (mScene->GetResourceManager().GetTextureManager().HasTexture(textureId))
    {
        textureName = mScene->GetResourceManager().GetTextureManager().GetTexture(textureId).GetName();
    }

    if (ImGui::Button("Clear"))
    {
        textureId = "";
    }
    ImGui::SameLine();
    if (!ImGui::BeginCombo(label.data(), textureName.c_str()))
    {
        ImGui::PopID();
        return;
    }

    for (const auto &[id, texture] : mScene->GetResourceManager().GetTextureManager().GetMap())
    {
        if (ImGui::Selectable(id.data(), id == textureId))
        {
            textureId = id;
        }
    }

    ImGui::EndCombo();
    ImGui::PopID();
}

void EditorUI::FontSelector(std::string_view label, std::string &fontId)
{
    std::string fontName = "None";
    if (mScene->GetResourceManager().GetFontManager().HasFont(fontId))
    {
        fontName = mScene->GetResourceManager().GetFontManager().GetFont(fontId).GetFileName();
    }

    if (!ImGui::BeginCombo(label.data(), fontName.c_str()))
    {
        return;
    }

    for (const auto &[id, font] : mScene->GetResourceManager().GetFontManager().GetMap())
    {
        if (ImGui::Selectable(font.GetFileName().c_str(), id == fontId))
        {
            fontId = id;
        }
    }

    ImGui::EndCombo();
}

void EditorUI::ShaderImporter(bool &opened)
{
    ImGui::Begin("Shader importer", &opened);

    ImGui::InputText("Identifier", &mShaderPath.id);

    ImGui::PushID("vertex");

    Button("...", [&] {
        ImGui::GetStateStorage()->SetBool(ImGui::GetID("openVertexPath"), true);
    });

    if (ImGui::GetStateStorage()->GetBool(ImGui::GetID("openVertexPath")))
    {
        if (mFileDialog.OpenFile("Vertex Path", ".", mShaderPath.vertexPath))
        {
            ImGui::GetStateStorage()->SetBool(ImGui::GetID("openVertexPath"), false);
        }
    }

    ImGui::SameLine();
    ImGui::InputText("vertex", &mShaderPath.vertexPath);
    ImGui::PopID();

    ImGui::PushID("fragment");

    Button("...", [&] {
        ImGui::GetStateStorage()->SetBool(ImGui::GetID("openFragmentPath"), true);
    });

    if (ImGui::GetStateStorage()->GetBool(ImGui::GetID("openFragmentPath")))
    {
        if (mFileDialog.OpenFile("Fragment Path", ".", mShaderPath.fragmentPath))
        {
            ImGui::GetStateStorage()->SetBool(ImGui::GetID("openFragmentPath"), false);
        }
    }

    ImGui::SameLine();
    ImGui::InputText("fragment", &mShaderPath.fragmentPath);
    ImGui::PopID();

    Button("Import", [&] {
        mScene->GetResourceManager().GetShaderManager().Load(mShaderPath.id, mShaderPath.vertexPath, mShaderPath.fragmentPath, Renderer::SetupSceneShader);
        mShaderPath = {};
    });
    ImGui::SameLine();
    Button("Cancel", [&] {
        opened = false;
        mShaderPath = {};
    });

    ImGui::End();
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
    ShaderImporter(mEnableShaderImporter);
    MaterialEditor();

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
        }
        ImGui::EndPopup();
    }

    mScene->Each<EntityMetadata>([&](Entity entity, EntityMetadata &metadata) {
        if (ImGui::Selectable(metadata.name.c_str(), mSelectedEntity.GetId() == entity.GetId()))
        {
            mSelectedEntity = entity;
        }
    });

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
            mScene->DestroyEntity(mSelectedEntity);
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
        std::shared_ptr<ModelImporter> importer = std::make_shared<AssimpImporter>();
        importer->Import(filename.c_str(), *mScene);
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
            controller.EnableMouseControl(true);
            DisableCursor();
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
        mScene->GetResourceManager().GetTextureManager().LoadTexture(filename, filename);
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
    FontSelector("Font", component.font);
    ImGui::ColorEdit4("Forground", &component.forgroundColor.x);
    ImGui::ColorEdit4("Background", &component.backgroundColor.x);

    ImGui::PopID();
}

void EditorUI::MaterialEditor()
{
    ImGui::Begin("Material editor", ImGui::GetStateStorage()->GetBoolRef(ImGui::GetID("EnableMaterialEditor")));
    static std::string preview = "None";

    if (ImGui::BeginCombo("Material", preview.c_str()))
    {
        for (const auto &[id, material] : mScene->GetResourceManager().GetMaterialManager().GetMap())
        {
            if (ImGui::Selectable(id.c_str(), preview == id))
            {
                preview = id;
            }
        }
        ImGui::EndCombo();
    }

    if (mScene->GetResourceManager().GetMaterialManager().HasMaterial(preview))
    {
        Material &material = mScene->GetResourceManager().GetMaterialManager().GetMaterial(preview);

        ImGui::InputText("Name", &material.name);

        ImGui::SliderFloat("Roughness", &material.roughnessFactor, 0.1f, 1.f);
        ImGui::SliderFloat("Metallic", &material.metallicFactor, 0.01f, 1.f);
        ImGui::ColorEdit4("Color", &material.colorFactor.x);

        TextureSelector("Albedo Texture", material.albedoTexture);
        TextureSelector("Roughness Texture", material.roughnessTexture);
        TextureSelector("Metallic Texture", material.metallicTexture);

        const char *cullModeString[] =
            {
                "None",
                "Front",
                "Back",
            };

        ImGui::Checkbox("Depth test", &material.enableDepthTest);
        ImGui::Checkbox("Depth write", &material.enableDepthWrite);

        if (ImGui::BeginCombo("Cull mode", cullModeString[(uint64_t)material.cullMode]))
        {
            for (int i = 0; i < 3; i++)
            {
                if (ImGui::Selectable(cullModeString[i], (uint64_t)material.cullMode == i))
                {
                    material.cullMode = (CullMode)i;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::InputInt("Priority", &material.drawPriority);
    }
    else
    {
        static Material material;

        ImGui::InputText("Name", &material.name);

        if (ImGui::BeginCombo("Shader", material.shader.c_str()))
        {
            for (const auto &[id, shader] : mScene->GetResourceManager().GetShaderManager().GetMap())
            {
                if (ImGui::Selectable(id.c_str(), id == material.shader))
                {
                    material.shader = id;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SliderFloat("Roughness", &material.roughnessFactor, 0.1f, 1.f);
        ImGui::SliderFloat("Metallic", &material.metallicFactor, 0.01f, 1.f);
        ImGui::ColorEdit4("Color", &material.colorFactor.x);

        TextureSelector("Albedo Texture", material.albedoTexture);
        TextureSelector("Roughness Texture", material.roughnessTexture);
        TextureSelector("Metallic Texture", material.metallicTexture);

        const char *cullModeString[] =
            {
                "None",
                "Front",
                "Back",
            };

        if (ImGui::BeginCombo("Cull mode", cullModeString[(uint64_t)material.cullMode]))
        {
            for (int i = 0; i < 3; i++)
            {
                if (ImGui::Selectable(cullModeString[i], (uint64_t)material.cullMode == i))
                {
                    material.cullMode = (CullMode)i;
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Checkbox("Depth test", &material.enableDepthTest);
        ImGui::Checkbox("Depth write", &material.enableDepthWrite);

        ImGui::InputInt("Priority", &material.drawPriority);

        Button("Create", [&] {
            mScene->GetResourceManager().GetMaterialManager().AddMaterial(material, material.name);
            material = Material();
        });
        ImGui::SameLine();
        Button("Cancel", [&] {
            material = Material();
        });
    }

    ImGui::End();
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

    if (ImGui::BeginCombo("Material", component.material.c_str()))
    {
        for (const auto &[id, material] : mScene->GetResourceManager().GetMaterialManager().GetMap())
        {
            if (ImGui::Selectable(material.name.c_str(), id == component.material))
            {
                component.material = id;
            }
        }

        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Mesh", component.mesh.c_str()))
    {
        for (const auto &[id, mesh] : mScene->GetResourceManager().GetMeshManager().GetMap())
        {
            if (ImGui::Selectable(mesh.GetName().c_str(), id == component.mesh))
            {
                component.mesh = id;
            }
        }

        ImGui::EndCombo();
    }

    if (mScene->GetResourceManager().GetMaterialManager().HasMaterial(component.material))
    {
        Material &cMaterial = mScene->GetResourceManager().GetMaterialManager().GetMaterial(component.material);

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

void EditorUI::Present(const Surface &surface)
{
    uint32_t imageIndex = surface.swapchain.GetNextImageIndex(
        Renderer::mImageAcquiredSemaphore, {});
    if (imageIndex == UINT32_MAX)
    {
        return;
    }

    Renderer::mPresentCommandBuffer.BeginRecording();
    Renderer::mPresentRenderPass.CmdBeginRenderPass(Renderer::mPresentCommandBuffer, surface.frameBuffers[imageIndex], surface.swapchain.GetSize(), {{1, 1, 1, 1}});

    VkViewport viewport =
        {
            .width = (float)surface.swapchain.GetSize().x,
            .height = (float)surface.swapchain.GetSize().y,
            .minDepth = 0.f,
            .maxDepth = 1.f,
        };

    VkRect2D scissor = {
        .extent = {(uint32_t)viewport.width, (uint32_t)viewport.height},
    };

    VkDescriptorSet descriptorSets[] = {Renderer::mPresentInputDescriptor.GetDescriptorSet()};
    vkCmdBindDescriptorSets(Renderer::mPresentCommandBuffer.GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, Renderer::mPresentShader.GetGraphicsPipeline().GetPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);
    Renderer::mPresentShader.GetGraphicsPipeline().CmdBindPipeline(Renderer::mPresentCommandBuffer);

    vkCmdSetViewport(Renderer::mPresentCommandBuffer.GetHandle(), 0, 1, &viewport);
    vkCmdSetScissor(Renderer::mPresentCommandBuffer.GetHandle(), 0, 1, &scissor);
    vkCmdSetCullMode(Renderer::mPresentCommandBuffer.GetHandle(), VK_CULL_MODE_NONE);
    vkCmdSetDepthTestEnable(Renderer::mPresentCommandBuffer.GetHandle(), false);
    vkCmdSetDepthWriteEnable(Renderer::mPresentCommandBuffer.GetHandle(), false);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Renderer::mPresentCommandBuffer.GetHandle());

    Renderer::mPresentRenderPass.CmdEndRenderPass(Renderer::mPresentCommandBuffer);
    Renderer::mPresentCommandBuffer.EndRecording();

    Renderer::mPresentCommandBuffer.QueueSubmit(GraphicsContext::GetCurrentContext().GetQueues().graphics, Renderer::mImageAcquiredSemaphore, Renderer::mSwapchainRenderFinished, PipelineStage::ColorAttachmentOutput);

    VkSwapchainKHR swapchain[] =
        {
            surface.swapchain.GetHandle(),
        };
    VkSemaphore waitSemaphores[] =
        {
            Renderer::mSwapchainRenderFinished.GetHandle(),
        };

    VkPresentInfoKHR presentInfo =
        {
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = waitSemaphores,
            .swapchainCount = 1,
            .pSwapchains = swapchain,
            .pImageIndices = &imageIndex,
        };

    vkQueuePresentKHR(GraphicsContext::GetCurrentContext().GetQueues().graphics, &presentInfo);

    vkDeviceWaitIdle(GraphicsContext::GetCurrentContext().GetDevice());
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
        MenuItem("Material Editor", [&]() { ImGui::GetStateStorage()->SetBool(ImGui::GetID("EnableMaterialEditor"), true); });
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Assets"))
    {
        MenuItem("Import Model", [&]() { mEnableModelImporter = true; });
        MenuItem("Import Image", [&]() { mEnableImageImporter = true; });
        MenuItem("Import Shader", [&]() { mEnableShaderImporter = true; });
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
    vkUpdateDescriptorSets(GraphicsContext::GetCurrentContext().GetDevice(), 1, &write, 0, nullptr);
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
    Application::GetInstance()->ResetCursor();
}
void EditorUI::DisableCursor()
{
    Application::GetInstance()->DisableCursor();
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
