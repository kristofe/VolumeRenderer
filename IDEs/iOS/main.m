#if 1 //ESSENTIAL_GL_PRACTICES_IOS
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

int main(int argc, char *argv[])
{
	int retVal = 1;
#if 1 //ESSENTIAL_GL_PRACTICES_IOS
    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    retVal = UIApplicationMain(argc, argv, nil, nil);
    [pool release];
#else
	retVal = NSApplicationMain(argc,  (const char **) argv);
#endif
	
    return retVal;
}
