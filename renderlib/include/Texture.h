//
//  Texture.h
//  VolumeRenderer
//
//  Created by Kristofer Schlachter on 6/10/14.
//
//

#ifndef __VolumeRenderer__Texture__
#define __VolumeRenderer__Texture__

#include <iostream>
#include <sstream>
#include <string>
#include "vmath.hpp"

namespace renderlib{
  
using namespace vmath;
  
class Texture
{
public:
  //FIXME: this should be in the renderer or some global class header
  enum DataType{
    UBYTE,
    USHORT,
    FLOAT16,
    FLOAT32
  };
  
public:
  Texture(int width, int height, DataType dataType);

  void load(std::string name, int unit);
  void loadTextureComplete();
  void activate();
  void deactivate();
  void bindFBO();
  void unbindFBO();
  void setupFBO(int width, int height, bool create, DataType dataType = UBYTE);
  void setupDepthFBO(int width, int height);
 
  //C++ 11 way of hiding these methods
  //Default constructor, no copy constructor or copy assignment operato
  Texture() = default;
  Texture(const Texture&) = delete;
  Texture & operator=(const Texture&) = delete;
  
protected:
  int glTexture;
  int glTextureUnit;
  void * image;
  std::string name;
  bool loaded;
  int renderbuffer;
  int framebuffer;
  bool fbo;
  bool depthBuffer;

  /*
  int glTexture = -1;
  int glTextureUnit = 0;
  void * image = {};
  std::string name = "";
  bool loaded = false;
  int renderbuffer = -1;
  int framebuffer = -1;
  bool fbo = false;
  bool depthBuffer = false;
  */
};

/*
void load(std::string name, int unit) {
  console.debug("Starting loadTexture " + name + " " + this.gl);
  this.name = name;
  this.glTexture = gl.createTexture(); 
  this.glTextureUnit = unit;
  this.image = new Image();
  var tex = this;
  this.image.onload = function() {tex.loadTextureComplete();};
  this.image.src = name;
}

void loadTextureComplete(){
  var gl = this.gl;
  gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, 1);
  gl.activeTexture(gl.TEXTURE0 + this.glTextureUnit);
  gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.image);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_NEAREST);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
  gl.generateMipmap(gl.TEXTURE_2D);
  this.loaded = true;
}

void activate() {
  if(this.loaded){
    gl.activeTexture(gl.TEXTURE0 + this.glTextureUnit);
    gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
  }
}

void deactivate() {
  if(this.loaded){
    gl.activeTexture(gl.TEXTURE0 + this.glTextureUnit);
    gl.bindTexture(gl.TEXTURE_2D, null);
  }
}


void bindFBO() {
  gl.bindTexture( gl.TEXTURE_2D, null);
  gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);
};

void unbindFBO() {
  gl.bindFramebuffer( gl.FRAMEBUFFER, null); 
};


void setupFBO(int width, int height, bool create, DataType dataType){
  this.fbo = true;
  this.loaded = true;
  if(dataType == undefined){
    dataType = gl.UNSIGNED_BYTE;
  }
  // 1. Init Color Texture 
  if(create){
    this.glTexture = gl.createTexture(); 
  }
  gl.bindTexture( gl.TEXTURE_2D, this.glTexture);
  gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
  gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
  gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri( gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE); 
  gl.texImage2D( gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, 
                 dataType, null);

  // 2. Init Render Buffer 
  if(create){
    this.renderbuffer = gl.createRenderbuffer();
  }
  gl.bindRenderbuffer( gl.RENDERBUFFER, this.renderbuffer); 
  gl.renderbufferStorage( gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, width, height);

  // 3. Init Frame Buffer
  if(create){
    this.framebuffer = gl.createFramebuffer();
  }
  gl.bindFramebuffer( gl.FRAMEBUFFER, this.framebuffer); 
  gl.framebufferTexture2D( gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D,
                           this.glTexture, 0);
  gl.framebufferRenderbuffer( gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, 
                              gl.RENDERBUFFER, this.renderbuffer);

  var stat = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
  if(stat != gl.FRAMEBUFFER_COMPLETE){
    throw("FBO Not Complete");
  }


  // 4. Cleanup
  gl.bindTexture(gl.TEXTURE_2D, null);
  gl.bindRenderbuffer(gl.RENDERBUFFER, null);
  gl.bindFramebuffer( gl.FRAMEBUFFER, null); 

};

void setupDepthFBO(int width, int height){
  this.fbo = true;
  this.loaded = true;
  this.depthBuffer = true;

  // Create a color texture
  var tmpTexture = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, tmpTexture);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, 
                gl.UNSIGNED_BYTE, null);

  // Create the depth texture
  this.glTexture = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  gl.texImage2D(gl.TEXTURE_2D, 0, gl.DEPTH_COMPONENT, width, height, 0, 
                gl.DEPTH_COMPONENT, gl.UNSIGNED_SHORT, null);

  this.framebuffer = gl.createFramebuffer();
  gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);
  gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, 
                          tmpTexture, 0);
  gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.TEXTURE_2D, 
                          this.glTexture, 0);

  if(!gl.checkFramebufferStatus(gl.FRAMEBUFFER) === gl.FRAMEBUFFER_COMPLETE) {
            console.error("Framebuffer incomplete!");
        }
        
  gl.bindFramebuffer(gl.FRAMEBUFFER, null);
};
*/
} //namespace renderlib


#endif /* defined(__VolumeRenderer__Texture__) */
