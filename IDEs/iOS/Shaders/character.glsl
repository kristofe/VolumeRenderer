-- VS
#ifdef GL_ES
precision highp float;
#endif

// Declare our modelViewProjection matrix that we'll compute
//  outside the shader and set each frame
uniform mat4 modelViewProjectionMatrix;

// Declare inputs and outputs
// inPosition : Position attributes from the VAO/VBOs
// inTexcoord : Texcoord attributes from the VAO/VBOs
// varTexcoord : TexCoord we'll pass to the rasterizer
// gl_Position : implicitly declared in all vertex shaders. Clip space position
//               passed to rasterizer used to build the triangles

#if __VERSION__ >= 140
in vec4  inPosition;  
in vec2  inTexcoord;
out vec2 varTexcoord;
#else
attribute vec4 inPosition;  
attribute vec2 inTexcoord;
varying vec2 varTexcoord;
#endif

void main (void) 
{
	// Transform the vertex by the model view projection matrix so
	// the polygon shows up in the right place
	gl_Position	= modelViewProjectionMatrix * inPosition;
	
	// Pass the unmodified texture coordinate from the vertex buffer
	// directly down to the rasterizer.
    varTexcoord = inTexcoord;
}

-- FS
#ifdef GL_ES
precision highp float;
#endif

// Declare inputs and outputs
// varTexcoord : TexCoord for the fragment computed by the rasterizer based on
//               the varTexcoord values output in the vertex shader.
// gl_FragColor : Implicitly declare in fragments shaders less than 1.40.
//                Output color of our fragment.
// fragColor : Output color of our fragment.  Basically the same as gl_FragColor,
//             but we must explicitly declared this in shaders version 1.40 and
//             above.

#if __VERSION__ >= 140
in vec2      varTexcoord;
out vec4     fragColor;
#else
varying vec2 varTexcoord;
#endif

uniform sampler2D diffuseTexture;


void main (void)
{
	#if __VERSION__ >= 140
	fragColor = texture(diffuseTexture, varTexcoord.st, 0.0);
	#else
    gl_FragColor = texture2D(diffuseTexture, varTexcoord.st, 0.0);
	#endif
}
