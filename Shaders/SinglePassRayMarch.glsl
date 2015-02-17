-- VS
#ifdef GL_ES
precision highp float;
#endif

in vec4 Position;
out vec4 vPosition;
uniform mat4 ModelviewProjection;

void main()
{
    gl_Position = ModelviewProjection * Position;
    vPosition = Position;
}

-- GS

layout(points) in;
layout(triangle_strip, max_vertices = 24) out;

in vec4 vPosition[1];

uniform mat4 ModelviewProjection;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 Modelview;

vec4 objCube[8]; // Object space coordinate of cube corner
vec4 ndcCube[8]; // Normalized device coordinate of cube corner
ivec4 faces[6];  // Vertex indices of the cube faces

void emit_vert(int vert)
{
    gl_Position = ndcCube[vert];
    EmitVertex();
}

void emit_face(int face)
{
    emit_vert(faces[face][1]); emit_vert(faces[face][0]);
    emit_vert(faces[face][3]); emit_vert(faces[face][2]);
    EndPrimitive();
}

void main()
{
    faces[0] = ivec4(0,1,3,2); faces[1] = ivec4(5,4,6,7);
    faces[2] = ivec4(4,5,0,1); faces[3] = ivec4(3,2,7,6);
    faces[4] = ivec4(0,3,4,7); faces[5] = ivec4(2,1,6,5);

    vec4 P = vPosition[0];
    vec4 I = vec4(1,0,0,0);
    vec4 J = vec4(0,1,0,0);
    vec4 K = vec4(0,0,1,0);

    objCube[0] = P+K+I+J; objCube[1] = P+K+I-J;
    objCube[2] = P+K-I-J; objCube[3] = P+K-I+J;
    objCube[4] = P-K+I+J; objCube[5] = P-K+I-J;
    objCube[6] = P-K-I-J; objCube[7] = P-K-I+J;

    // Transform the corners of the box:
    for (int vert = 0; vert < 8; vert++)
        ndcCube[vert] = ModelviewProjection * objCube[vert];

    // Emit the six faces:
    for (int face = 0; face < 6; face++)
        emit_face(face);
}

-- FS_CLOUD

out vec4 FragColor;

uniform sampler3D Density;
uniform vec3 LightPosition = vec3(0.25, 1.0, 3.0);
uniform vec3 LightIntensity = vec3(15.0);
uniform float Absorption = 2.0;
uniform mat4 Modelview;
uniform float FocalLength;
uniform vec2 WindowSize;
uniform vec3 RayOrigin;

const float maxDist = sqrt(2.0);
const int numSamples = 256;
const float stepSize = maxDist/float(numSamples);
const int numLightSamples = 32;
const float lscale = maxDist / float(numLightSamples);
const float densityFactor = 0.9;

struct Ray {
    vec3 Origin;
    vec3 Dir;
};

struct AABB {
    vec3 Min;
    vec3 Max;
};

bool IntersectBox(Ray r, AABB aabb, out float t0, out float t1)
{
    vec3 invR = 1.0 / r.Dir;
    vec3 tbot = invR * (aabb.Min-r.Origin);
    vec3 ttop = invR * (aabb.Max-r.Origin);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    t1 = min(t.x, t.y);
    return t0 <= t1;
}

void main()
{
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);

    vec3 rayDirection;
    rayDirection.xy = 2.0 * gl_FragCoord.xy / WindowSize - 1.0;
    rayDirection.z = -FocalLength;
    rayDirection = (vec4(rayDirection, 0) * Modelview).xyz;

    Ray eye = Ray( RayOrigin, normalize(rayDirection) );
    AABB aabb = AABB(vec3(-1.0), vec3(+1.0));

    float tnear, tfar;
    IntersectBox(eye, aabb, tnear, tfar);
    if (tnear < 0.0) tnear = 0.0;

    vec3 rayStart = eye.Origin + eye.Dir * tnear;
    vec3 rayStop = eye.Origin + eye.Dir * tfar;
    rayStart = 0.5 * (rayStart + 1.0);
    rayStop = 0.5 * (rayStop + 1.0);

    vec3 pos = rayStart;
    vec3 step = normalize(rayStop-rayStart) * stepSize;
    float travel = distance(rayStop, rayStart);
    float T = 1.0;
    vec3 Lo = vec3(0.0);

    for (int i=0; i < numSamples && travel > 0.0; ++i, pos += step, travel -= stepSize) {

        float density = texture(Density, pos).x * densityFactor;
        if (density <= 0.0)
            continue;

        T *= 1.0-density*stepSize*Absorption;
        if (T <= 0.01)
            break;

        vec3 lightDir = normalize(LightPosition-pos)*lscale;
        float Tl = 1.0;
        vec3 lpos = pos + lightDir;

        for (int s=0; s < numLightSamples; ++s) {
            float ld = texture(Density, lpos).x;
            Tl *= 1.0-Absorption*stepSize*ld;
            if (Tl <= 0.01)
            lpos += lightDir;
        }

        vec3 Li = LightIntensity*Tl;
        Lo += Li*T*density*stepSize;
    }

    FragColor.rgb = Lo;
    FragColor.a = 1-T;

}
-- FS_CTSCAN
#ifdef GL_ES
precision highp float;
#endif

