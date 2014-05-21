#import <UIKit/UIKit.h>


@class EAGLView;
@class MainViewController;

@interface VolumeRenderIOSAppDelegate : NSObject <UIApplicationDelegate> {

}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet EAGLView *glView;
@property (nonatomic, retain) IBOutlet MainViewController *mainViewController;


@end

