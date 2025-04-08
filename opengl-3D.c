#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif

#define OEMRESOURCE

#include <stddef.h> 

#define GLAD_GL_IMPLEMENTATION
#define GLAD_MALLOC malloc
#define GLAD_FREE free

#include "glad.h"

#define RGFW_IMPLEMENTATION
#include "RGFW.h"

#include <math.h>
#ifndef M_PI
#define M_PI 3.1415
#endif

#define DEG2RAD (180.0 / M_PI)

typedef struct mat4 { float m[16]; } mat4;
mat4 loadIdentity(void) {
    return (mat4) { 
        {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        }
    };
}
 
mat4 mat4Multiply(float left[16], float right[16]);

mat4 perspective(float matrix[16], float fovY, float aspect, float zNear, float zFar) {
    fovY =  (fovY * DEG2RAD) / 2.0f;
    const float f = (cosf(fovY) / sinf(fovY));
    
    float perspective[16] = {
            (f / aspect), 0.0f,  0.0f,                                   0.0f,
            0,            f,     0.0f,                                   0.0f,
            0.0f,         0.0f,  (zFar + zNear) / (zNear - zFar),       -1.0f,
            0.0f,         0.0f,  (2.0 * zFar * zNear) / (zNear - zFar),  0.0f
    };
    
	return mat4Multiply(matrix, perspective);
}

#define MULTILINE_STR(...) #__VA_ARGS__

const char *vertexShaderSource = MULTILINE_STR(
    \x23version 330 core\n
    layout (location = 0) in vec3 aPos;
    uniform mat4 mat; \n
    uniform float depth; \n
    void main() {
        gl_Position =  mat * vec4(aPos.x, aPos.y, aPos.z + depth, 1.0);
    }
);

const char *fragmentShaderSource = MULTILINE_STR(
    \x23version 330 core\n
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
);

int main(void) {
    RGFW_setGLHint(RGFW_glMinor, 3);
    RGFW_setGLHint(RGFW_glMajor, 3);

	RGFW_window* window = RGFW_createWindow("RGFW OpenGL 3D", RGFW_RECT(0, 0, 800, 600), RGFW_windowCenter);
    RGFW_window_makeCurrent(window);
    
    // enable vsync (for ~60fps)
    RGFW_window_swapInterval(window, 1);

    if (gladLoadGL(RGFW_getProcAddress) == 0) {
        printf("failed to load modern opengl funcs\n");
        return 0;
    }
    
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {     
        -0.5f, -1.75f, -3.0f,  // bottom left
        0.0f,  -1.75f, -20.0f,  // top middle
        1.5f, -1.75f, -3.0f,  // bottom right
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
   
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    GLint depthLocation = glGetUniformLocation(shaderProgram, "depth");
    GLint matLocation = glGetUniformLocation(shaderProgram, "mat");
    mat4 matrix = loadIdentity();

    matrix = perspective(matrix.m, 60, (float)window->r.w / (float)window->r.h, 1, 1000);
        
    float playerDepth = 0.0f;

    while (RGFW_window_shouldClose(window) == RGFW_FALSE) {
        while (RGFW_window_checkEvent(window));
        if (RGFW_isPressed(window, RGFW_up))
           playerDepth += 1;
        if (RGFW_isPressed(window, RGFW_down))
           playerDepth -= 1;

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        
        glUniform1f(depthLocation, playerDepth);
        glUniformMatrix4fv(matLocation, 1, GL_FALSE, matrix.m); 
        glDrawArrays(GL_TRIANGLES, 0, 3);
                
        RGFW_window_swapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    RGFW_window_close(window);
    return 0;
}

mat4 mat4Multiply(float left[16], float right[16]) {
    return (mat4) {
        {
            left[0] * right[0] + left[1] * right[4] + left[2] * right[8] + left[3] * right[12],
            left[0] * right[1] + left[1] * right[5] + left[2] * right[9] + left[3] * right[13],
            left[0] * right[2] + left[1] * right[6] + left[2] * right[10] + left[3] * right[14],
            left[0] * right[3] + left[1] * right[7] + left[2] * right[11] + left[3] * right[15],
            left[4] * right[0] + left[5] * right[4] + left[6] * right[8] + left[7] * right[12],
            left[4] * right[1] + left[5] * right[5] + left[6] * right[9] + left[7] * right[13],
            left[4] * right[2] + left[5] * right[6] + left[6] * right[10] + left[7] * right[14],
            left[4] * right[3] + left[5] * right[7] + left[6] * right[11] + left[7] * right[15],
            left[8] * right[0] + left[9] * right[4] + left[10] * right[8] + left[11] * right[12],
            left[8] * right[1] + left[9] * right[5] + left[10] * right[9] + left[11] * right[13],
            left[8] * right[2] + left[9] * right[6] + left[10] * right[10] + left[11] * right[14],
            left[8] * right[3] + left[9] * right[7] + left[10] * right[11] + left[11] * right[15],
            left[12] * right[0] + left[13] * right[4] + left[14] * right[8] + left[15] * right[12],
            left[12] * right[1] + left[13] * right[5] + left[14] * right[9] + left[15] * right[13],
            left[12] * right[2] + left[13] * right[6] + left[14] * right[10] + left[15] * right[14],
            left[12] * right[3] + left[13] * right[7] + left[14] * right[11] + left[15] * right[15]
        }
    };
}


