#include "Gui.hh"

#include "../Synthle.hh"
#include "../index.html.h"

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

                m_plugin->updateParamFromMainThread(id, value);
            }
            else if ([action isEqualToString:@"initializeValues"])
            {
                m_plugin->m_gui->updateGuiValues();
            }
            else if ([action isEqualToString:@"sendPluginInfo"])
            {
                [self sendPluginInfo];
            }
            else if ([action isEqualToString:@"getParameterInfo"])
            {
                // [self
                // std::cout << "init" << std::endl;
                // m_plugin->m_gui->updateGuiValues();
            }
        }
    }
}

// This method calls a function in JavaScript
- (void)sendPluginInfo
{

    for (int i = 0; i < m_plugin->m_parameters.size(); i++)
    {
        auto &param = m_plugin->m_parameters[i];

        NSString *name = [NSString stringWithCString:param.name.c_str() encoding:NSASCIIStringEncoding];
        NSString *param_id = [NSString stringWithCString:param.jsId.c_str() encoding:NSASCIIStringEncoding];
        NSString *unit = [NSString stringWithCString:param.unit.c_str() encoding:NSASCIIStringEncoding];

        // Builds the string to execute
        NSString *js = [NSString
            stringWithFormat:@"linkParameter({handle: %i, name: '%@', id: '%@', minValue: '%f', maxValue: "
                             @"'%f', defaultValue: '%f', step: %f, unit: '%@'});",
                             i, name, param_id, param.minValue, param.maxValue, param.defaultValue, param.step, unit];
        [self.webView evaluateJavaScript:js
                       completionHandler:^(id _Nullable result, NSError *_Nullable error) {
                         if (error)
                         {
                             // std::cout << "error executing js" << std::endl;
                             // NSLog(@"Error executing JavaScript: %@", error.localizedDescription);
                         }
                       }];
    }

    // Updates all the parameters
    const auto &params = m_plugin->m_parameters;
    for (int i = 0; i < m_plugin->m_parameters.size(); i++)
    {
        std::cout << params[i].audio_thread.value << std::endl;
        [self updateParamWithId:i andValue:params[i].audio_thread.value];
    }
}

// This method calls a function in JavaScript
- (void)updateParamWithId:(int)i andValue:(float)value
{
    // Get the JS ID for the parameter.
    auto &param = m_plugin->m_parameters[i];
    NSString *param_id = [NSString stringWithCString:param.jsId.c_str() encoding:NSASCIIStringEncoding];

    NSString *js = [NSString stringWithFormat:@"receiveParamValueUpdate('%@', '%f');", param_id, value];
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
    m_pimpl = new SynthleGuiPimpl;
    m_pimpl->m_htmlView = [[HTMLView alloc] initWithFrame:NSMakeRect(0, 0, GUI_WIDTH, GUI_HEIGHT) plugin:plugin];
    std::string html = (const char *)index_html;
    NSString *html_ns_string = [NSString stringWithCString:html.c_str() encoding:NSASCIIStringEncoding];
    [m_pimpl->m_htmlView loadHTMLContent:html_ns_string];
    m_plugin = plugin;
}

SynthleGui::~SynthleGui()
{
    m_pimpl->stopDisplayLink();
    [m_pimpl->m_htmlView release];
    delete m_pimpl;
}

void SynthleGui::updateGuiValues()
{
}

void SynthleGui::paint()
{
    if (!m_plugin->syncAudioToMain())
        return;

    const auto &params = m_plugin->m_parameters;
    for (int i = 0; i < m_plugin->m_parameters.size(); i++)
    {
        std::cout << params[i].gui_thread.value << std::endl;
        [m_pimpl->m_htmlView updateParamWithId:i andValue:params[i].gui_thread.value];
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
