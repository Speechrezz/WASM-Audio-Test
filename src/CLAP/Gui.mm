#include "Gui.hh"

#include "../Synthle.hh"

#import <WebKit/WebKit.h>
#include <iostream>

// Note: For Speechrezz: in Objective-C if you inherit from multiple classes
// you add the subsecquent classes in angle-brackets. This is not a template.
@interface HTMLView : NSView <WKScriptMessageHandler>
{
    Synthle *m_plugin;
}
@property(nonatomic, strong) WKWebView *webView;
- (void)loadHTMLContent:(NSString *)htmlString;
@end

@implementation HTMLView
- (instancetype)initWithFrame:(NSRect)frameRect plugin:(Synthle *)plugin
{
    self = [super initWithFrame:frameRect];
    if (self)
    {
        WKWebViewConfiguration *config = [[WKWebViewConfiguration alloc] init];

        // Create a user content controller to manage script messages
        WKUserContentController *userContentController = [[WKUserContentController alloc] init];

        // Add the script message handler, using the name "jsHandler"
        [userContentController addScriptMessageHandler:self name:@"jsHandler"];

        config.userContentController = userContentController;

        // Initialize the web view with the configuration
        self.webView = [[WKWebView alloc] initWithFrame:self.bounds configuration:config];
        self.webView.inspectable = true;
        self.webView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;

        [self addSubview:self.webView];
        m_plugin = plugin;
    }
    return self;
}

// This method that receives the update from JavaScript.
- (void)userContentController:(WKUserContentController *)userContentController
      didReceiveScriptMessage:(WKScriptMessage *)message
{
    // Handle the message from JavaScript
    if ([message.name isEqualToString:@"jsHandler"])
    {
        NSDictionary *message_body = message.body;

        if ([message_body isKindOfClass:[NSDictionary class]])
        {
            NSString *action = message_body[@"action"];
            if ([action isEqualToString:@"sendParamValueUpdate"])
            {
                NSString *id_str = message_body[@"id"];
                NSString *param_str = message_body[@"value"];

                int id = id_str.intValue;
                float value = param_str.floatValue;

                m_plugin->updateParamMainThread(id, value);
            }
            else if ([action isEqualToString:@"initializeValues"])
            {
                std::cout << "init" << std::endl;
                m_plugin->m_gui->updateGuiValues();
            }
        }
    }
}

// This method calls a function in JavaScript
- (void)updateParamWithId:(int)param_id andValue:(float)value
{
    // Builds the string to execute
    NSString *js = [NSString stringWithFormat:@"receiveParamValueUpdate('%i', '%f');", param_id, value];
    [self.webView evaluateJavaScript:js
                   completionHandler:^(id _Nullable result, NSError *_Nullable error) {
                     if (error)
                     {
                         // std::cout << "error executing js" << std::endl;
                         // NSLog(@"Error executing JavaScript: %@", error.localizedDescription);
                     }
                   }];
}

- (void)showAlertWithMessage:(NSString *)message
{
    NSAlert *alert = [[NSAlert alloc] init];
    alert.messageText = message;
    [alert runModal];
}

- (void)loadHTMLContent:(NSString *)htmlString
{
    [self.webView loadHTMLString:htmlString baseURL:nil];
}

@end

// Performs the redrawing of the window.
static int display_link_callback(CVDisplayLinkRef displayLink, const CVTimeStamp *now, const CVTimeStamp *outputTime,
                                 CVOptionFlags flagsIn, CVOptionFlags *flagsOut, void *userData)
{
    dispatch_async(dispatch_get_main_queue(), ^{
      auto *gui = (__bridge xynth::SynthleGui *)userData;
      gui->paint();
    });
    (void)displayLink, (void)now, (void)outputTime, (void)flagsIn, (void)flagsOut;
    return kCVReturnSuccess;
}

namespace xynth
{

struct SynthleGuiPimpl
{
    HTMLView *m_htmlView = nullptr;
    CVDisplayLinkRef m_displayLink;