out vec4 FragColor;

uniform mediump sampler3D Density;
uniform vec3 LightPosition = vec3(0.25, 1.0, 3.0);
uniform vec3 LightIntensity = vec3(15.0);
uniform float Absorption = 1.0;
uniform mat4 Modelview;
uniform float FocalLength;
uniform vec2 WindowSize;
uniform vec3 RayOrigin;
uniform float maxDensity = 1.0;
uniform float minDensity = 0.4;
const float maxDist = sqrt(2.0);
const int numSamples = 512;
const float stepSize = maxDist/float(numSamples);
const int numLightSamples = 0;
const float lscale = maxDist / float(numLightSamples);
const float densityFactor = 1.75;

struct Ray {
    vec3 Origin;
    vec3 Dir;
};

struct AABB {
    vec3 Min;
    vec3 Max;
};

bool IntersectBox(Ray r, AABB aabb, out float t0, out float t1)
{
    vec3 invR = 1.0 / r.Dir;
    vec3 tbot = invR * (aabb.Min-r.Origin);
    vec3 ttop = invR * (aabb.Max-r.Origin);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    t1 = min(t.x, t.y);
    return t0 <= t1;
}

void main()
{

    vec3 rayDirection;
    rayDirection.xy = 2.0 * gl_FragCoord.xy / WindowSize - 1.0;
    rayDirection.z = -FocalLength;
    rayDirection = (vec4(rayDirection, 0) * Modelview).xyz;

    Ray eye = Ray( RayOrigin, normalize(rayDirection) );
    AABB aabb = AABB(vec3(-1.0), vec3(+1.0));

    float tnear, tfar;
    IntersectBox(eye, aabb, tnear, tfar);
    if (tnear < 0.0) tnear = 0.0;

    vec3 rayStart = eye.Origin + eye.Dir * tnear;
    vec3 rayStop = eye.Origin + eye.Dir * tfar;
    rayStart = 0.5 * (rayStart + 1.0);
    rayStop = 0.5 * (rayStop + 1.0);

    vec3 pos = rayStart;
    vec3 step = normalize(rayStop-rayStart) * stepSize;
    float travel = distance(rayStop, rayStart);
    float T = 1.0;
    vec3 Lo = vec3(0.0);

    for (int i=0; i < numSamples && travel > 0.0; ++i, pos += step, travel -= stepSize) {

        float density = texture(Density, pos).x * densityFactor;
        /*
        if(density > 0.425){
          FragColor.rgb = vec3(density);
          FragColor.a = 1.0;
          break;
        }
        */
        if (density <= minDensity || density > maxDensity)
            continue;

        T *= 1.0-density*stepSize*Absorption;
        if (T <= 0.0)
            break;

        vec3 lightDir = normalize(LightPosition-pos)*lscale;
        float Tl = 1.0;
        vec3 lpos = pos + lightDir;

        for (int s=0; s < numLightSamples; ++s) {
            float ld = texture(Density, lpos).x;
            Tl *= 1.0-Absorption*stepSize*ld;
            if (Tl <= 0.01)
            lpos += lightDir;
        }

        vec3 Li = LightIntensity*Tl;
        Lo += Li*T*density*stepSize;

    }

    FragColor.rgb = Lo;
    FragColor.a = 1.0-T;

}


-- FS_ISO
#ifdef GL_ES
precision highp float;
#endif
out vec4 FragColor;

uniform highp sampler3D Density;
uniform vec3 LightPosition = vec3(0.25, 1.0, 3.0);
uniform vec3 LightIntensity = vec3(15.0);
uniform float Absorption = 1.0;
uniform mat4 Modelview;
uniform float FocalLength;
uniform vec2 WindowSize;
uniform vec3 RayOrigin;
uniform float maxDensity = 1.0;
uniform float minDensity = 0.4;
const float maxDist = sqrt(2.0);
const int numSamples = 1024;
const float stepSize = maxDist/float(numSamples);
const int numLightSamples = 0;
const float lscale = maxDist / float(numLightSamples);
const float densityFactor = 1.5;

struct Ray {
  vec3 Origin;
  vec3 Dir;
};

struct AABB {
  vec3 Min;
  vec3 Max;
};
//constants
const float DELTA = 0.01;			//the step size for gradient calculation
uniform float isoValue = 0.4;	//the isovalue for iso-surface detection

//function to give a more accurate position of where the given iso-value (iso) is found
//given the initial minimum limit (left) and maximum limit (right)
vec3 Bisection(vec3 left, vec3 right , float iso)
{
  //loop 4 times
  for(int i=0;i<4;i++)
  {
    //get the mid value between the left and right limit
    vec3 midpoint = (right + left) * 0.5;
    //sample the texture at the middle point
    float cM = texture(Density, midpoint).x ;
    //check if the value at the middle point is less than the given iso-value
    if(cM < iso)
    //if so change the left limit to the new middle point
    left = midpoint;
    else
    //otherwise change the right limit to the new middle point
    right = midpoint;
  }
  //finally return the middle point between the left and right limit
  return vec3(right + left) * 0.5;
}

