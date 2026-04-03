
#ifndef THREEPP_QT6CANVAS_HPP
#define THREEPP_QT6CANVAS_HPP

#include "threepp/canvas/WindowSize.hpp"
#include "threepp/input/PeripheralsEventSource.hpp"

#include <QOpenGLWidget>

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace threepp {

    class Qt6Canvas: public QOpenGLWidget, public PeripheralsEventSource {

        Q_OBJECT

    public:
        struct Parameters;

        explicit Qt6Canvas(const Parameters& params = Parameters(), QWidget* parent = nullptr);

        explicit Qt6Canvas(const std::string& name, QWidget* parent = nullptr);

        //the current size of the Canvas widget
        [[nodiscard]] WindowSize size() const override;

        [[nodiscard]] float aspect() const;

        void setSize(std::pair<int, int> size);

        void onWindowResize(std::function<void(WindowSize)> f);

        void animate(const std::function<void()>& f);

        // returns false if application should quit, true otherwise
        bool animateOnce(const std::function<void()>& f);

        [[nodiscard]] bool isOpen() const;

        void close() override;

        ~Qt6Canvas() override;

    protected:
        void initializeGL() override;
        void resizeGL(int w, int h) override;
        void paintGL() override;

        void keyPressEvent(QKeyEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;
        void mousePressEvent(QMouseEvent* event) override;
        void mouseReleaseEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void wheelEvent(QWheelEvent* event) override;

    private:
        struct Impl;
        std::unique_ptr<Impl> pimpl_;

    public:
        struct Parameters {

            Parameters() = default;

            Parameters& title(std::string value);

            Parameters& size(WindowSize size);

            Parameters& size(int width, int height);

            Parameters& antialiasing(int antialiasing);

            Parameters& vsync(bool flag);

            Parameters& resizable(bool flag);

            Parameters& exitOnKeyEscape(bool flag);

        private:
            std::optional<WindowSize> size_;
            int antialiasing_{2};
            std::string title_{"threepp"};
            bool vsync_{true};
            bool resizable_{true};
            bool exitOnKeyEscape_{true};

            friend struct Impl;
        };
    };

}// namespace threepp

#endif//THREEPP_QT6CANVAS_HPP
