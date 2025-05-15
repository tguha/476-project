#include <glad/glad.h>

class TextureManager {
public:
    static void initFallbacks(GLuint whiteID, GLuint flatNormalID, GLuint blackID) {
        get()._white = whiteID;
        get()._normal = flatNormalID;
        get()._black = blackID;
    }
    static GLuint white() { return get()._white; }
    static GLuint flatNormal() { return get()._normal; }
    static GLuint black() { return get()._black; }

private:
    GLuint _white = 0;
    GLuint _normal = 0;
    GLuint _black = 0;
    static TextureManager& get() {
        static TextureManager s;
        return s;
    }
};
