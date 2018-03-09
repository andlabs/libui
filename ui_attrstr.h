// uiAttribute stores information about an attribute in a
// uiAttributedString.
//
// You do not create uiAttributes directly; instead, you create a
// uiAttribute of a given type using the specialized constructor
// functions. For every Unicode codepoint in the uiAttributedString,
// at most one value of each attribute type can be applied.
//
// uiAttributes are immutable and the uiAttributedString takes
// ownership of the uiAttribute object once assigned, copying its
// contents as necessary.
typedef struct uiAttribute uiAttribute;

// uiFreeAttribute() frees a uiAttribute. You generally do not need to
// call this yourself, as uiAttributedString does this for you. In fact,
// it is an error to call this function on a uiAttribute that has been
// given to a uiAttributedString. You can call this, however, if you
// created a uiAttribute that you aren't going to use later.
_UI_EXTERN void uiFreeAttribute(uiAttribute *a);

// uiAttributeType holds the possible uiAttribute types that may be
// returned by uiAttributeGetType(). Refer to the documentation for
// each type's constructor function for details on each type.
_UI_ENUM(uiAttributeType) {
	uiAttributeTypeFamily,
	uiAttributeTypeSize,
	uiAttributeTypeWeight,
	uiAttributeTypeItalic,
	uiAttributeTypeStretch,
	uiAttributeTypeColor,
	uiAttributeTypeBackground,
	uiAttributeTypeUnderline,
	uiAttributeTypeUnderlineColor,
	uiAttributeTypeFeatures,
};

// uiAttributeGetType() returns the type of a.
// TODO I don't like this name
_UI_EXTERN uiAttributeType uiAttributeGetType(const uiAttribute *a);

// uiNewFamilyAttribute() creates a new uiAttribute that changes the
// font family of the text it is applied to. family is copied; you do not
// need to keep it alive after uiNewFamilyAttribute() returns. Font
// family names are case-insensitive.
_UI_EXTERN uiAttribute *uiNewFamilyAttribute(const char *family);

// uiAttributeFamily() returns the font family stored in a. The
// returned string is owned by a. It is an error to call this on a
// uiAttribute that does not hold a font family.
_UI_EXTERN const char *uiAttributeFamily(const uiAttribute *a);

// uiNewSizeAttribute() creates a new uiAttribute that changes the
// size of the text it is applied to, in typographical points.
_UI_EXTERN uiAttribute *uiNewSizeAttribute(double size);

// uiAttributeSize() returns the font size stored in a. It is an error to
// call this on a uiAttribute that does not hold a font size.
_UI_EXTERN double uiAttributeSize(const uiAttribute *a);

// uiTextWeight represents possible text weights. These roughly
// map to the OSx2 text weight field of TrueType and OpenType
// fonts, or to CSS weight numbers. The named constants are
// nominal values; the actual values may vary by font and by OS,
// though this isn't particularly likely. Any value between
// uiTextWeightMinimum and uiDrawTextWeightMaximum,
// inclusive, is allowed.
//
// Note that due to restrictions in early versions of Windows, some
// fonts have "special" weights be exposed in many programs as
// separate font families. This is perhaps most notable with
// Arial Black. libui does not do this, even on Windows (because the
// DirectWrite API libui uses on Windows does not do this); to
// specify Arial Black, use family Arial and weight uiTextWeightBlack.
_UI_ENUM(uiTextWeight) {
	uiTextWeightMinimum = 0,
	uiTextWeightThin = 100,
	uiTextWeightUltraLight = 200,
	uiTextWeightLight = 300,
	uiTextWeightBook = 350,
	uiTextWeightNormal = 400,
	uiTextWeightMedium = 500,
	uiTextWeightSemiBold = 600,
	uiTextWeightBold = 700,
	uiTextWeightUltraBold = 800,
	uiTextWeightHeavy = 900,
	uiTextWeightUltraHeavy = 950,
	uiTextWeightMaximum = 1000,
};

