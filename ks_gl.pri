INCLUDEPATH += $${PWD}
CONFIG += require_glm

# ks
PATH_KS_GL = $${PWD}/ks/gl

# gl
CONFIG(debug,debug|release) {
    DEFINES += KS_DEBUG_GL
    message("ks: OpenGL Debugging enabled")
}

HEADERS += \
    $${PATH_KS_GL}/KsGLInclude.hpp \
    $${PATH_KS_GL}/KsGLConfig.hpp \
    $${PATH_KS_GL}/KsGLDebug.hpp \
    $${PATH_KS_GL}/KsGLImplementation.hpp \
    $${PATH_KS_GL}/KsGLResource.hpp \
    $${PATH_KS_GL}/KsGLStateSet.hpp \
    $${PATH_KS_GL}/KsGLShaderProgram.hpp \
    $${PATH_KS_GL}/KsGLUniform.hpp \
    $${PATH_KS_GL}/KsGLTexture.hpp \
    $${PATH_KS_GL}/KsGLBuffer.hpp \
    $${PATH_KS_GL}/KsGLIndexBuffer.hpp \
    $${PATH_KS_GL}/KsGLVertexBuffer.hpp \
    $${PATH_KS_GL}/KsGLTexture2D.hpp \
    $${PATH_KS_GL}/KsGLCommands.hpp \
    $${PATH_KS_GL}/KsGLCamera.hpp

SOURCES += \
    $${PATH_KS_GL}/KsGLDebug.cpp \
    $${PATH_KS_GL}/KsGLResource.cpp \
    $${PATH_KS_GL}/KsGLImplementation.cpp \
    $${PATH_KS_GL}/KsGLStateSet.cpp \
    $${PATH_KS_GL}/KsGLShaderProgram.cpp \
    $${PATH_KS_GL}/KsGLTexture.cpp \
    $${PATH_KS_GL}/KsGLBuffer.cpp \
    $${PATH_KS_GL}/KsGLIndexBuffer.cpp \
    $${PATH_KS_GL}/KsGLVertexBuffer.cpp \
    $${PATH_KS_GL}/KsGLTexture2D.cpp

# opengl function loading lib if required
linux {
    !android {
        # glad opengl function loader
        PATH_GLAD = $${PATH_KS_GL}/opengl_2_1
        INCLUDEPATH += $${PATH_GLAD}
        HEADERS += $${PATH_GLAD}/glad/glad.h
        SOURCES += $${PATH_GLAD}/glad/glad.c
        LIBS += -ldl
    }
}
