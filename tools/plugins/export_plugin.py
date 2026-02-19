"""
Export Plugin - Generates C export/Lua binding files (_export.c)
"""

from . import OutputPlugin

class ExportPlugin(OutputPlugin):
	"""Plugin for generating C export/Lua binding files (_export.c)"""
	def __init__(self, filename):
		super().__init__(filename, "_export.c")
