TODO find out specifics of -fvisibility=hidden in static libs, which is the point of this

In general, all names that begin with "ui" and are followed by a capital letter and all names htat begin with "uipriv" and are followed by a capita lletter are reserved by libui. This applies even in C++, where name mangling may affect the actual names in the object file.

Within libui itself, the following rules apply:

C

C++

# Objective-C

Objective-C rules amend the C rules with the following.

libui should not expose any Objective-C classes or protocols of its own in the public API. This may be changed in the future; in the meantime, this section implies the `uipriv` prefix.

All class and protocol names must be prefixed, since there is only one namespace for these. This includes file-scope classes.

The following set of name prefixes are also reserved in addition to the regular `uipriv` prefix, each for different reasons; the reasons (and by extension, the correct usage) are described below.

- `initWithUipriv`
- `initWithFrame:uipriv`
- `isUipriv`
- `setUipriv`
- `_uipriv`

Each of these prefixes is also followed by an uppercase letter.

If an `init` method requires a custom method name (that is, it can't just use one of the superclass's), it should prefix its first name part with `initWithUipriv` instead of just `initWIth`. For instance, instead of

In the case of NSView subclasses, libui usually passes NSZeroRect as the initial frame, so if you need a custom init function, you could get away with embedding that directly in the `[super initWithFrame:]` call. (TODO should this rule be removed instead?) However, the `initWithFrame:uipriv` prefix is reserved it his is not sutable. (TODO maybe this isn't a good idea either...)

Examples:

```objective-c
// instead of...
- (id)initWithThing:(Thing *)thing measure:(int)value;
- (id)initWithFrame:(NSRect)r thing:(Thing *)thing;

// ...use
- (id)initWithUiprivThing:(Thing *)thing measure:(int)value;
- (id)initWithFrame:(NSRect)r uiprivThing:(Thing *)thing;
```

GObject
