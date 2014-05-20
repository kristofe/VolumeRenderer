-- VS
#ifdef GL_ES
precision highp float;
#endif

in vec3 vert;
in vec3 color;
uniform mat4 modelview;
uniform float time;
out vec3 inuv;

void main() {
  gl_Position = modelview * vec4(vert, 1);
  inuv = vec3(color.xy,sin(time)*0.5 + 0.5);
}

-- FS
#ifdef GL_ES
precision highp float;
#endif
in vec3 inuv;
out vec4 finalColor;
uniform mediump sampler3D Density;

void main() {

  //vec3 color = texture(Density, inuv).xxx;
  //finalColor = vec4(1.0, 1.0, 1.0,color.x);
  finalColor = vec4(inuv,1.0);
}