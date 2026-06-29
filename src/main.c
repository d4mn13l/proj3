#include <3ds.h>

#include <stdio.h>
#include <string.h>

#include "map.h"
#include "maths.h"
#include "ppm.h"
#include "renderer.h"
#include "util.h"

#define MAP_FILE_PATH "romfs:/map.txt"

int main() {
	romfsInit();
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);

	FILE *map_file = fopen(MAP_FILE_PATH, "r");
	ASSERT_ALWAYS(map_file != NULL, __LINE__, __FILE__);

	map_t map;
	map_load(&map, map_file);

	fclose(map_file);
	map_file = NULL;

	map_print_debug_info(&map);

	image_t img;
	ppm_image_init(&img, 400, 240);
	// size of the top screen of the 3ds

	// FILE *img_f = fopen("romfs:/3ds.ppm", "r");
	// ASSERT_ALWAYS(img_f != NULL, __LINE__, __FILE__);
	// ppm_image_load(&img, img_f);
	// fclose(img_f);

	tex_atlas_t ta;
	FILE *tex_f = fopen("romfs:/wolftextures.ppm", "r");
	ASSERT_ALWAYS(tex_f != NULL, __LINE__, __FILE__);
	tex_atlas_load(&ta, tex_f, 8, 1);
	fclose(tex_f);

	render(&img, &map, 0.5, 1.5, deg_to_rad(60), deg_to_rad(0),
		draw_textured, &ta, DRAW_FLAG_RENDER_FLOOR_CEIL);
		// draw_untextured, NULL, DRAW_FLAGS_EMPTY);

	FILE *out_f = fopen("sdmc:/out.ppm", "w");
	ASSERT_ALWAYS(out_f != NULL, __LINE__, __FILE__);
	// ppm_image_write(&img, out_f);
	ppm_image_write(ta.tex[0], out_f);
	fclose(out_f);

	gfxSetDoubleBuffering(GFX_TOP, false);
	u8* framebuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
	// printf("sizeof(*fb) = %d\n", sizeof(*framebuffer));
	memcpy(framebuffer, img.pixels, sizeof(colour_t) * img.w * img.h);
	// memset(framebuffer, 255, 6769);
	/* for (int i = 0; i < 60000; i++) {
		if (i % 3 == 1) {
			framebuffer[i] = 255;
		}
	} */
	
	while (aptMainLoop()) {
		hidScanInput();
		u32 keys_down = hidKeysDown();
		if (keys_down & KEY_START) break;
		if (keys_down & KEY_A) puts(":3");

		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
		
	}
	
	
	map_free(&map);

	romfsExit();
	gfxExit();

	return 0;
}