//function to calculate the gradient at the given location in the volume dataset
//The function user center finite difference approximation to estimate the
//gradient
vec3 GetGradient(vec3 uvw)
{
  vec3 s1, s2;

  //Using center finite difference
  s1.x = texture(Density, uvw-vec3(DELTA,0.0,0.0)).x ;
  s2.x = texture(Density, uvw+vec3(DELTA,0.0,0.0)).x ;

  s1.y = texture(Density, uvw-vec3(0.0,DELTA,0.0)).x ;
  s2.y = texture(Density, uvw+vec3(0.0,DELTA,0.0)).x ;

  s1.z = texture(Density, uvw-vec3(0.0,0.0,DELTA)).x ;
  s2.z = texture(Density, uvw+vec3(0.0,0.0,DELTA)).x ;

  return normalize((s1-s2)/2.0);
}

//function to estimate the PhongLighting component given the light vector (L),
//the normal (N), the view vector (V), the specular power (specPower) and the
//given diffuse colour (diffuseColor). The diffuse component is first calculated
//Then, the half way vector is computed to obtain the specular component. Finally
//the diffuse and specular contributions are added together
vec4 PhongLighting(vec3 L, vec3 N, vec3 V, float specPower, vec3 diffuseColor)
{
  float diffuse = max(dot(L,N),0.0);
  vec3 halfVec = normalize(L+V);
  float specular = pow(max(0.00001,dot(halfVec,N)),specPower);
  //return vec4((diffuse*diffuseColor + specular),1.0);
  return vec4(diffuse,diffuse, diffuse,1.0);
}



bool IntersectBox(Ray r, AABB aabb, out float t0, out float t1)
{
  vec3 invR = 1.0 / r.Dir;
  vec3 tbot = invR * (aabb.Min-r.Origin);
  vec3 ttop = invR * (aabb.Max-r.Origin);
  vec3 tmin = min(ttop, tbot);
  vec3 tmax = max(ttop, tbot);
  vec2 t = max(tmin.xx, tmin.yz);
  t0 = max(t.x, t.y);
  t = min(tmax.xx, tmax.yz);
  t1 = min(t.x, t.y);
  return t0 <= t1;
}

void main()
{

  vec3 rayDirection;
  rayDirection.xy = 2.0 * gl_FragCoord.xy / WindowSize - 1.0;
  rayDirection.z = -FocalLength;
  rayDirection = (vec4(rayDirection, 0) * Modelview).xyz;

  Ray eye = Ray( RayOrigin, normalize(rayDirection) );
  AABB aabb = AABB(vec3(-1.0), vec3(+1.0));

  float tnear, tfar;
  IntersectBox(eye, aabb, tnear, tfar);
  if (tnear < 0.0) tnear = 0.0;

  vec3 rayStart = eye.Origin + eye.Dir * tnear;
  vec3 rayStop = eye.Origin + eye.Dir * tfar;
  rayStart = 0.5 * (rayStart + 1.0);
  rayStop = 0.5 * (rayStop + 1.0);

  vec3 pos = rayStart;
  vec3 step = normalize(rayStop-rayStart) * stepSize;
  float travel = distance(rayStop, rayStart);


  for (int i=0; i < numSamples && travel > 0.0; ++i, pos += step, travel -= stepSize) {

    float density = texture(Density, pos).x * densityFactor;

    float sample1 = texture(Density, pos).x;			//current sample
    float sample2 = texture(Density, pos+step).x;	//next sample

    //In case of iso-surface rendering, we do not use compositing.
    //Instead, we find the zero crossing of the volume dataset iso function
    //by sampling two consecutive samples.
    if( (sample1 -isoValue) < 0  && (sample2-isoValue) >= 0.0)  {
      //If there is a zero crossing, we refine the detected iso-surface
      //location by using bisection based refinement.
      vec3 xN = pos;
      vec3 xF = pos+step;
      vec3 tc = Bisection(xN, xF, isoValue);

      //This returns the first hit surface
      //vFragColor = make_float4(xN,1);

      //To get the shaded iso-surface, we first estimate the normal
      //at the refined position
      vec3 N = GetGradient(tc);

      //The view vector is simply opposite to the ray marching
      //direction
      vec3 V = -eye.Dir;

      //We keep the view vector as the light vector to give us a head
      //light
      vec3 L =  V;

      //Finally, we call PhongLighing function to get the final colour
      //with diffuse and specular components. Try changing this call to this
      //vFragColor =  PhongLighting(L,N,V,250,  tc); to get a multi colour
      //iso-surface
      FragColor =  PhongLighting(L,N,V,1000, vec3(0.3));
      return;
    }

  }
  FragColor = vec4(0);
}

