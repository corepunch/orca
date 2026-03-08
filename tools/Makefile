XMLS = \
	../source/geometry/geometry.xml \
	../source/core/core.xml \
	../source/UIKit/UIKit.xml \
	../source/SceneKit/SceneKit.xml \
	../source/SpriteKit/SpriteKit.xml \
	../source/renderer/api/renderer.xml \
	../source/filesystem/filesystem.xml

.PHONY: php dtd html all

all: php dtd html

php:
	php templates/export.php ../source/geometry/geometry.xml > ../source/geometry/geometry_export.c
	php templates/export.php ../source/core/core.xml > ../source/core/core_export.c
	php templates/export.php ../source/UIKit/UIKit.xml > ../source/UIKit/UIKit_export.c
	php templates/export.php ../source/SceneKit/SceneKit.xml > ../source/SceneKit/SceneKit_export.c
	php templates/export.php ../source/SpriteKit/SpriteKit.xml > ../source/SpriteKit/SpriteKit_export.c
	php templates/export.php ../source/renderer/api/renderer.xml > ../source/renderer/api/renderer_export.c
	php templates/export.php ../source/filesystem/filesystem.xml > ../source/filesystem/filesystem_export.c

dtd:
	{ php templates/dtd.php $(XMLS); cat ../docs/schemas/append.dtd; } > ../docs/schemas/orca.dtd

html:
	php templates/html.php $(XMLS) > ../docs/index.html