    void startDisplayLink(void *user_ptr)
    {
        // If we are already running don't do anything.
        if (m_displayLink)
            return;
        CVDisplayLinkCreateWithActiveCGDisplays(&m_displayLink);
        CVDisplayLinkSetOutputCallback(m_displayLink, &display_link_callback, (__bridge void *)user_ptr);
        CVDisplayLinkStart(m_displayLink);
    }
    void stopDisplayLink()
    {
        if (m_displayLink)
        {
            NSLog(@"Stopping display link");
            CVDisplayLinkStop(m_displayLink);
            CVDisplayLinkRelease(m_displayLink);
            m_displayLink = nullptr;
        }
    }
};

SynthleGui::SynthleGui(Synthle *plugin)
{
    std::string html = R"""(
<!DOCTYPE html>
<html>
<head>
    <title>Test</title>
</head>
<body>
    <h1>Hello, World!</h1>
    Choose volume:

    <input style="width: 300px;" type="range" min="0" max="100" value="30" class="slider" id="volume">
    <span id="volumeValue">30%</span><br>

    <input style="width: 300px;" type="range" min="0" max="100" value="30" class="slider" id="volume">
    <span id="volumeValue">30%</span><br>

    <script>
    function sendParamValueUpdate(_id, _value) {
        window.webkit.messageHandlers.jsHandler.postMessage({
            action: "sendParamValueUpdate",
            id: _id,
            value: _value
        });
    }

    function receiveParamValueUpdate(_id, _value) {
        // FIXME: Ignoring _id right now.
        const volumeSlider = document.querySelector('#volume');
        document.querySelector('#volumeValue').innerHTML = parseInt(_value * 100) + '%';
        volumeSlider.value = _value * 100;
    }

    const volumeSlider = document.querySelector('#volume');
    volumeSlider.oninput = (e) => {
      document.querySelector('#volumeValue').innerHTML = parseInt(e.target.value) + '%';
      let value = parseFloat(e.target.value) / 100;
      sendParamValueUpdate(0, value);
    };

    document.addEventListener("DOMContentLoaded", function(event) {
        // Send a request to fully update parameters.
        window.webkit.messageHandlers.jsHandler.postMessage({
            action: "initializeValues",
        });
        getParam();
    });

    </script>
</body>
</html>
    )""";
    m_pimpl = new SynthleGuiPimpl;
    m_pimpl->m_htmlView = [[HTMLView alloc] initWithFrame:NSMakeRect(0, 0, GUI_WIDTH, GUI_HEIGHT) plugin:plugin];

    NSString *html_ns_string = [NSString stringWithCString:html.c_str()];
    [m_pimpl->m_htmlView loadHTMLContent:html_ns_string];
    m_plugin = plugin;
}

SynthleGui::~SynthleGui()
{
    m_pimpl->stopDisplayLink();
    delete m_pimpl;
}

void SynthleGui::updateGuiValues()
{
    // Updates all the parameters
    const auto &params = m_plugin->m_parameters_main_thread;
    for (int i = 0; i < m_plugin->m_real_parameters.size(); i++)
    {
        std::cout << params[i].value << std::endl;
        [m_pimpl->m_htmlView updateParamWithId:i andValue:params[i].value];
    }
}

void SynthleGui::paint()
{
    if (!m_plugin->syncAudioToMain())
        return;

    const auto &params = m_plugin->m_parameters_main_thread;
    for (int i = 0; i < m_plugin->m_real_parameters.size(); i++)
    {
        std::cout << params[i].value << std::endl;
        [m_pimpl->m_htmlView updateParamWithId:i andValue:params[i].value];
    }
}

void SynthleGui::setParent(const void *p)
{
    if (m_pimpl->m_htmlView.superview != nil)
        [m_pimpl->m_htmlView removeFromSuperview];

    [(NSView *)p addSubview:(NSView *)m_pimpl->m_htmlView];

    // required to get mouse move events in Ableton Live (and possibly other hosts)
    [[(NSView *)p window] setAcceptsMouseMovedEvents:YES];
    [[(NSView *)p window] makeFirstResponder:(NSView *)m_pimpl->m_htmlView];
    [m_pimpl->m_htmlView setNeedsDisplay:YES];
}

void SynthleGui::setVisible(bool show)
{
    if (show)
    {
        m_pimpl->startDisplayLink(this);
        [m_pimpl->m_htmlView setHidden:NO];
    }
    else
    {
        m_pimpl->stopDisplayLink();
        [m_pimpl->m_htmlView setHidden:YES];
    }
}
} // namespace xynth
