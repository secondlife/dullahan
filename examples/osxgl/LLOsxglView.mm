/*
 @brief Dullahan - a headless browser rendering engine
 based around the Chromium Embedded Framework

 Example: simple OSX OpenGL example

 Copyright (c) 2016, Linden Research, Inc.

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */


#import "LLOsxglView.h"


#include <OpenGL/gl.h>

static const int textureWidth = 1024;
static const int textureHeight = 1024;
static GLuint textureHandle = 0;

static LLOsxglView *gCurrent = nil;

static void onPageChangedCallback(const unsigned char *pixels, int x, int y, int width, int height)
{
    [[LLOsxglView current] onPageChangedCallbackPixels:pixels x:x y:y width:width height:height];
}

static void onRequestExitCallback() {
    [LLOsxglView current].needsShutdown = YES;
}

@implementation LLOsxglView

@synthesize openGLContext = _openGLContext;
@synthesize mDullahan = _mDullahan;
@synthesize timer = _timer;
@synthesize isClosing = _isClosing;
@synthesize needsShutdown = _needsShutdown;

+ (LLOsxglView *)current {
    return gCurrent;
}

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        [self setupView];
    }
    return self;
}

- (instancetype)initWithFrame:(NSRect)frameRect {
    self = [super initWithFrame:frameRect];
    if (self) {
        [self setupView];
    }
    return self;
}

- (void)setupView {
    gCurrent = self;

    self.layerContentsRedrawPolicy = NSViewLayerContentsRedrawDuringViewResize;
    self.postsBoundsChangedNotifications = YES;
    self.postsFrameChangedNotifications = YES;
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(viewGlobalFrameDidChange:) name:NSViewGlobalFrameDidChangeNotification object:self];

    NSTrackingAreaOptions options = NSTrackingActiveAlways | NSTrackingInVisibleRect | NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved;
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:self.bounds
                                                                options:options
                                                                  owner:self
                                                               userInfo:nil];

    [self addTrackingArea:trackingArea];

    _mDullahan = new dullahan();

    _mDullahan->setOnPageChangedCallback(std::bind(&onPageChangedCallback,
                                                        std::placeholders::_1,
                                                        std::placeholders::_2,
                                                        std::placeholders::_3,
                                                        std::placeholders::_4,
                                                        std::placeholders::_5));

    _mDullahan->setOnRequestExitCallback(std::bind(&onRequestExitCallback));

    dullahan::dullahan_settings settings;
    settings.initial_width = 1024;
    settings.initial_height = 1024;
    settings.javascript_enabled = true;
    settings.cache_enabled = true;
    settings.cache_path = "/tmp/cef_cachez";
    settings.cookies_enabled = true;
    settings.cookie_store_path = "/tmp/cef_cookiez";
    settings.user_agent_substring = "osxgl";
    settings.accept_language_list = "en-us";
    settings.autoplay_without_gesture = true;

    bool result = _mDullahan->init(settings);
    if (result) {
        _mDullahan->setSize(1024, 1024);

        _mDullahan->navigate("https://sl-viewer-media-system.s3.amazonaws.com/index.html");

        _timer = [[NSTimer scheduledTimerWithTimeInterval:0.016
                                                   target:self
                                                 selector:@selector(update:)
                                                 userInfo:nil
                                                  repeats:YES] retain];
    } else {
        NSLog(@"Failed to init Dullahan.");
        delete _mDullahan;
        _mDullahan = NULL;
    }
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];

    [_timer invalidate];
    [_timer release];
    _timer = nil;

    gCurrent = nil;

    delete _mDullahan;
    _mDullahan = NULL;

    [super dealloc];
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)flagsChanged:(NSEvent *)theEvent {
    if (self.mDullahan) {
        self.mDullahan->nativeKeyboardEventOSX(theEvent);
    }
}

-(void)setLayer:(CALayer*)layer
{
    [super setLayer:layer];
    [self.openGLContext update];
}

