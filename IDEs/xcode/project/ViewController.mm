//
//  ViewController.m
//  VolumeRenderer
//
//  Created by Kristofer Schlachter on 6/6/14.
//  Copyright (c) 2014 Kristofer Schlachter. All rights reserved.
//

#import "ViewController.h"
#include "Platform.h"
#include "Test3DTexture.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// Uniform index.
enum
{
    UNIFORM_MODELVIEWPROJECTION_MATRIX,
    UNIFORM_NORMAL_MATRIX,
    NUM_UNIFORMS
};
GLint uniforms[NUM_UNIFORMS];

// Attribute index.
enum
{
    ATTRIB_VERTEX,
    ATTRIB_NORMAL,
    NUM_ATTRIBUTES
};

GLfloat gCubeVertexData[216] = 
{
    // Data layout for each line below is:
    // positionX, positionY, positionZ,     normalX, normalY, normalZ,
    0.5f, -0.5f, -0.5f,        1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, -0.5f,         1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f,         1.0f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f,         1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, -0.5f,          1.0f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f,         1.0f, 0.0f, 0.0f,
    
    0.5f, 0.5f, -0.5f,         0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f,        0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f,          0.0f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f,          0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f,        0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f,         0.0f, 1.0f, 0.0f,
    
    -0.5f, 0.5f, -0.5f,        -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,       -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f,         -1.0f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f,         -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,       -1.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, 0.5f,        -1.0f, 0.0f, 0.0f,
    
    -0.5f, -0.5f, -0.5f,       0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, -0.5f,        0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f,        0.0f, -1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f,        0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, -0.5f,        0.0f, -1.0f, 0.0f,
    0.5f, -0.5f, 0.5f,         0.0f, -1.0f, 0.0f,
    
    0.5f, 0.5f, 0.5f,          0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f,         0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f,         0.0f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f,         0.0f, 0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f,         0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f,        0.0f, 0.0f, 1.0f,
    
    0.5f, -0.5f, -0.5f,        0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,       0.0f, 0.0f, -1.0f,
    0.5f, 0.5f, -0.5f,         0.0f, 0.0f, -1.0f,
    0.5f, 0.5f, -0.5f,         0.0f, 0.0f, -1.0f,
    -0.5f, -0.5f, -0.5f,       0.0f, 0.0f, -1.0f,
    -0.5f, 0.5f, -0.5f,        0.0f, 0.0f, -1.0f
};

@interface ViewController () {
  
}

@property (strong, nonatomic) EAGLContext *context;
@property int width;
@property int height;

- (void)setupGL;
- (void)tearDownGL;
@end

@implementation ViewController

- (void)viewDidLoad
{
  [super viewDidLoad];
  
  self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];

  if (!self.context) {
    NSLog(@"Failed to create ES context");
  }
    
  GLKView *view = (GLKView *)self.view;
  view.context = self.context;
  //  _width = self.view.bounds.size.width;
  //  _height = self.view.bounds.size.height;
  _width = (int)[view drawableWidth];
  _height = (int)[view drawableHeight];
  
  
  //This is important for fill rate on Retina devices
  view.contentScaleFactor = 0.5f;
  
  view.multipleTouchEnabled = true;
  view.drawableColorFormat = GLKViewDrawableColorFormatRGB565;
  view.drawableDepthFormat = GLKViewDrawableDepthFormat16;
//  //  view.drawableMultisample = GLKViewDrawableMultisampleNone;
//  view.drawableMultisample = GLKViewDrawableMultisample4X;
//  
//  //  //Auto redraw code... calls render
  view.enableSetNeedsDisplay = NO;
  CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(renderNow:)];
  [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
  
  [self setupGL];
}

- (void)dealloc
{    
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];

    if ([self isViewLoaded] && ([[self view] window] == nil)) {
        self.view = nil;
        
        [self tearDownGL];
      
        if ([EAGLContext currentContext] == self.context) {
            [EAGLContext setCurrentContext:nil];
        }
        self.context = nil;
    }

    // Dispose of any resources that can be recreated.
}
- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {
  
  NSSet* touchSet = [event allTouches];
  if(touchSet.count == 1)
  {
//    keyboard('1', 0, 0);
  }else if(touchSet.count == 2)
  {
//    keyboard('2', 0, 0);
  }
  else if(touchSet.count == 3)
  {
//    keyboard('o', 0, 0);
  }
}

- (void)renderNow:(CADisplayLink*)displayLink {
  GLKView *view = (GLKView *)self.view;
  [view display];
}

- (void)setupGL
{
  [EAGLContext setCurrentContext:self.context];
  initialize();
  resizeViewport(_width, _height);

}

- (void)tearDownGL
{
  [EAGLContext setCurrentContext:self.context];
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
  //call render to my code
  float seconds = GetTicks()*0.001f;
  update(seconds);
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
  long newWidth, newHeight;
  newWidth = [view drawableWidth];
  newHeight = [view drawableHeight];
  if(newWidth != _width || newHeight != _height)
  {
    _width = (int)newWidth;
    _height = (int)newHeight;
    resizeViewport(_width, _height);
  }

  render();
}


@end
