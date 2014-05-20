#import "OpenGLRenderer.h"
#include "OpenGLHelper.h"
#include "Platform.h"
#include "Test3DTexture.h"

#define GetGLError()									\
{														\
	GLenum err = glGetError();							\
	while (err != GL_NO_ERROR) {						\
		NSLog(@"GLError %s set in File:%s Line:%d\n",	\
				GetGLErrorString(err),					\
				__FILE__,								\
				__LINE__);								\
		err = glGetError();								\
	}													\
}


#ifndef NULL
#define NULL 0
#endif

@implementation OpenGLRenderer

GLuint m_viewWidth;
GLuint m_viewHeight;


EAGLContext * m_context;

// The OpenGL names for the framebuffer and renderbuffer used to render to this view
GLuint m_colorRenderbuffer;
GLuint m_depthRenderbuffer;

// Create an ES 3.0 context
- (OpenGLRenderer* ) initWithContext:(EAGLContext *) context AndDrawable:(id<EAGLDrawable>)drawable
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
	
	//self = [super initWithDefaultFBO:m_defaultFBOName];
	self = [self initWithDefaultFBO:m_defaultFBOName];
	
	if (nil == self)
	{
		glDeleteFramebuffers(1, &m_defaultFBOName);
		glDeleteRenderbuffers(1, &m_colorRenderbuffer);
		glDeleteRenderbuffers(1, &m_depthRenderbuffer);
	}
	
	return self;
}

- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height
{
	glViewport(0, 0, width, height);

	m_viewWidth = width;
	m_viewHeight = height;
  
  resizeViewport(width, height);
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
	
	[self resizeWithWidth:backingWidth AndHeight:backingHeight];
	
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    return NO;
  }
	
  return YES;
}


- (id) initWithDefaultFBO: (GLuint) defaultFBOName
{
	if((self = [super init]))
	{
		NSLog(@"%s %s", glGetString(GL_RENDERER), glGetString(GL_VERSION));
		
		////////////////////////////////////////////////////
		// Build all of our and setup initial state here  //
		// Don't wait until our real time run loop begins //
		////////////////////////////////////////////////////
		
		m_defaultFBOName = defaultFBOName;
//    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBOName);
		
		m_viewWidth = 100;
		m_viewHeight = 100;
        
    //From VolumeRender
    initialize();
    
  }

	return self;
}

- (void) render
{
  
    //call render to my code
    float seconds = GetTicks()*0.001f;
    update(seconds);
    render();
  
    glBindRenderbuffer(GL_RENDERBUFFER, m_colorRenderbuffer);
    [m_context presentRenderbuffer:GL_RENDERBUFFER];
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
	
	
    //cleanup my code
    
	[super dealloc];
}

@end


