#pragma once

namespace overlay {
	bool create(int width, int height);
	void present(const unsigned char* rgba, int width, int height);
	bool shouldClose();
	void destroy();
}