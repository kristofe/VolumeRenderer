#import "EAGLView.h"

#import "OpenGLRenderer.h"

@implementation EAGLView

//@dynamic animationFrameInterval;

// You must implement this method
+ (Class) layerClass
{
    return [CAEAGLLayer class];
}

//The GL view is stored in the nib file. When it's unarchived it's sent -initWithCoder:
- (id) initWithCoder:(NSCoder*)coder
{    
    if ((self = [super initWithCoder:coder]))
	{
        // Get the layer
        CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking, kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat, nil];
		
		
        m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        
        if (!m_context || ![EAGLContext setCurrentContext:m_context])
		{
			[self release];
			return nil;
		}
		
		m_renderer = [[OpenGLRenderer alloc] initWithContext:m_context AndDrawable:(id<EAGLDrawable>)self.layer];
		
		if (!m_renderer)
		{
			[self release];
			return nil;
		}
        
		_animating = FALSE;
		displayLinkSupported = FALSE;
		animationFrameInterval = 1;
		displayLink = nil;
		animationTimer = nil;
		
		// A system version of 3.1 or greater is required to use CADisplayLink. The NSTimer
		// class is used as fallback when it isn't available.
		NSString *reqSysVer = @"3.1";
		NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
		if ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending)
			displayLinkSupported = TRUE;
    }
	
    _gameEngine = &(Game::GetInstance());
    
    return self;
}

- (void) drawView:(id)sender
{   
	[EAGLContext setCurrentContext:m_context];
    [m_renderer render];
  [self update];
}

- (void) layoutSubviews
{
	[m_renderer resizeFromLayer:(CAEAGLLayer*)self.layer];
    [self drawView:nil];
}

- (NSInteger) animationFrameInterval
{
	return animationFrameInterval;
}

- (void) setAnimationFrameInterval:(NSInteger)frameInterval
{
	// Frame interval defines how many display frames must pass between each time the
	// display link fires. The display link will only fire 30 times a second when the
	// frame internal is two on a display that refreshes 60 times a second. The default
	// frame interval setting of one will fire 60 times a second when the display refreshes
	// at 60 times a second. A frame interval setting of less than one results in undefined
	// behavior.
	if (frameInterval >= 1)
	{
		animationFrameInterval = frameInterval;
		
		if (_animating)
		{
			[self stopAnimation];
			[self startAnimation];
		}
	}
}

- (void) startAnimation
{
	if (!_animating)
	{
		if (displayLinkSupported)
		{
			// CADisplayLink is API new to iPhone SDK 3.1. Compiling against earlier versions will result in a warning, but can be dismissed
			// if the system version runtime check for CADisplayLink exists in -initWithCoder:. The runtime check ensures this code will
			// not be called in system versions earlier than 3.1.
			
			displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(drawView:)];
			[displayLink setFrameInterval:animationFrameInterval];
			[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
		}
		else
			animationTimer = [NSTimer scheduledTimerWithTimeInterval:(NSTimeInterval)((1.0 / 60.0) * animationFrameInterval) target:self selector:@selector(drawView:) userInfo:nil repeats:TRUE];
		
		_animating = TRUE;
	}
}

- (void)stopAnimation
{
	if (_animating)
	{
		if (displayLinkSupported)
		{
			[displayLink invalidate];
			displayLink = nil;
		}
		else
		{
			[animationTimer invalidate];
			animationTimer = nil;
		}
		
		_animating = FALSE;
	}
}

- (void) dealloc
{
    [m_renderer release];
	
	
	// tear down context
	if ([EAGLContext currentContext] == m_context)
        [EAGLContext setCurrentContext:nil];
	
	[m_context release];
	
    [super dealloc];
}


/*
 - (void) ClearRenderer{
 [_glView setCurrentContext];
 // Drawing code here.
 glEnable(GL_DEPTH_TEST);
 glClearColor(0.f, 0.f, 0.f, 0.0f);
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
 }
 */



- (void) update{
	_gameEngine->Update();
	_gameEngine->Render();
//	_renderer->Render();
	_gameEngine->LateUpdate();
}

// CONSTANTS
const float kAccelerometerFrequency		= 30.0f; //Hz
const float kUpdateFrequency			= 1.0f/120.0f; //Hz
const float kFilteringFactor			= 0.2f;
- (void) accelerometer:(UIAccelerometer*)accelerometer didAccelerate:(UIAcceleration*)acceleration {
	//Use a basic low-pass filter to only keep the gravity in the accelerometer values
	_accelerometer.x = acceleration.x * kFilteringFactor + _accelerometer.x * (1.0 - kFilteringFactor);
	_accelerometer.y = acceleration.y * kFilteringFactor + _accelerometer.y * (1.0 - kFilteringFactor);
	_accelerometer.z = acceleration.z * kFilteringFactor + _accelerometer.z * (1.0 - kFilteringFactor);
	
	
	
  
  
  
  float xx = fabs(_accelerometer.x);
  if(xx < 0.025f) xx = 0.025f;
  float angleForX = atan2(_accelerometer.y, xx);
  float angleForY = atan2(_accelerometer.z, -_accelerometer.x);
  
  
  
  _orientation.z = _accelerometer.x;
  _orientation.x = angleForX;
  _orientation.y = angleForY;
  
  
  _gameEngine->UpdateAccelerometerAndOrientation(_accelerometer,_orientation);
  
  
}

