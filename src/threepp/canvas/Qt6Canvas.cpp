
#include "threepp/canvas/Qt6Canvas.hpp"

#include "threepp/utils/LoadGlad.hpp"

#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QSurfaceFormat>
#include <QTimer>
#include <QWheelEvent>

#include <iostream>

using namespace threepp;

namespace {

    Key qtKeyCodeToKey(int keyCode) {

        // clang-format off
        switch (keyCode) {
            case Qt::Key_0: return Key::NUM_0;
            case Qt::Key_1: return Key::NUM_1;
            case Qt::Key_2: return Key::NUM_2;
            case Qt::Key_3: return Key::NUM_3;
            case Qt::Key_4: return Key::NUM_4;
            case Qt::Key_5: return Key::NUM_5;
            case Qt::Key_6: return Key::NUM_6;
            case Qt::Key_7: return Key::NUM_7;
            case Qt::Key_8: return Key::NUM_8;
            case Qt::Key_9: return Key::NUM_9;

            case Qt::Key_F1: return Key::F1;
            case Qt::Key_F2: return Key::F2;
            case Qt::Key_F3: return Key::F3;
            case Qt::Key_F4: return Key::F4;
            case Qt::Key_F5: return Key::F5;
            case Qt::Key_F6: return Key::F6;
            case Qt::Key_F7: return Key::F7;
            case Qt::Key_F8: return Key::F8;
            case Qt::Key_F9: return Key::F9;
            case Qt::Key_F10: return Key::F10;
            case Qt::Key_F11: return Key::F11;
            case Qt::Key_F12: return Key::F12;

            case Qt::Key_A: return Key::A;
            case Qt::Key_B: return Key::B;
            case Qt::Key_C: return Key::C;
            case Qt::Key_D: return Key::D;
            case Qt::Key_E: return Key::E;
            case Qt::Key_F: return Key::F;
            case Qt::Key_G: return Key::G;
            case Qt::Key_H: return Key::H;
            case Qt::Key_J: return Key::J;
            case Qt::Key_K: return Key::K;
            case Qt::Key_L: return Key::L;
            case Qt::Key_M: return Key::M;
            case Qt::Key_N: return Key::N;
            case Qt::Key_O: return Key::O;
            case Qt::Key_P: return Key::P;
            case Qt::Key_Q: return Key::Q;
            case Qt::Key_R: return Key::R;
            case Qt::Key_S: return Key::S;
            case Qt::Key_T: return Key::T;
            case Qt::Key_U: return Key::U;
            case Qt::Key_V: return Key::V;
            case Qt::Key_W: return Key::W;
            case Qt::Key_X: return Key::X;
            case Qt::Key_Y: return Key::Y;
            case Qt::Key_Z: return Key::Z;

            case Qt::Key_Up: return Key::UP;
            case Qt::Key_Down: return Key::DOWN;
            case Qt::Key_Left: return Key::LEFT;
            case Qt::Key_Right: return Key::RIGHT;

            case Qt::Key_Space: return Key::SPACE;
            case Qt::Key_Comma: return Key::COMMA;
            case Qt::Key_Minus: return Key::MINUS;
            case Qt::Key_Period: return Key::PERIOD;
            case Qt::Key_Slash: return Key::SLASH;

            case Qt::Key_Return: return Key::ENTER;
            case Qt::Key_Tab: return Key::TAB;
            case Qt::Key_Backspace: return Key::BACKSPACE;
            case Qt::Key_Insert: return Key::INSERT;
            case Qt::Key_Delete: return Key::DEL;

            case Qt::Key_Shift: return Key::LEFT_SHIFT;
            case Qt::Key_Control: return Key::LEFT_CONTROL;
            case Qt::Key_Alt: return Key::LEFT_ALT;

            default: return Key::UNKNOWN;
        }
        // clang-format on
    }