- (void)sendKeyEvent:(NSEvent*)theEvent
{
        dullahan::EKeyEvent event_type;
        if ([theEvent type] == NSKeyDown)
            event_type = dullahan::KE_KEY_DOWN;
        else
            event_type = dullahan::KE_KEY_UP;

        uint32_t event_modifiers = [theEvent modifierFlags];
        uint32_t event_keycode = [theEvent keyCode];

        uint32_t event_chars = 0;
        NSString* c = [theEvent characters];
        if ([c length] > 0)
        {
            event_chars = (uint32_t)[c characterAtIndex:0];
        }

        uint32_t event_umodchars = 0;
        NSString* cim = [theEvent charactersIgnoringModifiers];
        if ([cim length] > 0)
        {
            event_umodchars = (uint32_t)[cim characterAtIndex:0];
        }

        bool event_isrepeat = [theEvent isARepeat];

        self.mDullahan->nativeKeyboardEventOSX(event_type, event_modifiers, event_keycode, event_chars,
                                               event_umodchars, event_isrepeat);
}

- (void)keyDown:(NSEvent *)theEvent {
    if (self.mDullahan) {
        // version that uses NSEvent (much cleaner but not all consumers have this)
        // self.mDullahan->nativeKeyboardEventOSX(theEvent);

        // version that uses individual values for testing
        [self sendKeyEvent:theEvent];
    }
}

- (void)keyUp:(NSEvent *)theEvent {
    if (self.mDullahan) {
        // version that uses NSEvent (much cleaner but not all consumers have this)
        // self.mDullahan->nativeKeyboardEventOSX(theEvent);

        // version that uses individual values for testing
        [self sendKeyEvent:theEvent];
    }
}

- (void)mouseDown:(NSEvent *)theEvent {
    if (self.mDullahan) {
        const int x = [self getTextureMouseX:theEvent];
        const int y = [self getTextureMouseY:theEvent];
        self.mDullahan->mouseButton(dullahan::MB_MOUSE_BUTTON_LEFT, dullahan::ME_MOUSE_DOWN, x, y);
        self.mDullahan->setFocus();
    }
}

