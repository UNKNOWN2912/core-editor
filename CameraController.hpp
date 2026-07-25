#pragma once
#include "Core/Window.hpp"
#include "Input/Keyboard.hpp"
#include "Input/Mouse.hpp"
#include "Renderer/Camera.hpp"

class CameraController
{
public:
    void SetCamera(Camera &camera, Window &window);
    void SetCamera(Camera &camera);
    const Camera &GetCamera() const;

    void ConnectWindow(Window &window);

    void Update();

    void OnKeyPress(Key key);
    void OnKeyRelease(Key key);
    void OnMouseMove(const glm::vec2 &position, const glm::vec2 &offset);
    void OnWindowResize(const glm::vec2 &size);
    void OnMouseButtonPress(MouseButton button);
    void OnMouseButtonRelease(MouseButton button);
    void OnScroll(const glm::vec2 &scroll);

    bool WindowEventCallback(uint32_t code, void *data);

    float GetSensitivity() const
    {
        return mSensitivity;
    }
    float GetSpeed() const
    {
        return mSpeed;
    }

    void SetSpeed(float speed)
    {
        mSpeed = speed;
    }
    void SetSensitivity(float sensitivity)
    {
        mSensitivity = sensitivity;
    }

    void EnableControl(bool enable)
    {
        mEnableControl = enable;
    }

    void EnableKeyboardControl(bool enable);
    void EnableMouseControl(bool enable);

    float &GetSpeedRef()
    {
        return mSpeed;
    }
    float &GetSensitivityRef()
    {
        return mSensitivity;
    }

private:
    glm::vec2 previousMousePos;
    Camera *mCamera = nullptr;

    bool mMoveForward = false;
    bool mMoveBackward = false;
    bool mMoveLeft = false;
    bool mMoveRight = false;
    bool mMoveUp = false;
    bool mMoveDown = false;

    float mYaw = 0, mPitch = 0;

    float mSpeed = 5;
    float mSensitivity = 0.4;

    bool mEnableControl = true;
    bool mEnableKeyboardControl = true;
    bool mEnableMouseControl = true;

    float mSmoothness = 1.f;

    glm::vec3 mEulerAngles = glm::vec3(0);
};