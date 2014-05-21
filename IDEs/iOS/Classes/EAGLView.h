#import <UIKit/UIKit.h>

#import "OpenGLRenderer.h"
#include "vmath.hpp"
//#include "Renderer.h"
#include "Game.h"

// This class wraps the CAEAGLLayer from CoreAnimation into a convenient UIView subclass.
// The view content is basically an EAGL surface you render your OpenGL scene into.
// Note that setting the view non-opaque will only work if the EAGL surface has an alpha channel.
@interface EAGLView : UIView<UIAccelerometerDelegate>
{
  vmath::Vector3				_orientation;
  vmath::Vector3				_accelerometer;
  //	NSTimer*				_renderTimer;
	
  //	Renderer*			_renderer;
	Game*				_gameEngine;
  
  int      mButtonDown;
  
  float    mMouseX;
  float    mMouseY;
  float    mMouseDownX;
  float    mMouseDownY;
	
	int		mFrameWidth;
	int		mFrameHeight;
	unsigned int mLastUpdateTime;
	unsigned int mStartTime;
	unsigned int mDeltaTime;
	unsigned int mCurrentTime;
  
@private
	OpenGLRenderer *m_renderer;
	
	EAGLContext *m_context;
	
	BOOL displayLinkSupported;
	NSInteger animationFrameInterval;
	// Use of the CADisplayLink class is the preferred method for controlling your animation timing.
	// CADisplayLink will link to the main display and fire every vsync when added to a given run-loop.
	// The NSTimer class is used only as fallback when running on a pre 3.1 device where CADisplayLink
	// isn't available.
	id displayLink;
    NSTimer *animationTimer;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;

- (void) startAnimation;
- (void) stopAnimation;
- (void) drawView:(id)sender;


- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event;
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event;
- (void) update;
- (void) reshapeWithWidth:(int)w Height:(int)h;
- (void)didReceiveMemoryWarning:(UIApplication *)application;
@end
