-- VS
#ifdef GL_ES
precision highp float;
#endif

in vec3 vert;
in vec3 color;
uniform mat4 modelview;
out vec4 incolor;

void main() {
    // does not alter the vertices at all
    gl_Position = modelview * vec4(vert, 1);
    //gl_Position = vec4(vert, 1);
    //gl_Color = vec4(color,1);
    incolor = vec4(color,1);
}

-- FS
#ifdef GL_ES
precision highp float;
#endif
in vec4 incolor;
out vec4 finalColor;

void main() {
    //set every drawn pixel to white
    //finalColor = vec4(1.0, 1.0, 1.0, 1.0);
    finalColor = incolor;
}