    int qtModsToInt(Qt::KeyboardModifiers mods) {

        int result = 0;
        if (mods & Qt::ShiftModifier) result |= 1;
        if (mods & Qt::ControlModifier) result |= 2;
        if (mods & Qt::AltModifier) result |= 4;
        return result;
    }

    int qtMouseButtonToInt(Qt::MouseButton button) {

        switch (button) {
            case Qt::LeftButton: return 0;
            case Qt::RightButton: return 1;
            case Qt::MiddleButton: return 2;
            default: return -1;
        }
    }

}// namespace

struct Qt6Canvas::Impl {

    Qt6Canvas& scope;

    WindowSize size_;
    Vector2 lastMousePos_;

    bool close_{false};
    bool exitOnKeyEscape_;
    bool glInitialized_{false};

    std::function<void()> renderFunction_;
    QTimer* animationTimer_{nullptr};

    std::vector<std::function<void(WindowSize)>> resizeListener;

    explicit Impl(Qt6Canvas& scope, const Parameters& params)
        : scope(scope), exitOnKeyEscape_(params.exitOnKeyEscape_) {

        if (params.size_) {
            size_ = *params.size_;
        } else {
            size_ = {640, 480};
        }

        QSurfaceFormat format;
        format.setVersion(3, 3);
        format.setProfile(QSurfaceFormat::CoreProfile);
        format.setDepthBufferSize(24);
        if (params.antialiasing_ > 0) {
            format.setSamples(params.antialiasing_);
        }
        if (params.vsync_) {
            format.setSwapInterval(1);
        } else {
            format.setSwapInterval(0);
        }
        scope.setFormat(format);

        scope.setWindowTitle(QString::fromStdString(params.title_));
        scope.resize(size_.width(), size_.height());

        if (!params.resizable_) {
            scope.setFixedSize(size_.width(), size_.height());
        }
    }

    [[nodiscard]] const WindowSize& getSize() const {

        return size_;
    }

    void setSize(std::pair<int, int> size) {

        scope.resize(size.first, size.second);
    }

    bool animateOnce(const std::function<void()>& f) {

        if (close_) {
            return false;
        }

        renderFunction_ = f;
        scope.update();

        return true;
    }

    void animate(const std::function<void()>& f) {

        renderFunction_ = f;

        animationTimer_ = new QTimer(&scope);
        QObject::connect(animationTimer_, &QTimer::timeout, &scope, [this]() {
            if (close_) {
                animationTimer_->stop();
                if (auto app = QApplication::instance()) {
                    app->quit();
                }
                return;
            }
            scope.update();
        });
        animationTimer_->start(0);

        scope.show();

        if (auto app = QApplication::instance()) {
            app->exec();
        }
    }

    void onWindowResize(std::function<void(WindowSize)> f) {

        this->resizeListener.emplace_back(std::move(f));
    }

    void close() {

        close_ = true;
    }
};

