/*
     File: ES2Renderer.m
 Abstract: n/a
  Version: 1.7
 */

#import "ES3Renderer.h"
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

@interface ES3Renderer (PrivateMethods)

@end

@implementation ES3Renderer

EAGLContext * m_context;

// The OpenGL names for the framebuffer and renderbuffer used to render to this view
GLuint m_colorRenderbuffer;
GLuint m_depthRenderbuffer;

// Create an ES 3.0 context
- (ES3Renderer* ) initWithContext:(EAGLContext *) context AndDrawable:(id<EAGLDrawable>)drawable
{	
	glGenFramebuffers(1, &m_defaultFBOName);
	
	// Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
	glGenRenderbuffers(1, &m_colorRenderbuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBOName);
	glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
	m_context = context;
	
	// This call associates the storage for the current render buffer with the EAGLDrawable (our CAEAGLLayer)
	// allowing us to draw into a buffer that will later be rendered to the screen wherever the layer is (which corresponds with our view).
	[m_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:drawable];
	
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorRenderbuffer);
	
	GLint backingWidth;
	GLint backingHeight;
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
	
	glGenRenderbuffers(1, &m_depthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		NSLog(@"failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		return NO;
	}
	
	self = [super initWithDefaultFBO:m_defaultFBOName];
	
	if (nil == self)
	{
		glDeleteFramebuffers(1, &m_defaultFBOName);
		glDeleteRenderbuffers(1, &m_colorRenderbuffer);
		glDeleteRenderbuffers(1, &m_depthRenderbuffer);
	}
	
	return self;
}

- (void)render {
    
    // Replace the implementation of this method to do your own custom drawing

    
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBOName);

	[super render];
	    
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
    [m_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer
{
	// The pixel dimensions of the CAEAGLLayer
	GLint backingWidth;
	GLint backingHeight;
	
	// Allocate color buffer backing based on the current layer size
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
    [m_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);
	
	glGenRenderbuffers(1, &m_depthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, backingWidth, backingHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
	
	[super resizeWithWidth:backingWidth AndHeight:backingHeight];
	
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
        NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        return NO;
    }
	
    return YES;
}

- (void) dealloc
{
	// tear down GL
	if (m_defaultFBOName)
	{
		glDeleteFramebuffers(1, &m_defaultFBOName);
		m_defaultFBOName = 0;
	}
	
	if (m_colorRenderbuffer)
	{
		glDeleteRenderbuffers(1, &m_colorRenderbuffer);
		m_colorRenderbuffer = 0;
	}
	
	[super dealloc];
}

@end