// uiNewWeightAttribute() creates a new uiAttribute that changes the
// weight of the text it is applied to. It is an error to specify a weight
// outside the range [uiTextWeightMinimum,
// uiTextWeightMaximum].
_UI_EXTERN uiAttribute *uiNewWeightAttribute(uiTextWeight weight);

// uiAttributeWeight() returns the font weight stored in a. It is an error
// to call this on a uiAttribute that does not hold a font weight.
_UI_EXTERN uiTextWeight uiAttributeWeight(const uiAttribute *a);

// uiTextItalic represents possible italic modes for a font. Italic
// represents "true" italics where the slanted glyphs have custom
// shapes, whereas oblique represents italics that are merely slanted
// versions of the normal glyphs. Most fonts usually have one or the
// other.
_UI_ENUM(uiTextItalic) {
	uiTextItalicNormal,
	uiTextItalicOblique,
	uiTextItalicItalic,
};

// uiNewItalicAttribute() creates a new uiAttribute that changes the
// italic mode of the text it is applied to. It is an error to specify an
// italic mode not specified in uiTextItalic.
_UI_EXTERN uiAttribute *uiNewItalicAttribute(uiTextItalic italic);

// uiAttributeItalic() returns the font italic mode stored in a. It is an
// error to call this on a uiAttribute that does not hold a font italic
// mode.
_UI_EXTERN uiTextItalic uiAttributeItalic(const uiAttribute *a);

// uiTextStretch represents possible stretches (also called "widths")
// of a font.
//
// Note that due to restrictions in early versions of Windows, some
// fonts have "special" stretches be exposed in many programs as
// separate font families. This is perhaps most notable with
// Arial Condensed. libui does not do this, even on Windows (because
// the DirectWrite API libui uses on Windows does not do this); to
// specify Arial Condensed, use family Arial and stretch
// uiTextStretchCondensed.
_UI_ENUM(uiTextStretch) {
	uiTextStretchUltraCondensed,
	uiTextStretchExtraCondensed,
	uiTextStretchCondensed,
	uiTextStretchSemiCondensed,
	uiTextStretchNormal,
	uiTextStretchSemiExpanded,
	uiTextStretchExpanded,
	uiTextStretchExtraExpanded,
	uiTextStretchUltraExpanded,
};

// uiNewStretchAttribute() creates a new uiAttribute that changes the
// stretch of the text it is applied to. It is an error to specify a strech
// not specified in uiTextStretch.
_UI_EXTERN uiAttribute *uiNewStretchAttribute(uiTextStretch stretch);

// uiAttributeStretch() returns the font stretch stored in a. It is an
// error to call this on a uiAttribute that does not hold a font stretch.
_UI_EXTERN uiTextStretch uiAttributeStretch(const uiAttribute *a);

// uiNewColorAttribute() creates a new uiAttribute that changes the
// color of the text it is applied to. It is an error to specify an invalid
// color.
_UI_EXTERN uiAttribute *uiNewColorAttribute(double r, double g, double b, double a);

// uiAttributeColor() returns the text color stored in a. It is an
// error to call this on a uiAttribute that does not hold a text color.
_UI_EXTERN void uiAttributeColor(const uiAttribute *a, double *r, double *g, double *b, double *alpha);

// uiNewBackgroundAttribute() creates a new uiAttribute that
// changes the background color of the text it is applied to. It is an
// error to specify an invalid color.
_UI_EXTERN uiAttribute *uiNewBackgroundAttribute(double r, double g, double b, double a);

// TODO reuse uiAttributeColor() for background colors, or make a new function...

// uiUnderline specifies a type of underline to use on text.
_UI_ENUM(uiUnderline) {
	uiUnderlineNone,
	uiUnderlineSingle,
	uiUnderlineDouble,
	uiUnderlineSuggestion,		// wavy or dotted underlines used for spelling/grammar checkers
};

// uiNewUnderlineAttribute() creates a new uiAttribute that changes
// the type of underline on the text it is applied to. It is an error to
// specify an underline type not specified in uiUnderline.
_UI_EXTERN uiAttribute *uiNewUnderlineAttribute(uiUnderline u);

