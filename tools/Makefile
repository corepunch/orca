php:
	for mod in \
		core/core \
		renderer/api/renderer \
		geometry/geometry \
		UIKit/UIKit \
		SceneKit/SceneKit \
		SpriteKit/SpriteKit; do \
		echo "Generating $$mod..."; \
		xml=../source/$$mod.xml; \
		base=../source/$$mod; \
		python3 -m cProfile -s cumtime -o profile.out -m pyphp.pyphp templates/header.php $$xml > $${base}.h; \
		python3 -m cProfile -s cumtime -o profile.out -m pyphp.pyphp templates/properties.php $$xml > $${base}_properties.h; \
		python3 -m cProfile -s cumtime -o profile.out -m pyphp.pyphp templates/export.php $$xml > $${base}_export.c; \
	done