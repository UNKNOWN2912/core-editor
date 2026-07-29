#include "CameraController.hpp"
#include "EditorUi.hpp"
#include "Maths/Random.hpp"
#include "backends/imgui_impl_vulkan.h"
#include <Engine.hpp>

#define BindCommandCallback(callback) \
    std::bind(&callback, this, std::placeholders::_1)

#define ENABLE_EDITOR 1
#define IMPORT_MODELS 0

class Editor : public Application
{
    Surface mSurface;
    Camera mCamera;
    CameraController mController;
    Scene mScene;
#if ENABLE_EDITOR
    EditorUI mEditorUi;
#endif
    ShaderID mIdShader;

    std::shared_ptr<Mesh> mBillboard = std::make_shared<Mesh>();

    void OnInitialize() override
    {
        int scale = 240;
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

        Renderer::SetBasicShader("Shaders/physical.vert.spv",
                                 "Shaders/physical.frag.spv");

        TextRenderer::Initialize();

        if (std::filesystem::exists("./test.json"))
        {
            SceneSerializer serializer;
            serializer.Import("test.json", mScene);
        }

        Material skyboxMaterial;
        skyboxMaterial.shader = ShaderManager::Load("Shaders/skybox.vert.spv", "Shaders/skybox.frag.spv");
        skyboxMaterial.cullMode = CullMode::None;
        skyboxMaterial.enableDepthTest = false;
        skyboxMaterial.enableDepthWrite = false;
        skyboxMaterial.name = "skybox";
        MaterialManager::AddMaterial(skyboxMaterial);

#if IMPORT_MODELS
        ModelImporter modelImporter;
        modelImporter.Import("./Models/cube/cube.gltf", mScene);
        modelImporter.Import("./Models/City/scene.gltf", mScene);

        std::shared_ptr<Material> skyboxMaterial = std::make_shared<Material>();
        skyboxMaterial->shader = ShaderManager::Load("Shaders/skybox.vert.spv",
                                                     "Shaders/skybox.frag.spv");
        skyboxMaterial->cullMode = CullMode::None;
        skyboxMaterial->enableDepthTest = false;
        skyboxMaterial->enableDepthWrite = false;
        skyboxMaterial->name = "skybox";

        Entity skyboxEntity = mScene.GetEntityByName("Cube");
        MeshRendererComponent &meshRenderer =
            skyboxEntity.GetComponent<MeshRendererComponent>();
        meshRenderer.material = MaterialManager::AddMaterial(skyboxMaterial);

        FontManager::Load("Fonts/GoogleSans-Regular.ttf");

#endif

        Light::Initialize();

#if ENABLE_EDITOR
        mEditorUi.Initialize(Renderer::mSceneResolveAttachment, GetWindow(),
                             mSurface);
        mEditorUi.SetScene(mScene);

#endif

        // mTextEntity.GetComponent<TextComponent>().text = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ";
    }

    void OnWindowResize(const glm::uvec2 &size) override
    {
        Renderer::ResizeSurface(mSurface);
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

        for (auto &[entity, component] : mScene.GetEntities<Light>())
        {
            const Transform &transform = entity.GetComponent<Transform>();
            component.SetPosition(transform.position);
            component.SetDirection(transform.rotation);
            component.SetCamera(mCamera);
            component.GenerateShadowMap(Renderer::GetRenderCommands());
            Renderer::AddLight(component);
        }

        Renderer::EndLightPlacement();

        Renderer::BeginFrame(mCamera);

        for (const auto &[entity, component] :
             mScene.GetEntities<MeshRendererComponent>())
        {
            if (component.material == (MaterialID)UINT64_MAX ||
                component.mesh == (MeshID)UINT64_MAX)
            {
                continue;
            }

            Renderer::Submit(component.material, component.mesh,
                             entity.GetComponent<Transform>());
        }

        for (const auto &[entity, textComponent] : mScene.GetEntities<TextComponent>())
        {
            if (textComponent.fontId == INVALID_FONT_ID && entity.HasComponent<Transform>())
            {
                continue;
            }

            TextRenderer::DrawText(textComponent.fontId, textComponent.text, textComponent.spacing, textComponent.forgroundColor, textComponent.backgroundColor, entity.GetComponent<Transform>());
        }

        TextRenderer::Flush();

        Renderer::EndFrame();

        RenderUI();

#if ENABLE_EDITOR
        Present();
#else
        HideCursor();
        Renderer::Present(mSurface);
#endif
    }

    void RenderUI()
    {
#if ENABLE_EDITOR
        mEditorUi.OnRender(mCamera, mController);
#endif
    }

    void Present()
    {
        uint32_t imageIndex = mSurface.swapchain.GetNextImageIndex(
            Renderer::mImageAcquiredSemaphore, {});
        if (imageIndex == UINT32_MAX)
        {
            return;
        }

        Renderer::mPresentCommandBuffer.BeginRecording();
        Renderer::mPresentRenderPass.CmdBeginRenderPass(Renderer::mPresentCommandBuffer, mSurface.frameBuffers[imageIndex], mSurface.swapchain.GetSize(), {{1, 1, 1, 1}});

        VkViewport viewport =
            {
                .width = (float)mSurface.swapchain.GetSize().x,
                .height = (float)mSurface.swapchain.GetSize().y,
                .minDepth = 0.f,
                .maxDepth = 1.f,
            };

        VkRect2D scissor = {
            .extent = {(uint32_t)viewport.width, (uint32_t)viewport.height},
        };

        VkDescriptorSet descriptorSets[] = {
            Renderer::mPresentInputDescriptor.GetDescriptorSet()};
        vkCmdBindDescriptorSets(Renderer::mPresentCommandBuffer.GetHandle(),
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                Renderer::mPresentPipeline.GetPipelineLayout(), 0,
                                1, descriptorSets, 0, nullptr);
        Renderer::mPresentPipeline.CmdBindPipeline(Renderer::mPresentCommandBuffer);

        vkCmdSetViewport(Renderer::mPresentCommandBuffer.GetHandle(), 0, 1, &viewport);
        vkCmdSetScissor(Renderer::mPresentCommandBuffer.GetHandle(), 0, 1, &scissor);
        vkCmdSetCullMode(Renderer::mPresentCommandBuffer.GetHandle(), VK_CULL_MODE_NONE);
        vkCmdSetDepthTestEnable(Renderer::mPresentCommandBuffer.GetHandle(), false);
        vkCmdSetDepthWriteEnable(Renderer::mPresentCommandBuffer.GetHandle(), false);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), Renderer::mPresentCommandBuffer.GetHandle());

        Renderer::mPresentRenderPass.CmdEndRenderPass(Renderer::mPresentCommandBuffer);
        Renderer::mPresentCommandBuffer.EndRecording();

        Renderer::mPresentCommandBuffer.QueueSubmit(GraphicsContext::GetQueues().graphics, Renderer::mImageAcquiredSemaphore, Renderer::mSwapchainRenderFinished, PipelineStage::ColorAttachmentOutput);

        VkSwapchainKHR swapchain[] =
            {
                mSurface.swapchain.GetHandle(),
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

        vkQueuePresentKHR(GraphicsContext::GetQueues().graphics, &presentInfo);

        vkDeviceWaitIdle(GraphicsContext::GetDevice());
    }

    void OnEnd() override
    {
        SceneSerializer serializer;
        serializer.Export("test.json", mScene);
        Light::Terminate();
        TextRenderer::Terminate();
    }
};

CREATE_APPLICATION(Editor)
