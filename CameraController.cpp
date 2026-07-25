#include "CameraController.hpp"
#include "Core/Application.hpp"
#include "Input/Mouse.hpp"

void CameraController::SetCamera(Camera &camera, Window &window)
{
    CHROME_TRACE_FUNCTION();
    SetCamera(camera);
    mCamera->SetAspectRatio(float(window.GetSize().x) / float(window.GetSize().y));

    ConnectWindow(window);
}
void CameraController::SetCamera(Camera &camera)
{
    CHROME_TRACE_FUNCTION();
    mCamera = &camera;

    mCamera->SetCameraType(CameraType::Perspective)
        .SetFront(glm::vec3(0.0f))
        .SetPosition(glm::vec3(0.f, 2.f, 2.f))
        .SetNearPlane(0.01f)
        .SetFarPlane(1000.f);
}
const Camera &CameraController::GetCamera() const
{
    CHROME_TRACE_FUNCTION();
    return *mCamera;
}

void CameraController::Update()
{
    CHROME_TRACE_FUNCTION();

    glm::vec3 cameraFront = mCamera->GetFront();
    if (mEnableMouseControl)
    {
        cameraFront.x = glm::sin(glm::radians(mYaw)) * glm::cos(glm::radians(-mPitch));
        cameraFront.y = glm::sin(glm::radians(-mPitch));
        cameraFront.z = glm::cos(glm::radians(mYaw)) * glm::cos(glm::radians(-mPitch));

        mEulerAngles.x = glm::mix(mEulerAngles.x, mYaw, 1.f);
        mEulerAngles.y = glm::mix(mEulerAngles.y, mPitch, 1.f);

        mCamera->SetFront(cameraFront);
    }

    glm::vec3 cameraPosition = mCamera->GetPosition();
    if (mEnableKeyboardControl)
    {
        glm::vec3 forward = normalize(glm::vec3(cameraFront.x, 0, cameraFront.z));
        glm::vec3 side = normalize(cross(glm::vec3(cameraFront.x, 0, cameraFront.z), mCamera->GetUp()));
        float deltaTime = Application::GetInstance()->GetDeltaTime();

        if (mMoveForward)
        {
            cameraPosition += forward * mSpeed * deltaTime;
        }
        if (mMoveBackward)
        {
            cameraPosition -= forward * mSpeed * deltaTime;
        }
        if (mMoveLeft)
        {
            cameraPosition -= side * mSpeed * deltaTime;
        }
        if (mMoveRight)
        {
            cameraPosition += side * mSpeed * deltaTime;
        }
        if (mMoveUp)
        {
            cameraPosition += mCamera->GetUp() * mSpeed * deltaTime;
        }
        if (mMoveDown)
        {
            cameraPosition -= mCamera->GetUp() * mSpeed * deltaTime;
        }

        mCamera->SetPosition(cameraPosition);
    }
}

void CameraController::OnKeyPress(Key key)
{
    CHROME_TRACE_FUNCTION();

    if (!mEnableControl || !mEnableKeyboardControl)
    {
        return;
    }

    if (key == Key::W)
    {
        mMoveForward = true;
    }
    if (key == Key::S)
    {
        mMoveBackward = true;
    }
    if (key == Key::A)
    {
        mMoveLeft = true;
    }
    if (key == Key::D)
    {
        mMoveRight = true;
    }
    if (key == Key::Space)
    {
        mMoveUp = true;
    }
    if (key == Key::LeftShift)
    {
        mMoveDown = true;
    }
    if (key == Key::LeftControl)
    {
        mSpeed *= 5;
    }
}
void CameraController::OnKeyRelease(Key key)
{
    CHROME_TRACE_FUNCTION();

    if (key == Key::W)
    {
        mMoveForward = false;
    }
    if (key == Key::S)
    {
        mMoveBackward = false;
    }
    if (key == Key::A)
    {
        mMoveLeft = false;
    }
    if (key == Key::D)
    {
        mMoveRight = false;
    }
    if (key == Key::Space)
    {
        mMoveUp = false;
    }
    if (key == Key::LeftShift)
    {
        mMoveDown = false;
    }
    if (key == Key::LeftControl)
    {
        mSpeed /= 5;
    }
}
void CameraController::OnMouseMove(const glm::vec2 &position, const glm::vec2 &offset)
{
    CHROME_TRACE_FUNCTION();

    if (!mEnableControl || !mEnableMouseControl)
    {
        return;
    }

    mYaw -= offset.x * mSensitivity;
    mPitch += offset.y * mSensitivity;

    mPitch = glm::clamp(mPitch, -89.f, 89.f);
}
void CameraController::OnWindowResize(const glm::vec2 &size)
{
    CHROME_TRACE_FUNCTION();

    mCamera->SetAspectRatio(float(size.x) / float(size.y));
}

void CameraController::OnMouseButtonPress(MouseButton button)
{
    CHROME_TRACE_FUNCTION();

    if (!mEnableControl || !mEnableMouseControl)
        return;
}

void CameraController::OnMouseButtonRelease(MouseButton button)
{
    CHROME_TRACE_FUNCTION();
}

void CameraController::OnScroll(const glm::vec2 &scroll)
{
    CHROME_TRACE_FUNCTION();

    if (!mEnableControl || !mEnableMouseControl)
        return;
}

void CameraController::ConnectWindow(Window &window)
{
    CHROME_TRACE_FUNCTION();

    window.AddListener(BindMember(CameraController::WindowEventCallback));
}

bool CameraController::WindowEventCallback(uint32_t code, void *data)
{
    CHROME_TRACE_FUNCTION();

    WindowEvent event = (WindowEvent)code;

    switch (event)
    {
    case WindowEvent::WindowResize: {
        glm::uvec2 size = *(glm::uvec2 *)data;
        OnWindowResize(size);
        break;
    }
    case WindowEvent::WindowMousePress: {
        MouseButton button = *(MouseButton *)data;
        OnMouseButtonPress(button);
        break;
    }
    case WindowEvent::WindowMouseRelease: {
        MouseButton button = *(MouseButton *)data;
        OnMouseButtonRelease(button);
        break;
    }
    case WindowEvent::WindowMouseMove: {
        glm::vec2 position = *(glm::vec2 *)data;
        glm::vec2 offset = position - previousMousePos;
        previousMousePos = position;
        OnMouseMove(position, offset);
        break;
    }
    case WindowEvent::WindowScroll: {
        glm::vec2 scroll = *(glm::vec2 *)data;
        OnScroll(scroll);
        break;
    }
    case WindowEvent::WindowKeyPress: {
        Key key = *(Key *)data;
        OnKeyPress(key);
        break;
    }
    case WindowEvent::WindowKeyRelease: {
        Key key = *(Key *)data;
        OnKeyRelease(key);
        break;
    }
    default: {
        break;
    }
    }

    return false;
}

void CameraController::EnableKeyboardControl(bool enable)
{
    mEnableKeyboardControl = enable;
}

void CameraController::EnableMouseControl(bool enable)
{
    mEnableMouseControl = enable;
}