- (void)mouseDragged:(NSEvent *)theEvent {
    if (self.mDullahan) {
        const int x = [self getTextureMouseX:theEvent];
        const int y = [self getTextureMouseY:theEvent];
        self.mDullahan->mouseMove(x, y);
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    if (self.mDullahan) {
        const int x = [self getTextureMouseX:theEvent];
        const int y = [self getTextureMouseY:theEvent];
        self.mDullahan->mouseButton(dullahan::MB_MOUSE_BUTTON_LEFT, dullahan::ME_MOUSE_UP, x, y);
    }
}

- (void)mouseMoved:(NSEvent *)theEvent {
    if (self.mDullahan) {
        const int x = [self getTextureMouseX:theEvent];
        const int y = [self getTextureMouseY:theEvent];
        self.mDullahan->mouseMove(x, y);
    }
}

- (void)rightMouseUp:(NSEvent *)theEvent {
    if (self.mDullahan) {
    }
}

- (void)scrollWheel:(NSEvent *)theEvent {
    if (self.mDullahan) {
        const int dx = 3 * theEvent.deltaX;
        const int dy = 3 * theEvent.deltaY;
        self.mDullahan->mouseWheel(dx, dy);
    }
}

- (void)viewGlobalFrameDidChange:(NSNotification *)notification {
    [self.openGLContext update];
}

- (void)update:(NSTimer *)timer {
    if (self.mDullahan) {
        if (self.needsShutdown) {
            [self.timer invalidate];
            [self.timer release];
            self.timer = nil;

            [self.window close];

            self.mDullahan->shutdown();

            [[NSApplication sharedApplication] terminate:self];
        } else {
            self.mDullahan->update();
        }
    }
}

- (void)onPageChangedCallbackPixels:(const unsigned char *)pixels x:(int)x y:(int)y width:(int)width height:(int)height {
    if (width != textureWidth) {
        NSLog(@"onPageChagnedCallback width does not match.");
        return;
    }
    if (height != textureHeight) {
        NSLog(@"onPageChangedCallback height does not match.");
        return;
    }

    if (self.openGLContext) {
        [self.openGLContext makeCurrentContext];

        glBindTexture(GL_TEXTURE_2D, textureHandle);

        glTexSubImage2D(GL_TEXTURE_2D, // target
                        0, // level
                        x, // xoffset
                        y, // yoffset
                        width, // width
                        height, // height
                        GL_BGRA_EXT, // format
                        GL_UNSIGNED_BYTE, // type
                        pixels); // pixels

        [self setNeedsDisplay:YES];
    }
}

- (void)drawRect:(NSRect)dirtyRect {
    if (!self.openGLContext) {
        NSOpenGLPixelFormatAttribute attrs[] = {
            NSOpenGLPFADoubleBuffer,
            NSOpenGLPFADepthSize, 32,
            0
        };

        NSOpenGLPixelFormat *openGLPixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];

        NSOpenGLContext *openGLContext = [[NSOpenGLContext alloc] initWithFormat:openGLPixelFormat
                                                                    shareContext:nil];

        [openGLContext setView:self];

        [openGLContext makeCurrentContext];

        glGenTextures(1, &textureHandle);
        glBindTexture(GL_TEXTURE_2D, textureHandle);
        glTexImage2D(GL_TEXTURE_2D, // target
                     0, // level
                     GL_RGBA, // internalformat
                     textureWidth, // width
                     textureHeight, // height
                     0, // border
                     GL_RGBA, // format
                     GL_UNSIGNED_BYTE, // type
                     NULL); // pixels

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glViewport(0, 0, NSWidth(self.bounds), NSHeight(self.bounds));
        glOrtho(0.0, textureWidth, textureHeight, 0.0, -1.0, 1.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        self.openGLContext = openGLContext;
    }

    [self.openGLContext makeCurrentContext];

    glClear(GL_COLOR_BUFFER_BIT);

    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    // 0,1    1,1

    // 0,0    1,0


    glTexCoord2f(1.0f, 1.0f);
    glVertex2d(textureWidth, textureHeight);

    glTexCoord2f(0.0f, 1.0f);
    glVertex2d(0.0f, textureHeight);

    glTexCoord2f(0.0f, 0.0);
    glVertex2d(0.0f, 0.0);

    glTexCoord2f(1.0f, 0.0f);
    glVertex2d(textureWidth, 0.0);

    glEnd();

    [self.openGLContext flushBuffer];
}

#pragma mark - NSWindowDelegate

- (BOOL)windowShouldClose:(id)sender {
    if (!self.isClosing) {
        self.isClosing = YES;
        if (self.mDullahan) {
            self.mDullahan->requestExit();
        }
        return NO;
    } else {
        return YES;
    }
}

#pragma mark - Private Methods

- (int)getTextureMouseX:(NSEvent *)theEvent {
    NSPoint locationInView = [self convertPoint:theEvent.locationInWindow fromView:nil];
    return textureWidth * locationInView.x / NSWidth(self.bounds);
}

- (int)getTextureMouseY:(NSEvent *)theEvent {
    NSPoint locationInView = [self convertPoint:theEvent.locationInWindow fromView:nil];
    return textureHeight - (textureHeight * locationInView.y / NSHeight(self.bounds));
}

-(IBAction)setCookie:(id)sender {
    if (self.mDullahan) {
        self.mDullahan->setCookie("http://sasm.com", "my_cookie_sasm", "my_cookie_value_sasm", ".sasm.com", "/", true, true);
    }
}


-(IBAction)showDevTools:(id)sender {
    if (self.mDullahan) {
        self.mDullahan->showDevTools();
    }
}

@end