// uiAttributeUnderline() returns the underline type stored in a. It is
// an error to call this on a uiAttribute that does not hold an underline
// style.
_UI_EXTERN uiUnderline uiAttributeUnderline(const uiAttribute *a);

// uiUnderlineColor specifies the color of any underline on the text it
// is applied to, regardless of the type of underline. In addition to
// being able to specify a custom color, you can explicitly specify
// platform-specific colors for suggestion underlines; to use them
// correctly, pair them with uiUnderlineSuggestion (though they can
// be used on other types of underline as well).
// 
// If an underline type is applied but no underline color is
// specified, the text color is used instead. If an underline color
// is specified without an underline type, the underline color
// attribute is ignored, but not removed from the uiAttributedString.
_UI_ENUM(uiUnderlineColor) {
	uiUnderlineColorCustom,
	uiUnderlineColorSpelling,
	uiUnderlineColorGrammar,
	uiUnderlineColorAuxiliary,		// for instance, the color used by smart replacements on macOS or in Microsoft Office
};

// uiNewUnderlineColorAttribute() creates a new uiAttribute that
// changes the color of the underline on the text it is applied to.
// It is an error to specify an underline color not specified in
// uiUnderlineColor.
//
// If the specified color type is uiUnderlineColorCustom, it is an
// error to specify an invalid color value. Otherwise, the color values
// are ignored and should be specified as zero.
_UI_EXTERN uiAttribute *uiNewUnderlineColorAttribute(uiUnderlineColor u, double r, double g, double b, double a);

// uiAttributeUnderlineColor() returns the underline color stored in
// a. It is an error to call this on a uiAttribute that does not hold an
// underline color.
_UI_EXTERN void uiAttributeUnderlineColor(const uiAttribute *a, uiUnderlineColor *u, double *r, double *g, double *b, double *alpha);

// uiOpenTypeFeatures represents a set of OpenType feature
// tag-value pairs, for applying OpenType features to text.
// OpenType feature tags are four-character codes defined by
// OpenType that cover things from design features like small
// caps and swashes to language-specific glyph shapes and
// beyond. Each tag may only appear once in any given
// uiOpenTypeFeatures instance. Each value is a 32-bit integer,
// often used as a Boolean flag, but sometimes as an index to choose
// a glyph shape to use.
// 
// If a font does not support a certain feature, that feature will be
// ignored. (TODO verify this on all OSs)
// 
// See the OpenType specification at
// https://www.microsoft.com/typography/otspec/featuretags.htm
// for the complete list of available features, information on specific
// features, and how to use them.
// TODO invalid features
typedef struct uiOpenTypeFeatures uiOpenTypeFeatures;

// uiOpenTypeFeaturesForEachFunc is the type of the function
// invoked by uiOpenTypeFeaturesForEach() for every OpenType
// feature in otf. Refer to that function's documentation for more
// details.
typedef uiForEach (*uiOpenTypeFeaturesForEachFunc)(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value, void *data);

// @role uiOpenTypeFeatures constructor
// uiNewOpenTypeFeatures() returns a new uiOpenTypeFeatures
// instance, with no tags yet added.
_UI_EXTERN uiOpenTypeFeatures *uiNewOpenTypeFeatures(void);

// @role uiOpenTypeFeatures destructor
// uiFreeOpenTypeFeatures() frees otf.
_UI_EXTERN void uiFreeOpenTypeFeatures(uiOpenTypeFeatures *otf);

// uiOpenTypeFeaturesClone() makes a copy of otf and returns it.
// Changing one will not affect the other.
_UI_EXTERN uiOpenTypeFeatures *uiOpenTypeFeaturesClone(const uiOpenTypeFeatures *otf);

// uiOpenTypeFeaturesAdd() adds the given feature tag and value
// to otf. The feature tag is specified by a, b, c, and d. If there is
// already a value associated with the specified tag in otf, the old
// value is removed.
_UI_EXTERN void uiOpenTypeFeaturesAdd(uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t value);

