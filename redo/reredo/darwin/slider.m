// 14 august 2015
#import "uipriv_darwin.h"

struct uiSlider {
	uiDarwinControl c;
	NSSlider *slider;
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
};

@interface sliderDelegateClass : NSObject {
	NSMapTable *sliders;
}
- (IBAction)onChanged:(id)sender;
- (void)registerSlider:(uiSlider *)b;
- (void)unregisterSlider:(uiSlider *)b;
@end

@implementation sliderDelegateClass

- (id)init
{
	self = [super init];
	if (self)
		self->sliders = newMap();
	return self;
}

- (void)dealloc
{
	if ([self->sliders count] != 0)
		complain("attempt to destroy shared slider delegate but sliders are still registered to it");
	[self->sliders release];
	[super dealloc];
}

- (IBAction)onChanged:(id)sender
{
	uiSlider *s;

	s = (uiSlider *) mapGet(self->sliders, sender);
	(*(s->onChanged))(s, s->onChangedData);
}

- (void)registerSlider:(uiSlider *)s
{
	mapSet(self->sliders, s->slider, s);
	[s->slider setTarget:self];
	[s->slider setAction:@selector(onChanged:)];
}

- (void)unregisterSlider:(uiSlider *)s
{
	[s->slider setTarget:nil];
	[self->sliders removeObjectForKey:s->slider];
}

@end

static sliderDelegateClass *sliderDelegate = nil;

uiDarwinDefineControlWithOnDestroy(
	uiSlider,								// type name
	uiSliderType,							// type function
	slider,								// handle
	[sliderDelegate unregisterSlider:this];		// on destroy
)

intmax_t uiSliderValue(uiSlider *s)
{
	// NSInteger is the most similar to intmax_t
	return [s->slider integerValue];
}

void uiSliderSetValue(uiSlider *s, intmax_t value)
{
	[s->slider setIntegerValue:value];
}

void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

static void defaultOnChanged(uiSlider *s, void *data)
{
	// do nothing
}

uiSlider *uiNewSlider(intmax_t min, intmax_t max)
{
	uiSlider *s;
	NSSliderCell *cell;

	s = (uiSlider *) uiNewControl(uiSliderType());

	s->slider = [[NSSlider alloc] initWithFrame:NSZeroRect];
	// TODO vertical is defined by wider than tall
	[s->slider setMinValue:min];
	[s->slider setMaxValue:max];
	[s->slider setAllowsTickMarkValuesOnly:NO];
	[s->slider setNumberOfTickMarks:0];
	[s->slider setTickMarkPosition:NSTickMarkAbove];

	cell = (NSSliderCell *) [s->slider cell];
	[cell setSliderType:NSLinearSlider];

	if (sliderDelegate == nil) {
		sliderDelegate = [sliderDelegateClass new];
		[delegates addObject:sliderDelegate];
	}
	[sliderDelegate registerSlider:s];
	uiSliderOnChanged(s, defaultOnChanged, NULL);

	uiDarwinFinishNewControl(s, uiSlider);

	return s;
}
