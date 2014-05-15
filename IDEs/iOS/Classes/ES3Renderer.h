/*
     File: ES3Renderer.h
 Abstract: 
 The ES3Renderer class creates an OpenGL ES 3.0 context and delegates
 to its OS independent super class, OpenGLRenderer, for creating and 
 drawing objects.
 
  Version: 1.7
 
 */


#import <QuartzCore/QuartzCore.h>

#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import "OpenGLRenderer.h"

@interface ES3Renderer : OpenGLRenderer
{
	
}

- (ES3Renderer* ) initWithContext:(EAGLContext *) context AndDrawable:(id<EAGLDrawable>)drawable;
- (void)render;
- (BOOL)resizeFromLayer:(CAEAGLLayer*)layer;

@end