// uiOpenTypeFeaturesRemove() removes the given feature tag
// and value from otf. If the tag is not present in otf,
// uiOpenTypeFeaturesRemove() does nothing.
_UI_EXTERN void uiOpenTypeFeaturesRemove(uiOpenTypeFeatures *otf, char a, char b, char c, char d);

// uiOpenTypeFeaturesGet() determines whether the given feature
// tag is present in otf. If it is, *value is set to the tag's value and
// nonzero is returned. Otherwise, zero is returned.
// 
// Note that if uiOpenTypeFeaturesGet() returns zero, value isn't
// changed. This is important: if a feature is not present in a
// uiOpenTypeFeatures, the feature is NOT treated as if its
// value was zero anyway. Script-specific font shaping rules and
// font-specific feature settings may use a different default value
// for a feature. You should likewise not treat a missing feature as
// having a value of zero either. Instead, a missing feature should
// be treated as having some unspecified default value.
_UI_EXTERN int uiOpenTypeFeaturesGet(const uiOpenTypeFeatures *otf, char a, char b, char c, char d, uint32_t *value);

// uiOpenTypeFeaturesForEach() executes f for every tag-value
// pair in otf. The enumeration order is unspecified. You cannot
// modify otf while uiOpenTypeFeaturesForEach() is running.
_UI_EXTERN void uiOpenTypeFeaturesForEach(const uiOpenTypeFeatures *otf, uiOpenTypeFeaturesForEachFunc f, void *data);

// uiNewFeaturesAttribute() creates a new uiAttribute that changes
// the font family of the text it is applied to. otf is copied; you may
// free it after uiNewFeaturesAttribute() returns.
_UI_EXTERN uiAttribute *uiNewFeaturesAttribute(const uiOpenTypeFeatures *otf);

// uiAttributeFeatures() returns the OpenType features stored in a.
// The returned uiOpenTypeFeatures object is owned by a. It is an
// error to call this on a uiAttribute that does not hold OpenType
// features.
_UI_EXTERN const uiOpenTypeFeatures *uiAttributeFeatures(const uiAttribute *a);

// uiAttributedString represents a string of UTF-8 text that can
// optionally be embellished with formatting attributes. libui
// provides the list of formatting attributes, which cover common
// formatting traits like boldface and color as well as advanced
// typographical features provided by OpenType like superscripts
// and small caps. These attributes can be combined in a variety of
// ways.
//
// Attributes are applied to runs of Unicode codepoints in the string.
// Zero-length runs are elided. Consecutive runs that have the same
// attribute type and value are merged. Each attribute is independent
// of each other attribute; overlapping attributes of different types
// do not split each other apart, but different values of the same
// attribute type do.
//
// The empty string can also be represented by uiAttributedString,
// but because of the no-zero-length-attribute rule, it will not have
// attributes.
//
// A uiAttributedString takes ownership of all attributes given to
// it, as it may need to duplicate or delete uiAttribute objects at
// any time. By extension, when you free a uiAttributedString,
// all uiAttributes within will also be freed. Each method will
// describe its own rules in more details.
//
// In addition, uiAttributedString provides facilities for moving
// between grapheme clusters, which represent a character
// from the point of view of the end user. The cursor of a text editor
// is always placed on a grapheme boundary, so you can use these
// features to move the cursor left or right by one "character".
// TODO does uiAttributedString itself need this
//
// uiAttributedString does not provide enough information to be able
// to draw itself onto a uiDrawContext or respond to user actions.
// In order to do that, you'll need to use a uiDrawTextLayout, which
// is built from the combination of a uiAttributedString and a set of
// layout-specific properties.
typedef struct uiAttributedString uiAttributedString;

// uiAttributedStringForEachAttributeFunc is the type of the function
// invoked by uiAttributedStringForEachAttribute() for every
// attribute in s. Refer to that function's documentation for more
// details.
typedef uiForEach (*uiAttributedStringForEachAttributeFunc)(const uiAttributedString *s, const uiAttribute *a, size_t start, size_t end, void *data);

