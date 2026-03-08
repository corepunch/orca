XML_FILES = ../source/geometry/geometry.xml \
            ../source/core/core.xml \
            ../source/renderer/api/renderer.xml \
            ../source/UIKit/UIKit.xml \
            ../source/SceneKit/SceneKit.xml \
            ../source/SpriteKit/SpriteKit.xml \
            ../source/filesystem/filesystem.xml

php:
	php templates/export.php ../source/UIKit/UIKit.xml > example.c

dtd:
	php templates/dtd.php $(XML_FILES) > ../docs/schemas/orca.dtd

html:
	php templates/html.php $(XML_FILES) > ../docs/index.html
