#include "Assets/FontImporter.hpp"
#include "EditorUi.hpp"
#include "EntityComponentSystem/SceneSerializer.hpp"
#include "Renderer/Converter.hpp"
#include "Renderer/TextRenderer.hpp"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Assets/ModelImporter.hpp"
#include "CameraController.hpp"
#include "EntityComponentSystem/Component.hpp"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include <Engine.hpp>

#define BindCommandCallback(callback) std::bind(&callback, this, std::placeholders::_1)

struct PickerPushConstant
{
    glm::mat4 model;
    glm::vec4 idcolor;
};

struct PickerUniformData
{
    glm::mat4 view;
    glm::mat4 projection;
};

class Editor : public Application
{
    Surface mSurface;
    Camera mCamera;
    CameraController mController;
    Scene mScene;

    EditorUI mEditorUi;

    ShaderID mIdShader;

    std::shared_ptr<Mesh> mBillboard = std::make_shared<Mesh>();

    void OnInitialize() override
    {
        int scale = 240;
        Renderer::SetResolution({16 * scale, 9 * scale});
    }

    void OnStart() override
    {
        mCamera.SetFov(90.f);
        mController.SetSensitivity(0.1f);
        mController.SetCamera(mCamera, GetWindow());

        mSurface = Renderer::CreateSurface(GetWindow());
        GetWindow().SetTitle("Editor");
        GetWindow().Maximize();

        Renderer::SetBasicShader("Shaders/physical.vert.spv", "Shaders/physical.frag.spv");

        ModelImporter modelImporter;
        modelImporter.Import("./Models/cube/cube.gltf", mScene);
        modelImporter.Import("./Models/City/scene.gltf", mScene);

        std::shared_ptr<Material> skyboxMaterial = std::make_shared<Material>();
        skyboxMaterial->shader = ShaderManager::Load("Shaders/skybox.vert.spv", "Shaders/skybox.frag.spv");
        skyboxMaterial->cullMode = CullMode::None;
        skyboxMaterial->enableDepthTest = false;
        skyboxMaterial->enableDepthWrite = false;
        skyboxMaterial->name = "skybox";

        Entity skyboxEntity = mScene.GetEntityByName("Cube");
        MeshRendererComponent &meshRenderer = skyboxEntity.GetComponent<MeshRendererComponent>();
        meshRenderer.material = MaterialManager::AddMaterial(skyboxMaterial);

        Light::Initialize();

        mEditorUi.Initialize(Renderer::mSceneResolveAttachment, GetWindow(), mSurface);
        mEditorUi.SetScene(mScene);

        TextRenderer::Initialize();

        FontID id = FontManager::Load("Fonts/Orbitron-Regular.ttf", 128);

        mTextEntity = mScene.CreateEntity("Text");
        mTextEntity.AddComponent<Transform>();
        auto &text = mTextEntity.AddComponent<TextComponent>();
        text.text = "Hello world";
        text.fontId = id;
    }

    Entity mTextEntity;

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

        mTextEntity.GetComponent<TextComponent>().text = std::to_string(GetFps());

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

        for (const auto &[entity, component] : mScene.GetEntities<MeshRendererComponent>())
        {
            if (component.material == (MaterialID)UINT64_MAX || component.mesh == (MeshID)UINT64_MAX)
            {
                continue;
            }

            Renderer::Submit(component.material, component.mesh, entity.GetComponent<Transform>());
        }

        for (const auto &[entity, textComponent] : mScene.GetEntities<TextComponent>())
        {
            if (textComponent.fontId == INVALID_FONT_ID && entity.HasComponent<Transform>())
            {
                continue;
            }

            TextRenderer::DrawText(FontManager::GetFont(textComponent.fontId), textComponent.text, textComponent.spacing, textComponent.color, entity.GetComponent<Transform>());
        }

        TextRenderer::Flush();

        Renderer::EndFrame();

        RenderUI();

        Present();
    }

    void RenderUI()
    {
        mEditorUi.OnRender(mCamera, mController);
    }

    void Present()
    {
        uint32_t imageIndex = mSurface.swapchain.GetNextImageIndex(Renderer::mImageAcquiredSemaphore, {});
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

        VkRect2D scissor =
            {
                .extent = {(uint32_t)viewport.width, (uint32_t)viewport.height},
            };

        VkDescriptorSet descriptorSets[] = {Renderer::mPresentInputDescriptor.GetDescriptorSet()};
        vkCmdBindDescriptorSets(Renderer::mPresentCommandBuffer.GetHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, Renderer::mPresentPipeline.GetPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);
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

        VkSwapchainKHR swapchain[] = {mSurface.swapchain.GetHandle()};
        VkSemaphore waitSemaphores[] = {Renderer::mSwapchainRenderFinished.GetHandle()};

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
        TextRenderer::Terminate();
    }
};

CREATE_APPLICATION(Editor)
