#pragma once

// Makes every pixel of exactly (r, g, b) invisible and click-through.
void makeColorKeyTransparent(void* windowHandle, unsigned char r, unsigned char g, unsigned char b);