// @role uiAttributedString constructor
// uiNewAttributedString() creates a new uiAttributedString from
// initialString. The string will be entirely unattributed.
_UI_EXTERN uiAttributedString *uiNewAttributedString(const char *initialString);

// @role uiAttributedString destructor
// uiFreeAttributedString() destroys the uiAttributedString s.
// It will also free all uiAttributes within.
_UI_EXTERN void uiFreeAttributedString(uiAttributedString *s);

// uiAttributedStringString() returns the textual content of s as a
// '\0'-terminated UTF-8 string. The returned pointer is valid until
// the next change to the textual content of s.
_UI_EXTERN const char *uiAttributedStringString(const uiAttributedString *s);

// uiAttributedStringLength() returns the number of UTF-8 bytes in
// the textual content of s, excluding the terminating '\0'.
_UI_EXTERN size_t uiAttributedStringLen(const uiAttributedString *s);

// uiAttributedStringAppendUnattributed() adds the '\0'-terminated
// UTF-8 string str to the end of s. The new substring will be
// unattributed.
_UI_EXTERN void uiAttributedStringAppendUnattributed(uiAttributedString *s, const char *str);

// uiAttributedStringInsertAtUnattributed() adds the '\0'-terminated
// UTF-8 string str to s at the byte position specified by at. The new
// substring will be unattributed; existing attributes will be moved
// along with their text.
_UI_EXTERN void uiAttributedStringInsertAtUnattributed(uiAttributedString *s, const char *str, size_t at);

// TODO add the Append and InsertAtExtendingAttributes functions
// TODO and add functions that take a string + length

// uiAttributedStringDelete() deletes the characters and attributes of
// s in the byte range [start, end).
_UI_EXTERN void uiAttributedStringDelete(uiAttributedString *s, size_t start, size_t end);

// TODO add a function to uiAttributedString to get an attribute's value at a specific index or in a specific range, so we can edit

// uiAttributedStringSetAttribute() sets a in the byte range [start, end)
// of s. Any existing attributes in that byte range of the same type are
// removed. s takes ownership of a; you should not use it after
// uiAttributedStringSetAttribute() returns.
_UI_EXTERN void uiAttributedStringSetAttribute(uiAttributedString *s, uiAttribute *a, size_t start, size_t end);

// uiAttributedStringForEachAttribute() enumerates all the
// uiAttributes in s. It is an error to modify s in f. Within f, s still
// owns the attribute; you can neither free it nor save it for later
// use.
// TODO reword the above for consistency (TODO and find out what I meant by that)
// TODO define an enumeration order (or mark it as undefined); also define how consecutive runs of identical attributes are handled here and sync with the definition of uiAttributedString itself
_UI_EXTERN void uiAttributedStringForEachAttribute(const uiAttributedString *s, uiAttributedStringForEachAttributeFunc f, void *data);

// TODO const correct this somehow (the implementation needs to mutate the structure)
_UI_EXTERN size_t uiAttributedStringNumGraphemes(uiAttributedString *s);

// TODO const correct this somehow (the implementation needs to mutate the structure)
_UI_EXTERN size_t uiAttributedStringByteIndexToGrapheme(uiAttributedString *s, size_t pos);

// TODO const correct this somehow (the implementation needs to mutate the structure)
_UI_EXTERN size_t uiAttributedStringGraphemeToByteIndex(uiAttributedString *s, size_t pos);

// uiFontDescriptor provides a complete description of a font where
// one is needed. Currently, this means as the default font of a
// uiDrawTextLayout and as the data returned by uiFontButton.
// All the members operate like the respective uiAttributes.
typedef struct uiFontDescriptor uiFontDescriptor;

struct uiFontDescriptor {
	// TODO const-correct this or figure out how to deal with this when getting a value
	char *Family;
	double Size;
	uiTextWeight Weight;
	uiTextItalic Italic;
	uiTextStretch Stretch;
};

