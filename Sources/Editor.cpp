#include "CameraController.hpp"
#include "EditorUi.hpp"
#include "Maths/Random.hpp"
#include "Renderer/DebugRenderer.hpp"
#include <Engine.hpp>
#include <filesystem>

#define BindCommandCallback(callback) \
    std::bind(&callback, this, std::placeholders::_1)

#define ENABLE_EDITOR 1
#define IMPORT_MODELS 0

#define ENABLE_EXPORT 1
#define ENABLE_IMPORT 1

class Editor : public Application
{

    Surface mSurface;
    Camera mCamera;
    CameraController mController;
    Scene mScene;
#if ENABLE_EDITOR
    EditorUI mEditorUi;
#endif

    std::string mIdShader;
    std::shared_ptr<Mesh> mBillboard = std::make_shared<Mesh>();

    DebugRenderer mDebugRenderer;

    void OnInitialize() override
    {
        int scale = 240;
        RendererSpecification spec = GetRendererSpecification();
        spec.deviceType = DeviceType::Dedicated;
        SetRendererSpecification(spec);
        Renderer::SetSampleCount(SampleCount::One);
        Renderer::SetResolution({3840, 2160});
    }

    void OnStart() override
    {

        mCamera.SetFov(90.f);
        mController.SetSensitivity(0.1f);
        mController.SetCamera(mCamera, GetWindow());

        mSurface = Renderer::CreateSurface(GetWindow());
        GetWindow().SetTitle("Editor");
        GetWindow().SetFullscreen(true);

        Light::Initialize();
        TextRenderer::Initialize();

#if ENABLE_IMPORT
        SceneSerializer serializer;
        serializer.Import("test.json", mScene);
#endif

        // Renderer::SetBasicShader(ShaderManager::GetBuiltinIdentifier().pbr, "Shaders/physical.vert.spv", "Shaders/physical.frag.spv");

        // if (std::filesystem::exists("./test.json"))
        // {
        //     SceneSerializer serializer;
        //     serializer.Import("test.json", mScene);
        // }

#if IMPORT_MODELS
        mScene.GetResourceManager().GetFontManager().Load("Fonts/GoogleSans-Regular.ttf", "MainFont");
        mScene.GetResourceManager().GetShaderManager().Load("pbr", "Shaders/physical.vert.spv", "Shaders/physical.frag.spv");
        ModelImporter modelImporter;
        modelImporter.Import("./Models/Cube/Cube.gltf", mScene);
        modelImporter.Import("./Models/Room/room.gltf", mScene);
#endif

#if ENABLE_EDITOR
        mEditorUi.Initialize(GetWindow(), mSurface);
        mEditorUi.SetScene(mScene);
#endif

        mDebugRenderer.Initialize();
        mDebugRenderer.Enable(true);

        mScene.GetResourceManager().GetTextureManager().SetTextureDescriptor(Renderer::GetTextureDescriptor());
    }

    void OnWindowResize(const glm::uvec2 &size) override
    {
        Renderer::ResizeSurface(mSurface, ImageFormat::BGRA8);
    }

    void OnKeyPress(Key key) override
    {
        if (key == Key::Escape)
        {
            Close();
        }
    }

    void OnUpdate() override
    {
        mController.Update();
        mCamera.Calculate();

        TextRenderer::SetCamera(mCamera);

        Renderer::BeginLightPlacement();

        mScene.Each<Light>([&](Entity entity, Light &light) {
            const Transform &transform = entity.GetComponent<Transform>();
            light.SetPosition(transform.position);
            light.SetDirection(transform.rotation);
            light.SetCamera(mCamera);
            light.GenerateShadowMap(Renderer::GetRenderCommands());
            Renderer::AddLight(light);
        });

        Renderer::EndLightPlacement();

        Renderer::BeginFrame(mCamera);

        mDebugRenderer.DrawCuboid({0, 0, 0}, {10, 10, 10}, RandomUnitVec3());
        mDebugRenderer.Flush();

        mScene.Each<MeshRendererComponent>([&](Entity entity, MeshRendererComponent &meshRenderer) {
            if (meshRenderer.material.size() != 0 && meshRenderer.mesh.size() != 0)
            {
                const Mesh &mesh = mScene.GetResourceManager().GetMeshManager().GetMesh(meshRenderer.mesh);
                const Material &material = mScene.GetResourceManager().GetMaterialManager().GetMaterial(meshRenderer.material);
                Renderer::Submit(mesh, material, entity.GetComponent<Transform>(), mScene.GetResourceManager().GetTextureManager());
            }
        });

        mScene.Each<TextComponent>([&](Entity entity, TextComponent &textComponent) {
            if (textComponent.font.size() != 0 && entity.HasComponent<Transform>())
            {
                TextRenderer::DrawText(mScene.GetResourceManager().GetFontManager().GetFont(textComponent.font), textComponent.text, textComponent.spacing, textComponent.forgroundColor, textComponent.backgroundColor, entity.GetComponent<Transform>());
            }
        });

        TextRenderer::Flush();

        Renderer::EndFrame();

        RenderUI();

#if ENABLE_EDITOR
        mEditorUi.Present(mSurface);
#else
        DisableCursor();
        Renderer::Present(mSurface);
#endif
    }

    void RenderUI()
    {
#if ENABLE_EDITOR
        mEditorUi.OnRender(mCamera, mController);
#endif
    }

    void OnEnd() override
    {

#if ENABLE_EXPORT
        SceneSerializer serializer;
        serializer.Export("test.json", mScene);
#endif
        Light::Terminate();
        TextRenderer::Terminate();
    }
};

CREATE_APPLICATION(Editor)