/*
 
 ////////////////////////////////////////////////////////////////////////////////////////////////////
 private const float AccelerometerUpdateInterval = 1.0f / 60.0f;
 private const float LowPassKernelWidthInSeconds = 0.1f;
 private const float LowPassFilterFactor = AccelerometerUpdateInterval / LowPassKernelWidthInSeconds; // tweakable
 private Vector3 lowPassValue = Vector3.zero; // should be initialized with 1st sample
 ////////////////////////////////////////////////////////////////////////////////////////////////////
 Vector3 LowPassFilter(Vector3 newSample) {
 lowPassValue = Vector3.Lerp(lowPassValue, newSample, LowPassFilterFactor);
 return lowPassValue;
 }
 
 if(iPhoneInput.accelerationEventCount > 0 &&  iPhoneInput.accelerationEventCount <= 5){
 Vector3 acceleration = Vector3.zero;
 
 int eventCount = iPhoneInput.accelerationEventCount;
 if(eventCount > 5){
 Debug.Log("iPhoneInput.accelerationEventCount > 10 " + iPhoneInput.accelerationEventCount);
 eventCount = 5;
 }
 float period = 0.0f;
 for (int i = 0; i < eventCount; ++i) {
 iPhoneAccelerationEvent accEvent  = iPhoneInput.GetAccelerationEvent(i);
 acceleration += accEvent.acceleration * accEvent.deltaTime;
 period += accEvent.deltaTime;
 LowPassFilter(accEvent.acceleration * accEvent.deltaTime);
 }
 if(period > 0)
 {
 acceleration *= 1.0f/period;
 }
 
 accelerator = lowPassValue;
 lastAccelerator = iPhoneInput.acceleration;
 
 
 
 float xx = Mathf.Abs(accelerator.x);
 if(xx < 0.025f) xx = 0.025f;
 float angleForX = Mathf.Atan2(accelerator.y, xx);
 float angleForY = Mathf.Atan2(accelerator.z, -accelerator.x);
 //float angleRadians = angle * 180.0f/3.14159f;
 
 
 accelerator.z = accelerator.x/(Mathf.PI);
 accelerator.x = -angleForX/(Mathf.PI);
 accelerator.y = -angleForY/(Mathf.PI);
 
 //float maxAcc = 0.1f;
 accelerator *= 0.55f;
 //accelerator.x = Mathf.Clamp(accelerator.x,-maxAcc,maxAcc);
 //accelerator.y = Mathf.Clamp(accelerator.y,-maxAcc,maxAcc);
 //accelerator.z = Mathf.Clamp(accelerator.z,-maxAcc,maxAcc);
 
 
 
 }
 }
 
 */





- (void) reshapeWithWidth:(int)w Height:(int)h
{
	mFrameWidth = w;
	mFrameHeight = h;
  
	//_renderer->SetWindowSize(mFrameWidth, mFrameHeight);
}


- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event{
	//Like a mouse down
	//Double tap
  UITouch       *touch = [touches anyObject];
	if([touch tapCount] == 2) {
		
		return;
	}
	
	NSArray* allTouches = touches.allObjects;
	for(unsigned int i = 0; i < [allTouches count]; i++){
		UITouch *touch = [allTouches objectAtIndex:i];
		CGPoint currentTouchPosition = [touch locationInView:self];
		int tapCountForTouch = [touch tapCount];
		_gameEngine->MouseDown(currentTouchPosition.x/((float)mFrameWidth),1.0 - (currentTouchPosition.y/((float)mFrameHeight)), i, tapCountForTouch);
	}
	
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event{
	//Like a mousedragged
	NSArray* allTouches = touches.allObjects;
	for(unsigned int i = 0; i < [allTouches count]; i++){
		UITouch *touch = [allTouches objectAtIndex:i];
		CGPoint currentTouchPosition = [touch locationInView:self];
		CGPoint lastTouchPosition = [touch previousLocationInView:self];
		
		_gameEngine->MouseDragged(currentTouchPosition.x/((float)mFrameWidth),1.0 - (currentTouchPosition.y/((float)mFrameHeight)),lastTouchPosition.x/((float)mFrameWidth),1.0 - (lastTouchPosition.y/((float)mFrameHeight)), i);
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event{
	//Like a mouse up
	NSArray* allTouches = touches.allObjects;
	for(unsigned int i = 0; i < [allTouches count]; i++){
		UITouch *touch = [allTouches objectAtIndex:i];
		CGPoint currentTouchPosition = [touch locationInView:self];
		
		_gameEngine->MouseUp(currentTouchPosition.x/((float)mFrameWidth),1.0 - (currentTouchPosition.y/((float)mFrameHeight)), i);
	}
}

- (void)didReceiveMemoryWarning:(UIApplication *)application  {
  _gameEngine->MemoryWarning();
  
  //[super didReceiveMemoryWarning];
}
@end
