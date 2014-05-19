#include "glUtil_OLD.h"
#import <Foundation/Foundation.h>

@interface OpenGLRenderer : NSObject {
	
	GLuint m_defaultFBOName;
}

- (id) initWithDefaultFBO: (GLuint) defaultFBOName;
- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height;
- (void) render;
- (void) dealloc;
- (BOOL)resizeFromLayer:(CAEAGLLayer*)layer;
- (OpenGLRenderer* ) initWithContext:(EAGLContext *) context AndDrawable:(id<EAGLDrawable>)drawable;
@end