// uiDrawTextLayout is a concrete representation of a
// uiAttributedString that can be displayed in a uiDrawContext.
// It includes information important for the drawing of a block of
// text, including the bounding box to wrap the text within, the
// alignment of lines of text within that box, areas to mark as
// being selected, and other things.
//
// Unlike uiAttributedString, the content of a uiDrawTextLayout is
// immutable once it has been created.
//
// TODO talk about OS-specific differences with text drawing that libui can't account for...
typedef struct uiDrawTextLayout uiDrawTextLayout;

// uiDrawTextAlign specifies the alignment of lines of text in a
// uiDrawTextLayout.
_UI_ENUM(uiDrawTextAlign) {
	uiDrawTextAlignLeft,
	uiDrawTextAlignCenter,
	uiDrawTextAlignRight,
};

// uiDrawTextLayoutParams describes a uiDrawTextLayout.
// DefaultFont is used to render any text that is not attributed
// sufficiently in String. Width determines the width of the bounding
// box of the text; the height is determined automatically.
typedef struct uiDrawTextLayoutParams uiDrawTextLayoutParams;

// TODO const-correct this somehow
struct uiDrawTextLayoutParams {
	uiAttributedString *String;
	uiFontDescriptor *DefaultFont;
	double Width;
	uiDrawTextAlign Align;
};

// @role uiDrawTextLayout constructor
// uiDrawNewTextLayout() creates a new uiDrawTextLayout from
// the given parameters.
//
// TODO
// - allow creating a layout out of a substring
// - allow marking compositon strings
// - allow marking selections, even after creation
// - add the following functions:
// 	- uiDrawTextLayoutHeightForWidth() (returns the height that a layout would need to be to display the entire string at a given width)
// 	- uiDrawTextLayoutRangeForSize() (returns what substring would fit in a given size)
// 	- uiDrawTextLayoutNewWithHeight() (limits amount of string used by the height)
// - some function to fix up a range (for text editing)
_UI_EXTERN uiDrawTextLayout *uiDrawNewTextLayout(uiDrawTextLayoutParams *params);

// @role uiDrawFreeTextLayout destructor
// uiDrawFreeTextLayout() frees tl. The underlying
// uiAttributedString is not freed.
_UI_EXTERN void uiDrawFreeTextLayout(uiDrawTextLayout *tl);

// uiDrawText() draws tl in c with the top-left point of tl at (x, y).
_UI_EXTERN void uiDrawText(uiDrawContext *c, uiDrawTextLayout *tl, double x, double y);

// uiDrawTextLayoutExtents() returns the width and height of tl
// in width and height. The returned width may be smaller than
// the width passed into uiDrawNewTextLayout() depending on
// how the text in tl is wrapped. Therefore, you can use this
// function to get the actual size of the text layout.
_UI_EXTERN void uiDrawTextLayoutExtents(uiDrawTextLayout *tl, double *width, double *height);

// uiDrawTextLayoutNumLines() returns the number of lines in tl.
// This number will always be greater than or equal to 1; a text
// layout with no text only has one line.
_UI_EXTERN int uiDrawTextLayoutNumLines(uiDrawTextLayout *tl);

// uiDrawTextLayoutLineByteRange() returns the byte indices of the
// text that falls into the given line of tl as [start, end).
_UI_EXTERN void uiDrawTextLayoutLineByteRange(uiDrawTextLayout *tl, int line, size_t *start, size_t *end);

// TODO metrics functions

// TODO number of lines visible for clipping rect, range visible for clipping rect?

typedef struct uiFontButton uiFontButton;
#define uiFontButton(this) ((uiFontButton *) (this))
// TODO have a function that sets an entire font descriptor to a range in a uiAttributedString at once, for SetFont?
_UI_EXTERN void uiFontButtonFont(uiFontButton *b, uiFontDescriptor *desc);
// TOOD SetFont, mechanics
_UI_EXTERN void uiFontButtonOnChanged(uiFontButton *b, void (*f)(uiFontButton *, void *), void *data);
_UI_EXTERN uiFontButton *uiNewFontButton(void);