Qt6Canvas::Qt6Canvas(const Parameters& params, QWidget* parent)
    : QOpenGLWidget(parent),
      pimpl_(std::make_unique<Impl>(*this, params)) {

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

Qt6Canvas::Qt6Canvas(const std::string& name, QWidget* parent)
    : Qt6Canvas(Parameters().title(name), parent) {}

void Qt6Canvas::initializeGL() {

    loadGlad();
    pimpl_->glInitialized_ = true;

    if (pimpl_->size_.width() > 0 && pimpl_->size_.height() > 0) {
        glEnable(GL_PROGRAM_POINT_SIZE);
    }

    auto fmt = format();
    if (fmt.samples() > 0) {
        glEnable(GL_MULTISAMPLE);
    }
}

void Qt6Canvas::resizeGL(int w, int h) {

    pimpl_->size_ = {w, h};
    for (const auto& listener : pimpl_->resizeListener) {
        listener(pimpl_->size_);
    }
}

void Qt6Canvas::paintGL() {

    if (pimpl_->renderFunction_) {
        pimpl_->renderFunction_();
    }
}

void Qt6Canvas::keyPressEvent(QKeyEvent* event) {

    if (event->key() == Qt::Key_Escape && pimpl_->exitOnKeyEscape_) {
        pimpl_->close();
        return;
    }

    const KeyEvent evt{qtKeyCodeToKey(event->key()), event->nativeScanCode(), qtModsToInt(event->modifiers())};
    if (event->isAutoRepeat()) {
        onKeyEvent(evt, KeyAction::REPEAT);
    } else {
        onKeyEvent(evt, KeyAction::PRESS);
    }
}

void Qt6Canvas::keyReleaseEvent(QKeyEvent* event) {

    if (event->isAutoRepeat()) return;

    const KeyEvent evt{qtKeyCodeToKey(event->key()), event->nativeScanCode(), qtModsToInt(event->modifiers())};
    onKeyEvent(evt, KeyAction::RELEASE);
}

void Qt6Canvas::mousePressEvent(QMouseEvent* event) {

    Vector2 pos(static_cast<float>(event->position().x()), static_cast<float>(event->position().y()));
    pimpl_->lastMousePos_.copy(pos);
    onMousePressedEvent(qtMouseButtonToInt(event->button()), pos, MouseAction::PRESS);
}

void Qt6Canvas::mouseReleaseEvent(QMouseEvent* event) {

    Vector2 pos(static_cast<float>(event->position().x()), static_cast<float>(event->position().y()));
    pimpl_->lastMousePos_.copy(pos);
    onMousePressedEvent(qtMouseButtonToInt(event->button()), pos, MouseAction::RELEASE);
}

void Qt6Canvas::mouseMoveEvent(QMouseEvent* event) {

    Vector2 pos(static_cast<float>(event->position().x()), static_cast<float>(event->position().y()));
    onMouseMoveEvent(pos);
    pimpl_->lastMousePos_.copy(pos);
}

void Qt6Canvas::wheelEvent(QWheelEvent* event) {

    QPoint delta = event->angleDelta();
    onMouseWheelEvent({static_cast<float>(delta.x()) / 120.0f, static_cast<float>(delta.y()) / 120.0f});
}

WindowSize Qt6Canvas::size() const {

    return pimpl_->getSize();
}

float Qt6Canvas::aspect() const {

    return size().aspect();
}

void Qt6Canvas::setSize(std::pair<int, int> size) {

    pimpl_->setSize(size);
}

void Qt6Canvas::onWindowResize(std::function<void(WindowSize)> f) {

    pimpl_->onWindowResize(std::move(f));
}

void Qt6Canvas::animate(const std::function<void()>& f) {

    pimpl_->animate(f);
}

bool Qt6Canvas::animateOnce(const std::function<void()>& f) {

    return pimpl_->animateOnce(f);
}

bool Qt6Canvas::isOpen() const {

    return !pimpl_->close_;
}

void Qt6Canvas::close() {

    pimpl_->close();
}

Qt6Canvas::~Qt6Canvas() = default;


Qt6Canvas::Parameters& Qt6Canvas::Parameters::title(std::string value) {

    this->title_ = std::move(value);

    return *this;
}

Qt6Canvas::Parameters& Qt6Canvas::Parameters::size(WindowSize size) {

    this->size_ = size;

    return *this;
}

Qt6Canvas::Parameters& Qt6Canvas::Parameters::size(int width, int height) {

    return this->size({width, height});
}

Qt6Canvas::Parameters& Qt6Canvas::Parameters::antialiasing(int antialiasing) {

    this->antialiasing_ = antialiasing;

    return *this;
}

Qt6Canvas::Parameters& Qt6Canvas::Parameters::vsync(bool flag) {

    this->vsync_ = flag;

    return *this;
}

Qt6Canvas::Parameters& Qt6Canvas::Parameters::resizable(bool flag) {

    this->resizable_ = flag;

    return *this;
}

Qt6Canvas::Parameters& Qt6Canvas::Parameters::exitOnKeyEscape(bool flag) {

    exitOnKeyEscape_ = flag;

    return *this;
}
