// 5 may 2016

// draw.cpp
extern ID2D1Factory *d2dfactory;
struct uiDrawContext {
	ID2D1RenderTarget *rt;
	std::vector<struct drawState> *states;
	ID2D1PathGeometry *currentClip;
};

// drawpath.cpp
extern ID2D1PathGeometry *pathGeometry(uiDrawPath *p);
