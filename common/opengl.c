// 28 may 2016

#include <stdlib.h>
#include "../ui.h"
#include "uipriv.h"

uiOpenGLAttributes *uiNewOpenGLAttributes() {
    uiOpenGLAttributes *a = uiprivAlloc(sizeof(uiOpenGLAttributes), "uiOpenGLAttributes");
    if (a == NULL)
        return NULL;

    //TODO document availability on different platforms
    // compare with wxWidgets/glcanvas

    a->RedBits = a->GreenBits = a->BlueBits = a->AlphaBits = 8;
    a->DepthBits = 24;
    a->StencilBits = 8;
    a->Stereo = 0;
    a->Samples = 0;
    a->SRGBCapable = 0;
    a->DoubleBuffer = 1;
    a->UseOpenGLES = 0;
    a->MajorVersion = 1;
    a->MinorVersion = 0;
    a->ForwardCompat = 0;
    a->DebugContext = 0;
    a->CompatProfile = 0;
    a->Robustness = 0;

    return a;
}

void uiFreeOpenGLAttributes(uiOpenGLAttributes *attribs) {
    uiprivFree(attribs);
}

void uiOpenGLAttributesSetAttribute(uiOpenGLAttributes *attribs, uiOpenGLAttribute attribute, int value) {
    //TODO validate bool params
    switch (attribute) {
    case uiOpenGLAttributeRedBits:
        attribs->RedBits = value;
        break;
    case uiOpenGLAttributeGreenBits:
        attribs->GreenBits = value;
        break;
    case uiOpenGLAttributeBlueBits:
        attribs->BlueBits = value;
        break;
    case uiOpenGLAttributeAlphaBits:
        attribs->AlphaBits = value;
        break;
    case uiOpenGLAttributeDepthBits:
        attribs->DepthBits = value;
        break;
    case uiOpenGLAttributeStencilBits:
        attribs->StencilBits = value;
        break;
    case uiOpenGLAttributeStereo:
        attribs->Stereo = value;
        break;
    case uiOpenGLAttributeSamples:
        attribs->Samples = value;
        break;
    case uiOpenGLAttributeSRGBCapable:
        attribs->SRGBCapable = value;
        break;
    case uiOpenGLAttributeDoubleBuffer:
        attribs->DoubleBuffer = value;
        break;
    case uiOpenGLAttributeUseOpenGLES:
        attribs->UseOpenGLES = value;
        break;
    case uiOpenGLAttributeMajorVersion:
        attribs->MajorVersion = value;
        break;
    case uiOpenGLAttributeMinorVersion:
        attribs->MinorVersion = value;
        break;
    case uiOpenGLAttributeForwardCompat:
        attribs->ForwardCompat = value;
        break;
    case uiOpenGLAttributeDebugContext:
        attribs->DebugContext = value;
        break;
    case uiOpenGLAttributeCompatProfile:
        attribs->CompatProfile = value;
        break;
    case uiOpenGLAttributeRobustness:
        attribs->Robustness = value;
        break;
    default:
        uiprivUserBug("Invalid OpenGL attribute");
    }
}

