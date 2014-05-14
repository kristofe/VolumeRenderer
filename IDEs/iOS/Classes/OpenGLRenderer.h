#include "glUtil.h"
#import <Foundation/Foundation.h>

@interface OpenGLRenderer : NSObject {
	
	GLuint m_defaultFBOName;

}

- (id) initWithDefaultFBO: (GLuint) defaultFBOName;
- (void) resizeWithWidth:(GLuint)width AndHeight:(GLuint)height;
- (void) render;
- (void) dealloc;

@end
