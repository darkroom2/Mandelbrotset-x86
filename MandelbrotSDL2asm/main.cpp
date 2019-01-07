#include <iostream>
#include <SDL.h>

extern "C" void mandelbrotASM(void* pBuffer, double pWidth, double pHeight, double CxMin, double CyMin, long long width, long long height);

unsigned map(unsigned in_min, unsigned in_max, unsigned out_min, unsigned out_max, unsigned x) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

class Display {
	SDL_Surface *surf;
	SDL_Window *window;
	SDL_Renderer *rend;

	void mandelbrot(double CxMin, double CyMin, double pixelWidth, double pixelHeight) {
		auto bpp = surf->format->BytesPerPixel;
		auto pitch = surf->pitch;
		auto pixelsBegin = surf->pixels;

		int iX, iY;
		const int iXmax = surf->w;
		const int iYmax = surf->h;

		double Cx, Cy;

		double Zx, Zy;
		double Zx2, Zy2;

		int Iteration;
		const int IterationMax = 127;

		double ER2 = 4.0;

		for (iY = 0; iY < iYmax; iY++) {
			Cy = CyMin + iY * pixelHeight;

			if (fabs(Cy) < pixelHeight / 2) Cy = 0.0;

			for (iX = 0; iX < iXmax; iX++) {
				Cx = CxMin + iX * pixelWidth;

				Zx = 0.0;
				Zy = 0.0;
				Zx2 = Zx * Zx;
				Zy2 = Zy * Zy;

				for (Iteration = 0; Iteration < IterationMax && ((Zx2 + Zy2) < ER2); Iteration++) {
					Zy = 2 * Zx*Zy + Cy;
					Zx = Zx2 - Zy2 + Cx;
					Zx2 = Zx * Zx;
					Zy2 = Zy * Zy;
				};

				uint8_t *p = static_cast<uint8_t *>(pixelsBegin) + iY * pitch + iX * bpp;

				if (Iteration == IterationMax) {
					p[0] = 255;
					p[1] = 255;
					p[2] = 255;
				}
				else {
					auto x = map(0, IterationMax, 0, 255, Iteration);
					p[0] = x;
					p[1] = x;
					p[2] = x;
				}
			}
		}
	}

	void draw(SDL_Texture *texture) {
		//SDL_SetRenderDrawColor(rend, 255, 0, 0, SDL_ALPHA_OPAQUE);
		//SDL_RenderClear(rend);
		SDL_RenderCopy(rend, texture, NULL, &surf->clip_rect);
		SDL_RenderPresent(rend);
		SDL_DestroyTexture(texture);
	}

public:
	Display(const char *path) {
		SDL_Init(SDL_INIT_VIDEO);

		surf = SDL_LoadBMP(path);

		window = SDL_CreateWindow(
			"Mandelbrot Set",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			surf->w,
			surf->h,
			SDL_WINDOW_OPENGL
		);

		rend = SDL_CreateRenderer(
			window,
			-1,
			SDL_RENDERER_ACCELERATED |
			SDL_RENDERER_PRESENTVSYNC
		);
	}

	void cleanUp() {
		SDL_DestroyRenderer(rend);
		SDL_DestroyWindow(window);
		SDL_FreeSurface(surf);
		SDL_Quit();
	}

	void showMandelbrotSet(double CxMin, double CyMin, double pixelWidth, double pixelHeight) {
		auto pixelz = surf->pixels;
		auto width = surf->w;
		auto height = surf->h;
		auto CxMin2 = CxMin;
		auto CyMin2 = CyMin;
		auto pixelWidth2 = pixelWidth;
		auto pixelHeight2 = pixelHeight;
		mandelbrotASM(pixelz, CyMin, pixelWidth2, pixelHeight2, CxMin, width, height);
		//mandelbrot(CxMin, CyMin, pixelWidth, pixelHeight);
		SDL_Texture *texture = SDL_CreateTextureFromSurface(rend, surf);
		draw(texture);
	}

	int getW() const {
		return surf->w;
	}
	int getH() const {
		return surf->h;
	}
};

int wmain(int argc, char *argv[]) {
	Display disp("in.bmp");

	int width = disp.getW();
	int height = disp.getH();

	double xmin = -1.5;
	double ymin = -1.0;

	double pixelWidth = 2.0 / width;
	double pixelHeight = 2.0 / height;

	disp.showMandelbrotSet(xmin, ymin, pixelWidth, pixelHeight);

	SDL_Event ev;

	bool running = true;

	while (running) {

		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT) {
				running = false;
				break;
			}

			if (ev.type == SDL_KEYUP) {
				if (ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					running = false;
					break;
				}

				if (ev.key.keysym.scancode == SDL_SCANCODE_R) { // reset to default
					xmin = -1.5;
					ymin = -1.0;
					pixelWidth = 2.0 / width;
					pixelHeight = 2.0 / height;
					disp.showMandelbrotSet(xmin, ymin, pixelWidth, pixelHeight);
				}
			}

			if (ev.type == SDL_MOUSEBUTTONUP) {
				if (ev.button.button == SDL_BUTTON_LEFT) {
					xmin += (ev.button.x - width / 4) * pixelWidth;
					ymin += (ev.button.y - height / 4) * pixelHeight;
					std::cout << ev.button.x << " " << ev.button.y << " " << xmin << " " << ymin << std::endl;
					pixelWidth /= 2.0;
					pixelHeight /= 2.0;
					disp.showMandelbrotSet(xmin, ymin, pixelWidth, pixelHeight);
				}

				if (ev.button.button == SDL_BUTTON_RIGHT) {
					xmin -= pixelWidth * width / 2.0;
					ymin -= pixelHeight * height / 2.0;
					pixelWidth *= 2.0;
					pixelHeight *= 2.0;
					disp.showMandelbrotSet(xmin, ymin, pixelWidth, pixelHeight);
				}
			}
		}
	}

	disp.cleanUp();

	return 0;
}